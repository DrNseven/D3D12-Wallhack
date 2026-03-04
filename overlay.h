//overlay.h by N7 + UtterlyTV(Win11 Fix) 02/07/2026

//#include <windows.h>
//#include <d3d12.h>
//#include <dxgi1_4.h>
#include <dcomp.h>
#include <wrl.h>
#include <atomic>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_win32.h"
//#include "imgui/imgui_impl_dx12.h"

//#pragma comment(lib, "d3d12.lib")
//#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

using Microsoft::WRL::ComPtr;

// ============================================================
// CONFIG
// ============================================================

//menu
int countnum = -1;
bool menuisOpen = false;
int countstride1 = -1;
int countstride2 = -1;
int countstride3 = -1;
int countstride4 = -1;
int countstride5 = -1;
int countExIStride1 = -1;
int countExIStride2 = -1;
int countExIStride3 = -1;
int countExIStride4 = -1;
int countGRootConstantBuffer = -2;
int countGRootDescriptor = -2;
int countIndexCount = -1;
int countfindrendertarget = -1;

bool enabletemporaryids = false;
int countcurrentRootSigID = -1;
int countcurrentRootSigID2 = -1;
int countcurrentIndexAddress = -1;
int countcurrentIndexAddress2 = -1;
int countcurrentIndexAddress3 = -1;
int countcurrentPSOAddress = -1;

bool enablefilters = false;
bool filterrendertarget = false;
int countfilterrendertarget = -1;
int countfilterrendertarget2 = -1;
bool filterGRootConstantBuffer = false;
int countfilterGRootConstantBuffer = -2;
int countfilterGRootConstantBuffer2 = -2;
int countfilterGRootConstantBuffer3 = -2;
bool filterGRootDescriptor = false;
int countfilterGRootDescriptor = -2;
int countfilterGRootDescriptor2 = -2;
int countfilterGRootDescriptor3 = -2;
bool filterindexformat = false;
int countfilterindexformat = -1;
bool filternumViews = false;
int countfilternumViews = -1;
bool filterIndexCountPerInstance = false;//
int countfilterIndexCountPerInstance = -1;
int countfilterIndexCountPerInstance2 = -1;

bool enableignores = false;
bool ignorerendertarget = false;
int countignorerendertarget = -1;
bool ignoreGRootConstantBuffer = false;
int countignoreGRootConstantBuffer = -2;
int countignoreGRootConstantBuffer2 = -2;
int countignoreGRootConstantBuffer3 = -2;
bool ignoreGRootDescriptor = false;
int countignoreGRootDescriptor = -2;
int countignoreGRootDescriptor2 = -2;
int countignoreGRootDescriptor3 = -2;
bool ignorenumViews = false;
int countignorenumViews = -1;
bool ignoreIndexCountPerInstance = false;//
int countignoreIndexCountPerInstance = -1;

bool reversedDepth = false;
bool DisableOcclusionCulling = true;
bool enablecolor = false;
int cri0 = 0;
int cri1 = 0;
int cri2 = 0;
int cri3 = 0;
int cri4 = 0;
int cri5 = 0;
int cri6 = 0;
int cri7 = 0;
int cri8 = 0;
int cri9 = 0;
int cri10 = 0;

bool ExecuteIndirectisCalled = 0;

using namespace std;
#include <string>
#include <fstream>
void SaveConfig()
{
    ofstream fout("d3d12wh.ini", ios::trunc);

    fout << "countstride1 " << countstride1 << endl;
    fout << "countstride2 " << countstride2 << endl;
    fout << "countstride3 " << countstride3 << endl;
    fout << "countstride4 " << countstride4 << endl;
    fout << "countstride5 " << countstride5 << endl;

    fout << "exinstride1 " << countExIStride1 << endl;
    fout << "exinstride2 " << countExIStride2 << endl;
    fout << "exinstride3 " << countExIStride3 << endl;
    fout << "exinstride4 " << countExIStride4 << endl;

    fout << "countGRootConstantBuffer " << countGRootConstantBuffer << endl;
    fout << "countGRootDescriptor " << countGRootDescriptor << endl;
    fout << "countIndexCount " << countIndexCount << endl;
    fout << "countfindrendertarget " << countfindrendertarget << endl;

    fout << "enabletemporaryids " << enabletemporaryids << endl;
    fout << "countcurrentRootSigID " << countcurrentRootSigID << endl;
    fout << "countcurrentRootSigID2 " << countcurrentRootSigID2 << endl;
    fout << "countcurrentIndexAddress " << countcurrentIndexAddress << endl;
    fout << "countcurrentIndexAddress2 " << countcurrentIndexAddress2 << endl;
    fout << "countcurrentIndexAddress3 " << countcurrentIndexAddress3 << endl;
    fout << "countcurrentPSOAddress " << countcurrentPSOAddress << endl;

    fout << "enablefilters " << enablefilters << endl;
    fout << "filterrendertarget " << filterrendertarget << endl;
    fout << "countfilterrendertarget " << countfilterrendertarget << endl;
    fout << "countfilterrendertarget2 " << countfilterrendertarget2 << endl;

    fout << "filterGRootConstantBuffer " << filterGRootConstantBuffer << endl;
    fout << "countfilterGRootConstantBuffer " << countfilterGRootConstantBuffer << endl;
    fout << "countfilterGRootConstantBuffer2 " << countfilterGRootConstantBuffer2 << endl;
    fout << "countfilterGRootConstantBuffer3 " << countfilterGRootConstantBuffer3 << endl;

    fout << "filterGRootDescriptor " << filterGRootDescriptor << endl;
    fout << "countfilterGRootDescriptor " << countfilterGRootDescriptor << endl;
    fout << "countfilterGRootDescriptor2 " << countfilterGRootDescriptor2 << endl;
    fout << "countfilterGRootDescriptor3 " << countfilterGRootDescriptor3 << endl;

    fout << "filterindexformat " << filterindexformat << endl;
    fout << "countfilterindexformat " << countfilterindexformat << endl;

    fout << "filternumViews " << filternumViews << endl;
    fout << "countfilternumViews " << countfilternumViews << endl;

    fout << "filterIndexCountPerInstance " << filterIndexCountPerInstance << endl;
    fout << "countfilterIndexCountPerInstance " << countfilterIndexCountPerInstance << endl;
    fout << "countfilterIndexCountPerInstance2 " << countfilterIndexCountPerInstance2 << endl;

    fout << "enableignores " << enableignores << endl;
    fout << "ignorerendertarget " << ignorerendertarget << endl;
    fout << "countignorerendertarget " << countignorerendertarget << endl;

    fout << "ignoreGRootConstantBuffer " << ignoreGRootConstantBuffer << endl;
    fout << "countignoreGRootConstantBuffer " << countignoreGRootConstantBuffer << endl;
    fout << "countignoreGRootConstantBuffer2 " << countignoreGRootConstantBuffer2 << endl;
    fout << "countignoreGRootConstantBuffer3 " << countignoreGRootConstantBuffer3 << endl;

    fout << "ignoreGRootDescriptor " << ignoreGRootDescriptor << endl;
    fout << "countignoreGRootDescriptor " << countignoreGRootDescriptor << endl;
    fout << "countignoreGRootDescriptor2 " << countignoreGRootDescriptor2 << endl;
    fout << "countignoreGRootDescriptor3 " << countignoreGRootDescriptor3 << endl;

    fout << "ignorenumViews " << ignorenumViews << endl;
    fout << "countignorenumViews " << countignorenumViews << endl;

    fout << "ignoreIndexCountPerInstance " << ignoreIndexCountPerInstance << endl;
    fout << "countignoreIndexCountPerInstance " << countignoreIndexCountPerInstance << endl;

    fout << "reversedDepth " << reversedDepth << endl;
    fout << "DisableOcclusionCulling " << DisableOcclusionCulling << endl;

    fout << "enablecolor " << enablecolor << endl;
    fout << "coloroffset " << coloroffset << endl;

    fout << "cri0 " << cri0 << endl;
    fout << "cri1 " << cri1 << endl;
    fout << "cri2 " << cri2 << endl;
    fout << "cri3 " << cri3 << endl;
    fout << "cri4 " << cri4 << endl;
    fout << "cri5 " << cri5 << endl;
    fout << "cri6 " << cri6 << endl;
    fout << "cri7 " << cri7 << endl;
    fout << "cri8 " << cri8 << endl;
    fout << "cri9 " << cri9 << endl;
    fout << "cri10 " << cri10 << endl;

    fout.close();
}

void LoadConfig()
{
    ifstream fin("d3d12wh.ini", ios::in);

    string key;
    while (fin >> key)
    {
        if (key == "countstride1")                  fin >> countstride1;
        else if (key == "countstride2")             fin >> countstride2;
        else if (key == "countstride3")             fin >> countstride3;
        else if (key == "countstride4")             fin >> countstride4;
        else if (key == "countstride5")             fin >> countstride5;

        else if (key == "exinstride1")              fin >> countExIStride1;
        else if (key == "exinstride2")              fin >> countExIStride2;
        else if (key == "exinstride3")              fin >> countExIStride3;
        else if (key == "exinstride4")              fin >> countExIStride4;

        else if (key == "countGRootConstantBuffer")  fin >> countGRootConstantBuffer;
        else if (key == "countGRootDescriptor")      fin >> countGRootDescriptor;
        else if (key == "countIndexCount")                  fin >> countIndexCount;
        else if (key == "countfindrendertarget")            fin >> countfindrendertarget;

        else if (key == "enabletemporaryids")       fin >> enabletemporaryids;
        else if (key == "countcurrentRootSigID")    fin >> countcurrentRootSigID;
        else if (key == "countcurrentRootSigID2")   fin >> countcurrentRootSigID2;
        else if (key == "countcurrentIndexAddress") fin >> countcurrentIndexAddress;
        else if (key == "countcurrentIndexAddress2")fin >> countcurrentIndexAddress2;
        else if (key == "countcurrentIndexAddress3")fin >> countcurrentIndexAddress3;
        else if (key == "countcurrentPSOAddress")   fin >> countcurrentPSOAddress;

        else if (key == "enablefilters")            fin >> enablefilters;
        else if (key == "filterrendertarget")       fin >> filterrendertarget;
        else if (key == "countfilterrendertarget")  fin >> countfilterrendertarget;
        else if (key == "countfilterrendertarget2") fin >> countfilterrendertarget2;

        else if (key == "filterGRootConstantBuffer")     fin >> filterGRootConstantBuffer;
        else if (key == "countfilterGRootConstantBuffer") fin >> countfilterGRootConstantBuffer;
        else if (key == "countfilterGRootConstantBuffer2")fin >> countfilterGRootConstantBuffer2;
        else if (key == "countfilterGRootConstantBuffer3")fin >> countfilterGRootConstantBuffer3;

        else if (key == "filterGRootDescriptor")     fin >> filterGRootDescriptor;
        else if (key == "countfilterGRootDescriptor") fin >> countfilterGRootDescriptor;
        else if (key == "countfilterGRootDescriptor2")fin >> countfilterGRootDescriptor2;
        else if (key == "countfilterGRootDescriptor3")fin >> countfilterGRootDescriptor3;

        else if (key == "filterindexformat")        fin >> filterindexformat;
        else if (key == "countfilterindexformat")   fin >> countfilterindexformat;

        else if (key == "filternumViews")           fin >> filternumViews;
        else if (key == "countfilternumViews")      fin >> countfilternumViews;

        else if (key == "filterIndexCountPerInstance")           fin >> filterIndexCountPerInstance;
        else if (key == "countfilterIndexCountPerInstance")      fin >> countfilterIndexCountPerInstance;
        else if (key == "countfilterIndexCountPerInstance2")      fin >> countfilterIndexCountPerInstance2;

        else if (key == "enableignores")            fin >> enableignores;
        else if (key == "ignorerendertarget")       fin >> ignorerendertarget;
        else if (key == "countignorerendertarget")  fin >> countignorerendertarget;

        else if (key == "ignoreGRootConstantBuffer")     fin >> ignoreGRootConstantBuffer;
        else if (key == "countignoreGRootConstantBuffer") fin >> countignoreGRootConstantBuffer;
        else if (key == "countignoreGRootConstantBuffer2")fin >> countignoreGRootConstantBuffer2;
        else if (key == "countignoreGRootConstantBuffer3")fin >> countignoreGRootConstantBuffer3;

        else if (key == "ignoreGRootDescriptor")     fin >> ignoreGRootDescriptor;
        else if (key == "countignoreGRootDescriptor") fin >> countignoreGRootDescriptor;
        else if (key == "countignoreGRootDescriptor2")fin >> countignoreGRootDescriptor2;
        else if (key == "countignoreGRootDescriptor3")fin >> countignoreGRootDescriptor3;

        else if (key == "ignorenumViews")           fin >> ignorenumViews;
        else if (key == "countignorenumViews")      fin >> countignorenumViews;

        else if (key == "ignoreIndexCountPerInstance")           fin >> ignoreIndexCountPerInstance;
        else if (key == "countignoreIndexCountPerInstance")      fin >> countignoreIndexCountPerInstance;

        else if (key == "reversedDepth")            fin >> reversedDepth;
        else if (key == "DisableOcclusionCulling")  fin >> DisableOcclusionCulling;

        else if (key == "enablecolor")              fin >> enablecolor;
        else if (key == "coloroffset")              fin >> coloroffset;

        else if (key == "cri0")  fin >> cri0;
        else if (key == "cri1")  fin >> cri1;
        else if (key == "cri2")  fin >> cri2;
        else if (key == "cri3")  fin >> cri3;
        else if (key == "cri4")  fin >> cri4;
        else if (key == "cri5")  fin >> cri5;
        else if (key == "cri6")  fin >> cri6;
        else if (key == "cri7")  fin >> cri7;
        else if (key == "cri8")  fin >> cri8;
        else if (key == "cri9")  fin >> cri9;
        else if (key == "cri10") fin >> cri10;

        // Ignore unknown keys (just consume the value)
        else
        {
            string dummy;
            fin >> dummy;
        }
    }

    fin.close();
}

// ============================================================
// OVERLAY GLOBALS (SINGLE SOURCE OF TRUTH)
// ============================================================
#define FRAME_COUNT 2
static std::atomic<bool> g_running = true;
static HWND g_gameHwnd = nullptr;
static HWND g_overlayHwnd = nullptr;
static bool g_clickThrough = true;

// DX12
static ComPtr<ID3D12Device> g_device;
static ComPtr<ID3D12CommandQueue> g_queue;
static ComPtr<ID3D12CommandAllocator> g_alloc[FRAME_COUNT];
static ComPtr<ID3D12GraphicsCommandList> g_cmd;
static ComPtr<ID3D12Fence> g_fence;
static HANDLE g_fenceEvent = nullptr;
//static UINT64 g_fenceValue = 0;
//UINT64 g_fenceValues[FRAME_COUNT] = { 0 };
static UINT64 g_mainFenceValue = 0; // The single "truth" for fence values
static UINT64 g_frameFenceValues[FRAME_COUNT] = { 0 }; // Values assigned to specific buffers

// Swapchain + RTV
static ComPtr<IDXGISwapChain3> g_swapchain;
static ComPtr<ID3D12DescriptorHeap> g_rtvHeap;
static ComPtr<ID3D12DescriptorHeap> g_srvHeap;
static ComPtr<ID3D12Resource> g_buffers[FRAME_COUNT];
static UINT g_rtvSize = 0;

// DirectComposition
static ComPtr<IDCompositionDevice> g_dcomp;
static ComPtr<IDCompositionTarget> g_target;
static ComPtr<IDCompositionVisual> g_visual;

// Resize tracking
static UINT g_width = 0;
static UINT g_height = 0;

// ============================================================
// HELPERS
// ============================================================

void FlushGPU()
{
    if (g_queue && g_fence && g_fenceEvent)
    {
        g_mainFenceValue++;
        g_queue->Signal(g_fence.Get(), g_mainFenceValue);
        if (g_fence->GetCompletedValue() < g_mainFenceValue)
        {
            g_fence->SetEventOnCompletion(g_mainFenceValue, g_fenceEvent);
            WaitForSingleObject(g_fenceEvent, INFINITE);
        }
    }
}

RECT GetGameRect()
{
    RECT r{};
    GetWindowRect(g_gameHwnd, &r);
    return r;
}

HWND FindMainGameWindow()
{
    DWORD pid = GetCurrentProcessId();
    HWND hwnd = nullptr;
    HWND best = nullptr;
    long long bestArea = 0;

    while ((hwnd = FindWindowEx(nullptr, hwnd, nullptr, nullptr)) != nullptr)
    {
        if (GetParent(hwnd) == nullptr && IsWindowVisible(hwnd))
        {
            DWORD windowPid = 0;
            GetWindowThreadProcessId(hwnd, &windowPid);
            if (windowPid == pid)
            {
                RECT rect;
                if (GetWindowRect(hwnd, &rect))
                {
                    long long width = rect.right - rect.left;
                    long long height = rect.bottom - rect.top;
                    long long area = width * height;

                    if (area > bestArea && width >= 640 && height >= 480) // reasonable minimum game size
                    {
                        bestArea = area;
                        best = hwnd;
                    }
                }
            }
        }
    }
    return best;
}

// ============================================================
// WINDOW
// ============================================================
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // Always let ImGui process messages first if we aren't clicking through
    if (!g_clickThrough)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
            return true;
    }

    switch (msg)
    {
    case WM_DESTROY:
        g_running = false;
        return 0;

    case WM_NCHITTEST:
        // HTTRANSPARENT makes clicks "pass through" to the game
        // HTCLIENT makes clicks stay on the overlay
        return g_clickThrough ? HTTRANSPARENT : HTCLIENT;

    case WM_SYSCOMMAND:
        if ((wp & 0xFFF0) == SC_KEYMENU) // Disable ALT focusing the menu bar
            return 0;
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

HWND CreateOverlayWindow()
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"OverlayClass";
    RegisterClassEx(&wc);

    // Updated extended styles as you requested
    // Added WS_EX_TRANSPARENT (critical for click-through)
    // Added WS_EX_TOOLWINDOW (hides from taskbar/Alt+Tab)
    // Kept WS_EX_LAYERED (required for per-pixel alpha or any layered behavior)
    // Removed WS_EX_NOREDIRECTIONBITMAP (often causes issues on Win11 with external overlays)
    DWORD exStyles = WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;

    HWND hwnd = CreateWindowEx(
        exStyles,
        wc.lpszClassName,
        L"Overlay",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, wc.hInstance, nullptr
    );

    // Removed SetLayeredWindowAttributes completely (no LWA_COLORKEY or LWA_ALPHA)
    // Removed DwmExtendFrameIntoClientArea (conflicts with pure layered window approach)

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

// ============================================================
// D3D12 INIT
// ============================================================
void InitD3D12()
{
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device));

    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    g_device->CreateCommandQueue(&qd, IID_PPV_ARGS(&g_queue));

    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        g_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&g_alloc[i])
        );
    }

    g_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        g_alloc[0].Get(),
        nullptr,
        IID_PPV_ARGS(&g_cmd)
    );
    g_cmd->Close();

    g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence));
    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

// ============================================================
// DIRECTCOMPOSITION
// ============================================================
void InitDirectComposition(UINT width, UINT height)
{
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferCount = FRAME_COUNT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    ComPtr<IDXGISwapChain1> sc1;
    factory->CreateSwapChainForComposition(
        g_queue.Get(),
        &sd,
        nullptr,
        &sc1
    );

    sc1.As(&g_swapchain);

    DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&g_dcomp));
    g_dcomp->CreateTargetForHwnd(g_overlayHwnd, TRUE, &g_target);
    g_dcomp->CreateVisual(&g_visual);
    g_visual->SetContent(g_swapchain.Get());
    g_target->SetRoot(g_visual.Get());
    g_dcomp->Commit();
}

// ============================================================
// RTV + IMGUI
// ============================================================
void InitRTVsAndImGui()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = FRAME_COUNT;
    g_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_rtvHeap));
    g_rtvSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
    srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.NumDescriptors = 1;
    srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    g_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_srvHeap));

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        g_swapchain->GetBuffer(i, IID_PPV_ARGS(&g_buffers[i]));
        g_device->CreateRenderTargetView(g_buffers[i].Get(), nullptr, rtv);
        rtv.ptr += g_rtvSize;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(g_overlayHwnd);
    ImGui_ImplDX12_Init(
        g_device.Get(),
        FRAME_COUNT,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        g_srvHeap.Get(),
        g_srvHeap->GetCPUDescriptorHandleForHeapStart(),
        g_srvHeap->GetGPUDescriptorHandleForHeapStart()
    );
}

// ============================================================
// RESIZE HANDLING
// ============================================================
void ResizeOverlayIfNeeded()
{
    if (!IsWindow(g_gameHwnd) || GetWindowLong(g_gameHwnd, GWL_STYLE) == 0)
    {
        g_gameHwnd = FindMainGameWindow();
        if (!g_gameHwnd) return;
    }

    RECT r = GetGameRect();
    UINT w = r.right - r.left;
    UINT h = r.bottom - r.top;

    if (w <= 0 || h <= 0) return;
    if (w == g_width && h == g_height) return;

    // 1. Wait for ALL GPU work to finish across all buffers
    FlushGPU();

    // 2. IMPORTANT: You MUST release all buffer references before resizing
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        g_buffers[i].Reset();
        g_frameFenceValues[i] = 0; // Reset these so Render() doesn't wait on old values
    }

    g_width = w;
    g_height = h;

    // 3. Resize the swapchain
    HRESULT hr = g_swapchain->ResizeBuffers(
        FRAME_COUNT,
        g_width,
        g_height,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        0
    );

    if (FAILED(hr)) return;

    // 4. Recreate RTVs
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        g_swapchain->GetBuffer(i, IID_PPV_ARGS(&g_buffers[i]));
        g_device->CreateRenderTargetView(g_buffers[i].Get(), nullptr, rtv);
        rtv.ptr += g_rtvSize;
    }

    // 5. Update window and ImGui
    SetWindowPos(g_overlayHwnd, HWND_TOPMOST, r.left, r.top, w, h, SWP_NOACTIVATE);

    if (ImGui::GetCurrentContext())
        ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
}

void UpdateInputState()
{
    static bool lastClickThrough = true;
    if (g_clickThrough != lastClickThrough)
    {
        LONG_PTR exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);
        if (g_clickThrough)
        {
            // Add flags to make it transparent to mouse
            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
        }
        else
        {
            // Remove flags to make it catch mouse clicks
            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
            // Bring to front and give focus
            SetForegroundWindow(g_overlayHwnd);
        }
        lastClickThrough = g_clickThrough;
    }
}

// ============================================================
// RENDER
// ============================================================
static bool g_showMenu = false; // Menu visible or not

void Render()
{
    ResizeOverlayIfNeeded();

    if (GetAsyncKeyState(VK_INSERT) & 1) //menu key
    {
        SaveConfig(); //Save settings
        g_showMenu = !g_showMenu;
        g_clickThrough = !g_showMenu;

        LONG_PTR exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);

        if (g_showMenu)
        {
            // REMOVE NOACTIVATE and TRANSPARENT
            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT & ~WS_EX_NOACTIVATE);

            // Force the window to the front and give it keyboard focus
            SetForegroundWindow(g_overlayHwnd);
            SetActiveWindow(g_overlayHwnd);
            SetFocus(g_overlayHwnd);
        }
        else
        {
            // ADD NOACTIVATE and TRANSPARENT
            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);
            SetForegroundWindow(g_gameHwnd);
        }
    }

    UpdateInputState();

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Only draw the menu if g_showMenu is true
    if (g_showMenu)
    {
        // When the menu is open, we generally want inputs ENABLED, unless you specifically want to 'lock' it.
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowSize(ImVec2(480, 440), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(25, 25), ImGuiCond_FirstUseEver);

        ImGui::Begin("Wallhack", &g_showMenu, flags);
        //ImGui::Begin("Wallhack", nullptr, flags | ImGuiWindowFlags_NoFocusOnAppearing);

        const UINT minus_val = -1;
        const UINT minus_val2 = -2;
        const UINT min_val = 0;
        const UINT max_val = 100;
        const UINT max_valisone = 1;
        ImGui::Text("Find Permanent Values");
        ImGui::SliderInt("Stridehash1", &countstride1, minus_val, max_val);
        ImGui::SliderInt("Stridehash2", &countstride2, minus_val, max_val);
        ImGui::SliderInt("Stridehash3", &countstride3, minus_val, max_val);
        ImGui::SliderInt("Stridehash4", &countstride4, minus_val, max_val);
        ImGui::SliderInt("Stridehash5", &countstride5, minus_val, max_val);
        if(ExecuteIndirectisCalled == 1) ImGui::Text("ExInStrides Available");
        ImGui::SliderInt("ExInStride1", &countExIStride1, minus_val, max_val);
        ImGui::SliderInt("ExInStride2", &countExIStride2, minus_val, max_val);
        ImGui::SliderInt("ExInStride3", &countExIStride3, minus_val, max_val);
        ImGui::SliderInt("ExInStride4", &countExIStride4, minus_val, max_val);
        ImGui::SliderInt("GRootConstantBuffer", &countGRootConstantBuffer, minus_val2, max_val);
        ImGui::SliderInt("GRootDescriptor", &countGRootDescriptor, minus_val2, max_val);
        ImGui::SliderInt("IndexCount * 1000", &countIndexCount, minus_val, max_val);
        ImGui::SliderInt("RenderTarget", &countfindrendertarget, minus_val, max_val);

        //Temporary IDs below (changes after game restart)
        ImGui::Checkbox("Find Temporary Values", &enabletemporaryids);
        if (enabletemporaryids)
        {
            ImGui::SliderInt("CurrentRootSigID1", &countcurrentRootSigID, minus_val, max_val);
            ImGui::SliderInt("CurrentRootSigID2", &countcurrentRootSigID2, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress1", &countcurrentIndexAddress, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress2", &countcurrentIndexAddress2, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress3", &countcurrentIndexAddress3, minus_val, max_val);
            ImGui::SliderInt("CurrentPSOAddress", &countcurrentPSOAddress, minus_val, max_val);
        }

        //Filters
        ImGui::Checkbox("Enable Filters", &enablefilters);
        if (enablefilters)
        {
            ImGui::Checkbox("Filter RenderTarget", &filterrendertarget);
            if (filterrendertarget)
            {
                ImGui::SliderInt("Render Target", &countfilterrendertarget, minus_val, max_val);
                ImGui::SliderInt("Render Target2", &countfilterrendertarget2, minus_val, max_val);
            }

            ImGui::Checkbox("Filter GRootDescriptor", &filterGRootDescriptor);
            if (filterGRootDescriptor)
            {
                ImGui::SliderInt("GRootDescriptor1", &countfilterGRootDescriptor, minus_val2, max_val);
                ImGui::SliderInt("GRootDescriptor2", &countfilterGRootDescriptor2, minus_val2, max_val);
                ImGui::SliderInt("GRootDescriptor3", &countfilterGRootDescriptor3, minus_val2, max_val);
            }

            ImGui::Checkbox("Filter GRootConstantBuffer", &filterGRootConstantBuffer);
            if (filterGRootConstantBuffer)
            {
                ImGui::SliderInt("GRootConstantBuffer1", &countfilterGRootConstantBuffer, minus_val2, max_val);
                ImGui::SliderInt("GRootConstantBuffer2", &countfilterGRootConstantBuffer2, minus_val2, max_val);
                ImGui::SliderInt("GRootConstantBuffer3", &countfilterGRootConstantBuffer3, minus_val2, max_val);
            }

            ImGui::Checkbox("Filter IndexFormat", &filterindexformat);
            if (filterindexformat)
            {
                ImGui::SliderInt("IndexFormat", &countfilterindexformat, minus_val, max_val);
            }

            ImGui::Checkbox("Filter NumViews", &filternumViews);
            if (filternumViews)
            {
                ImGui::SliderInt("NumViews", &countfilternumViews, minus_val, max_val);
            }

            ImGui::Checkbox("Filter IndexCount", &filterIndexCountPerInstance);
            if (filterIndexCountPerInstance)
            {
                ImGui::SliderInt("IndexCount1*1000", &countfilterIndexCountPerInstance, minus_val, max_val);
                ImGui::SliderInt("IndexCount2*1000", &countfilterIndexCountPerInstance2, minus_val, max_val);
            }
        }//end of filters


        //Ignores
        ImGui::Checkbox("Enable Ignores", &enableignores);
        if (enableignores)
        {
            ImGui::Checkbox("Ignore RenderTarget", &ignorerendertarget);
            if (ignorerendertarget)
            {
                ImGui::SliderInt("RenderTarget ", &countignorerendertarget, minus_val, max_val);
            }

            ImGui::Checkbox("Ignore GRootDescriptor", &ignoreGRootDescriptor);
            if (ignoreGRootDescriptor)
            {
                ImGui::SliderInt("GRootDescriptor 1", &countignoreGRootDescriptor, minus_val2, max_val);
                ImGui::SliderInt("GRootDescriptor 2", &countignoreGRootDescriptor2, minus_val2, max_val);
                ImGui::SliderInt("GRootDescriptor 3", &countignoreGRootDescriptor3, minus_val2, max_val);
            }

            ImGui::Checkbox("Ignore GRootConstantBuffer", &ignoreGRootConstantBuffer);
            if (ignoreGRootConstantBuffer)
            {
                ImGui::SliderInt("GRootConstantBuffer 1", &countignoreGRootConstantBuffer, minus_val2, max_val);
                ImGui::SliderInt("GRootConstantBuffer 2", &countignoreGRootConstantBuffer2, minus_val2, max_val);
                ImGui::SliderInt("GRootConstantBuffer 3", &countignoreGRootConstantBuffer3, minus_val2, max_val);
            }

            ImGui::Checkbox("Ignore NumViews", &ignorenumViews);
            if (ignorenumViews)
            {
                ImGui::SliderInt("NumViews ", &countignorenumViews, minus_val, max_val);
            }

            ImGui::Checkbox("Ignore IndexCount below", &ignoreIndexCountPerInstance);
            if (ignoreIndexCountPerInstance)
            {
                ImGui::SliderInt("IndexCount*1000 ", &countignoreIndexCountPerInstance, minus_val, max_val);
            }
        } //end of ignores

        ImGui::Checkbox("Reverse Depth", &reversedDepth);
        ImGui::Checkbox("Try to Disable Occlusion", &DisableOcclusionCulling);
        ImGui::Checkbox("Enable Color UE5+", &enablecolor);
        ImGui::SliderInt("Coloroffset", &coloroffset, min_val, max_val);
        if (enablecolor)
        {
            ImGui::SliderInt("Color RootIndex 0", &cri0, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 1", &cri1, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 2", &cri2, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 3", &cri3, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 4", &cri4, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 5", &cri5, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 6", &cri6, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 7", &cri7, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 8", &cri8, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 9", &cri9, min_val, max_valisone);
            ImGui::SliderInt("Color RootIndex 10", &cri10, min_val, max_valisone);
        }

        ImGui::End();
    }

    // Always draw ESP/Overlay elements (even if menu is hidden)
    //if (espEnabled)
    //{
        // Use the Background DrawList to draw while the menu is hidden
        //ImDrawList* backgroundDrawList = ImGui::GetBackgroundDrawList();
        //backgroundDrawList->AddText(ImVec2(100, 100), IM_COL32(255, 255, 0, 255), "ESP ACTIVE");
    //}

    ImGui::Render();

    // 1. Get the current index
    UINT backBufferIdx = g_swapchain->GetCurrentBackBufferIndex();

    // 2. Wait for this specific buffer to be ready
    if (g_frameFenceValues[backBufferIdx] != 0 && g_fence->GetCompletedValue() < g_frameFenceValues[backBufferIdx])
    {
        g_fence->SetEventOnCompletion(g_frameFenceValues[backBufferIdx], g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    // 3. Record commands
    g_alloc[backBufferIdx]->Reset();
    g_cmd->Reset(g_alloc[backBufferIdx].Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = g_buffers[backBufferIdx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    g_cmd->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += backBufferIdx * g_rtvSize;

    const float clear[4] = { 0,0,0,0 };
    g_cmd->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    g_cmd->ClearRenderTargetView(rtv, clear, 0, nullptr);
    g_cmd->SetDescriptorHeaps(1, g_srvHeap.GetAddressOf());

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_cmd.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_cmd->ResourceBarrier(1, &barrier);

    g_cmd->Close();

    // 4. Execute and Present
    ID3D12CommandList* lists[] = { g_cmd.Get() };
    g_queue->ExecuteCommandLists(1, lists);

    g_swapchain->Present(1, 0); // 1 for smoother resizing

    // 5. Signal the fence for this specific buffer
    g_mainFenceValue++;
    g_queue->Signal(g_fence.Get(), g_mainFenceValue);
    g_frameFenceValues[backBufferIdx] = g_mainFenceValue;
}

// ============================================================
// THREAD
// ============================================================

void CleanupOverlay()
{
    //Log("[Cleanup] Starting Graceful Shutdown...\n");

    // 1. Wait for GPU to finish all pending frames
    // If we don't do this, we release memory the GPU is still using!
    if (g_device && g_queue && g_fence && g_fenceEvent)
    {
        g_mainFenceValue++;
        if (SUCCEEDED(g_queue->Signal(g_fence.Get(), g_mainFenceValue)))
        {
            if (g_fence->GetCompletedValue() < g_mainFenceValue)
            {
                WaitForSingleObject(g_fenceEvent, INFINITE);
            }
        }
    }

    // 2. Shutdown ImGui (Must happen while Device and SrvHeap are still alive)
    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        Log("[Cleanup] ImGui context destroyed.\n");
    }

    // 3. Clear DirectComposition
    // We Reset() in specific order: Visual -> Target -> Device
    if (g_visual) g_visual.Reset();
    if (g_target) g_target.Reset();
    if (g_dcomp)  g_dcomp.Reset();

    // 4. Release Swapchain and Buffers
    for (int i = 0; i < FRAME_COUNT; ++i)
    {
        g_buffers[i].Reset();
        g_alloc[i].Reset();
    }
    if (g_swapchain) g_swapchain.Reset();
    if (g_rtvHeap)   g_rtvHeap.Reset();
    if (g_srvHeap)   g_srvHeap.Reset();

    // 5. Release Core DX12 Objects
    if (g_cmd)   g_cmd.Reset();
    if (g_fence) g_fence.Reset();
    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = nullptr;
    }
    if (g_queue)  g_queue.Reset();
    if (g_device) g_device.Reset();

    // 6. Destroy the window
    if (g_overlayHwnd && IsWindow(g_overlayHwnd))
    {
        DestroyWindow(g_overlayHwnd);
        g_overlayHwnd = nullptr;
    }

    Log("[Cleanup] Shutdown successful.\n");
}

DWORD WINAPI OverlayThread(LPVOID)
{
    // New - reliably find the game's main window
    while (g_running)
    {
        g_gameHwnd = FindMainGameWindow();
        if (g_gameHwnd && IsWindow(g_gameHwnd))
            break;
        Sleep(200);
    }
    if (!g_running) return 0;

    RECT r = GetGameRect();
    g_overlayHwnd = CreateOverlayWindow();
    MoveWindow(g_overlayHwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

    InitD3D12();
    InitDirectComposition(r.right - r.left, r.bottom - r.top);
    InitRTVsAndImGui();

    LoadConfig();

    while (g_running)
    {
        // If the game window is closed or alt-f4'd, exit the loop
        if (!IsWindow(g_gameHwnd))
        {
            g_running = false;
            break;
        }

        Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // CLEANUP BEFORE EXITING THREAD
    CleanupOverlay(); // Implement this to destroy g_overlayHwnd and D3D objects
    return 0;
}

//=========================================================================================================================//
//End of Overlay
//=========================================================================================================================//

// Utility helpers for backend initialization checks
using IsInitFn = bool (*)();

static bool WaitForInitialization(IsInitFn fn, int attempts = 50, int sleepMs = 100)
{
    for (int i = 0; i < attempts; ++i)
    {
        if (fn())
            return true;
        Sleep(sleepMs);
    }
    return false;
}

static bool TryInitBackend(globals::Backend backend)
{
    switch (backend)
    {
    case globals::Backend::DX12:
        if (GetModuleHandleA("d3d12.dll") || GetModuleHandleA("dxgi.dll"))
        {
            //Log("[DllMain] Attempting DX12 initialization.\n");
            hooks::InitH();
            if (WaitForInitialization(d3d12hook::IsInitialized))
            {
                Log("[DllMain] DX12 initialization succeeded.");
                globals::activeBackend = globals::Backend::DX12;
                return true;
            }
            Log("[DllMain] DX12 initialization failed, falling back.\n");
            d3d12hook::release();
        }
        break;
    default:
        break;
    }
    return false;
}

static bool TryInitializeFrom(globals::Backend start)
{
    const globals::Backend order[] = {
        globals::Backend::DX12,
    };
    const int num_backends = sizeof(order) / sizeof(globals::Backend);
    int idx = 0;
    for (; idx < num_backends; ++idx)
    {
        if (order[idx] == start)
            break;
    }
    for (; idx < num_backends; ++idx)
    {
        if (TryInitBackend(order[idx]))
            return true;
    }
    Log("[DllMain] All backend initialization attempts failed.\n");
    return false;
}

// Helper: check loaded module name and initialize hooks if needed
static int GetBackendPriority(globals::Backend backend)
{
    switch (backend)
    {
    case globals::Backend::DX12:   return 1;
    default:                       return 0;
    }
}

static void InitForModule(const char* name)
{
    if (!name)
        return;

    const char* base = strrchr(name, '\\');
    base = base ? base + 1 : name;

    globals::Backend detected = globals::Backend::None;
    if (_stricmp(base, "d3d12.dll") == 0 || _stricmp(base, "dxgi.dll") == 0) {
        detected = globals::Backend::DX12;
    }
    else {
        return;
    }

    if (globals::preferredBackend != globals::Backend::None && detected != globals::preferredBackend)
        return;

    if (GetBackendPriority(detected) <= GetBackendPriority(globals::activeBackend))
        return;

    switch (globals::activeBackend)
    {
    case globals::Backend::DX12:   d3d12hook::release(); break;
    default: break;
    }

    globals::activeBackend = globals::Backend::None;
    if (globals::preferredBackend != globals::Backend::None)
        TryInitBackend(globals::preferredBackend);
    else
        TryInitializeFrom(detected);
}

static std::atomic<bool> g_BackendInitialized = false;
static std::atomic<bool> g_BackendWatcherRunning = false;

static DWORD WINAPI BackendWatcherThread(LPVOID)
{
    //Log("[BackendWatcher] Started\n");

    g_BackendWatcherRunning = true;

    while (g_running && !g_BackendInitialized)
    {
        // Preferred backend override
        if (globals::preferredBackend != globals::Backend::None)
        {
            if (TryInitBackend(globals::preferredBackend))
            {
                g_BackendInitialized = true;
                break;
            }
        }
        else
        {
            // Auto-detect order (DX12 preferred)
            if (GetModuleHandleA("d3d12.dll") || GetModuleHandleA("dxgi.dll"))
            {
                if (TryInitBackend(globals::Backend::DX12))
                {
                    g_BackendInitialized = true;
                    break;
                }
            }
        }

        Sleep(100); // Low overhead, no busy loop
    }

    //Log("[BackendWatcher] Exiting\n");
    g_BackendWatcherRunning = false;
    return 0;
}

static DWORD WINAPI onAttach(LPVOID)
{
    //Log("[DllMain] onAttach starting\n");

    if (MH_Initialize() != MH_OK)
    {
        Log("[DllMain] MinHook init failed\n");
        return 1;
    }

    // Start backend watcher thread (Hooks)
    HANDLE hThread = CreateThread(nullptr,0,BackendWatcherThread,nullptr,0,nullptr);
    if (hThread)
        CloseHandle(hThread);

    // START OVERLAY
    CreateThread(nullptr, 0, OverlayThread, nullptr, 0, nullptr);

    //Log("[DllMain] onAttach finished\n");
    return 0;
}

void ReleaseActiveBackend()
{
    switch (globals::activeBackend)
    {
    case globals::Backend::DX12:
        d3d12hook::release();
        break;
    default:
        break;
    }

    globals::activeBackend = globals::Backend::None;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        globals::mainModule = hModule;
        //Log("[DllMain] DLL_PROCESS_ATTACH: hModule=%p\n", hModule);
        // hook setup
        {
            HANDLE thread = CreateThread(nullptr, 0,onAttach,nullptr,0,nullptr);
            if (thread) CloseHandle(thread);
            else Log("[DllMain] Failed to create hook thread: %d\n", GetLastError());
        }
        break;

    case DLL_PROCESS_DETACH:
        g_running = false; // Signal all threads to stop
        g_BackendInitialized = true; // Break the watcher loop

        //Log("[DllMain] DLL_PROCESS_DETACH. Cleaning up.\n");

        Sleep(100); // Give threads a moment to finish (don't use WaitForSingleObject in DllMain, it causes deadlocks)
        
        // Unhook everything first to prevent hooks from calling into a dying DLL
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        CleanupColorBuffer();

        // Release DX resources
        ReleaseActiveBackend();

        // Final ImGui cleanup
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK NextHook(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

