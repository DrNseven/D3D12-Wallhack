#include <windows.h>
#include <psapi.h>
#include <vector>
#include <wrl.h>
#include <iostream>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <map>
#include <unordered_map>
#include <mutex>
#include <array>      
#include <cstdint>
#include <shared_mutex>
#include <unordered_set>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"winmm.lib")

using namespace Microsoft::WRL;

#include <DirectXMath.h>
using namespace DirectX;

#include "MinHook\Include\MinHook.h"

//=========================================================================================================================//

//globals
bool wallhack = 1;
bool colors = 0;
ComPtr<ID3D12Device> pDevice = nullptr;
bool initialized = false;
UINT countnum = -1;

// Thread-local cache 
thread_local struct {
	UINT lastCbvRootParameterIndex = UINT_MAX;
	UINT lastCbvRootParameterIndex2 = UINT_MAX;
	UINT StartSlot = 0;
	UINT Strides[16] = { 0 };
	UINT vertexBufferSizes[16] = { 0 };
	UINT cachedStrideSum = 0;
	UINT fastStride = 0;
	D3D12_VIEWPORT currentViewport = {};
	UINT numViewports = 0;
	UINT currentiSize = 0;
	DXGI_FORMAT currentIndexFormat = DXGI_FORMAT_UNKNOWN;
} t_;

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


//SetGraphicsRootSignature
//shared_mutex for better read performance
std::shared_mutex rootSigMutex;
uint32_t nextRuntimeSigID = 1; 
std::unordered_map<ID3D12RootSignature*, uint32_t> rootSigToID;
std::unordered_map<ID3D12GraphicsCommandList*, uint32_t> cmdListToID;
thread_local ID3D12GraphicsCommandList* tlsLastCmdList = nullptr;
thread_local uint32_t tlsLastRootSigID = 0;


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

int getTwoDigitValue(int value) {
	uint32_t h = (uint32_t)value;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return (h % 90) + 10;
}

uint32_t fastStrideHash(const uint32_t* data, size_t count) {
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < count; ++i) {
		hash ^= data[i];
		hash *= 16777619u;
	}
	return hash % 100; // Two-digit number
}


//=========================================================================================================================//

// Custom resources for coloring
ComPtr<ID3D12Resource> g_pCustomConstantBuffer = nullptr;
UINT8* g_pMappedConstantBuffer = nullptr; // Pointer to mapped CPU-accessible memory
UINT g_constantBufferSize = 0;

struct MyMaterialConstants
{
	BYTE padding[4096];
};

// Creates our upload buffer resource (used for coloring)
bool CreateCustomConstantBuffer()
{
	if (!pDevice) Log("!pDevice");
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

/*
extern float screenWidth;
extern float screenHeight;

bool WorldToScreen(const DirectX::XMVECTOR& worldPos, DirectX::XMFLOAT2& screenPos) {
	// 1. Transform the 3D world coordinate into 4D clip space
	DirectX::XMVECTOR clipCoords = DirectX::XMVector3TransformCoord(worldPos, g_ViewProjectionMatrix);

	// 2. Perform perspective divide
	// The W component is the depth value. If it's less than or equal to 0,
	// the point is behind or on the camera's near plane, so we can't see it.
	float w = DirectX::XMVectorGetW(clipCoords);
	if (w < 0.1f) {
		return false;
	}

	// Perspective divide (NDC)
	DirectX::XMVECTOR ndc = DirectX::XMVectorDivide(clipCoords, DirectX::XMVectorSet(w, w, w, w));

	// 3. Viewport transform (from [-1, 1] to [0, screenWidth] and [0, screenHeight])
	screenPos.x = (DirectX::XMVectorGetX(ndc) + 1.0f) * 0.5f * screenWidth;
	screenPos.y = (1.0f - DirectX::XMVectorGetY(ndc)) * 0.5f * screenHeight; // Y is inverted

	return true;
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
