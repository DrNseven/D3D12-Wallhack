#include <windows.h>
#include <psapi.h>
#include <vector>
#include <wrl.h>
#include <iostream>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <unordered_map>
#include <mutex>
#include <array>      
#include <cstdint>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


using namespace Microsoft::WRL;
//using Microsoft::WRL::ComPtr;

#include <DirectXMath.h>
using namespace DirectX;


#include "MinHook\Include\MinHook.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"

//=========================================================================================================================//

//globals
bool InitOnce = true;
bool ShowMenu = false;
bool ImGui_Initialised = false;
D3D12_CPU_DESCRIPTOR_HANDLE g_RTVHandle = {}; // For Render Target View (RTV)
D3D12_CPU_DESCRIPTOR_HANDLE g_DSVHandle = {}; // For Depth-Stencil View (DSV)
ID3D12PipelineState* g_CurrentPSO = nullptr;
UINT mIndexCount;
UINT mVertexCount;
int twoDigitiSize;
ComPtr<ID3D12Device> pDevice = nullptr;
UINT vStartSlot;
UINT gRootParameterIndex;
D3D12_GPU_VIRTUAL_ADDRESS gBufferLocation;
//ID3D12DescriptorHeap* g_CurrentDescriptorHeap = nullptr;
ID3D12CommandQueue* commandQueue;
D3D12_VIEWPORT gpV, oVp, vp;
//volatile int countnum = -1;
UINT countnum = -1;


//iSize
std::unordered_map<ID3D12GraphicsCommandList*, UINT> iSizes;
std::unordered_map<ID3D12GraphicsCommandList*, UINT> iFormat;
std::mutex iSizesMutex;

//Stride
const UINT MAX_VERTEX_BUFFER_SLOTS = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
std::unordered_map<ID3D12GraphicsCommandList*, std::array<UINT, MAX_VERTEX_BUFFER_SLOTS>> vStride;
std::mutex vStrideMutex;

//rootsignature
std::unordered_map<ID3D12RootSignature*, UINT> rootSigIDs;
std::unordered_map<ID3D12GraphicsCommandList*, ID3D12RootSignature*> rootSignatures;
std::mutex rootSigMutex;

UINT nextRootSigID = 1;  // Start assigning IDs from 1

//=========================================================================================================================//

#include <fstream>
using namespace std;

char dlldir[320];
char* GetDirectoryFile(char* filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char* fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile((PCHAR)"log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

//=========================================================================================================================//

WNDCLASSEX WindowClass;
HWND WindowHwnd;

bool InitWindow() {

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = DefWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = NULL;
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = NULL;
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "MJ";
	WindowClass.hIconSm = NULL;
	RegisterClassEx(&WindowClass);
	WindowHwnd = CreateWindow(WindowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
	if (WindowHwnd == NULL) {
		return false;
	}
	return true;
}

bool DeleteWindow() {
	DestroyWindow(WindowHwnd);
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
	if (WindowHwnd != NULL) {
		return false;
	}
	return true;
}

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

static uintx_t* MethodsTable = NULL;

//=========================================================================================================================//

ComPtr<IDXGIFactory6> Factory;
ComPtr<IDXGIAdapter1> Adapter;
ComPtr<ID3D12Device> Device;
ComPtr<ID3D12CommandQueue> CommandQueue;
ComPtr<ID3D12CommandAllocator> CommandAllocator;
ComPtr<ID3D12GraphicsCommandList> CommandList;
ComPtr<IDXGISwapChain1> SwapChain;


namespace DirectX12 {
	bool Init() {
		if (InitWindow() == false) {
			return false;
		}

		HMODULE D3D12Module = GetModuleHandle("d3d12.dll");
		HMODULE DXGIModule = GetModuleHandle("dxgi.dll");
		if (D3D12Module == NULL || DXGIModule == NULL) {
			DeleteWindow();
			return false;
		}

		ComPtr<IDXGIFactory6> Factory;
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)))) {
			DeleteWindow();
			return false;
		}

		ComPtr<IDXGIAdapter1> Adapter;
		for (UINT i = 0; Factory->EnumAdapters1(i, &Adapter) != DXGI_ERROR_NOT_FOUND; i++) {
			DXGI_ADAPTER_DESC1 desc;
			Adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			break;
		}

		if (!Adapter) {
			DeleteWindow();
			return false;
		}

		ComPtr<ID3D12Device> Device;
		if (FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device)))) {
			DeleteWindow();
			return false;
		}

		D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		QueueDesc.Priority = 0;
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.NodeMask = 0;

		ComPtr<ID3D12CommandQueue> CommandQueue;
		if (FAILED(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)))) {
			DeleteWindow();
			return false;
		}

		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		if (FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)))) {
			DeleteWindow();
			return false;
		}

		ComPtr<ID3D12GraphicsCommandList> CommandList;
		if (FAILED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)))) {
			DeleteWindow();
			return false;
		}

		DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
		SwapChainDesc.BufferDesc.Width = 100;
		SwapChainDesc.BufferDesc.Height = 100;
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		SwapChainDesc.SampleDesc.Count = 1;
		SwapChainDesc.SampleDesc.Quality = 0;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 2;
		SwapChainDesc.OutputWindow = WindowHwnd;
		SwapChainDesc.Windowed = TRUE;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ComPtr<IDXGISwapChain> SwapChain;
		if (FAILED(Factory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, &SwapChain))) {
			DeleteWindow();
			return false;
		}

		MethodsTable = (uintx_t*)::calloc(150, sizeof(uintx_t));
		memcpy(MethodsTable, *(uintx_t**)Device.Get(), 44 * sizeof(uintx_t));
		memcpy(MethodsTable + 44, *(uintx_t**)CommandQueue.Get(), 19 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19, *(uintx_t**)CommandAllocator.Get(), 9 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9, *(uintx_t**)CommandList.Get(), 60 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9 + 60, *(uintx_t**)SwapChain.Get(), 18 * sizeof(uintx_t));

		MH_Initialize();
		Device.Reset();
		CommandQueue.Reset();
		CommandAllocator.Reset();
		CommandList.Reset();
		SwapChain.Reset();
		DeleteWindow();
		return true;
	}
}

//=========================================================================================================================//

bool CreateHook(uint16_t Index, void** Original, void* Function) {
	assert(_index >= 0 && _original != NULL && _function != NULL);
	void* target = (void*)MethodsTable[Index];
	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
		return false;
	}
	return true;
}

void DisableHook(uint16_t Index) {
	assert(Index >= 0);
	MH_DisableHook((void*)MethodsTable[Index]);
}

void DisableAll() {
	MH_DisableHook(MH_ALL_HOOKS);
	free(MethodsTable);
	MethodsTable = NULL;
}

//=========================================================================================================================//

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}

namespace DirectX12Interface {
	ID3D12Device* Device = nullptr;
	ID3D12DescriptorHeap* DescriptorHeapBackBuffers;
	ID3D12DescriptorHeap* DescriptorHeapImGuiRender;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandQueue* CommandQueue;

	struct _FrameContext {
		ID3D12CommandAllocator* CommandAllocator;
		ID3D12Resource* Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	};

	uintx_t BuffersCounts = -1;
	_FrameContext* FrameContext;
}

//=========================================================================================================================//
/*
#include <codecvt> // For wstring to string conversion (C++11 and later)
#include <locale>  // For wstring to string conversion (C++11 and later)

std::string GetDebugName(ID3D12Object* pObject)
{
	if (!pObject)
		return {};

	UINT dataSize = 0;

	// First, get the size of the debug name data
	HRESULT hResult = pObject->GetPrivateData(WKPDID_D3DDebugObjectNameW, &dataSize, nullptr);
	if (FAILED(hResult) || dataSize == 0)
		return {};

	// Allocate space for the debug name (wide characters)
	std::vector<wchar_t> nameW(dataSize / sizeof(wchar_t));

	// Retrieve the actual debug name data
	hResult = pObject->GetPrivateData(WKPDID_D3DDebugObjectNameW, &dataSize, nameW.data());
	if (SUCCEEDED(hResult))
	{
		// Convert wide characters to a standard string (UTF-8)
		std::wstring nameWide(nameW.data());

		// C++11 and later: Use wstring_convert
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string name = converter.to_bytes(nameWide);
		return name;


		// Older C++ versions (pre-C++11):  Less efficient, but works.  Avoid if possible.
		// std::string name(nameWide.begin(), nameWide.end());  This will truncate non-ASCII characters.
		// Consider using a library like ICU or boost::locale for more robust conversion.
	}

	return {};
}
//usage:
//std::string cliName = GetDebugName(dCommandList);
//std::string pipName = GetDebugName(g_CurrentPSO);
//Log("pipName.c_str() == %s && cliName.c_str() == %s..
*/

int getTwoDigitValue(int value) {
	value = ((value >> 2) ^ (value * 2654435761)) & 0xFFFF;
	return (value % 89) + 10;  // Result is between 10-99
}

void SetDepthRange(float dr, ID3D12GraphicsCommandList* dcl)
{
	oVp = gpV; dr; D3D12_VIEWPORT vp = {}; vp.TopLeftX = gpV.TopLeftX; vp.TopLeftY = gpV.TopLeftY; vp.Width = gpV.Width; vp.Height = gpV.Height;
	vp.MinDepth = dr; vp.MaxDepth = gpV.MaxDepth; 
	dcl->RSSetViewports(1, &vp);
}

void ResetDepthRange(ID3D12GraphicsCommandList* dcl)
{
	D3D12_VIEWPORT vp = {}; vp.TopLeftX = gpV.TopLeftX; vp.TopLeftY = gpV.TopLeftY; vp.Width = gpV.Width; vp.Height = gpV.Height;
	vp.MinDepth = 0.0f; vp.MaxDepth = gpV.MaxDepth;
	dcl->RSSetViewports(1, &vp);
}

//=========================================================================================================================//

// TEST code below


// GPU resources
ComPtr<ID3D12PipelineState> g_GreenPSO;
ComPtr<ID3D12PipelineState> g_RedPSO;
ComPtr<ID3D12RootSignature> g_RootSignature;


/*
// Shader source (HLSL)
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.tex = input.tex;"
"    return output;"
"}";
*/

/*
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"output.pos.z = 0.5;" // TEMPORARY: Force a specific Z value, fix graphical bugs. Directly setting this won't disable the z-buffer. It just sets the z-position for the depth test.
"output.pos.w = 1.0;"
"output.pos.x = output.pos.x * 0.1;"
"output.pos.y = output.pos.y * 0.1;"
"output.pos.z = 0.0;"
"    output.tex = input.tex;"
"    return output;"
"}";

const char* g_PixelShader =
"Texture2D g_Texture : register(t0);"
"SamplerState g_Sampler : register(s0);"
"float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_Target {"
"    return float4(1.0, 0.0, 0.0, 1.0);"  // red
"}";
*/

/*
const char* g_GreenVertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
//"    output.pos.x *= 0.1;"
//"    output.pos.y *= 0.1;"
//"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
//"    output.tex = input.tex;"
//"    output.debugColor = abs(float3(output.pos.x, output.pos.y, output.pos.z));"
"    output.debugColor = abs(float3(output.pos.x * 5.0, output.pos.y * 10.0, output.pos.z * 2.0));" // Scale each channel independently, Glow
"    return output;"
"};";
*/

/*
// Vertex Shader
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x *= 0.1;"
"    output.pos.y *= 0.1;"
"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
"    output.tex = input.tex;"
// Scale and clamp
"    float r = clamp(abs(output.pos.x) * 1.0, 0.0, 1.0);"
"    float g = clamp(abs(output.pos.y) * 1.0, 0.0, 1.0);"
"    float b = clamp(abs(output.pos.z) * 10.0, 10.0, 1.0);"
"    output.debugColor = float3(r, g, b);"
"    return output;"
"};";
*/

/*
// Vertex Shader
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x *= 0.1;"
"    output.pos.y *= 0.1;"
"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
"    output.tex = input.tex;"
// Mix mostly to blue
"    float r = clamp(abs(output.pos.x) * 0.1, 0.0, 0.2);"   // Small amount of red
"    float g = clamp(abs(output.pos.y) * 0.5, 0.0, 0.2);"   // Small amount of green
"    float b = clamp(abs(output.pos.z) * 2.0, 0.0, 1.0);"   // Strong blue
"    output.debugColor = float3(r, g, b);"
"    return output;"
"};";
*/

const char* g_GreenVertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"

"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"

"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x *= -0.5;"//warning can shrink the model by 90%
"    output.pos.y *= -0.5;"
"	 output.pos.z *= -0.5;"

// Offset the position
//Moving the model relative to the camera, is not possible without access to some kind of transformation matrix
"float offsetX = 0.2;"  // Example: Move right (positive X)
"float offsetY = -0.3;" // Example: Move down (negative Y)
"float offsetZ = 0.0;"  // Move closer/further (Z-axis, but depth behavior can be tricky)

"output.pos.x += offsetX;"
"output.pos.y += offsetY;"
"output.pos.z += offsetZ;"

"    output.tex = input.tex;"
"    output.debugColor = abs(float3(0.0, 1.0, 0.0));" //use specific color, with glow
"    return output;"
"};";

const char* g_RedVertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"

"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x *= 0.1;"//warning can shrink the model by 90%
"    output.pos.y *= 0.1;"
"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
"    output.tex = input.tex;"
"    output.debugColor = abs(float3(2.0, 0.0, 0.0));" //use specific color, with glow
"    return output;"
"};";


/*
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x *= 0.1;"//*= 0.1;"
//"    output.pos.x *= -1.0;" // Flip X-axis if needed
//"    output.pos.y *= 0.1;"
"    output.pos.y *= -0.1;" // Flip Y-axis if needed
"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
"    output.tex = input.tex;"
"    output.debugColor = abs(float3(0.0, 2.0, 0.0));" //use specific color
"    return output;"
"};";
*/
/*
const char* g_VertexShader =
"struct VS_INPUT { float3 pos : POSITION; float2 tex : TEXCOORD; };"
"struct VS_OUTPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };"
"VS_OUTPUT main(VS_INPUT input) {"
"    VS_OUTPUT output;"
"    output.pos = float4(input.pos, 1.0);"
"    output.pos.x = 0.0;"
"    output.pos.y = 0.0;"
"    output.pos.z = 0.0;"  // Setting z to 0.0 for consistent depth
"    output.tex = input.tex;"
"    output.debugColor = abs(float3(0.0, 1.0, 0.0));" //use specific color
"    return output;"
"};";
*/

/*
// Fragment Shader (pixelshader)
const char* g_FragmentShader =
"struct PS_INPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };" // Must match VS_OUTPUT
"float4 main(PS_INPUT input) : SV_TARGET {"
"    return float4(input.debugColor, 1.0);" // Use the debugColor
"}";
//debugColor values are now encoded as colors on the screen, you essentially need to read those colors back and decode them.
*/

const char* g_FragmentShader =
"struct PS_INPUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; float3 debugColor : COLOR; };" // Must match VS_OUTPUT

"cbuffer ConstantBufferData : register(b0) {"
"    float4 g_Color;" // Global color from constant buffer
"};"

"float4 main(PS_INPUT input) : SV_TARGET {"
"    return g_Color;" // Use the color from constant buffer
"};";


// Function to compile shader
HRESULT CompileShader(const char* source, const char* entry, const char* target, ID3DBlob** blob) {
	return D3DCompile(source, strlen(source), NULL, NULL, NULL, entry, target, 0, 0, blob, NULL);
}

//#include <dxgi1_6.h>
using Microsoft::WRL::ComPtr;

ID3D12RootSignature* rootSignature = nullptr; // Define it globally

void CreateRootSignature(ID3D12Device* device) {
	if (!device) {
		Log("Device is not initialized.\n");
		return;
	}

	// Describe root parameters.  Adjust the number as needed!
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(3); // For Constant Buffer, Texture (SRV), Sampler

	// Slot 0: Constant Buffer (Color) - needs to correspond to register(b0) in HLSL
	rootParameters[0].InitAsConstantBufferView(0); // Register b0, Shader visibility to all shaders by default (can be changed)

	// Slot 1: Texture (SRV) - Needs to correspond to register(t0) in HLSL
	CD3DX12_DESCRIPTOR_RANGE texRange;
	texRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // SRV, 1 descriptor, register t0
	rootParameters[1].InitAsDescriptorTable(1, &texRange, D3D12_SHADER_VISIBILITY_PIXEL); //Important: Shader visibility set to Pixel shader

	// Slot 2: Sampler - Needs to correspond to register(s0) in HLSL
	CD3DX12_DESCRIPTOR_RANGE sampRange;
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); //Sampler, 1 Descriptor, register s0
	rootParameters[2].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL); //Important: Shader visibility set to Pixel shader

	// Define the root signature description
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		rootParameters.size(),       // Number of root parameters
		rootParameters.data(),        // Pointer to array of root parameters
		0,                             // Number of static samplers
		nullptr,                       // Pointer to static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |  // Flags, allow IA layout
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
	);

	// Serialize the root signature.
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(hr)) {
		Log("Failed to serialize root signature.\n");
		if (error) {
			Log(static_cast<char*>(error->GetBufferPointer()));
		}
		return;
	}

	// Create the root signature.
	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	if (FAILED(hr)) {
		Log("Failed to create root signature.\n");
		return;
	}

	//Log("Successfully created root signature.\n");
}


void CreateGreenPipelineState(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Compile shaders
	ComPtr<ID3DBlob> vsBlob, psBlob;
	CompileShader(g_GreenVertexShader, "main", "vs_5_0", &vsBlob);
	CompileShader(g_FragmentShader, "main", "ps_5_0", &psBlob);

	// Root signature creation (no changes needed)
	//D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	//rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ComPtr<ID3DBlob> signatureBlob;
	//D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, nullptr);
	//device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_RootSignature));

	// Input Layout (no changes)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

	// Set the shaders and root signature
	psoDesc.pRootSignature = rootSignature; //g_RootSignature.Get();
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
	
	// Rasterizer and blend state (no changes needed)
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //D3D12_FILL_MODE_WIREFRAME
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Blend state
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE; // Disable blending
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState = blendDesc;

	// Disable depth testing
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;  // Disable depth testing
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // Don't write to the depth buffer
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;  // This ensures the depth test will always pass
	depthStencilDesc.StencilEnable = FALSE;  // Disable stencil testing
	psoDesc.DepthStencilState = depthStencilDesc;  // Set depth stencil state in PSO

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	
	// Create the pipeline state object
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_GreenPSO));
	if (FAILED(hr)) {
		Log("Failed to create pipeline state, HRESULT: 0x%08X", hr);
		return;
	}
}

void CreateRedPipelineState(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Compile shaders
	ComPtr<ID3DBlob> vsBlob, psBlob;
	CompileShader(g_RedVertexShader, "main", "vs_5_0", &vsBlob);
	CompileShader(g_FragmentShader, "main", "ps_5_0", &psBlob);

	// Root signature creation (no changes needed)
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signatureBlob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, nullptr);
	device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_RootSignature));

	//commandList->SetGraphicsRootSignature(g_RootSignature.Get());

	// Input Layout (no changes)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

	// Set the shaders and root signature
	psoDesc.pRootSignature = g_RootSignature.Get();
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

	// Rasterizer and blend state (no changes needed)
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //D3D12_FILL_MODE_WIREFRAME
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Blend state
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE; // Disable blending
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState = blendDesc;

	// Disable depth testing
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;  // Disable depth testing
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // Don't write to the depth buffer
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;  // This ensures the depth test will always pass
	depthStencilDesc.StencilEnable = FALSE;  // Disable stencil testing
	psoDesc.DepthStencilState = depthStencilDesc;  // Set depth stencil state in PSO

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// Create the pipeline state object
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_RedPSO));
	if (FAILED(hr)) {
		Log("Failed to create pipeline state, HRESULT: 0x%08X", hr);
		return;
	}
}

ComPtr<ID3D12Resource> g_MyConstantBuffer; // Global or class member variable

// Global color value
float g_color[4] = { 1.0f, 0.0f, 1.0f, 1.0f }; // Red

// Modified CreateConstantBuffer function
void CreateConstantBuffer(UINT rootpindex, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {

	// Define the constant buffer structure
	struct ConstantBufferData {
		float color[4]; // RGBA
	};

	// Check if the buffer already exists, reuse it
	if (!g_MyConstantBuffer) {
		ConstantBufferData cbData = { g_color[0], g_color[1], g_color[2], g_color[3] };

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = (sizeof(ConstantBufferData) + 255) & ~255; // Align to 256 bytes
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Create buffer
		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_MyConstantBuffer)
		);

		// Map and copy data
		void* mappedData;
		g_MyConstantBuffer->Map(0, nullptr, &mappedData);
		memcpy(mappedData, &cbData, sizeof(cbData));
		g_MyConstantBuffer->Unmap(0, nullptr);
	}

	// Use the buffer's GPU virtual address
	commandList->SetGraphicsRootConstantBufferView(rootpindex, g_MyConstantBuffer->GetGPUVirtualAddress()); //Root parameter 0
}

/*
void CreateRootSignature(ID3D12Device* device) {
	if (!device) {
		Log("Device is not initialized.\n");
		return;
	}

	// Describe the root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		0,                   // Number of root parameters (set to 0 if none are used)
		nullptr,             // Pointer to array of root parameters (nullptr if none are used)
		0,                   // Number of static samplers
		nullptr,             // Pointer to static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT // Flags
	);

	// Serialize the root signature.
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(hr)) {
		Log("Failed to serialize root signature.\n");
		if (error) {
			Log(static_cast<char*>(error->GetBufferPointer()));
		}
		return;
	}

	// Create the root signature.
	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	if (FAILED(hr)) {
		Log("Failed to create root signature.\n");
		return;
	}

	Log("Successfully created root signature.\n");
}
*/

/*
* const char* g_GreenVertexShader =
"struct VS_INPUT {"
"    float3 pos : POSITION;"
"    float2 tex : TEXCOORD;"
"};"

"cbuffer ConstantBufferData : register(b0) {"
"    float4 g_Color;" // Global color from constant buffer
"};"

"struct PS_INPUT {"
"    float4 pos : SV_POSITION;"
"    float2 tex : TEXCOORD;"
"    float3 debugColor : COLOR0;" // Color attribute for the pixel shader
"};"

"PS_INPUT main(VS_INPUT input) {"
"    PS_INPUT output;"
"    output.pos = float4(input.pos, 1.0);" // Convert to homogeneous coordinates
"    output.tex = input.tex;" // Pass texture coordinates
//"    output.debugColor = g_Color.rgb;" // Pass constant buffer color
"    output.debugColor = float3(0, 1, 0);"
"    return output;"
"};";

const char* g_FragmentShader =
"struct PS_INPUT {"
"    float4 pos : SV_POSITION;"
"    float2 tex : TEXCOORD;"
"    float3 debugColor : COLOR0;" // This must match the output of the vertex shader
"};"

"cbuffer ConstantBufferData : register(b0) {"
"    float4 g_Color;" // Global color from constant buffer
"};"

"float4 main(PS_INPUT input) : SV_TARGET {"
"    return float4(input.debugColor, 1.0);" // Use the color from the vertex shader's output
"};";

void CreateGreenPipelineState(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Compile shaders
	ComPtr<ID3DBlob> vsBlob, psBlob;
	CompileShader(g_GreenVertexShader, "main", "vs_5_0", &vsBlob);
	CompileShader(g_FragmentShader, "main", "ps_5_0", &psBlob);

	// Define root signature with constant buffer at register b0 (pixel shader visibility)
	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // Register b0
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // 

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signatureBlob, errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log("Root Signature Serialization Failed: %s", (char*)errorBlob->GetBufferPointer());
		return;
	}

	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_RootSignature));
	if (FAILED(hr)) {
		Log("Failed to create Root Signature!");
		return;
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Input Layout
	//D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

	// Set shaders and root signature
	psoDesc.pRootSignature = g_RootSignature.Get();
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

	// Rasterizer state
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Blend state
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState = blendDesc;

	// Depth-stencil state
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.StencilEnable = FALSE;
	psoDesc.DepthStencilState = depthStencilDesc;

	// Sample mask and primitive topology
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// Create the pipeline state object
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_GreenPSO));
	if (FAILED(hr)) {
		Log("Failed to create pipeline state, HRESULT: 0x%08X", hr);
		return;
	}
}
*/