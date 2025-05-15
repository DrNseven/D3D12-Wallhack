#include <windows.h>
#include <psapi.h>
#include <vector>
#include <wrl.h>
#include <iostream>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <map>
#include <unordered_map>
#include <mutex>
#include <array>      
#include <cstdint>
#include <unordered_set>
#include <guiddef.h>    // For GUID
#include <comdef.h>     // For _com_ptr_t (optional, for smart COM pointers)
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib,"winmm.lib")


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
bool wallh = 1;
bool ShowMenu = false;
ComPtr<ID3D12Device> pDevice = nullptr;
D3D12_CPU_DESCRIPTOR_HANDLE g_RTVHandle = {}; // For Render Target View (RTV)
D3D12_CPU_DESCRIPTOR_HANDLE g_DSVHandle = {}; // For Depth-Stencil View (DSV)
ID3D12PipelineState* g_CurrentPSO = nullptr;
//D3D12_GPU_VIRTUAL_ADDRESS gBufferLocation;
//ID3D12DescriptorHeap* g_CurrentDescriptorHeap = nullptr;
ID3D12CommandQueue* commandQueue;
UINT countnum = -1;

//rootparameterindex
//Optional, generate this using a GUID generator (e.g., Visual Studio's Tools -> Create GUID)
//{E21A228C-786F-4432-B97E-D48E4A0F78FB} - example
static const GUID MyCommandListPrivateDataGuid =
{ 0xe21a228c, 0x786f, 0x4432, { 0xb9, 0x7e, 0xd4, 0x8e, 0x4a, 0xf, 0x78, 0xfb } };

// Struct to hold the data associated with the command list
struct CommandListSpecificData {
	UINT lastCbvRootParameterIndex = UINT_MAX;
	// Add more here if needed
	int rDimension = 0;
	int rFormat = 0;
	int rWidth = 0;
	int rFlags = 0;

	// Default constructor
	CommandListSpecificData() : lastCbvRootParameterIndex(UINT_MAX) {}
};

//Stride ect.
UINT Strides;
struct CommandListState {
	UINT vertexBufferSizes[7] = {};
	UINT vertexStrides[7] = {};
	DXGI_FORMAT currentIndexFormat = DXGI_FORMAT_UNKNOWN;
	UINT currentiSize = 0;
	UINT StartSlot = 0;
	UINT NumViews = 0;
};
thread_local CommandListState t_cLS;

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

//setviewport
std::unordered_map<ID3D12GraphicsCommandList*, D3D12_VIEWPORT> gViewportMap;
std::mutex gViewportMutex;

//resource
ID3D12Resource* g_pResource;
ID3D12Resource* goodresource;

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

bool waitedOnce = false;
void lognospam(int duration, const char* name)
{
	if (!waitedOnce)
	{
		int n;

		for (n = 0; n < duration; n++)
		{
			if (GetTickCount() % 100)
				Log(name);
		}
		waitedOnce = true;
	}
}

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };

bool WorldToScreen(const float* matrix, const Vec3& worldPos, Vec2& screenPos, float screenWidth, float screenHeight)
{
	using namespace DirectX;

	//XMMATRIX viewProj = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(matrix));
	XMMATRIX viewProj = XMMatrixTranspose(XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(matrix))); //or transpose
	XMVECTOR pos = XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	XMVECTOR transformed = XMVector4Transform(pos, viewProj);

	XMFLOAT4 clipSpace;
	XMStoreFloat4(&clipSpace, transformed);

	if (clipSpace.w < 0.001f)
		return false;

	float invW = 1.0f / clipSpace.w;
	float ndcX = clipSpace.x * invW;
	float ndcY = clipSpace.y * invW;

	screenPos.x = (ndcX * 0.5f + 0.5f) * screenWidth;
	screenPos.y = (1.0f - (ndcY * 0.5f + 0.5f)) * screenHeight;

	return true;
}

//experimental/slow & unstable, warning: will kill your fps 
void ReadConstantBufferWithMapUnmap(ID3D12Resource* constantBufferResource, UINT constantBufferSize) {
	void* mappedData;
	D3D12_RANGE readRange = { 0, constantBufferSize };
	D3D12_RANGE writtenRange = { 0, 0 };

	HRESULT hr = constantBufferResource->Map(0, &readRange, &mappedData);

	if (SUCCEEDED(hr)) {
		unsigned char* buffer = new unsigned char[constantBufferSize];
		memcpy(buffer, mappedData, constantBufferSize);

		float* matrixData = (float*)buffer;

		// Log matrix
		//Log("2. INFO: Constant Buffer Data (Matrix):");
		for (int i = 0; i < 4; ++i) {
			std::string rowLog;
			for (int j = 0; j < 4; ++j) {
				char temp[64];
				sprintf_s(temp, 64, "%f ", matrixData[i * 4 + j]);
				rowLog += temp;
			}
			Log("3. INFO: %s", rowLog.c_str());
		}

		// World position to test
		Vec3 testPos = { 0.0f, 0.0f, 0.0f }; // Replace with actual world pos
		Vec2 screenPos;

		float screenWidth = 1920.0f;
		float screenHeight = 1080.0f;

		if (WorldToScreen(matrixData, testPos, screenPos, screenWidth, screenHeight)) {
			//if (GetAsyncKeyState(VK_F10) & 1)
			Log("W2S success: x=%.1f, y=%.1f", screenPos.x, screenPos.y);
		}
		//else {
			//Log("W2S failed: behind camera");
		//}

		constantBufferResource->Unmap(0, &writtenRange);
		delete[] buffer;
	}
	else {
		//Log("ERROR: Map failed in ReadConstantBufferWithMapUnmap. HR = %x", hr);
	}
}

/*
//crash
void FastReadConstantBuffer(ID3D12Resource* constantBufferResource, UINT constantBufferSize) {
	if (!constantBufferResource || constantBufferSize == 0) {
		return;
	}

	D3D12_HEAP_PROPERTIES heapProps;
	D3D12_HEAP_FLAGS heapFlags;
	// It's good practice to check the HRESULT of GetHeapProperties
	if (FAILED(constantBufferResource->GetHeapProperties(&heapProps, &heapFlags))) {
		//Log("ERROR: GetHeapProperties failed for resource 0x%p.", (void*)constantBufferResource);
		return;
	}

	// Only attempt to map UPLOAD heap resources (CPU-visible for direct read)
	if (heapProps.Type != D3D12_HEAP_TYPE_UPLOAD) {
		// Log("INFO: Skipping map for non-UPLOAD heap resource 0x%p, HeapType: %d", (void*)constantBufferResource, heapProps.Type);
		return; // Cannot directly map DEFAULT or READBACK (for writing from CPU)
	}

	void* pMappedData = nullptr;
	// For UPLOAD heaps, the readRange parameter to Map is often optional if reading the whole buffer,
	// but it's good practice to specify it. The system might ignore it for UPLOAD heaps.
	D3D12_RANGE readRange = { 0, constantBufferSize };
	// For UPLOAD heap, if you are ONLY reading, the CPU cache behavior might be better if you pass a valid readRange.
	// If you pass nullptr for readRange, it implies you might read the entire resource.

	HRESULT hr = constantBufferResource->Map(0, &readRange, &pMappedData); // Or use nullptr for pReadRange if always reading whole buffer

	if (SUCCEEDED(hr) && pMappedData) {
		const float* matrixData = reinterpret_cast<const float*>(pMappedData);

		// --- Your WorldToScreen Logic ---
		static int frameCounter = 0; // Be careful with statics in hooked functions if they can be called from multiple threads
		frameCounter++;
		if (frameCounter >= 100) { // Log every 100 calls to this function (if it's a UPLOAD buffer)
			frameCounter = 0;

			Vec3 testPos = { 0.0f, 0.0f, 0.0f }; // World origin
			Vec2 screenPos;
			// Ensure constantBufferSize is large enough for a 4x4 matrix (16 floats)
			if (constantBufferSize >= (16 * sizeof(float))) {
				if (WorldToScreen(matrixData, testPos, screenPos, 1920.0f, 1080.0f)) {
					Log("W2S: x=%.1f, y=%.1f (Resource: 0x%p)", screenPos.x, screenPos.y, (void*)constantBufferResource);
				}
				else {
					// Log("W2S: Point behind camera or W <= 0 (Resource: 0x%p)", (void*)constantBufferResource);
				}
			}
			else {
				// Log("WARN: Constant buffer size %u is too small for a 4x4 matrix. (Resource: 0x%p)", constantBufferSize, (void*)constantBufferResource);
			}
		}
		// --- End WorldToScreen Logic ---

		// IMPORTANT: Unmap the resource IMMEDIATELY after you are done reading from pMappedData
		//D3D12_RANGE writtenRange = { 0, 0 }; // We only read, so no bytes were written by the CPU
		//constantBufferResource->Unmap(0, &writtenRange);
	}
	else {
		Log("ERROR: Map failed for resource 0x%p. HR = 0x%X", (void*)constantBufferResource, hr);
	}
}
*/

/*
//crash
void FastReadConstantBuffer(ID3D12Resource* constantBufferResource, UINT constantBufferSize) {
	if (!constantBufferResource || constantBufferSize == 0) {
		return;
	}

	D3D12_HEAP_PROPERTIES heapProps;
	D3D12_HEAP_FLAGS heapFlags;
	// It's good practice to check the HRESULT of GetHeapProperties
	if (FAILED(constantBufferResource->GetHeapProperties(&heapProps, &heapFlags))) {
		Log("ERROR: GetHeapProperties failed for resource 0x%p.", (void*)constantBufferResource);
		return;
	}

	// Only attempt to map UPLOAD heap resources (CPU-visible for direct read)
	if (heapProps.Type != D3D12_HEAP_TYPE_UPLOAD) {
		// Log("INFO: Skipping map for non-UPLOAD heap resource 0x%p, HeapType: %d", (void*)constantBufferResource, heapProps.Type);
		return; // Cannot directly map DEFAULT or READBACK (for writing from CPU)
	}

	void* pMappedData = nullptr;
	// For UPLOAD heaps, the readRange parameter to Map is often optional if reading the whole buffer,
	// but it's good practice to specify it. The system might ignore it for UPLOAD heaps.
	D3D12_RANGE readRange = { 0, constantBufferSize };
	// For UPLOAD heap, if you are ONLY reading, the CPU cache behavior might be better if you pass a valid readRange.
	// If you pass nullptr for readRange, it implies you might read the entire resource.

	HRESULT hr = constantBufferResource->Map(0, &readRange, &pMappedData); // Or use nullptr for pReadRange if always reading whole buffer

	if (SUCCEEDED(hr) && pMappedData) {
		const float* matrixData = reinterpret_cast<const float*>(pMappedData);

		// --- Your WorldToScreen Logic ---
		static int frameCounter = 0; // Be careful with statics in hooked functions if they can be called from multiple threads
		frameCounter++;
		if (frameCounter >= 100) { // Log every 100 calls to this function (if it's a UPLOAD buffer)
			frameCounter = 0;

			Vec3 testPos = { 0.0f, 0.0f, 0.0f }; // World origin
			Vec2 screenPos;
			// Ensure constantBufferSize is large enough for a 4x4 matrix (16 floats)
			if (constantBufferSize >= (16 * sizeof(float))) {
				if (WorldToScreen(matrixData, testPos, screenPos, 1920.0f, 1080.0f)) {
					Log("W2S: x=%.1f, y=%.1f (Resource: 0x%p)", screenPos.x, screenPos.y, (void*)constantBufferResource);
				}
				else {
					// Log("W2S: Point behind camera or W <= 0 (Resource: 0x%p)", (void*)constantBufferResource);
				}
			}
			else {
				// Log("WARN: Constant buffer size %u is too small for a 4x4 matrix. (Resource: 0x%p)", constantBufferSize, (void*)constantBufferResource);
			}
		}
		// --- End WorldToScreen Logic ---

		// IMPORTANT: Unmap the resource IMMEDIATELY after you are done reading from pMappedData
		D3D12_RANGE writtenRange = { 0, 0 }; // We only read, so no bytes were written by the CPU
		constantBufferResource->Unmap(0, &writtenRange);
	}
	else {
		Log("ERROR: Map failed for resource 0x%p. HR = 0x%X", (void*)constantBufferResource, hr);
	}
}
*/

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

//=========================================================================================================================//

// TEST code below

// --- Use ComPtr for globals ---
ComPtr<ID3D12RootSignature> g_rootSig = nullptr;
ComPtr<ID3D12PipelineState> g_greenPSO = nullptr;

const char* GreenOverlayHLSL = R"(
struct VS_OUTPUT { float4 Pos : SV_POSITION; };

VS_OUTPUT mainVS(uint VertID : SV_VertexID) {
    VS_OUTPUT output = (VS_OUTPUT)0;
    float2 pos = float2((VertID << 1) & 2, VertID & 2);
    output.Pos = float4(pos * 2.0f - 1.0f, 0.0f, 1.0f);
    output.Pos.y = -output.Pos.y;
    return output;
}

float4 mainPS(VS_OUTPUT input) : SV_TARGET {
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}
)";



bool InitGreenOverlayPipeline(ID3D12Device* device, ID3D12RootSignature** ppRootSig, ID3D12PipelineState** ppPSO)
{
	if (!ppRootSig || !ppPSO || !device) {
		if (ppRootSig) *ppRootSig = nullptr;
		if (ppPSO) *ppPSO = nullptr;
		return false;
	}

	*ppRootSig = nullptr;
	*ppPSO = nullptr;

	HRESULT hr;
	ComPtr<ID3D12RootSignature> tempRootSig = nullptr;
	ComPtr<ID3D12PipelineState> tempPSO = nullptr;
	ComPtr<ID3DBlob> rsBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> vsErrorBlob = nullptr;
	ComPtr<ID3DBlob> psErrorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
	if (FAILED(hr) || !rsBlob) return false;

	hr = device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&tempRootSig));
	if (FAILED(hr)) return false;

	tempRootSig->SetName(L"GreenOverlay RootSig");

	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	hr = D3DCompile(GreenOverlayHLSL, strlen(GreenOverlayHLSL), nullptr, nullptr, nullptr,
		"mainVS", "vs_5_0", compileFlags, 0, &vsBlob, &vsErrorBlob);
	if (FAILED(hr) || !vsBlob) return false;

	hr = D3DCompile(GreenOverlayHLSL, strlen(GreenOverlayHLSL), nullptr, nullptr, nullptr,
		"mainPS", "ps_5_0", compileFlags, 0, &psBlob, &psErrorBlob);
	if (FAILED(hr) || !psBlob) return false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = tempRootSig.Get();
	desc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	desc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
	desc.BlendState.IndependentBlendEnable = FALSE;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_ALPHA;

	desc.SampleMask = UINT_MAX;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.DepthStencilState.DepthEnable = FALSE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	desc.InputLayout = {};
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&tempPSO));
	if (FAILED(hr)) return false;

	tempPSO->SetName(L"GreenOverlay PSO");

	*ppRootSig = tempRootSig.Detach();
	*ppPSO = tempPSO.Detach();
	return true;
}

//=========================================================================================================================//

// Custom resources for coloring
ComPtr<ID3D12Resource> g_pCustomConstantBuffer = nullptr;
UINT8* g_pMappedConstantBuffer = nullptr; // Pointer to mapped CPU-accessible memory
UINT g_constantBufferSize = 0;

// TODO: Define the EXACT structure of the constant buffer you're targeting.
//       This MUST match what the shader expects. Size needs to be 256-byte aligned.
//       This example assumes a simple structure. It's likely MUCH more complex.
struct MyMaterialConstants // EXAMPLE STRUCTURE 
{
	//Can leave empty for now, what matters more is bruteforcing colorOffset
	/*
	DirectX::XMFLOAT4X4 worldViewProj; // Example matrix (64 bytes)
	DirectX::XMFLOAT2   uvScale;       // Example other parameter (8 bytes)
	DirectX::XMFLOAT2   uvScale2;      // Example other parameter (8 bytes)
	float               specularPower; // Example other parameter (4 bytes)
	float               metallic;      // Example other parameter (4 bytes)
	DirectX::XMFLOAT4   diffuseColor;  // The color we want to change (16 bytes)
	DirectX::XMFLOAT4   unknown;	   // The color we want to change (16 bytes)
	DirectX::XMFLOAT4   unknown2;	   // The color we want to change (16 bytes)
	DirectX::XMFLOAT4   unknown3;	   // The color we want to change (16 bytes)
	*/
	BYTE padding[256 - (sizeof(DirectX::XMFLOAT4) * 4 + sizeof(float) * 12 + sizeof(DirectX::XMFLOAT4X4))];
	//BYTE padding[4096 - (sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(float) * 2 + sizeof(DirectX::XMFLOAT2))];
}; // Total size = 256 bytes (EXAMPLE)

// Creates our upload buffer resource
bool CreateCustomConstantBuffer()
{
	if (!pDevice) return false;

	// Calculate struct size, ensure alignment
	g_constantBufferSize = (sizeof(MyMaterialConstants) + 255) & ~255; // Align to 256 bytes

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // CPU write, GPU read
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = g_constantBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Initial state for upload heap
		nullptr,
		IID_PPV_ARGS(&g_pCustomConstantBuffer));

	if (FAILED(hr)) {
		Log("Failed to create custom constant buffer!");
		return false;
	}

	g_pCustomConstantBuffer->SetName(L"CustomHookConstantBuffer");

	// Map the buffer permanently. Upload heaps are CPU-accessible.
	// We don't need to unmap unless we destroy the resource.
	D3D12_RANGE readRange = { 0, 0 }; // We don't intend to read from CPU
	hr = g_pCustomConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&g_pMappedConstantBuffer));

	if (FAILED(hr)) {
		Log("Failed to map custom constant buffer!");
		g_pCustomConstantBuffer.Reset(); // Release the buffer if map failed
		return false;
	}

	return true;
}

//=========================================================================================================================//


