#pragma once

#include <fstream>
inline void Log(const char* fmt, ...) {
    char text[4096] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);

    std::ofstream logfile("log.txt", std::ios::app);
    if (logfile.is_open()) {
        logfile << text << std::endl;
    }
}

//=========================================================================================================================//

namespace hooks {
    extern void InitH();
}

namespace d3d12hook {
    typedef HRESULT(STDMETHODCALLTYPE* PresentD3D12)(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
    extern PresentD3D12 oPresentD3D12;

    typedef HRESULT(STDMETHODCALLTYPE* Present1Fn)(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams);
    extern Present1Fn   oPresent1D3D12;

    typedef void(STDMETHODCALLTYPE* ExecuteCommandListsFn)(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
    extern ExecuteCommandListsFn oExecuteCommandListsD3D12;

    typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffersFn)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    extern ResizeBuffersFn oResizeBuffersD3D12;

    typedef void (STDMETHODCALLTYPE* RSSetViewportsFn)(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
    extern RSSetViewportsFn oRSSetViewportsD3D12;

    typedef void(STDMETHODCALLTYPE* IASetVertexBuffersFn)(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
    extern IASetVertexBuffersFn oIASetVertexBuffersD3D12;

    typedef void(STDMETHODCALLTYPE* DrawIndexedInstancedFn)(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
    extern DrawIndexedInstancedFn oDrawIndexedInstancedD3D12;

    typedef void(STDMETHODCALLTYPE* DrawInstancedFn)(ID3D12GraphicsCommandList* _this, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
    extern DrawInstancedFn oDrawInstancedD3D12;

    typedef void(STDMETHODCALLTYPE* DispatchFn)(ID3D12GraphicsCommandList* _this, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
    extern DispatchFn oDispatchD3D12;
    
    typedef void(STDMETHODCALLTYPE* SetGraphicsRootConstantBufferViewFn)(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    extern SetGraphicsRootConstantBufferViewFn oSetGraphicsRootConstantBufferViewD3D12;

    typedef void(STDMETHODCALLTYPE* SetDescriptorHeapsFn)(ID3D12GraphicsCommandList* cmdList, UINT NumHeaps, ID3D12DescriptorHeap* const* ppHeaps);
    extern SetDescriptorHeapsFn oSetDescriptorHeapsD3D12;

    typedef void(STDMETHODCALLTYPE* SetGraphicsRootDescriptorTableFn)(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    extern SetGraphicsRootDescriptorTableFn oSetGraphicsRootDescriptorTableD3D12;

    typedef void(STDMETHODCALLTYPE* OMSetRenderTargetsFn)(ID3D12GraphicsCommandList* dCommandList, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
    extern OMSetRenderTargetsFn oOMSetRenderTargetsD3D12;

    typedef void(STDMETHODCALLTYPE* SetGraphicsRootSignatureFn)(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature);
    extern SetGraphicsRootSignatureFn oSetGraphicsRootSignatureD3D12;

    typedef void(STDMETHODCALLTYPE* ResetFn)(ID3D12GraphicsCommandList* _this, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState);
    extern ResetFn oResetD3D12;

    typedef void(STDMETHODCALLTYPE* CloseFn)(ID3D12GraphicsCommandList* cl);
    extern CloseFn oCloseD3D12;

    typedef void(STDMETHODCALLTYPE* IASetIndexBufferFn)(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView);
    extern IASetIndexBufferFn oIASetIndexBufferD3D12;

    typedef void(STDMETHODCALLTYPE* ResolveQueryDataFn)(
        ID3D12GraphicsCommandList* self,
        ID3D12QueryHeap* pQueryHeap,
        D3D12_QUERY_TYPE Type,
        UINT StartIndex,
        UINT NumQueries,
        ID3D12Resource* pDestinationBuffer,
        UINT64 AlignedDestinationBufferOffset);
    extern ResolveQueryDataFn oResolveQueryDataD3D12;

    typedef void(STDMETHODCALLTYPE* ExecuteIndirectFn)(
        ID3D12GraphicsCommandList* dCommandList,
        ID3D12CommandSignature* pCommandSignature,
        UINT MaxCommandCount,
        ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,
        ID3D12Resource* pCountBuffer,
        UINT64 CountBufferOffset);
    extern ExecuteIndirectFn oExecuteIndirectD3D12;

    // ID3D12GraphicsCommandList6
    typedef void(STDMETHODCALLTYPE* DispatchMeshFn)(ID3D12GraphicsCommandList6* cmd, UINT ThreadGroupCountX,UINT ThreadGroupCountY,UINT ThreadGroupCountZ);
    extern DispatchMeshFn oDispatchMeshD3D12;


    extern long __fastcall hookPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
    extern long __fastcall hookPresent1D3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams);
    extern void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
    extern HRESULT STDMETHODCALLTYPE hookResizeBuffersD3D12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    extern void STDMETHODCALLTYPE hookRSSetViewportsD3D12(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
    extern void STDMETHODCALLTYPE hookIASetVertexBuffersD3D12(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
    extern void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
    extern void STDMETHODCALLTYPE hookDrawInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
    extern void STDMETHODCALLTYPE hookDispatchD3D12(ID3D12GraphicsCommandList* _this, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
    extern void STDMETHODCALLTYPE hookSetGraphicsRootConstantBufferViewD3D12(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    extern void STDMETHODCALLTYPE hookSetDescriptorHeapsD3D12(ID3D12GraphicsCommandList* cmdList, UINT NumHeaps, ID3D12DescriptorHeap* const* ppHeaps);
    extern void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    extern void STDMETHODCALLTYPE hookOMSetRenderTargetsD3D12(ID3D12GraphicsCommandList* dCommandList, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
    extern void STDMETHODCALLTYPE hookResolveQueryDataD3D12(ID3D12GraphicsCommandList* self, ID3D12QueryHeap* pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource* pDestinationBuffer, UINT64 AlignedDestinationBufferOffset);
    extern void STDMETHODCALLTYPE hookSetGraphicsRootSignatureD3D12(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature);
    extern void STDMETHODCALLTYPE hookResetD3D12(ID3D12GraphicsCommandList* _this, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState);
    extern void STDMETHODCALLTYPE hookCloseD3D12(ID3D12GraphicsCommandList* cl);
    extern void STDMETHODCALLTYPE hookIASetIndexBufferD3D12(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView);
    extern void STDMETHODCALLTYPE hookDispatchMeshD3D12(ID3D12GraphicsCommandList6* cmd,UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

    extern void STDMETHODCALLTYPE hookExecuteIndirectD3D12(
        ID3D12GraphicsCommandList* dCommandList,
        ID3D12CommandSignature* pCommandSignature,
        UINT MaxCommandCount,
        ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,
        ID3D12Resource* pCountBuffer,
        UINT64 CountBufferOffset);

    extern void release();
    bool IsInitialized();
}

//=========================================================================================================================//

using Microsoft::WRL::ComPtr;

namespace hooks {
    // VTable indices derived from the official DirectX interface order.
    // These values are stable across Windows versions and SDKs.
    //constexpr size_t kPresentIndex = 8;            // IDXGISwapChain::Present
    //constexpr size_t kPresent1Index = 22;           // IDXGISwapChain1::Present1
    //constexpr size_t kResizeBuffersIndex = 13;     // IDXGISwapChain::ResizeBuffers
    //constexpr size_t kExecuteCommandListsIndex = 10; // ID3D12CommandQueue::ExecuteCommandLists
    constexpr size_t kRSSetViewportsIndex = 21; //ID3D12GraphicsCommandList::RSSetViewports
    constexpr size_t kIASetVertexBuffersIndex = 44; //ID3D12GraphicsCommandList::IASetVertexBuffersIndex
    constexpr size_t kDrawIndexedInstancedIndex = 13;
    constexpr size_t kDrawInstancedIndex = 12;
    constexpr size_t kDispatchIndex = 14;
    constexpr size_t kSetGraphicsRootConstantBufferViewIndex = 38;
    constexpr size_t kSetDescriptorHeapsIndex = 28;
    constexpr size_t kSetGraphicsRootDescriptorTableIndex = 32;
    constexpr size_t kOMSetRenderTargetsIndex = 46;
    constexpr size_t kResolveQueryDataIndex = 54;
    constexpr size_t kExecuteIndirectIndex = 59;
    constexpr size_t kSetGraphicsRootSignatureIndex = 30;
    constexpr size_t kResetIndex = 10;
    constexpr size_t kCloseIndex = 9;
    constexpr size_t kIASetIndexBufferIndex = 43;
    constexpr size_t kDispatchMeshIndex = 79; // stable across recent SDKs (AI claim)


    // Dummy objects pour extraire les v-tables
    static ComPtr<IDXGISwapChain3>       pSwapChain = nullptr;
    static ComPtr<ID3D12Device>          pDevice = nullptr;
    static ComPtr<ID3D12CommandQueue>    pCommandQueue = nullptr;

    static ComPtr<ID3D12CommandAllocator> pCommandAllocator = nullptr;
    static ComPtr<ID3D12GraphicsCommandList> pCommandList = nullptr;

    static HWND                          hDummyWindow = nullptr;
    static const wchar_t* dummyClassName = L"DummyWndClass";

    //static LPVOID pPresentTarget = nullptr;
    //static LPVOID pPresent1Target = nullptr;
    //static LPVOID pResizeBuffersTarget = nullptr;
    //static LPVOID pExecuteCommandListsTarget = nullptr;
    static LPVOID pRSSetViewportsTarget = nullptr;
    static LPVOID pIASetVertexBuffersTarget = nullptr;
    static LPVOID pDrawIndexedInstancedTarget = nullptr;
    static LPVOID pDrawInstancedTarget = nullptr;
    static LPVOID pDispatchTarget = nullptr;
    static LPVOID pSetGraphicsRootConstantBufferViewTarget = nullptr;
    static LPVOID pSetDescriptorHeapsTarget = nullptr;
    static LPVOID pSetGraphicsRootDescriptorTableTarget = nullptr;
    static LPVOID pOMSetRenderTargetsTarget = nullptr;
    static LPVOID pResolveQueryDataTarget = nullptr;
    static LPVOID pExecuteIndirectTarget = nullptr;
    static LPVOID pSetGraphicsRootSignatureTarget = nullptr;
    static LPVOID pResetTarget = nullptr;
    static LPVOID pCloseTarget = nullptr;
    static LPVOID pIASetIndexBufferTarget = nullptr;
    static LPVOID pDispatchMeshTarget = nullptr;


    static void CleanupDummyObjects()
    {
        if (hDummyWindow)
        {
            DestroyWindow(hDummyWindow);
            hDummyWindow = nullptr;
        }

        UnregisterClassW(dummyClassName, GetModuleHandle(nullptr));

        pSwapChain.Reset();
        pDevice.Reset();
        pCommandQueue.Reset();
        pCommandAllocator.Reset();
        pCommandList.Reset();
    }

    // Create hidden Window + device + DX12 swapchain
    static HRESULT CreateDeviceAndSwapChain() {
        // 1. Register dummy window
        WNDCLASSEXW wc = {
            sizeof(WNDCLASSEXW),
            CS_CLASSDC,
            DefWindowProcW,
            0, 0,
            GetModuleHandleW(nullptr),
            nullptr, nullptr, nullptr, nullptr,
            dummyClassName,
            nullptr
        };
        if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            Log("[hooks] RegisterClassExW failed: %u\n", GetLastError());
            return E_FAIL;
        }

        // 2. Create hidden window
        hDummyWindow = CreateWindowExW(
            0, dummyClassName, L"Dummy",
            WS_OVERLAPPEDWINDOW,
            0, 0, 1, 1,
            nullptr, nullptr, wc.hInstance, nullptr
        );
        if (!hDummyWindow) {
            Log("[hooks] CreateWindowExW failed: %u\n", GetLastError());
            return E_FAIL;
        }

        // 3. Factory DXGI
        ComPtr<IDXGIFactory4> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr)) {
            Log("[hooks] CreateDXGIFactory1 failed: 0x%08X\n", hr);
            return hr;
        }

        // 4. Device D3D12
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)); //D3D_FEATURE_LEVEL_12_0
        if (FAILED(hr)) {
            Log("[hooks] D3D12CreateDevice failed: 0x%08X\n", hr);
            return hr;
        }

        // 5. Command Queue
        D3D12_COMMAND_QUEUE_DESC cqDesc = {};
        cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        hr = pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pCommandQueue));
        if (FAILED(hr)) {
            Log("[hooks] CreateCommandQueue failed: 0x%08X\n", hr);
            return hr;
        }

        // 6. Command Allocator 
        hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator));
        if (FAILED(hr)) {
            Log("[hooks] CreateCommandAllocator failed: 0x%08X\n", hr);
            return hr;
        }

        // 7. Command List
        hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList));
        if (FAILED(hr)) {
            Log("[hooks] CreateCommandList failed: 0x%08X\n", hr);
            return hr;
        }
        // REQUIRED
        pCommandList->Close();

        // 8. SwapChainDesc1
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.BufferCount = 2;
        scDesc.Width = 1;
        scDesc.Height = 1;
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain1;
        hr = factory->CreateSwapChainForHwnd(
            pCommandQueue.Get(),
            hDummyWindow,
            &scDesc,
            nullptr, nullptr,
            &swapChain1
        );
        if (FAILED(hr)) {
            Log("[hooks] CreateSwapChainForHwnd failed: 0x%08X\n", hr);
            return hr;
        }

        // 9. Query IDXGISwapChain3
        hr = swapChain1.As(&pSwapChain);
        if (FAILED(hr)) {
            Log("[hooks] QueryInterface IDXGISwapChain3 failed: 0x%08X\n", hr);
            return hr;
        }

        //Detect Nanite?
        //bool hasMeshShaders = false;
        //D3D12_FEATURE_DATA_D3D12_OPTIONS7 opt7{};
        //if (SUCCEEDED(pDevice->CheckFeatureSupport(
            //D3D12_FEATURE_D3D12_OPTIONS7,
            //&opt7,
            //sizeof(opt7))))
        //{
            //hasMeshShaders = opt7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
        //}
        //Log("hasMeshShaders == %d", hasMeshShaders);

        return S_OK;
    }

    void InitH() {
        Log("d3d12.dll loaded at %p", GetModuleHandleA("d3d12.dll"));
        Log("D3D12Core.dll loaded at %p", GetModuleHandleA("D3D12Core.dll"));

        //Sleep(4000);
        Log("[hooks] Init starting\n");
        
        struct CleanupGuard {
            ~CleanupGuard() { CleanupDummyObjects(); }
        } cleanup;

        if (FAILED(CreateDeviceAndSwapChain())) {
            Log("[hooks] Failed to create dummy device/swapchain.\n");
            return;
        }

        MH_STATUS mh;

        // --- Hook Present on SwapChain ---
        //auto scVTable = *reinterpret_cast<void***>(pSwapChain.Get());
        //pPresentTarget = reinterpret_cast<LPVOID>(scVTable[kPresentIndex]);
        //mh = MH_CreateHook(pPresentTarget, reinterpret_cast<LPVOID>(d3d12hook::hookPresentD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oPresentD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook Present failed: %s\n", MH_StatusToString(mh));

        //pPresent1Target = reinterpret_cast<LPVOID>(scVTable[kPresent1Index]);
        //mh = MH_CreateHook(pPresent1Target, reinterpret_cast<LPVOID>(d3d12hook::hookPresent1D3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oPresent1D3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook Present1 failed: %s\n", MH_StatusToString(mh));

        // --- Hook ResizeBuffers ---
        //pResizeBuffersTarget = reinterpret_cast<LPVOID>(scVTable[kResizeBuffersIndex]);
        //mh = MH_CreateHook(pResizeBuffersTarget,reinterpret_cast<LPVOID>(d3d12hook::hookResizeBuffersD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oResizeBuffersD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook ResizeBuffers failed: %s\n", MH_StatusToString(mh));

        // --- Hook ExecuteCommandLists ---
        //auto cqVTable = *reinterpret_cast<void***>(pCommandQueue.Get());
        //pExecuteCommandListsTarget = reinterpret_cast<LPVOID>(cqVTable[kExecuteCommandListsIndex]);
        //mh = MH_CreateHook(pExecuteCommandListsTarget,reinterpret_cast<LPVOID>(d3d12hook::hookExecuteCommandListsD3D12),reinterpret_cast<LPVOID*>(&d3d12hook::oExecuteCommandListsD3D12));
        //if (mh != MH_OK)Log("[hooks] MH_CreateHook ExecuteCommandLists failed: %s\n", MH_StatusToString(mh));

        if (!pCommandList) {
            Log("[hooks] CRITICAL: pCommandList is NULL!\n");
            return;
        }

        // CommandList base
        //auto slVTable = *reinterpret_cast<void***>(pCommandList.Get());//wrong
        ComPtr<ID3D12GraphicsCommandList> baseCL;
        HRESULT hr = pCommandList.As(&baseCL);
        if (FAILED(hr))
        {
            Log("Failed to QI ID3D12GraphicsCommandList\n");
            return;
        }
        auto slVTable = *reinterpret_cast<void***>(baseCL.Get());

        // Device base not needed for now
        //auto cdVTable = *reinterpret_cast<void***>(pDevice.Get()); //wrong
        //ComPtr<ID3D12Device> baseDevice;
        //hr = pDevice.As(&baseDevice);
        //if (FAILED(hr))
        //{
            //Log("Failed to QI Device\n");
            //return;
        //}
        //auto cdVTable = *reinterpret_cast<void***>(baseDevice.Get());

        pRSSetViewportsTarget = reinterpret_cast<LPVOID>(slVTable[kRSSetViewportsIndex]);
        mh = MH_CreateHook(pRSSetViewportsTarget, reinterpret_cast<LPVOID>(d3d12hook::hookRSSetViewportsD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oRSSetViewportsD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook RSSetViewports failed: %s\n", MH_StatusToString(mh));

        pIASetVertexBuffersTarget = reinterpret_cast<LPVOID>(slVTable[kIASetVertexBuffersIndex]);
        mh = MH_CreateHook(pIASetVertexBuffersTarget, reinterpret_cast<LPVOID>(d3d12hook::hookIASetVertexBuffersD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oIASetVertexBuffersD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook IASetVertexBuffers failed: %s\n", MH_StatusToString(mh));

        pDrawIndexedInstancedTarget = reinterpret_cast<LPVOID>(slVTable[kDrawIndexedInstancedIndex]);
        mh = MH_CreateHook(pDrawIndexedInstancedTarget, reinterpret_cast<LPVOID>(d3d12hook::hookDrawIndexedInstancedD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oDrawIndexedInstancedD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook DrawIndexedInstanced failed: %s\n", MH_StatusToString(mh));

        pDrawInstancedTarget = reinterpret_cast<LPVOID>(slVTable[kDrawInstancedIndex]);
        mh = MH_CreateHook(pDrawInstancedTarget, reinterpret_cast<LPVOID>(d3d12hook::hookDrawInstancedD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oDrawInstancedD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook DrawInstanced failed: %s\n", MH_StatusToString(mh));

        //pDispatchTarget = reinterpret_cast<LPVOID>(slVTable[kDispatchIndex]);
        //mh = MH_CreateHook(pDispatchTarget, reinterpret_cast<LPVOID>(d3d12hook::hookDispatchD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oDispatchD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook Dispatch failed: %s\n", MH_StatusToString(mh));

        pSetGraphicsRootConstantBufferViewTarget = reinterpret_cast<LPVOID>(slVTable[kSetGraphicsRootConstantBufferViewIndex]);
        mh = MH_CreateHook(pSetGraphicsRootConstantBufferViewTarget, reinterpret_cast<LPVOID>(d3d12hook::hookSetGraphicsRootConstantBufferViewD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oSetGraphicsRootConstantBufferViewD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook SetGraphicsRootConstantBufferView failed: %s\n", MH_StatusToString(mh));

        //pSetDescriptorHeapsTarget = reinterpret_cast<LPVOID>(slVTable[kSetDescriptorHeapsIndex]);
        //mh = MH_CreateHook(pSetDescriptorHeapsTarget, reinterpret_cast<LPVOID>(d3d12hook::hookSetDescriptorHeapsD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oSetDescriptorHeapsD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook SetDescriptorHeaps failed: %s\n", MH_StatusToString(mh));

        pSetGraphicsRootDescriptorTableTarget = reinterpret_cast<LPVOID>(slVTable[kSetGraphicsRootDescriptorTableIndex]);
        mh = MH_CreateHook(pSetGraphicsRootDescriptorTableTarget, reinterpret_cast<LPVOID>(d3d12hook::hookSetGraphicsRootDescriptorTableD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oSetGraphicsRootDescriptorTableD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook SetGraphicsRootDescriptorTable failed: %s\n", MH_StatusToString(mh));

        pOMSetRenderTargetsTarget = reinterpret_cast<LPVOID>(slVTable[kOMSetRenderTargetsIndex]);
        mh = MH_CreateHook(pOMSetRenderTargetsTarget, reinterpret_cast<LPVOID>(d3d12hook::hookOMSetRenderTargetsD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oOMSetRenderTargetsD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook OMSetRenderTargets failed: %s\n", MH_StatusToString(mh));

        pResolveQueryDataTarget = reinterpret_cast<LPVOID>(slVTable[kResolveQueryDataIndex]);
        mh = MH_CreateHook(pResolveQueryDataTarget, reinterpret_cast<LPVOID>(d3d12hook::hookResolveQueryDataD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oResolveQueryDataD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook ResolveQueryData failed: %s\n", MH_StatusToString(mh));

        pExecuteIndirectTarget = reinterpret_cast<LPVOID>(slVTable[kExecuteIndirectIndex]);
        mh = MH_CreateHook(pExecuteIndirectTarget, reinterpret_cast<LPVOID>(d3d12hook::hookExecuteIndirectD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oExecuteIndirectD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook ExecuteIndirect failed: %s\n", MH_StatusToString(mh));

        pSetGraphicsRootSignatureTarget = reinterpret_cast<LPVOID>(slVTable[kSetGraphicsRootSignatureIndex]);
        mh = MH_CreateHook(pSetGraphicsRootSignatureTarget, reinterpret_cast<LPVOID>(d3d12hook::hookSetGraphicsRootSignatureD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oSetGraphicsRootSignatureD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook SetGraphicsRootSignature failed: %s\n", MH_StatusToString(mh));

        pResetTarget = reinterpret_cast<LPVOID>(slVTable[kResetIndex]);
        mh = MH_CreateHook(pResetTarget, reinterpret_cast<LPVOID>(d3d12hook::hookResetD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oResetD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook Reset failed: %s\n", MH_StatusToString(mh));

        //pCloseTarget = reinterpret_cast<LPVOID>(slVTable[kCloseIndex]);
        //mh = MH_CreateHook(pCloseTarget, reinterpret_cast<LPVOID>(d3d12hook::hookCloseD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oCloseD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook Close failed: %s\n", MH_StatusToString(mh));

        pIASetIndexBufferTarget = reinterpret_cast<LPVOID>(slVTable[kIASetIndexBufferIndex]);
        mh = MH_CreateHook(pIASetIndexBufferTarget, reinterpret_cast<LPVOID>(d3d12hook::hookIASetIndexBufferD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oIASetIndexBufferD3D12));
        if (mh != MH_OK) Log("[hooks] MH_CreateHook IASetIndexBuffer failed: %s\n", MH_StatusToString(mh));  

        //cmdlist 6 table
        //ComPtr<ID3D12GraphicsCommandList6> cl6;
        //hr = pCommandList.As(&cl6);
        //if (FAILED(hr))
        //{
            //Log("[hooks] ID3D12GraphicsCommandList6 not supported\n");
            //return;
        //}
        //auto cl6VTable = *reinterpret_cast<void***>(cl6.Get());
        //pDispatchMeshTarget = reinterpret_cast<LPVOID>(cl6VTable[kDispatchMeshIndex]);
        //mh = MH_CreateHook(pDispatchMeshTarget, reinterpret_cast<LPVOID>(d3d12hook::hookDispatchMeshD3D12), reinterpret_cast<LPVOID*>(&d3d12hook::oDispatchMeshD3D12));
        //if (mh != MH_OK) Log("[hooks] MH_CreateHook DispatchMesh failed: %s\n", MH_StatusToString(mh));


        // --- Enable all hooks ---
        mh = MH_EnableHook(MH_ALL_HOOKS);
        if (mh != MH_OK)Log("[hooks] MH_EnableHook failed: %s\n", MH_StatusToString(mh));
        else
        {
            Log("[hooks] Hooks enabled.");
        }
    }

    void Remove()
    {
        Log("[hooks] Preparing to remove hooks.");
        auto DisableAndRemove = [](LPVOID& target)
            {
                if (target)
                {
                    MH_DisableHook(target);
                    MH_RemoveHook(target);
                    target = nullptr;
                }
            };

        // SwapChain
        //DisableAndRemove(pPresentTarget);
        //DisableAndRemove(pPresent1Target);
        //DisableAndRemove(pResizeBuffersTarget);

        // CommandQueue
        //DisableAndRemove(pExecuteCommandListsTarget);

        // CommandList
        DisableAndRemove(pRSSetViewportsTarget);
        DisableAndRemove(pIASetVertexBuffersTarget);
        DisableAndRemove(pDrawIndexedInstancedTarget);
        DisableAndRemove(pDrawInstancedTarget);
        DisableAndRemove(pSetGraphicsRootConstantBufferViewTarget);
        //DisableAndRemove(pSetDescriptorHeapsTarget);
        DisableAndRemove(pSetGraphicsRootDescriptorTableTarget);
        DisableAndRemove(pOMSetRenderTargetsTarget);
        DisableAndRemove(pResolveQueryDataTarget);
        DisableAndRemove(pExecuteIndirectTarget);
        DisableAndRemove(pSetGraphicsRootSignatureTarget);
        DisableAndRemove(pResetTarget);
        DisableAndRemove(pIASetIndexBufferTarget);
        //DisableAndRemove(pCloseTarget);

        Log("[hooks] All hooks removed.");
        
    }
}

//=========================================================================================================================//

// GLOBALS
namespace globals {
    extern HMODULE mainModule;
    extern HWND mainWindow;
    extern int openMenuKey;

    // Rendering backend currently in use
    enum class Backend {
        None,
        DX12,
    };
    extern Backend activeBackend;
    // Preferred backend to hook. None means auto with fallback order
    extern Backend preferredBackend;
    extern bool enableDebugLog;
}

namespace globals {
    // Handle to our DLL module
    HMODULE mainModule = nullptr;
    // Main game window handle
    HWND mainWindow = nullptr;
    // Key to uninject and exit
    int openMenuKey = VK_INSERT;
    // Preferred backend to hook (None = auto fallback -> Not recommanded, specify your engine here)
    Backend preferredBackend = Backend::DX12;
    // Flag controlling runtime debug logging
    bool enableDebugLog = true;
    // Currently active rendering backend
    Backend activeBackend = Backend::None; // DO NOT MODIFY THIS LINE.
}

//=========================================================================================================================//

//menu
bool menuisOpen = false;
int countstride1 = -1;
int countstride2 = -1;
int countstride3 = -1;
int countstride4 = -1;
int countindexformat = -1;
int countcurrentRootSigID = -1;
int countcurrentRootSigID2 = -1;
int countfindrendertarget = -1;
int countfilterrendertarget = -1;
int countignorerendertarget = -1;
int countfilterrootConstant = -1;
int countfilterrootConstant2 = -1;
int countfilterrootConstant3 = -1;
int countignorerootConstant = -1;
int countignorerootConstant2 = -1;
int countignorerootConstant3 = -1;
int countfilterrootDescriptor = -1;
int countfilterrootDescriptor2 = -1;
int countfilterrootDescriptor3 = -1;
int countignorerootDescriptor = -1;
int countignorerootDescriptor2 = -1;
int countignorerootDescriptor3 = -1;
bool filterrendertarget = false;
bool ignorerendertarget = false;
bool reversedDepth = false;
bool filterRootConstant = false;
bool ignoreRootConstant = false;
bool filterRootDescriptor = false;
bool ignoreRootDescriptor = false;
//bool filterindexformat = false;

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
    fout << "countindexformat " << countindexformat << endl;
    fout << "countcurrentRootSigID " << countcurrentRootSigID << endl;
    fout << "countcurrentRootSigID2 " << countcurrentRootSigID2 << endl;
    fout << "countfindrendertarget " << countfindrendertarget << endl;
    fout << "countfilterrendertarget " << countfilterrendertarget << endl;
    fout << "countfilterrootConstant " << countfilterrootConstant << endl;
    fout << "countfilterrootConstant2 " << countfilterrootConstant2 << endl;
    fout << "countfilterrootConstant3 " << countfilterrootConstant3 << endl;
    fout << "countignorerootConstant " << countignorerootConstant << endl;
    fout << "countignorerootConstant2 " << countignorerootConstant2 << endl;
    fout << "countignorerootConstant3 " << countignorerootConstant3 << endl;
    fout << "countfilterrootDescriptor " << countfilterrootDescriptor << endl;
    fout << "countfilterrootDescriptor2 " << countfilterrootDescriptor2 << endl;
    fout << "countfilterrootDescriptor3 " << countfilterrootDescriptor3 << endl;
    fout << "countignorerootDescriptor " << countignorerootDescriptor << endl;
    fout << "countignorerootDescriptor2 " << countignorerootDescriptor2 << endl;
    fout << "countignorerootDescriptor3 " << countignorerootDescriptor3 << endl;
    fout << "filterrendertarget " << filterrendertarget << endl;
    fout << "filterRootConstant " << filterRootConstant << endl;
    fout << "ignoreRootConstant " << ignoreRootConstant << endl;
    fout << "filterRootDescriptor " << filterRootDescriptor << endl;
    fout << "ignoreRootDescriptor " << ignoreRootDescriptor << endl;
    fout << "reversedDepth " << reversedDepth << endl;
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
    fin >> Word >> countindexformat;
    fin >> Word >> countcurrentRootSigID;
    fin >> Word >> countcurrentRootSigID2;
    fin >> Word >> countfindrendertarget;
    fin >> Word >> countfilterrendertarget;
    fin >> Word >> countfilterrootConstant;
    fin >> Word >> countfilterrootConstant2;
    fin >> Word >> countfilterrootConstant3;
    fin >> Word >> countignorerootConstant;
    fin >> Word >> countignorerootConstant2;
    fin >> Word >> countignorerootConstant3;
    fin >> Word >> countfilterrootDescriptor;
    fin >> Word >> countfilterrootDescriptor2;
    fin >> Word >> countfilterrootDescriptor3;
    fin >> Word >> countignorerootDescriptor;
    fin >> Word >> countignorerootDescriptor2;
    fin >> Word >> countignorerootDescriptor3;
    fin >> Word >> filterrendertarget;
    fin >> Word >> filterRootConstant;
    fin >> Word >> ignoreRootConstant;
    fin >> Word >> filterRootDescriptor;
    fin >> Word >> ignoreRootDescriptor;
    fin >> Word >> reversedDepth;
    fin.close();
}

//=========================================================================================================================//

// Thread-local cache
thread_local struct {
    UINT StartSlot = 0;
    UINT Strides[16] = {};
    UINT numViews = 0;
    //UINT vertexBufferSizes[16] = {};

    UINT CanonicalStrides[8] = {};
    UINT CanonicalCount = 0;
    uint32_t StrideHash = 0;

    // states
    D3D12_VIEWPORT currentViewport = {};
    UINT numViewports = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE LastDescriptorBase;
    UINT currentNumRTVs = 0;
    UINT currentiSize = 0;
    DXGI_FORMAT currentIndexFormat = DXGI_FORMAT_UNKNOWN;
} t_;

//=========================================================================================================================//

//IASetVertexBuffers
static inline uint32_t HashStrides(const UINT* data, UINT count)
{
    uint32_t h = 2166136261u;
    for (UINT i = 0; i < count; ++i) {
        h ^= data[i];
        h *= 16777619u;
    }
    return h;
}

// Fold full 32-bit hash into 0–99 using high entropy bits
static inline uint32_t FoldToTwoDigits(uint32_t h)
{
    return (uint32_t)((uint64_t)h * 100 >> 32);
}

static inline UINT BuildCanonicalStrides(
    const D3D12_VERTEX_BUFFER_VIEW* views,
    UINT numViews,
    UINT* outStrides,
    UINT maxOut)
{
    UINT count = 0;

    for (UINT i = 0; i < numViews && count < maxOut; ++i) {
        UINT s = views[i].StrideInBytes;

        // Ignore junk / unused buffers
        if (s == 0 || s > 2048)
            continue;

        outStrides[count++] = s;
    }

    // Sort to remove IA slot ordering
    for (UINT i = 0; i < count; ++i) {
        for (UINT j = i + 1; j < count; ++j) {
            if (outStrides[j] < outStrides[i])
                std::swap(outStrides[i], outStrides[j]);
        }
    }

    return count;
}

//=========================================================================================================================//

//SetGraphicsRootSignature
#include <shared_mutex>
#include <unordered_map>

std::shared_mutex rootSigMutex;
uint32_t nextRuntimeSigID = 1;
std::unordered_map<ID3D12RootSignature*, uint32_t> rootSigToID;

// Thread-local storage: Each thread tracks its own active command list and ID
thread_local ID3D12GraphicsCommandList* tlsCurrentCmdList = nullptr;
thread_local uint32_t tlsCurrentRootSigID = 0;

//=========================================================================================================================//

//SetGraphicsRootConstantBufferView
struct CommandListState {
    ID3D12GraphicsCommandList* cmdListPtr = nullptr;
    UINT lastCbvIndex = UINT_MAX;
    // Add other state variables here
    UINT lastRDIndex = UINT_MAX;
};

// Extremely fast: no mutex, no map lookup 99% of the time
thread_local CommandListState tls_cache;

//=========================================================================================================================//
// 
//colors
#include <DirectXMath.h>
using namespace DirectX;

int countnum = 4;
ComPtr<ID3D12Device> pDevice = nullptr;
bool initialized = false;

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

//=========================================================================================================================//

/*
//vtable index values, from d3d12.h
0 #define ID3D12GraphicsCommandList_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) )

1 #define ID3D12GraphicsCommandList_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) )

2 #define ID3D12GraphicsCommandList_Release(This)	\
    ( (This)->lpVtbl -> Release(This) )

3 #define ID3D12GraphicsCommandList_GetPrivateData(This,guid,pDataSize,pData)	\
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) )

4 #define ID3D12GraphicsCommandList_SetPrivateData(This,guid,DataSize,pData)	\
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) )

5 #define ID3D12GraphicsCommandList_SetPrivateDataInterface(This,guid,pData)	\
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) )

6 #define ID3D12GraphicsCommandList_SetName(This,Name)	\
    ( (This)->lpVtbl -> SetName(This,Name) )

7 #define ID3D12GraphicsCommandList_GetDevice(This,riid,ppvDevice)	\
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) )

8 #define ID3D12GraphicsCommandList_GetType(This)	\
    ( (This)->lpVtbl -> GetType(This) )

9 #define ID3D12GraphicsCommandList_Close(This)	\
    ( (This)->lpVtbl -> Close(This) )

10 #define ID3D12GraphicsCommandList_Reset(This,pAllocator,pInitialState)	\
    ( (This)->lpVtbl -> Reset(This,pAllocator,pInitialState) )

11 #define ID3D12GraphicsCommandList_ClearState(This,pPipelineState)	\
    ( (This)->lpVtbl -> ClearState(This,pPipelineState) )

12 #define ID3D12GraphicsCommandList_DrawInstanced(This,VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation)	\
    ( (This)->lpVtbl -> DrawInstanced(This,VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation) )

13 #define ID3D12GraphicsCommandList_DrawIndexedInstanced(This,IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation)	\
    ( (This)->lpVtbl -> DrawIndexedInstanced(This,IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation) )

14 #define ID3D12GraphicsCommandList_Dispatch(This,ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ)	\
    ( (This)->lpVtbl -> Dispatch(This,ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ) )

15 #define ID3D12GraphicsCommandList_CopyBufferRegion(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,NumBytes)	\
    ( (This)->lpVtbl -> CopyBufferRegion(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,NumBytes) )

16 #define ID3D12GraphicsCommandList_CopyTextureRegion(This,pDst,DstX,DstY,DstZ,pSrc,pSrcBox)	\
    ( (This)->lpVtbl -> CopyTextureRegion(This,pDst,DstX,DstY,DstZ,pSrc,pSrcBox) )

17 #define ID3D12GraphicsCommandList_CopyResource(This,pDstResource,pSrcResource)	\
    ( (This)->lpVtbl -> CopyResource(This,pDstResource,pSrcResource) )

18 #define ID3D12GraphicsCommandList_CopyTiles(This,pTiledResource,pTileRegionStartCoordinate,pTileRegionSize,pBuffer,BufferStartOffsetInBytes,Flags)	\
    ( (This)->lpVtbl -> CopyTiles(This,pTiledResource,pTileRegionStartCoordinate,pTileRegionSize,pBuffer,BufferStartOffsetInBytes,Flags) )

19 #define ID3D12GraphicsCommandList_ResolveSubresource(This,pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format)	\
    ( (This)->lpVtbl -> ResolveSubresource(This,pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format) )

20 #define ID3D12GraphicsCommandList_IASetPrimitiveTopology(This,PrimitiveTopology)	\
    ( (This)->lpVtbl -> IASetPrimitiveTopology(This,PrimitiveTopology) )

21 #define ID3D12GraphicsCommandList_RSSetViewports(This,NumViewports,pViewports)	\
    ( (This)->lpVtbl -> RSSetViewports(This,NumViewports,pViewports) )

22 #define ID3D12GraphicsCommandList_RSSetScissorRects(This,NumRects,pRects)	\
    ( (This)->lpVtbl -> RSSetScissorRects(This,NumRects,pRects) )

23 #define ID3D12GraphicsCommandList_OMSetBlendFactor(This,BlendFactor)	\
    ( (This)->lpVtbl -> OMSetBlendFactor(This,BlendFactor) )

24 #define ID3D12GraphicsCommandList_OMSetStencilRef(This,StencilRef)	\
    ( (This)->lpVtbl -> OMSetStencilRef(This,StencilRef) )

25 #define ID3D12GraphicsCommandList_SetPipelineState(This,pPipelineState)	\
    ( (This)->lpVtbl -> SetPipelineState(This,pPipelineState) )

26 #define ID3D12GraphicsCommandList_ResourceBarrier(This,NumBarriers,pBarriers)	\
    ( (This)->lpVtbl -> ResourceBarrier(This,NumBarriers,pBarriers) )

27 #define ID3D12GraphicsCommandList_ExecuteBundle(This,pCommandList)	\
    ( (This)->lpVtbl -> ExecuteBundle(This,pCommandList) )

28 #define ID3D12GraphicsCommandList_SetDescriptorHeaps(This,NumDescriptorHeaps,ppDescriptorHeaps)	\
    ( (This)->lpVtbl -> SetDescriptorHeaps(This,NumDescriptorHeaps,ppDescriptorHeaps) )

29 #define ID3D12GraphicsCommandList_SetComputeRootSignature(This,pRootSignature)	\
    ( (This)->lpVtbl -> SetComputeRootSignature(This,pRootSignature) )

30 #define ID3D12GraphicsCommandList_SetGraphicsRootSignature(This,pRootSignature)	\
    ( (This)->lpVtbl -> SetGraphicsRootSignature(This,pRootSignature) )

31 #define ID3D12GraphicsCommandList_SetComputeRootDescriptorTable(This,RootParameterIndex,BaseDescriptor)	\
    ( (This)->lpVtbl -> SetComputeRootDescriptorTable(This,RootParameterIndex,BaseDescriptor) )

32 #define ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable(This,RootParameterIndex,BaseDescriptor)	\
    ( (This)->lpVtbl -> SetGraphicsRootDescriptorTable(This,RootParameterIndex,BaseDescriptor) )

33 #define ID3D12GraphicsCommandList_SetComputeRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues)	\
    ( (This)->lpVtbl -> SetComputeRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues) )

34 #define ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues)	\
    ( (This)->lpVtbl -> SetGraphicsRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues) )

35 #define ID3D12GraphicsCommandList_SetComputeRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues)	\
    ( (This)->lpVtbl -> SetComputeRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues) )

36 #define ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues)	\
    ( (This)->lpVtbl -> SetGraphicsRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues) )

37 #define ID3D12GraphicsCommandList_SetComputeRootConstantBufferView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetComputeRootConstantBufferView(This,RootParameterIndex,BufferLocation) )

38 #define ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetGraphicsRootConstantBufferView(This,RootParameterIndex,BufferLocation) )

39 #define ID3D12GraphicsCommandList_SetComputeRootShaderResourceView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetComputeRootShaderResourceView(This,RootParameterIndex,BufferLocation) )

40 #define ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetGraphicsRootShaderResourceView(This,RootParameterIndex,BufferLocation) )

41 #define ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetComputeRootUnorderedAccessView(This,RootParameterIndex,BufferLocation) )

42 #define ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView(This,RootParameterIndex,BufferLocation)	\
    ( (This)->lpVtbl -> SetGraphicsRootUnorderedAccessView(This,RootParameterIndex,BufferLocation) )

43 #define ID3D12GraphicsCommandList_IASetIndexBuffer(This,pView)	\
    ( (This)->lpVtbl -> IASetIndexBuffer(This,pView) )

44 #define ID3D12GraphicsCommandList_IASetVertexBuffers(This,StartSlot,NumViews,pViews)	\
    ( (This)->lpVtbl -> IASetVertexBuffers(This,StartSlot,NumViews,pViews) )

45 #define ID3D12GraphicsCommandList_SOSetTargets(This,StartSlot,NumViews,pViews)	\
    ( (This)->lpVtbl -> SOSetTargets(This,StartSlot,NumViews,pViews) )

46 #define ID3D12GraphicsCommandList_OMSetRenderTargets(This,NumRenderTargetDescriptors,pRenderTargetDescriptors,RTsSingleHandleToDescriptorRange,pDepthStencilDescriptor)	\
    ( (This)->lpVtbl -> OMSetRenderTargets(This,NumRenderTargetDescriptors,pRenderTargetDescriptors,RTsSingleHandleToDescriptorRange,pDepthStencilDescriptor) )

47 #define ID3D12GraphicsCommandList_ClearDepthStencilView(This,DepthStencilView,ClearFlags,Depth,Stencil,NumRects,pRects)	\
    ( (This)->lpVtbl -> ClearDepthStencilView(This,DepthStencilView,ClearFlags,Depth,Stencil,NumRects,pRects) )

48 #define ID3D12GraphicsCommandList_ClearRenderTargetView(This,RenderTargetView,ColorRGBA,NumRects,pRects)	\
    ( (This)->lpVtbl -> ClearRenderTargetView(This,RenderTargetView,ColorRGBA,NumRects,pRects) )

49 #define ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects)	\
    ( (This)->lpVtbl -> ClearUnorderedAccessViewUint(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects) )

50 #define ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects)	\
    ( (This)->lpVtbl -> ClearUnorderedAccessViewFloat(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects) )

51 #define ID3D12GraphicsCommandList_DiscardResource(This,pResource,pRegion)	\
    ( (This)->lpVtbl -> DiscardResource(This,pResource,pRegion) )

52 #define ID3D12GraphicsCommandList_BeginQuery(This,pQueryHeap,Type,Index)	\
    ( (This)->lpVtbl -> BeginQuery(This,pQueryHeap,Type,Index) )

53 #define ID3D12GraphicsCommandList_EndQuery(This,pQueryHeap,Type,Index)	\
    ( (This)->lpVtbl -> EndQuery(This,pQueryHeap,Type,Index) )

54 #define ID3D12GraphicsCommandList_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset)	\
    ( (This)->lpVtbl -> ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset) )

55 #define ID3D12GraphicsCommandList_SetPredication(This,pBuffer,AlignedBufferOffset,Operation)	\
    ( (This)->lpVtbl -> SetPredication(This,pBuffer,AlignedBufferOffset,Operation) )

56 #define ID3D12GraphicsCommandList_SetMarker(This,Metadata,pData,Size)	\
    ( (This)->lpVtbl -> SetMarker(This,Metadata,pData,Size) )

57 #define ID3D12GraphicsCommandList_BeginEvent(This,Metadata,pData,Size)	\
    ( (This)->lpVtbl -> BeginEvent(This,Metadata,pData,Size) )

58 #define ID3D12GraphicsCommandList_EndEvent(This)	\
    ( (This)->lpVtbl -> EndEvent(This) )

59 #define ID3D12GraphicsCommandList_ExecuteIndirect(This,pCommandSignature,MaxCommandCount,pArgumentBuffer,ArgumentBufferOffset,pCountBuffer,CountBufferOffset)	\
    ( (This)->lpVtbl -> ExecuteIndirect(This,pCommandSignature,MaxCommandCount,pArgumentBuffer,ArgumentBufferOffset,pCountBuffer,CountBufferOffset) )
*/


//buggy shit
/*
if (applyHack) {

    // Remember original DSV so we restore exactly
    D3D12_CPU_DESCRIPTOR_HANDLE originalDSV = t_.hasDSV ? t_.currentDSVHandle : D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

    // 1. Draw without depth
    _this->OMSetRenderTargets(
        t_.currentNumRTVs,
        t_.currentRTVHandles,               // Note: use array or single handle correctly
        t_.currentRTsSingleHandle,
        nullptr
    );

    oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    // 2. ALWAYS restore — even if no original DSV (pass nullptr again if none)
    _this->OMSetRenderTargets(
        t_.currentNumRTVs,
        t_.currentRTVHandles,
        t_.currentRTsSingleHandle,
        t_.hasDSV ? &originalDSV : nullptr
    );

    return;



    // 1. Temporarily UNBIND the depth buffer
    // This makes the object draw "on top" of everything without fighting the world or itself
    _this->OMSetRenderTargets(
        t_.currentNumRTVs,
        t_.currentRTVHandles,
        t_.currentRTsSingleHandle,
        nullptr // <--- Passing nullptr here disables depth testing/writing
    );

    // 2. Draw the model
    oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    // 3. Restore the depth buffer immediately so other objects render correctly
    if (t_.hasDSV) {
        _this->OMSetRenderTargets(
            t_.currentNumRTVs,
            t_.currentRTVHandles,
            t_.currentRTsSingleHandle,
            &t_.currentDSVHandle
        );
    }

    return; // Done



    // 1. Get the current RTVs
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];
    // You'll need to track the current RTVs in your OMSetRenderTargets hook
    // and store them in t_.currentRTVHandles

    // 2. Re-bind RTVs but pass NULL/0 for the Depth Stencil
    _this->OMSetRenderTargets(t_.currentNumRTVs, t_.currentRTVHandles, FALSE, nullptr);

    // 3. Draw
    oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    // 4. Restore the original Depth Stencil
    _this->OMSetRenderTargets(t_.currentNumRTVs, t_.currentRTVHandles, FALSE, &t_.currentDSVHandle);

    return;
*/            
