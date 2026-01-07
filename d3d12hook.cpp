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
    ResetFn oResetD3D12 = nullptr;
    //if unresolved external symbol d3d12hook::function, fix here


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

        // The specific fence value that must be reached 
    // before this specific back-buffer can be reused.
        UINT64 FenceValue;
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
        // 1. Handle Input Toggles
        if (GetAsyncKeyState(globals::openMenuKey) & 1) {
            menuisOpen = !menuisOpen;
        }

        if (GetAsyncKeyState(globals::uninjectKey) & 1) {
            return oPresentD3D12(pSwapChain, SyncInterval, Flags);
        }

        gAfterFirstPresent = true;

        // 2. CommandQueue Safety Check
        if (!gCommandQueue) {
            if (!gDevice) {
                pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice);
            }
            return oPresentD3D12(pSwapChain, SyncInterval, Flags);
        }

        // 3. One-Time Initialization
        if (!gInitialized) {
            Log("[d3d12hook] Initializing ImGui on first Present.\n");
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice))) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gBufferCount = desc.BufferCount;

            // Create Descriptor Heaps
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = gBufferCount;
            gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapRTV));

            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapSRV));

            // Allocate frame contexts
            gFrameContexts = new FrameContext[gBufferCount];
            UINT rtvSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();

            for (UINT i = 0; i < gBufferCount; ++i) {
                gDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gFrameContexts[i].allocator));
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&gFrameContexts[i].renderTarget));

                gFrameContexts[i].rtvHandle = rtvHandle;
                gDevice->CreateRenderTargetView(gFrameContexts[i].renderTarget, nullptr, rtvHandle);
                rtvHandle.ptr += rtvSize;

                gFrameContexts[i].FenceValue = 0; // Initialize fence value
            }

            // Initialize Fences
            gDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gOverlayFence));
            gOverlayFenceValue = 0;

            // ImGui Setup
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Re-enabled

            // Add these to prevent ImGui from fighting the game for the mouse/focus
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            io.IniFilename = nullptr; // Prevents disk I/O freezes on NewFrame

            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX12_Init(gDevice, gBufferCount, desc.BufferDesc.Format, gHeapSRV,
                gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
                gHeapSRV->GetGPUDescriptorHandleForHeapStart());

            inputhook::Init(desc.OutputWindow);
            gInitialized = true;
        }

        // 4. Synchronization and Rendering (The Fix)
        if (!gShutdown) {
            UINT frameIdx = pSwapChain->GetCurrentBackBufferIndex();
            FrameContext& ctx = gFrameContexts[frameIdx];

            // NON-BLOCKING CHECK: Check if the GPU has finished the command list associated 
            // with this specific back-buffer index.
            if (gOverlayFence->GetCompletedValue() < ctx.FenceValue) {
                // GPU is still busy with the UI for this specific buffer.
                // Skip overlay rendering this frame to avoid deadlocking the engine.
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Start New Frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            //if (menuisOpen) menuInit();
            if (menuisOpen) {
                // Only process navigation if the game window is actually active
                if (GetForegroundWindow() != globals::mainWindow) {
                    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
                }
                else {
                    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                }

                menuInit();
            }

            // Prepare Command List
            ctx.allocator->Reset();
            if (!gCommandList) {
                gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx.allocator, nullptr, IID_PPV_ARGS(&gCommandList));
            }
            else {
                gCommandList->Reset(ctx.allocator, nullptr);
            }

            // Transition: PRESENT -> RENDER_TARGET
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = ctx.renderTarget;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            gCommandList->ResourceBarrier(1, &barrier);

            // Render
            gCommandList->OMSetRenderTargets(1, &ctx.rtvHandle, FALSE, nullptr);
            ID3D12DescriptorHeap* heaps[] = { gHeapSRV };
            gCommandList->SetDescriptorHeaps(1, heaps);

            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

            // Transition: RENDER_TARGET -> PRESENT
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            gCommandList->ResourceBarrier(1, &barrier);

            gCommandList->Close();

            // 5. Execute and Signal
            oExecuteCommandListsD3D12(gCommandQueue, 1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));

            // Update the fence for this specific frame context
            gOverlayFenceValue++;
            ctx.FenceValue = gOverlayFenceValue;
            gCommandQueue->Signal(gOverlayFence, gOverlayFenceValue);
        }

        return oPresentD3D12(pSwapChain, SyncInterval, Flags);
    }

    //=========================================================================================================================//

    HRESULT WINAPI hookPresent1D3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams) {
        if (GetAsyncKeyState(globals::openMenuKey) & 1) {
            menuisOpen = !menuisOpen;
        }

        if (GetAsyncKeyState(globals::uninjectKey) & 1) {
            return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
        }

        gAfterFirstPresent = true;

        // 1. Initial Capture Check
        if (!gCommandQueue) {
            if (!gDevice) {
                pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice);
            }
            return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
        }

        // 2. Initialization (Shared with Present)
        if (!gInitialized) {
            Log("[d3d12hook] Initializing ImGui on first Present.\n");
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice))) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gBufferCount = desc.BufferCount;

            // Create Descriptor Heaps
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = gBufferCount;
            gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapRTV));

            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapSRV));

            // Allocate frame contexts
            gFrameContexts = new FrameContext[gBufferCount];
            UINT rtvSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();

            for (UINT i = 0; i < gBufferCount; ++i) {
                gDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gFrameContexts[i].allocator));
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&gFrameContexts[i].renderTarget));

                gFrameContexts[i].rtvHandle = rtvHandle;
                gDevice->CreateRenderTargetView(gFrameContexts[i].renderTarget, nullptr, rtvHandle);
                rtvHandle.ptr += rtvSize;

                gFrameContexts[i].FenceValue = 0; // Initialize fence value
            }

            // Initialize Fences
            gDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gOverlayFence));
            gOverlayFenceValue = 0;

            // ImGui Setup
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Re-enabled

            // Add these to prevent ImGui from fighting the game for the mouse/focus
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            io.IniFilename = nullptr; // Prevents disk I/O freezes on NewFrame

            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX12_Init(gDevice, gBufferCount, desc.BufferDesc.Format, gHeapSRV,
                gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
                gHeapSRV->GetGPUDescriptorHandleForHeapStart());

            inputhook::Init(desc.OutputWindow);
            gInitialized = true;
        }

        // 4. Synchronization and Rendering (The Fix)
        if (!gShutdown) {
            UINT frameIdx = pSwapChain->GetCurrentBackBufferIndex();
            FrameContext& ctx = gFrameContexts[frameIdx];

            // NON-BLOCKING CHECK: Check if the GPU has finished the command list associated 
            // with this specific back-buffer index.
            if (gOverlayFence->GetCompletedValue() < ctx.FenceValue) {
                // GPU is still busy with the UI for this specific buffer.
                // Skip overlay rendering this frame to avoid deadlocking the engine.
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Start New Frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            //if (menuisOpen) menuInit();
            if (menuisOpen) {
                // Only process navigation if the game window is actually active
                if (GetForegroundWindow() != globals::mainWindow) {
                    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
                }
                else {
                    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                }

                menuInit();
            }

            // Prepare Command List
            ctx.allocator->Reset();
            if (!gCommandList) {
                gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx.allocator, nullptr, IID_PPV_ARGS(&gCommandList));
            }
            else {
                gCommandList->Reset(ctx.allocator, nullptr);
            }

            // Transition: PRESENT -> RENDER_TARGET
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = ctx.renderTarget;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            gCommandList->ResourceBarrier(1, &barrier);

            // Render
            gCommandList->OMSetRenderTargets(1, &ctx.rtvHandle, FALSE, nullptr);
            ID3D12DescriptorHeap* heaps[] = { gHeapSRV };
            gCommandList->SetDescriptorHeaps(1, heaps);

            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

            // Transition: RENDER_TARGET -> PRESENT
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            gCommandList->ResourceBarrier(1, &barrier);

            gCommandList->Close();

            // 5. Execute and Signal
            oExecuteCommandListsD3D12(gCommandQueue, 1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));

            // Update the fence for this specific frame context
            gOverlayFenceValue++;
            ctx.FenceValue = gOverlayFenceValue;
            gCommandQueue->Signal(gOverlayFence, gOverlayFenceValue);
        }

        return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(
        ID3D12CommandQueue* _this,
        UINT                          NumCommandLists,
        ID3D12CommandList* const* ppCommandLists) {

        if (!_this) return oExecuteCommandListsD3D12(_this, NumCommandLists, ppCommandLists);

        // 1. Only attempt capture if we don't have a queue yet
        if (!gCommandQueue) {
            D3D12_COMMAND_QUEUE_DESC desc = _this->GetDesc();

            // 2. Must be a DIRECT queue (Graphics)
            if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {

                // 3. Optional: Verify it matches our device
                ID3D12Device* queueDevice = nullptr;
                if (SUCCEEDED(_this->GetDevice(__uuidof(ID3D12Device), (void**)&queueDevice))) {

                    // If we already have a device from Present, make sure they match
                    if (gDevice == nullptr || queueDevice == gDevice) {
                        _this->AddRef();
                        gCommandQueue = _this;
                        Log("[d3d12hook] Captured Correct DIRECT CommandQueue: %p\n", _this);
                    }

                    queueDevice->Release();
                }
            }
        }

        return oExecuteCommandListsD3D12(_this, NumCommandLists, ppCommandLists);
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

    void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        // 1. SAFETY CHECK
        // Skip if list is null or if it's a COMPUTE/COPY queue (RSSetViewports could crash these)
        if (!_this || _this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT) {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // 2. DATA CAPTURE
        UINT currentStrides = t_.StrideHash + t_.StartSlot;
        uint32_t currentRootSigID = (tlsCurrentCmdList == _this) ? tlsCurrentRootSigID : 0;

        // 3. IDENTIFICATION
        bool isModelDraw = (currentStrides == countstride1 || currentStrides == countstride2 || currentStrides == countstride3 || currentStrides == countstride4 ||
            currentRootSigID == countcurrentRootSigID || currentRootSigID == countcurrentRootSigID2);

        if (isModelDraw) {
            bool applyHack = true;

            // IGNORE/FILTER LOGIC
            if (ignoreRootDescriptor) {
                if (tls_cache.lastRDIndex == countignorerootDescriptor ||
                    tls_cache.lastRDIndex == countignorerootDescriptor2 ||
                    tls_cache.lastRDIndex == countignorerootDescriptor3) {
                    applyHack = false;
                }
            }

            if (applyHack && ignoreRootConstant) {
                if (tls_cache.lastCbvIndex == countignorerootConstant ||
                    tls_cache.lastCbvIndex == countignorerootConstant2 ||
                    tls_cache.lastCbvIndex == countignorerootConstant3) {
                    applyHack = false;
                }
            }

            // FILTER RULES
            if (applyHack && filterRootDescriptor) {
                if (tls_cache.lastRDIndex != countfilterrootDescriptor &&
                    tls_cache.lastRDIndex != countfilterrootDescriptor2 &&
                    tls_cache.lastRDIndex != countfilterrootDescriptor3) {
                    applyHack = false;
                }
            }

            if (applyHack && filterRootConstant) {
                if (tls_cache.lastCbvIndex != countfilterrootConstant &&
                    tls_cache.lastCbvIndex != countfilterrootConstant2 &&
                    tls_cache.lastCbvIndex != countfilterrootConstant3) {
                    applyHack = false;
                }
            }

            if (applyHack) {
                D3D12_VIEWPORT originalVp = t_.currentViewport;

                // 4. VIEWPORT VALIDATION
                // Only apply if the viewport looks like a valid screen-space viewport
                if (originalVp.Width > 0 && originalVp.Width < 16384 && originalVp.Height > 0) {
                    D3D12_VIEWPORT hVp = originalVp;
                    hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
                    hVp.MaxDepth = reversedDepth ? 0.01f : 1.0f;

                    _this->RSSetViewports(1, &hVp);
                    oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
                    _this->RSSetViewports(1, &originalVp);

                    return; // SUCCESSFUL: EXIT EARLY
                }
            }
        }

        oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
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
        // 1. SAFETY CHECK
        // Skip if list is null or if it's a COMPUTE/COPY queue (RSSetViewports could crash these)
        if (!dCommandList || dCommandList->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT) {
            return oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount,
                pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

        // 2. DATA CAPTURE
        UINT currentStrides = t_.StrideHash + t_.StartSlot;
        uint32_t currentRootSigID = (tlsCurrentCmdList == dCommandList) ? tlsCurrentRootSigID : 0;

        // 3. IDENTIFICATION
        bool isModelDraw = (currentStrides == countstride1 || currentStrides == countstride2 || currentStrides == countstride3 || currentStrides == countstride4 ||
            currentRootSigID == countcurrentRootSigID || currentRootSigID == countcurrentRootSigID2);

        if (isModelDraw) {
            bool applyHack = true;

            // IGNORE/FILTER LOGIC
            if (ignoreRootDescriptor) {
                if (tls_cache.lastRDIndex == countignorerootDescriptor ||
                    tls_cache.lastRDIndex == countignorerootDescriptor2 ||
                    tls_cache.lastRDIndex == countignorerootDescriptor3) {
                    applyHack = false;
                }
            }

            if (applyHack && ignoreRootConstant) {
                if (tls_cache.lastCbvIndex == countignorerootConstant ||
                    tls_cache.lastCbvIndex == countignorerootConstant2 ||
                    tls_cache.lastCbvIndex == countignorerootConstant3) {
                    applyHack = false;
                }
            }

            // FILTER RULES
            if (applyHack && filterRootDescriptor) {
                if (tls_cache.lastRDIndex != countfilterrootDescriptor &&
                    tls_cache.lastRDIndex != countfilterrootDescriptor2 &&
                    tls_cache.lastRDIndex != countfilterrootDescriptor3) {
                    applyHack = false;
                }
            }

            if (applyHack && filterRootConstant) {
                if (tls_cache.lastCbvIndex != countfilterrootConstant &&
                    tls_cache.lastCbvIndex != countfilterrootConstant2 &&
                    tls_cache.lastCbvIndex != countfilterrootConstant3) {
                    applyHack = false;
                }
            }

            if (applyHack) {
                D3D12_VIEWPORT originalVp = t_.currentViewport;

                // 4. VIEWPORT VALIDATION
                // Only apply if the viewport looks like a valid screen-space viewport
                if (originalVp.Width > 0 && originalVp.Width < 16384 && originalVp.Height > 0) {
                    D3D12_VIEWPORT hVp = originalVp;
                    hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
                    hVp.MaxDepth = reversedDepth ? 0.01f : 1.0f;

                    dCommandList->RSSetViewports(1, &hVp);
                    oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
                    dCommandList->RSSetViewports(1, &originalVp);

                    return; // SUCCESSFUL: EXIT EARLY
                }
            }
        }

        // 5. FALLBACK
        // This handles cases where:
        // - It's not a model we want.
        // - The hack was ignored/filtered.
        // - The viewport was invalid (0x0).
        oExecuteIndirectD3D12(dCommandList, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
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

        // Synchronize cache if the engine switched command lists on this thread
        if (tls_cache.cmdListPtr != _this) {
            tls_cache.cmdListPtr = _this;
            // Optionally: Reset state if it's a new command list context
            tls_cache.lastCbvIndex = UINT_MAX;
        }

        tls_cache.lastCbvIndex = RootParameterIndex;

        return oSetGraphicsRootConstantBufferViewD3D12(_this, RootParameterIndex, BufferLocation);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {

        if(tls_cache.cmdListPtr != dCommandList) {
            tls_cache.cmdListPtr = dCommandList;

            tls_cache.lastRDIndex = UINT_MAX;

            if (BaseDescriptor.ptr != 0)
            t_.LastDescriptorBase = BaseDescriptor;
        }

        tls_cache.lastRDIndex = RootParameterIndex;

        return oSetGraphicsRootDescriptorTableD3D12(dCommandList, RootParameterIndex, BaseDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookResetD3D12(ID3D12GraphicsCommandList* _this, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState) {

        // Reset our cache for this specific command list context
        if (tls_cache.cmdListPtr == _this) {
            tls_cache.lastCbvIndex = UINT_MAX;
        }

        return oResetD3D12(_this, pAllocator, pInitialState);
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
    //If you want to make it compatible with more games, consider these improvements:
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
