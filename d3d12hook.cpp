//d3d12 hook/wallhack
#include "stdafx.h"
#include "d3dx12.h"
#include "menu.h"

namespace hooks { void Remove(); }

namespace d3d12hook {
    PresentD3D12            oPresentD3D12 = nullptr;
    Present1Fn              oPresent1D3D12 = nullptr;
    ExecuteCommandListsFn   oExecuteCommandListsD3D12 = nullptr;
    ResizeBuffersFn         oResizeBuffersD3D12 = nullptr;
    RSSetViewportsFn        oRSSetViewportsD3D12 = nullptr;
    IASetVertexBuffersFn    oIASetVertexBuffersD3D12 = nullptr;
    DrawIndexedInstancedFn  oDrawIndexedInstancedD3D12 = nullptr;
    SetGraphicsRootConstantBufferViewFn oSetGraphicsRootConstantBufferViewD3D12 = nullptr; //3
    SetDescriptorHeapsFn    oSetDescriptorHeapsD3D12 = nullptr;
    SetGraphicsRootDescriptorTableFn oSetGraphicsRootDescriptorTableD3D12 = nullptr;
    OMSetRenderTargetsFn oOMSetRenderTargetsD3D12 = nullptr;
    ResolveQueryDataFn oResolveQueryDataD3D12 = nullptr;
    ExecuteIndirectFn oExecuteIndirectD3D12 = nullptr;
    SetGraphicsRootSignatureFn oSetGraphicsRootSignatureD3D12 = nullptr;


    static ID3D12Device* gDevice = nullptr;
    static ID3D12CommandQueue* gCommandQueue = nullptr;
    static ID3D12DescriptorHeap* gHeapRTV = nullptr;
    static ID3D12DescriptorHeap* gHeapSRV = nullptr;
    static ID3D12GraphicsCommandList* gCommandList = nullptr;
    static ID3D12Fence* gOverlayFence = nullptr;
    static HANDLE                   gFenceEvent = nullptr;
    static UINT64                  gOverlayFenceValue = 0;
    static uintx_t                 gBufferCount = 0;

    struct FrameContext {
        ID3D12CommandAllocator* allocator;
        ID3D12Resource* renderTarget;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    };
    static FrameContext* gFrameContexts = nullptr;
    static bool                   gInitialized = false;
    static bool                   gShutdown = false;
    static bool                   gAfterFirstPresent = false;

    void release();

    // Utility to log HRESULTs
    inline void LogHRESULT(const char* label, HRESULT hr) {
        Log("[d3d12hook] %s: hr=0x%08X\n", label, hr);
    }

    //=========================================================================================================================//

    uint32_t TwoDigitStrideHash(const uint32_t* data, size_t count) {
        uint32_t hash = 2166136261u;
        for (size_t i = 0; i < count; ++i) {
            hash ^= data[i];
            hash *= 16777619u;
        }
        return hash % 100; // Two-digit number
    }

    //=========================================================================================================================//

    HRESULT WINAPI hookPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
        if (GetAsyncKeyState(globals::openMenuKey) & 1) {
            menuisOpen = !menuisOpen;
            //Log("[d3d12hook] Toggle menu: isOpen=%d\n", menuisOpen);
        }

        if (GetAsyncKeyState(globals::uninjectKey) & 1) {
            //Uninject();
            return oPresentD3D12(pSwapChain, SyncInterval, Flags);
        }

        gAfterFirstPresent = true;
        if (!gCommandQueue) {
            Log("[d3d12hook] CommandQueue not yet captured, skipping frame\n");
            if (!gDevice) {
                pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice);
            }
            return oPresentD3D12(pSwapChain, SyncInterval, Flags);
        }

        if (!gInitialized) {
            Log("[d3d12hook] Initializing ImGui on first Present.\n");
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice))) {
                LogHRESULT("GetDevice", E_FAIL);
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Swap Chain description
            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gBufferCount = desc.BufferCount;
            Log("[d3d12hook] BufferCount=%u\n", gBufferCount);

            // Create descriptor heaps
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = gBufferCount;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapRTV)))) {
                LogHRESULT("CreateDescriptorHeap RTV", E_FAIL);
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapSRV)))) {
                LogHRESULT("CreateDescriptorHeap SRV", E_FAIL);
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Allocate frame contexts
            gFrameContexts = new FrameContext[gBufferCount];
            ZeroMemory(gFrameContexts, sizeof(FrameContext) * gBufferCount);

            // Create command allocator for each frame
            for (UINT i = 0; i < gBufferCount; ++i) {
                if (FAILED(gDevice->CreateCommandAllocator(
                        D3D12_COMMAND_LIST_TYPE_DIRECT,
                        IID_PPV_ARGS(&gFrameContexts[i].allocator)))) {
                    LogHRESULT("CreateCommandAllocator", E_FAIL);
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
            }

            // Create RTVs for each back buffer
            UINT rtvSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < gBufferCount; ++i) {
                ID3D12Resource* back;
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&back));
                gDevice->CreateRenderTargetView(back, nullptr, rtvHandle);
                gFrameContexts[i].renderTarget = back;
                gFrameContexts[i].rtvHandle = rtvHandle;
                rtvHandle.ptr += rtvSize;
            }

            // ImGui setup
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX12_Init(gDevice, gBufferCount,
                desc.BufferDesc.Format,
                gHeapSRV,
                gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
                gHeapSRV->GetGPUDescriptorHandleForHeapStart());
            Log("[d3d12hook] ImGui initialized.\n");

            inputhook::Init(desc.OutputWindow);

            if (!gOverlayFence) {
                if (FAILED(gDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gOverlayFence)))) {
                    LogHRESULT("CreateFence", E_FAIL);
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
            }

            if (!gFenceEvent) {
                gFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (!gFenceEvent) {
                    Log("[d3d12hook] Failed to create fence event: %lu\n", GetLastError());
                }
            }

            // Hook CommandQueue and Fence are already captured by minhook
            gInitialized = true;
        }

        if (!gShutdown) {
            // Render ImGui
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (menuisOpen) menuInit();

            UINT frameIdx = pSwapChain->GetCurrentBackBufferIndex();
            FrameContext& ctx = gFrameContexts[frameIdx];

            // Wait for the GPU to finish with the previous frame
            bool canRender = true;
            if (!gOverlayFence || !gFenceEvent) {
                // Missing synchronization objects, skip waiting
            } else if (gOverlayFence->GetCompletedValue() < gOverlayFenceValue) {
                HRESULT hr = gOverlayFence->SetEventOnCompletion(gOverlayFenceValue, gFenceEvent);
                if (SUCCEEDED(hr)) {
                    const DWORD waitTimeoutMs = 2000; // Extended timeout
                    DWORD waitRes = WaitForSingleObject(gFenceEvent, waitTimeoutMs);
                    if (waitRes == WAIT_TIMEOUT) {
                        Log("[d3d12hook] WaitForSingleObject timeout\n");
                        canRender = false;
                    } else if (waitRes != WAIT_OBJECT_0) {
                        Log("[d3d12hook] WaitForSingleObject failed: %lu\n", GetLastError());
                        canRender = false;
                    }
                } else {
                    LogHRESULT("SetEventOnCompletion", hr);
                    canRender = false;
                }
            }

            if (!canRender) {
                Log("[d3d12hook] Skipping ImGui render for this frame\n");
                ImGui::EndFrame();
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Reset allocator and command list using frame-specific allocator
            HRESULT hr = ctx.allocator->Reset();
            if (FAILED(hr)) {
                LogHRESULT("CommandAllocator->Reset", hr);
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            if (!gCommandList) {
                hr = gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    ctx.allocator, nullptr, IID_PPV_ARGS(&gCommandList));
                if (FAILED(hr)) {
                    LogHRESULT("CreateCommandList", hr);
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
                gCommandList->Close();
            }
            hr = gCommandList->Reset(ctx.allocator, nullptr);
            if (FAILED(hr)) {
                LogHRESULT("CommandList->Reset", hr);
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Transition to render target
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = ctx.renderTarget;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            gCommandList->ResourceBarrier(1, &barrier);

            gCommandList->OMSetRenderTargets(1, &ctx.rtvHandle, FALSE, nullptr);
            ID3D12DescriptorHeap* heaps[] = { gHeapSRV };
            gCommandList->SetDescriptorHeaps(1, heaps);

            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

            // Transition back to present
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            gCommandList->ResourceBarrier(1, &barrier);
            gCommandList->Close();

            // Execute
            if (!gCommandQueue) {
                Log("[d3d12hook] CommandQueue not set, skipping ExecuteCommandLists.\n");
            }
            else {
                oExecuteCommandListsD3D12(gCommandQueue, 1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));
                if (gOverlayFence) {
                    // Call Signal directly on the command queue to synchronize the internal overlay.
                    HRESULT hr = gCommandQueue->Signal(gOverlayFence, ++gOverlayFenceValue);
                    if (FAILED(hr)) {
                        LogHRESULT("Signal", hr);
                        if (gDevice) {
                            HRESULT reason = gDevice->GetDeviceRemovedReason();
                            Log("[d3d12hook] DeviceRemovedReason=0x%08X\n", reason);
                            if (reason != S_OK) {
                                Log("[d3d12hook] Device lost. Releasing resources.\n");
                                release();
                            }
                        }
                    }
                }
            }
        }

        return oPresentD3D12(pSwapChain, SyncInterval, Flags);
    }

    //=========================================================================================================================//

    HRESULT WINAPI hookPresent1D3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams) {
        if (GetAsyncKeyState(globals::openMenuKey) & 1) {
            menuisOpen = !menuisOpen;
            //Log("[d3d12hook] Toggle menu: isOpen=%d\n", menuisOpen);
        }

        if (GetAsyncKeyState(globals::uninjectKey) & 1) {
            //Uninject();
            return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
        }

        gAfterFirstPresent = true;
        if (!gCommandQueue) {
            Log("[d3d12hook] CommandQueue not yet captured, skipping frame\n");
            if (!gDevice) {
                pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice);
            }
            return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
        }

        if (!gInitialized) {
            Log("[d3d12hook] Initializing ImGui on first Present1.\n");
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice))) {
                LogHRESULT("GetDevice", E_FAIL);
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            // Swap Chain description
            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gBufferCount = desc.BufferCount;
            Log("[d3d12hook] BufferCount=%u\n", gBufferCount);

            // Create descriptor heaps
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = gBufferCount;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapRTV)))) {
                LogHRESULT("CreateDescriptorHeap RTV", E_FAIL);
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapSRV)))) {
                LogHRESULT("CreateDescriptorHeap SRV", E_FAIL);
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            // Allocate frame contexts
            gFrameContexts = new FrameContext[gBufferCount];
            ZeroMemory(gFrameContexts, sizeof(FrameContext) * gBufferCount);

            // Create command allocator for each frame
            for (UINT i = 0; i < gBufferCount; ++i) {
                if (FAILED(gDevice->CreateCommandAllocator(
                        D3D12_COMMAND_LIST_TYPE_DIRECT,
                        IID_PPV_ARGS(&gFrameContexts[i].allocator)))) {
                    LogHRESULT("CreateCommandAllocator", E_FAIL);
                    return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
                }
            }

            // Create RTVs for each back buffer
            UINT rtvSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < gBufferCount; ++i) {
                ID3D12Resource* back;
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&back));
                gDevice->CreateRenderTargetView(back, nullptr, rtvHandle);
                gFrameContexts[i].renderTarget = back;
                gFrameContexts[i].rtvHandle = rtvHandle;
                rtvHandle.ptr += rtvSize;
            }

            // ImGui setup
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX12_Init(gDevice, gBufferCount,
                desc.BufferDesc.Format,
                gHeapSRV,
                gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
                gHeapSRV->GetGPUDescriptorHandleForHeapStart());
            Log("[d3d12hook] ImGui initialized.\n");

            inputhook::Init(desc.OutputWindow);

            if (!gOverlayFence) {
                if (FAILED(gDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gOverlayFence)))) {
                    LogHRESULT("CreateFence", E_FAIL);
                    return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
                }
            }

            if (!gFenceEvent) {
                gFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (!gFenceEvent) {
                    Log("[d3d12hook] Failed to create fence event: %lu\n", GetLastError());
                }
            }

            // Hook CommandQueue and Fence are already captured by minhook
            gInitialized = true;
        }

        if (!gShutdown) {
            // Render ImGui
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (menuisOpen) menuInit();

            UINT frameIdx = pSwapChain->GetCurrentBackBufferIndex();
            FrameContext& ctx = gFrameContexts[frameIdx];

            // Wait for the GPU to finish with the previous frame
            bool canRender = true;
            if (!gOverlayFence || !gFenceEvent) {
                // Missing synchronization objects, skip waiting
            } else if (gOverlayFence->GetCompletedValue() < gOverlayFenceValue) {
                HRESULT hr = gOverlayFence->SetEventOnCompletion(gOverlayFenceValue, gFenceEvent);
                if (SUCCEEDED(hr)) {
                    const DWORD waitTimeoutMs = 2000; // Extended timeout
                    DWORD waitRes = WaitForSingleObject(gFenceEvent, waitTimeoutMs);
                    if (waitRes == WAIT_TIMEOUT) {
                        Log("[d3d12hook] WaitForSingleObject timeout\n");
                        canRender = false;
                    } else if (waitRes != WAIT_OBJECT_0) {
                        Log("[d3d12hook] WaitForSingleObject failed: %lu\n", GetLastError());
                        canRender = false;
                    }
                } else {
                    LogHRESULT("SetEventOnCompletion", hr);
                    canRender = false;
                }
            }

            if (!canRender) {
                Log("[d3d12hook] Skipping ImGui render for this frame\n");
                ImGui::EndFrame();
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            // Reset allocator and command list using frame-specific allocator
            HRESULT hr = ctx.allocator->Reset();
            if (FAILED(hr)) {
                LogHRESULT("CommandAllocator->Reset", hr);
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            if (!gCommandList) {
                hr = gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    ctx.allocator, nullptr, IID_PPV_ARGS(&gCommandList));
                if (FAILED(hr)) {
                    LogHRESULT("CreateCommandList", hr);
                    return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
                }
                gCommandList->Close();
            }
            hr = gCommandList->Reset(ctx.allocator, nullptr);
            if (FAILED(hr)) {
                LogHRESULT("CommandList->Reset", hr);
                return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
            }

            // Transition to render target
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = ctx.renderTarget;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            gCommandList->ResourceBarrier(1, &barrier);

            gCommandList->OMSetRenderTargets(1, &ctx.rtvHandle, FALSE, nullptr);
            ID3D12DescriptorHeap* heaps[] = { gHeapSRV };
            gCommandList->SetDescriptorHeaps(1, heaps);

            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

            // Transition back to present
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            gCommandList->ResourceBarrier(1, &barrier);
            gCommandList->Close();

            // Execute
            if (!gCommandQueue) {
                Log("[d3d12hook] CommandQueue not set, skipping ExecuteCommandLists.\n");
            }
            else {
                oExecuteCommandListsD3D12(gCommandQueue, 1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));
                if (gOverlayFence) {
                    // Call Signal directly on the command queue to synchronize the internal overlay.
                    HRESULT hr = gCommandQueue->Signal(gOverlayFence, ++gOverlayFenceValue);
                    if (FAILED(hr)) {
                        LogHRESULT("Signal", hr);
                        if (gDevice) {
                            HRESULT reason = gDevice->GetDeviceRemovedReason();
                            Log("[d3d12hook] DeviceRemovedReason=0x%08X\n", reason);
                            if (reason != S_OK) {
                                Log("[d3d12hook] Device lost. Releasing resources.\n");
                                release();
                            }
                        }
                    }
                }
            }
        }

        return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(
        ID3D12CommandQueue* _this,
        UINT                          NumCommandLists,
        ID3D12CommandList* const* ppCommandLists) {

        // 1. Safety check
        if (!_this || !ppCommandLists)
            return oExecuteCommandListsD3D12(_this, NumCommandLists, ppCommandLists);


        if (!gCommandQueue && gAfterFirstPresent) {
            ID3D12Device* queueDevice = nullptr;
            if (SUCCEEDED(_this->GetDevice(__uuidof(ID3D12Device), (void**)&queueDevice))) {
                if (!gDevice) {
                    gDevice = queueDevice;
                }

                if (queueDevice == gDevice) {
                    D3D12_COMMAND_QUEUE_DESC desc = _this->GetDesc();
                    Log("[d3d12hook] CommandQueue type=%u\n", desc.Type);
                    if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
                        _this->AddRef();
                        gCommandQueue = _this;
                        Log("[d3d12hook] Captured CommandQueue=%p\n", _this);

                        //color
                        //if (!g_Initialized &&
                          //  gDevice &&
                            //g_GameSRVHeap && // Ensure heap was captured
                            //t_.LastDescriptorBase.ptr != 0 &&
                            //InterlockedCompareExchange(&g_InitLock, 1, 0) == 0)
                        //{
                          //  InitializeColorTextures(gDevice, _this);
                            //g_InitLock = 0;
                        //}
                    }
                    else {
                        Log("[d3d12hook] Skipping capture: non-direct queue\n");
                    }
                }
                else {
                    Log("[d3d12hook] Skipping capture: CommandQueue uses different device (%p != %p)\n", queueDevice, gDevice);
                }

                if (queueDevice && queueDevice != gDevice)
                    queueDevice->Release();
            }
        }
        gAfterFirstPresent = false;
        oExecuteCommandListsD3D12(_this, NumCommandLists, ppCommandLists);
    }

    //=========================================================================================================================//

    HRESULT STDMETHODCALLTYPE hookResizeBuffersD3D12(
        IDXGISwapChain3* pSwapChain,
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags)
    {
        Log("[d3d12hook] ResizeBuffers called: %ux%u Buffers=%u\n",
            Width, Height, BufferCount);

        if (gInitialized)
        {
            Log("[d3d12hook] Releasing resources for resize\n");

            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            inputhook::Remove(globals::mainWindow);

            if (gCommandList)
            {
                gCommandList->Release();
                gCommandList = nullptr;
            }
            if (gHeapRTV)
            {
                gHeapRTV->Release();
                gHeapRTV = nullptr;
            }
            if (gHeapSRV)
            {
                gHeapSRV->Release();
                gHeapSRV = nullptr;
            }

            for (UINT i = 0; i < gBufferCount; ++i)
            {
                if (gFrameContexts[i].renderTarget)
                {
                    gFrameContexts[i].renderTarget->Release();
                    gFrameContexts[i].renderTarget = nullptr;
                }
                if (gFrameContexts[i].allocator)
                {
                    gFrameContexts[i].allocator->Release();
                    gFrameContexts[i].allocator = nullptr;
                }
            }

            delete[] gFrameContexts;
            gFrameContexts = nullptr;
            gBufferCount = 0;

            gInitialized = false;
        }

        return oResizeBuffersD3D12(
            pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookRSSetViewportsD3D12(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports) {

        if (NumViewports > 0 && pViewports) {
            // Cache the game's intended viewport
            t_.currentViewport = pViewports[0];
        }

        oRSSetViewportsD3D12(_this, NumViewports, pViewports);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookIASetVertexBuffersD3D12(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {

        // Reset stride and size data
        for (int i = 0; i < 16; ++i) {
            t_.Strides[i] = 0;
            t_.vertexBufferSizes[i] = 0;
        }

        t_.numViewports = NumViews;
        t_.StartSlot = StartSlot;
        uint32_t strideData[16] = {};

        if (NumViews > 0 && pViews) {
            for (UINT i = 0; i < NumViews && i < 16; ++i) {
                // Check for valid stride
                if (pViews[i].StrideInBytes <= 999) {
                    t_.Strides[i] = pViews[i].StrideInBytes;
                    strideData[i] = pViews[i].StrideInBytes;
                }
            }

            // hash strides
            t_.StrideHash = TwoDigitStrideHash(strideData, NumViews);
        }

        return oIASetVertexBuffersD3D12(_this, StartSlot, NumViews, pViews);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookOMSetRenderTargetsD3D12(
        ID3D12GraphicsCommandList* dCommandList,
        UINT NumRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        BOOL RTsSingleHandleToDescriptorRange,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
    {
        t_.currentNumRTVs = NumRenderTargetDescriptors;
        //if (t_.currentNumRTVs > 0 && t_.currentViewport.Width > 512) filter for coloring

        return oOMSetRenderTargetsD3D12(dCommandList, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
        
        UINT Strides = t_.StrideHash + t_.StartSlot; //will be the same after restarting the game

        uint32_t currentRootSigID = 0; //this number will change after restarting the game

        if (tlsCurrentCmdList == _this) {
            currentRootSigID = tlsCurrentRootSigID;
        }

        //wallhack
        if (Strides == countstride1 || Strides == countstride2 || Strides == countstride3 || currentRootSigID == countcurrentRootSigID) { //models
            // Save original viewport (do this only once per frame if possible, but safe here)
            D3D12_VIEWPORT originalVp = t_.currentViewport;

            // Set hacked viewport: full size, very near depth slice
            D3D12_VIEWPORT hVp = originalVp;
            
            hVp.MinDepth = 0.9f;
            hVp.MaxDepth = 1.0f;
            
            //few games use reversed depth 
            if(reversedDepth)
            {
            hVp.MinDepth = 0.0f;
            hVp.MaxDepth = 0.01f;
            }

            _this->RSSetViewports(1, &hVp);

            oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

            // Restore FULL original viewport
            _this->RSSetViewports(1, &originalVp);
        }
        else {
            // Normal path — no extra state changes
            oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

    }

    //=========================================================================================================================//
   
    void STDMETHODCALLTYPE hookExecuteIndirectD3D12(
        ID3D12GraphicsCommandList* dCommandList,
        ID3D12CommandSignature* pCommandSignature,
        UINT MaxCommandCount,
        ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,
        ID3D12Resource* pCountBuffer,
        UINT64 CountBufferOffset)
    {
        // At top of hook — safety sanity checks
        if (MaxCommandCount == 0 || MaxCommandCount > 200000) {  // insane count = driver bug or detection
            return oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount,
                pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

        UINT Strides = t_.StrideHash + t_.StartSlot;

        //wallhack
        if (Strides == countstride1 || Strides == countstride2 || Strides == countstride3) { //models
            // Save original viewport (do this only once per frame if possible, but safe here)
            D3D12_VIEWPORT originalVp = t_.currentViewport;

            // Set hacked viewport: full size, very near depth slice
            D3D12_VIEWPORT hVp = originalVp;
            hVp.MinDepth = 0.9f;
            hVp.MaxDepth = 1.0f;

            //few games use reversed depth 
            if (reversedDepth)
            {
                hVp.MinDepth = 0.0f;
                hVp.MaxDepth = 0.01f;
            }

            dCommandList->RSSetViewports(1, &hVp);

            oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount,pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

            // Restore FULL original viewport
            dCommandList->RSSetViewports(1, &originalVp);
        }
        else {
            // Normal path — no extra state changes
            oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount,pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootSignatureD3D12(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature) {
        
        if (!dCommandList || !pRootSignature) return oSetGraphicsRootSignatureD3D12(dCommandList, pRootSignature);

        if (dCommandList && pRootSignature) {
            uint32_t idToStore = 0;

            // 1. Try to find the ID with a shared lock (multiple threads can do this at once)
            {
                std::shared_lock<std::shared_mutex> lock(rootSigMutex);
                auto it = rootSigToID.find(pRootSignature);
                if (it != rootSigToID.end()) {
                    idToStore = it->second;
                }
            }

            // 2. If not found, use an exclusive lock to register the new signature
            if (idToStore == 0) {
                std::unique_lock<std::shared_mutex> lock(rootSigMutex);
                // Re-check inside the lock to handle race conditions
                if (rootSigToID.find(pRootSignature) == rootSigToID.end()) {
                    idToStore = nextRuntimeSigID++;
                    rootSigToID[pRootSignature] = idToStore;
                }
                else {
                    idToStore = rootSigToID[pRootSignature];
                }
            }

            // 3. Store the state in TLS - NO GLOBAL MAP REQUIRED
            tlsCurrentCmdList = dCommandList;
            tlsCurrentRootSigID = idToStore;
        }

        return oSetGraphicsRootSignatureD3D12(dCommandList, pRootSignature);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootConstantBufferViewD3D12(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

        t_.lastCbvRootParameterIndex = RootParameterIndex;

        return oSetGraphicsRootConstantBufferViewD3D12(_this, RootParameterIndex, BufferLocation);
    }

    //=========================================================================================================================//
    
    void STDMETHODCALLTYPE hookSetDescriptorHeapsD3D12(ID3D12GraphicsCommandList* cmdList, UINT NumHeaps, ID3D12DescriptorHeap* const* ppHeaps)
    {
        for (UINT i = 0; i < NumHeaps; i++) {
            D3D12_DESCRIPTOR_HEAP_DESC desc = ppHeaps[i]->GetDesc();
            if (desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            {
                g_GameSRVHeap = ppHeaps[i];
            }
            //if (desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
                //g_GameSamplerHeap = ppHeaps[i];
        }
        
        oSetDescriptorHeapsD3D12(cmdList, NumHeaps, ppHeaps);
    }
    
    //=========================================================================================================================//
    
    void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {

        if (BaseDescriptor.ptr != 0)
        {
            t_.lastCbvRootParameterIndex2 = RootParameterIndex;
            t_.LastDescriptorBase = BaseDescriptor;
        }
        
        return oSetGraphicsRootDescriptorTableD3D12(dCommandList, RootParameterIndex, BaseDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookResolveQueryDataD3D12(
        ID3D12GraphicsCommandList* self,
        ID3D12QueryHeap* pQueryHeap,
        D3D12_QUERY_TYPE Type,
        UINT StartIndex,
        UINT NumQueries,
        ID3D12Resource* pDestinationBuffer,
        UINT64 AlignedDestinationBufferOffset) {
       
        if (Type == D3D12_QUERY_TYPE_OCCLUSION || Type == D3D12_QUERY_TYPE_BINARY_OCCLUSION) {
            // We use a GPU command to write '1' directly into the destination buffer.
            // This avoids the CPU-GPU sync bottleneck.

            for (UINT i = 0; i < NumQueries; ++i) {
                D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param;
                param.Dest = pDestinationBuffer->GetGPUVirtualAddress() + AlignedDestinationBufferOffset + (i * sizeof(UINT64));
                param.Value = 1; // 1 = Visible

                // Note: Requires ID3D12GraphicsCommandList2 or higher
                ID3D12GraphicsCommandList2* cl2 = nullptr;
                if (SUCCEEDED(self->QueryInterface(IID_PPV_ARGS(&cl2)))) {
                    cl2->WriteBufferImmediate(1, &param, nullptr);
                    cl2->Release();
                }
            }
            return; // Skip original
        }
        oResolveQueryDataD3D12(self, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
    }
  
    //ResolveQueryData Improvements for Stability
    //If you want to keep your approach but make it compatible with more games, consider these improvements:
    //Check for Binary Occlusion: Some games expect 0 or 1 (Binary), while others expect the actual number of samples passed (Occlusion). Writing a very large number like 0xFFFF can sometimes be more effective for standard occlusion to ensure the game treats it as "fully visible."
    //Handle Resource Barriers: If you use GPU commands to overwrite the data, you must ensure the pDestinationBuffer is in the D3D12_RESOURCE_STATE_COPY_DEST or COMMON state.
    //Performance: WriteBufferImmediate is excellent, but if NumQueries is large (e.g., 500+), it’s better to have a small "source" buffer containing many 1s and use CopyBufferRegion.
    
    //=========================================================================================================================//

    void release() {
        Log("[d3d12hook] Releasing resources and hooks.\n");
        gShutdown = true;
        if (globals::mainWindow) {
            inputhook::Remove(globals::mainWindow);
        }

        // Shutdown ImGui before releasing any D3D resources
        if (gInitialized && ImGui::GetCurrentContext())
        {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            gInitialized = false;
        }

        if (gCommandList) gCommandList->Release();
        if (gHeapRTV) gHeapRTV->Release();
        if (gHeapSRV) gHeapSRV->Release();

        for (UINT i = 0; i < gBufferCount; ++i) {
            if (gFrameContexts[i].renderTarget) gFrameContexts[i].renderTarget->Release();
        }
        if (gOverlayFence)
        {
            gOverlayFence->Release();
            gOverlayFence = nullptr;
        }

        if (gFenceEvent)
        {
            CloseHandle(gFenceEvent);
            gFenceEvent = nullptr;
        }

        if (gCommandQueue)
        {
            gCommandQueue->Release();
            gCommandQueue = nullptr;
        }

        if (gDevice) gDevice->Release();
        delete[] gFrameContexts;

        // Disable hooks installed for D3D12
        hooks::Remove();
    }

    bool IsInitialized()
    {
        return gInitialized;
    }
}
