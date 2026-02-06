//overlay.h by N7

//#include <windows.h>
//#include <d3d12.h>
//#include <dxgi1_4.h>
#include <dcomp.h>
#include <wrl.h>
#include <atomic>

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
bool menuisOpen = false;
int countstride1 = -1;
int countstride2 = -1;
int countstride3 = -1;
int countstride4 = -1;
int countExIStride1 = -1;
int countExIStride2 = -1;
int countExIStride3 = -1;
int countIndexCount = -1;
int countcurrentRootSigID = -1;
int countcurrentRootSigID2 = -1;
int countcurrentIndexAddress = -1;
int countcurrentIndexAddress2 = -1;
int countcurrentIndexAddress3 = -1;
int countcurrentPSOAddress = -1;
int countfindrendertarget = -1;

int countGraphicsRootConstantBuffer = -2;
int countGraphicsRootDescriptor = -2;
int countComputeRootDescriptor = -2;

int countindexformat = -1;
int countfilternumViews = -1;
int countfilternumViewports = -1;
int countignorenumViews = -1;
int countignorenumViewports = -1;
int countfilterrendertarget = -1;
int countignorerendertarget = -1;

int countfilterGraphicsRootConstantBuffer = -2;
int countfilterGraphicsRootConstantBuffer2 = -2;
int countfilterGraphicsRootConstantBuffer3 = -2;

int countignoreGraphicsRootConstantBuffer = -2;
int countignoreGraphicsRootConstantBuffer2 = -2;
int countignoreGraphicsRootConstantBuffer3 = -2;

int countfilterGraphicsRootDescriptor = -2;
int countfilterGraphicsRootDescriptor2 = -2;
int countfilterGraphicsRootDescriptor3 = -2;

int countignoreGraphicsRootDescriptor = -2;
int countignoreGraphicsRootDescriptor2 = -2;
int countignoreGraphicsRootDescriptor3 = -2;

//int countfilterComputeRootDescriptor = -2;
//int countignoreComputeRootDescriptor = -2;

bool temporaryids = false;
bool filternumViews = false;
bool filternumViewports = false;
bool ignorenumViews = false;
bool ignorenumViewports = false;
bool filterrendertarget = false;
bool ignorerendertarget = false;

bool filterGraphicsRootConstantBuffer = false;
bool ignoreGraphicsRootConstantBuffer = false;
bool filterGraphicsRootDescriptor = false;
bool ignoreGraphicsRootDescriptor = false;
bool filterComputeRootDescriptor = false;
bool ignoreComputeRootDescriptor = false;

bool reversedDepth = false;
bool DisableOcclusionCulling = true;

using namespace std;
#include <string>
#include <fstream>
void SaveConfig()
{
    ofstream fout;
    fout.open("d3d12wh.ini", ios::trunc);
    fout << "countstride1 " << countstride1 << endl;
    fout << "countstride2 " << countstride2 << endl;
    fout << "countstride3 " << countstride3 << endl;
    fout << "countstride4 " << countstride4 << endl;
    fout << "ExIStride1 " << countExIStride1 << endl;
    fout << "ExIStride2 " << countExIStride2 << endl;
    fout << "ExIStride3 " << countExIStride3 << endl;
    fout << "countGraphicsRootConstantBuffer " << countGraphicsRootConstantBuffer << endl;
    fout << "countGraphicsRootDescriptor " << countGraphicsRootDescriptor << endl;
    //fout << "countComputeRootDescriptor " << countComputeRootDescriptor << endl;
    fout << "countIndexCount " << countIndexCount << endl;
    fout << "countfindrendertarget " << countfindrendertarget << endl;
    //fout << "countindexformat " << countindexformat << endl;

    fout << "temporaryids " << temporaryids << endl;
    fout << "countcurrentRootSigID " << countcurrentRootSigID << endl;
    fout << "countcurrentRootSigID2 " << countcurrentRootSigID2 << endl;  
    fout << "countcurrentIndexAddress " << countcurrentIndexAddress << endl;
    fout << "countcurrentIndexAddress2 " << countcurrentIndexAddress2 << endl;
    fout << "countcurrentIndexAddress3 " << countcurrentIndexAddress3 << endl;
    fout << "countcurrentPSOAddress " << countcurrentPSOAddress << endl;

    fout << "filterrendertarget " << filterrendertarget << endl;
    fout << "countfilterrendertarget " << countfilterrendertarget << endl;

    fout << "ignorerendertarget " << ignorerendertarget << endl;
    fout << "countignorerendertarget " << countignorerendertarget << endl;

    fout << "filternumViews " << filternumViews << endl;
    fout << "countfilternumViews " << countfilternumViews << endl;

    fout << "filternumViewports " << filternumViewports << endl;
    fout << "countfilternumViewports " << countfilternumViewports << endl;

    fout << "ignorenumViews " << ignorenumViews << endl;
    fout << "countignorenumViews " << countignorenumViews << endl;

    fout << "ignorenumViewports " << ignorenumViewports << endl;
    fout << "countignorenumViewports " << countignorenumViewports << endl;

    fout << "filterGraphicsRootConstantBuffer " << filterGraphicsRootConstantBuffer << endl;
    fout << "countfilterGraphicsRootConstantBuffer " << countfilterGraphicsRootConstantBuffer << endl;
    fout << "countfilterGraphicsRootConstantBuffer2 " << countfilterGraphicsRootConstantBuffer2 << endl;
    fout << "countfilterGraphicsRootConstantBuffer3 " << countfilterGraphicsRootConstantBuffer3 << endl;

    fout << "ignoreGraphicsRootConstantBuffer " << ignoreGraphicsRootConstantBuffer << endl;
    fout << "countignoreGraphicsRootConstantBuffer " << countignoreGraphicsRootConstantBuffer << endl;
    fout << "countignoreGraphicsRootConstantBuffer2 " << countignoreGraphicsRootConstantBuffer2 << endl;
    fout << "countignoreGraphicsRootConstantBuffer3 " << countignoreGraphicsRootConstantBuffer3 << endl;

    fout << "filterGraphicsRootDescriptor " << filterGraphicsRootDescriptor << endl;
    fout << "countfilterGraphicsRootDescriptor " << countfilterGraphicsRootDescriptor << endl;
    fout << "countfilterGraphicsRootDescriptor2 " << countfilterGraphicsRootDescriptor2 << endl;
    fout << "countfilterGraphicsRootDescriptor3 " << countfilterGraphicsRootDescriptor3 << endl;

    fout << "ignoreGraphicsRootDescriptor " << ignoreGraphicsRootDescriptor << endl;
    fout << "countignoreGraphicsRootDescriptor " << countignoreGraphicsRootDescriptor << endl;
    fout << "countignoreGraphicsRootDescriptor2 " << countignoreGraphicsRootDescriptor2 << endl;
    fout << "countignoreGraphicsRootDescriptor3 " << countignoreGraphicsRootDescriptor3 << endl;
   
    //fout << "filterComputeRootDescriptor " << filterComputeRootDescriptor << endl;
    //fout << "countfilterComputeRootDescriptor " << countfilterComputeRootDescriptor << endl;

    //fout << "ignoreComputeRootDescriptor " << ignoreComputeRootDescriptor << endl;
    //fout << "countignoreComputeRootDescriptor " << countignoreComputeRootDescriptor << endl;
    
    fout << "reversedDepth " << reversedDepth << endl;
    fout << "DisableOcclusionCulling " << DisableOcclusionCulling << endl;
    fout.close();
}

void LoadConfig()
{
    ifstream fin;
    string Word = "";
    fin.open("d3d12wh.ini", ifstream::in);
    fin >> Word >> countstride1;
    fin >> Word >> countstride2;
    fin >> Word >> countstride3;
    fin >> Word >> countstride4;
    fin >> Word >> countExIStride1;
    fin >> Word >> countExIStride2;
    fin >> Word >> countExIStride3;
    fin >> Word >> countGraphicsRootConstantBuffer;
    fin >> Word >> countGraphicsRootDescriptor;
    //fin >> Word >> countComputeRootDescriptor;
    fin >> Word >> countIndexCount;
    fin >> Word >> countfindrendertarget;
    //fin >> Word >> countindexformat;

    fin >> Word >> temporaryids;
    fin >> Word >> countcurrentRootSigID;
    fin >> Word >> countcurrentRootSigID2;
    fin >> Word >> countcurrentIndexAddress;
    fin >> Word >> countcurrentIndexAddress2;
    fin >> Word >> countcurrentIndexAddress3;
    fin >> Word >> countcurrentPSOAddress;

    fin >> Word >> filterrendertarget;
    fin >> Word >> countfilterrendertarget;

    fin >> Word >> ignorerendertarget;
    fin >> Word >> countignorerendertarget;

    fin >> Word >> filternumViews;
    fin >> Word >> countfilternumViews;

    fin >> Word >> filternumViewports;
    fin >> Word >> countfilternumViewports;

    fin >> Word >> ignorenumViews;
    fin >> Word >> countignorenumViews;

    fin >> Word >> ignorenumViewports;
    fin >> Word >> countignorenumViewports;

    fin >> Word >> filterGraphicsRootConstantBuffer;
    fin >> Word >> countfilterGraphicsRootConstantBuffer;
    fin >> Word >> countfilterGraphicsRootConstantBuffer2;
    fin >> Word >> countfilterGraphicsRootConstantBuffer3;

    fin >> Word >> ignoreGraphicsRootConstantBuffer;
    fin >> Word >> countignoreGraphicsRootConstantBuffer;
    fin >> Word >> countignoreGraphicsRootConstantBuffer2;
    fin >> Word >> countignoreGraphicsRootConstantBuffer3;

    fin >> Word >> filterGraphicsRootDescriptor;
    fin >> Word >> countfilterGraphicsRootDescriptor;
    fin >> Word >> countfilterGraphicsRootDescriptor2;
    fin >> Word >> countfilterGraphicsRootDescriptor3;

    fin >> Word >> ignoreGraphicsRootDescriptor;
    fin >> Word >> countignoreGraphicsRootDescriptor;
    fin >> Word >> countignoreGraphicsRootDescriptor2;
    fin >> Word >> countignoreGraphicsRootDescriptor3;

    //fin >> Word >> filterComputeRootDescriptor;
    //fin >> Word >> countfilterComputeRootDescriptor;

    //fin >> Word >> ignoreComputeRootDescriptor;
    //fin >> Word >> countignoreComputeRootDescriptor;

    fin >> Word >> reversedDepth;
    fin >> Word >> DisableOcclusionCulling;
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

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
HWND CreateOverlayWindow()
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"OverlayClass";
    RegisterClassEx(&wc);

    DWORD exStyles =
        WS_EX_LAYERED |      // Required for transparency
        WS_EX_TOOLWINDOW |   // No taskbar icon
        WS_EX_TOPMOST;       // Always on top

    HWND hwnd = CreateWindowEx(
        exStyles,
        wc.lpszClassName,
        L"Overlay",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, wc.hInstance, nullptr
    );

    // IMPORTANT: Do NOT use LWA_ALPHA or LWA_COLORKEY for D3D12
    // Just keep WS_EX_LAYERED and let D3D12 render transparency

    // Remove problematic Win11 flag if present
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    style &= ~WS_EX_NOREDIRECTIONBITMAP;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);

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

    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        SaveConfig();
        g_showMenu = !g_showMenu;
        g_clickThrough = !g_showMenu;

        // Always re-read the current style BEFORE modifying it
        LONG_PTR exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);

        if (g_showMenu)
        {
            // REMOVE TRANSPARENT + NOACTIVATE
            exStyle &= ~WS_EX_TRANSPARENT;
            exStyle &= ~WS_EX_NOACTIVATE;

            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle);

            // Force Windows 11 to re-evaluate the window
            SetWindowPos(
                g_overlayHwnd, HWND_TOPMOST,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED
            );

            // Give overlay focus so ImGui receives input
            SetForegroundWindow(g_overlayHwnd);
            SetActiveWindow(g_overlayHwnd);
            SetFocus(g_overlayHwnd);
        }
        else
        {
            // Re-read style again to avoid stale flags
            exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);

            // ADD TRANSPARENT + NOACTIVATE
            exStyle |= WS_EX_TRANSPARENT;
            exStyle |= WS_EX_NOACTIVATE;

            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle);

            // Return focus to the game
            SetForegroundWindow(g_gameHwnd);
        }
    }

    /*
    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        SaveConfig(); //Save settings
        g_showMenu = !g_showMenu;
        g_clickThrough = !g_showMenu;

        LONG_PTR exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);

        if (g_showMenu)
        {
            // REMOVE NOACTIVATE and TRANSPARENT
            SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT & ~WS_EX_NOACTIVATE);

            // CRITICAL: Force Windows to re-evaluate the window regions
            SetWindowPos(g_overlayHwnd, HWND_TOPMOST, 0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);

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
    */
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
        ImGui::Text("Find Permanent Values");
        ImGui::SliderInt("Stridehash1", &countstride1, minus_val, max_val);
        ImGui::SliderInt("Stridehash2", &countstride2, minus_val, max_val);
        ImGui::SliderInt("Stridehash3", &countstride3, minus_val, max_val);
        ImGui::SliderInt("Stridehash4", &countstride4, minus_val, max_val);
        ImGui::SliderInt("ExIStride1", &countExIStride1, minus_val, max_val);
        ImGui::SliderInt("ExIStride2", &countExIStride2, minus_val, max_val);
        ImGui::SliderInt("ExIStride3", &countExIStride3, minus_val, max_val);
        ImGui::SliderInt("GraphicsRootConstantBuffer", &countGraphicsRootConstantBuffer, minus_val2, max_val);
        ImGui::SliderInt("GraphicsRootDescriptor", &countGraphicsRootDescriptor, minus_val2, max_val);
        ImGui::SliderInt("ComputeRootDescriptor", &countComputeRootDescriptor, minus_val2, max_val);
        ImGui::SliderInt("IndexCount*1000", &countIndexCount, minus_val, max_val);
        ImGui::SliderInt("RenderTarget", &countfindrendertarget, minus_val, max_val);
        //ImGui::SliderInt("IndexFormat+Vp", &countindexformat, minus_val, max_val);

        //temporary IDs below (changes after game restart)
        ImGui::Checkbox("Find Temporary Values", &temporaryids);
        if (temporaryids)
        {
            ImGui::SliderInt("CurrentRootSigID1", &countcurrentRootSigID, minus_val, max_val);
            ImGui::SliderInt("CurrentRootSigID2", &countcurrentRootSigID2, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress1", &countcurrentIndexAddress, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress2", &countcurrentIndexAddress2, minus_val, max_val);
            ImGui::SliderInt("CurrentIndexAddress3", &countcurrentIndexAddress3, minus_val, max_val);
            ImGui::SliderInt("CurrentPSOAddress", &countcurrentPSOAddress, minus_val, max_val);
        }

        //Filter/Ignore
        ImGui::Checkbox("Filter RenderTarget", &filterrendertarget);
        if (filterrendertarget)
        {
            ImGui::SliderInt("Render Target", &countfilterrendertarget, minus_val, max_val);
        }

        ImGui::Checkbox("Ignore RenderTarget", &ignorerendertarget);
        if (ignorerendertarget)
        {
            ImGui::SliderInt("RenderTarget ", &countignorerendertarget, minus_val, max_val);
        }

        ImGui::Checkbox("Filter NumViews", &filternumViews);
        if (filternumViews)
        {
            ImGui::SliderInt("NumViews", &countfilternumViews, minus_val, max_val);
        }

        ImGui::Checkbox("Ignore NumViews", &ignorenumViews);
        if (ignorenumViews)
        {
            ImGui::SliderInt("NumViews ", &countignorenumViews, minus_val, max_val);
        }

        ImGui::Checkbox("Filter NumViewports", &filternumViewports);
        if (filternumViewports)
        {
            ImGui::SliderInt("NumViewports", &countfilternumViewports, minus_val, max_val);
        }

        ImGui::Checkbox("Ignore NumViewports", &ignorenumViewports);
        if (ignorenumViewports)
        {
            ImGui::SliderInt("NumViewports ", &countignorenumViewports, minus_val, max_val);
        }

        ImGui::Checkbox("Filter GraphicsRootDescriptor", &filterGraphicsRootDescriptor);
        if (filterGraphicsRootDescriptor)
        {
            ImGui::SliderInt("GraphicsRootDescriptor1", &countfilterGraphicsRootDescriptor, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootDescriptor2", &countfilterGraphicsRootDescriptor2, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootDescriptor3", &countfilterGraphicsRootDescriptor3, minus_val2, max_val);
        }

        ImGui::Checkbox("Filter GraphicsRootConstantBuffer", &filterGraphicsRootConstantBuffer);
        if (filterGraphicsRootConstantBuffer)
        {
            ImGui::SliderInt("GraphicsRootConstantBuffer1", &countfilterGraphicsRootConstantBuffer, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootConstantBuffer2", &countfilterGraphicsRootConstantBuffer2, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootConstantBuffer3", &countfilterGraphicsRootConstantBuffer3, minus_val2, max_val);
        }

        //ImGui::Checkbox("Filter ComputeRootDescriptor", &filterComputeRootDescriptor);
        //if (filterComputeRootDescriptor)
        //{
            //ImGui::SliderInt("Compute RootDescriptor", &countfilterComputeRootDescriptor, minus_val2, max_val);
        //}


        ImGui::Checkbox("Ignore GraphicsRootDescriptor", &ignoreGraphicsRootDescriptor);
        if (ignoreGraphicsRootDescriptor)
        {
            ImGui::SliderInt("GraphicsRootDescriptor 1", &countignoreGraphicsRootDescriptor, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootDescriptor 2", &countignoreGraphicsRootDescriptor2, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootDescriptor 3", &countignoreGraphicsRootDescriptor3, minus_val2, max_val);
        }

        ImGui::Checkbox("Ignore GraphicsRootConstantBuffer", &ignoreGraphicsRootConstantBuffer);
        if (ignoreGraphicsRootConstantBuffer)
        {
            ImGui::SliderInt("GraphicsRootConstantBuffer 1", &countignoreGraphicsRootConstantBuffer, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootConstantBuffer 2", &countignoreGraphicsRootConstantBuffer2, minus_val2, max_val);
            ImGui::SliderInt("GraphicsRootConstantBuffer 3", &countignoreGraphicsRootConstantBuffer3, minus_val2, max_val);
        }

        //ImGui::Checkbox("Ignore ComputeRootDescriptor", &ignoreComputeRootDescriptor);
        //if (ignoreComputeRootDescriptor)
        //{
            //ImGui::SliderInt("Compute RootDescriptor", &countignoreComputeRootDescriptor, minus_val2, max_val);
        //}

        ImGui::Checkbox("Reverse Depth", &reversedDepth);
        ImGui::Checkbox("Try to Disable Occlusion", &DisableOcclusionCulling);
        //ImGui::Checkbox("Color", &colors);

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
    Log("[Cleanup] Starting Graceful Shutdown...\n");

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
    // Wait for game window
    while (g_running && !(g_gameHwnd = GetForegroundWindow()))
    //while (g_running && !(g_gameHwnd = FindWindowA("UnrealWindow", nullptr))) //<---------------------------------------------------------
    {
        Sleep(500);
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
            Log("[DllMain] Attempting DX12 initialization.\n");
            hooks::InitH();
            if (WaitForInitialization(d3d12hook::IsInitialized))
            {
                Log("[DllMain] DX12 initialization succeeded.\n");
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
    Log("[BackendWatcher] Started\n");

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

    Log("[BackendWatcher] Exiting\n");
    g_BackendWatcherRunning = false;
    return 0;
}

static DWORD WINAPI onAttach(LPVOID)
{
    Log("[DllMain] onAttach starting\n");

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

    Log("[DllMain] onAttach finished\n");
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
        Log("[DllMain] DLL_PROCESS_ATTACH: hModule=%p\n", hModule);
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

        Log("[DllMain] DLL_PROCESS_DETACH. Cleaning up.\n");

        Sleep(100); // Give threads a moment to finish (don't use WaitForSingleObject in DllMain, it causes deadlocks)
        
        // Unhook everything first to prevent hooks from calling into a dying DLL
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

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