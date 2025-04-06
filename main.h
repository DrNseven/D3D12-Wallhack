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
#include <unordered_set>
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
bool ShowMenu = false;
ComPtr<ID3D12Device> pDevice = nullptr;
D3D12_CPU_DESCRIPTOR_HANDLE g_RTVHandle = {}; // For Render Target View (RTV)
D3D12_CPU_DESCRIPTOR_HANDLE g_DSVHandle = {}; // For Depth-Stencil View (DSV)
ID3D12PipelineState* g_CurrentPSO = nullptr;
//D3D12_GPU_VIRTUAL_ADDRESS gBufferLocation;
//ID3D12DescriptorHeap* g_CurrentDescriptorHeap = nullptr;
ID3D12CommandQueue* commandQueue;
D3D12_VIEWPORT gpV, oVp, vp;
UINT countnum = -1;


//rootsignature
std::unordered_map<ID3D12RootSignature*, UINT> rootSigIDs;
std::unordered_map<ID3D12GraphicsCommandList*, ID3D12RootSignature*> rootSignatures;
std::mutex rootSigMutex;
UINT nextRootSigID = 1;

//rootparameterindex
std::atomic<UINT> g_lastSetRootParameterIndex = UINT_MAX;
//std::unordered_map<ID3D12GraphicsCommandList*, UINT> gRootParameterMap;

//Stride, iSize, iFormat
struct CommandListState {
	UINT currentStride0 = 0;
	UINT currentStride1 = 0;
	UINT currentStride2 = 0;
	UINT currentStride3 = 0;
	DXGI_FORMAT currentIndexFormat = DXGI_FORMAT_UNKNOWN;
	UINT currentiSize = 0;
	// Potentially add other states you need
};
thread_local CommandListState tls_commandListState; // Thread-local storage

//setpipelinestate
// Global map for green variants
std::unordered_map<ID3D12PipelineState*, ID3D12PipelineState*> greenVariants;
std::mutex pipelineMutex;
// Global map to track the last PSO set per command list
std::unordered_map<ID3D12GraphicsCommandList*, ID3D12PipelineState*> currentCommandListPSO;
std::mutex commandListPSOMutex;
ID3D12PipelineState* originalPSO = nullptr;
ID3D12PipelineState* greenPSO = nullptr;
bool swapped = false;


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
	uint32_t h = (uint32_t)value;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return (h % 90) + 10;
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

// --- Use ComPtr for globals ---
ComPtr<ID3D12Device> g_pDevice = nullptr; // Store the device globally if needed elsewhere
ComPtr<ID3D12RootSignature> g_rootSig = nullptr;
ComPtr<ID3D12PipelineState> g_greenPSO = nullptr;

bool InitGreenOverlayPipeline(ID3D12Device* device, ID3D12RootSignature** ppRootSig, ID3D12PipelineState** ppPSO)
{
	// Ensure output pointers are valid and initialize them to nullptr
	if (!ppRootSig || !ppPSO || !device) {
		//Log("ERROR: InitGreenOverlayPipeline called with null device or output pointers.");
		if (ppRootSig) *ppRootSig = nullptr;
		if (ppPSO) *ppPSO = nullptr;
		return false;
	}
	*ppRootSig = nullptr;
	*ppPSO = nullptr;

	HRESULT hr;
	ComPtr<ID3D12RootSignature> tempRootSig = nullptr; // Use local ComPtr for RAII
	ComPtr<ID3D12PipelineState> tempPSO = nullptr;     // Use local ComPtr for RAII
	ComPtr<ID3DBlob> rsBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> vsErrorBlob = nullptr;
	ComPtr<ID3DBlob> psErrorBlob = nullptr;

	// --- 1. Create Empty Root Signature ---
	//Log("Creating Root Signature...");
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			//Log("Failed to serialize root signature: %s", (char*)errorBlob->GetBufferPointer());
		}
		else {
			//Log("Failed to serialize root signature, HRESULT: 0x%08X", hr);
		}
		return false; // Cannot proceed
	}
	if (!rsBlob) {
		//Log("Failed to serialize root signature: Blob is null despite success HRESULT.");
		return false;
	}

	hr = device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&tempRootSig));
	if (FAILED(hr)) {
		//Log("Failed to create root signature, HRESULT: 0x%08X", hr);
		return false; // Cannot proceed
	}
	tempRootSig->SetName(L"GreenOverlay RootSig"); // Optional: Name for debugging
	//Log("Root Signature Created: %p", tempRootSig.Get());


	// --- 2. Compile HLSL Shaders ---
	//Log("Compiling Shaders...");
	// Compiler flags
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	// Enable debug info and disable optimizations in debug builds
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	// Enable optimizations in release builds
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	// Ensure GreenOverlay.hlsl is in the correct path (game exe) relative to the executable
	// or provide an absolute path.
	// The HLSL should contain mainVS and mainPS entry points.
	// A minimal example for a fullscreen triangle:
	/*
	// GreenOverlay.hlsl
	struct VS_OUTPUT { float4 Pos : SV_POSITION; };

	VS_OUTPUT mainVS(uint VertID : SV_VertexID) {
		VS_OUTPUT output = (VS_OUTPUT)0;
		// Generate fullscreen triangle verts directly
		float2 pos = float2((VertID << 1) & 2, VertID & 2);
		output.Pos = float4(pos * 2.0f - 1.0f, 0.0f, 1.0f);
		output.Pos.y = -output.Pos.y; // Flip Y for typical screen coords
		return output;
	}

	float4 mainPS(VS_OUTPUT input) : SV_TARGET {
		return float4(0.0f, 1.0f, 0.0f, 1.0f); // Solid Green
	}
	*/

	hr = D3DCompileFromFile(L"GreenOverlay.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", compileFlags, 0, &vsBlob, &vsErrorBlob);
	if (FAILED(hr)) {
		if (vsErrorBlob) {
			//Log("Failed to compile Vertex Shader: %s", (char*)vsErrorBlob->GetBufferPointer());
		}
		else {
			//Log("Failed to compile Vertex Shader (file not found or other error), HRESULT: 0x%08X", hr);
		}
		return false; // tempRootSig will auto-release via ComPtr
	}
	if (!vsBlob) {
		//Log("Failed to compile Vertex Shader: Blob is null despite success HRESULT.");
		return false;
	}

	hr = D3DCompileFromFile(L"GreenOverlay.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &psBlob, &psErrorBlob);
	if (FAILED(hr)) {
		if (psErrorBlob) {
			//Log("Failed to compile Pixel Shader: %s", (char*)psErrorBlob->GetBufferPointer());
		}
		else {
			//Log("Failed to compile Pixel Shader (file not found or other error), HRESULT: 0x%08X", hr);
		}
		return false; // tempRootSig will auto-release via ComPtr
	}
	if (!psBlob) {
		//Log("Failed to compile Pixel Shader: Blob is null despite success HRESULT.");
		return false;
	}
	//Log("Shaders Compiled Successfully.");


	// --- 3. Create Graphics Pipeline State Object ---
	//Log("Creating Graphics Pipeline State...");
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = tempRootSig.Get(); // Use the successfully created root signature
	desc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	desc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

	// --- Blend State ---
	// Set up for additive blending (Green + Background)
	// If you want opaque green, set BlendEnable = FALSE and leave others default
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // Start with default
	desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;       // PS output (source color)
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;      // RT content (destination color)
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;     // Source + Destination
	desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;  // Use source alpha
	desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO; // Ignore destination alpha
	desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;    // Add alpha (though likely not used)
	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_GREEN; //D3D12_COLOR_WRITE_ENABLE_ALL; // Allow writing all channels

	desc.SampleMask = UINT_MAX;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// Disable culling for a simple fullscreen triangle/quad overlay
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// --- Depth Stencil State ---
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// Disable depth testing/writing for an overlay that should draw on top
	desc.DepthStencilState.DepthEnable = FALSE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.StencilEnable = FALSE; // Ensure stencil is off too

	desc.InputLayout = {}; // No input elements from vertex buffer for this shader
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Common format, adjust if game uses different backbuffer (e.g., R10G10B10A2_UNORM, R16G16B16A16_FLOAT)
	desc.DSVFormat = DXGI_FORMAT_UNKNOWN; // Valid because DepthEnable is FALSE
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0; // Standard quality level

	// --- Create PSO ---
	hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&tempPSO));
	if (FAILED(hr)) {
		Log("Failed to create Graphics Pipeline State, HRESULT: 0x%08X", hr);
		return false; // tempRootSig will auto-release via ComPtr
	}
	tempPSO->SetName(L"GreenOverlay PSO"); // Optional: Name for debugging
	//Log("Graphics Pipeline State Created: %p", tempPSO.Get());

	// --- Success ---
	//Log("InitGreenOverlayPipeline completed successfully!");
	*ppRootSig = tempRootSig.Detach(); // Transfer ownership to output pointer
	*ppPSO = tempPSO.Detach();       // Transfer ownership to output pointer
	return true; // Indicate success
}