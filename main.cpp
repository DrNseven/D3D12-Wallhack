//d3d12 hook/wallhack with imgui overlay 2026

#pragma once
#include <windows.h>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d12.h>

#include <wrl/client.h>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

// minhook
#include "minhook/include/MinHook.h"

// libs
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")

// helper funcs
#include "main.h"

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

//=========================================================================================================================//

namespace hooks { void Remove(); }

namespace d3d12hook {
    PresentD3D12            oPresentD3D12 = nullptr;
    Present1Fn              oPresent1D3D12 = nullptr;
    ExecuteCommandListsFn   oExecuteCommandListsD3D12 = nullptr;
    ResizeBuffersFn         oResizeBuffersD3D12 = nullptr;
    RSSetViewportsFn        oRSSetViewportsD3D12 = nullptr;
    IASetVertexBuffersFn    oIASetVertexBuffersD3D12 = nullptr;
    DrawIndexedInstancedFn  oDrawIndexedInstancedD3D12 = nullptr;
    DrawInstancedFn  oDrawInstancedD3D12 = nullptr;
    DispatchFn oDispatchD3D12 = nullptr;
    SetGraphicsRootConstantBufferViewFn oSetGraphicsRootConstantBufferViewD3D12 = nullptr;
    SetDescriptorHeapsFn    oSetDescriptorHeapsD3D12 = nullptr;
    SetGraphicsRootDescriptorTableFn oSetGraphicsRootDescriptorTableD3D12 = nullptr;
    OMSetRenderTargetsFn oOMSetRenderTargetsD3D12 = nullptr;
    ResolveQueryDataFn oResolveQueryDataD3D12 = nullptr;
    ExecuteIndirectFn oExecuteIndirectD3D12 = nullptr;
    SetGraphicsRootSignatureFn oSetGraphicsRootSignatureD3D12 = nullptr;
    ResetFn oResetD3D12 = nullptr;
    CloseFn oCloseD3D12 = nullptr;
    IASetIndexBufferFn oIASetIndexBufferD3D12 = nullptr;
    DispatchMeshFn oDispatchMeshD3D12 = nullptr;
    //if unresolved external symbol d3d12hook::function, fix here


    static ID3D12Device* gDevice = nullptr;
    static ID3D12CommandQueue* gCommandQueue = nullptr;
    static ID3D12DescriptorHeap* gHeapRTV = nullptr;
    static ID3D12DescriptorHeap* gHeapSRV = nullptr;
    static ID3D12GraphicsCommandList* gCommandList = nullptr;
    static ID3D12Fence*            gOverlayFence = nullptr;
    static HANDLE                  gFenceEvent = nullptr;
    static UINT64                  gOverlayFenceValue = 0;
    static uintx_t                 gBufferCount = 0;

    struct FrameContext {
        ID3D12CommandAllocator* allocator;
        ID3D12Resource* renderTarget;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

        // The specific fence value that must be reached before this specific back-buffer can be reused
        UINT64 FenceValue;
    };
    static FrameContext*          gFrameContexts = nullptr;
    static bool                   gInitialized = false;
    static bool                   gShutdown = false;
    static bool                   gAfterFirstPresent = false;

    void release();

    //=========================================================================================================================//

    HRESULT WINAPI hookPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {

        return oPresentD3D12(pSwapChain, SyncInterval, Flags);
    }

    //=========================================================================================================================//

    HRESULT WINAPI hookPresent1D3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams) {

        return oPresent1D3D12(pSwapChain, SyncInterval, Flags, pParams);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {

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
        return oResizeBuffersD3D12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookRSSetViewportsD3D12(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports) {
        gInitialized = true; //

        if (!pViewports || NumViewports == 0) {
            oRSSetViewportsD3D12(_this, NumViewports, pViewports);
            return;
        }

        if (NumViewports > 0 && pViewports) {
            // Cache the game's intended viewport
            t_.currentViewport = pViewports[0];
            t_.numViewports = NumViewports;
        }
        
    //for fucked up games, where it draws in unknown function (doesn't really happen)
    if (countindexformat > 0 && t_.currentNumRTVs > 0) {
        UINT currentindexformat = t_.currentIndexFormat + t_.numViewports;

        if (currentindexformat == countindexformat &&
            t_.currentViewport.Width > 0 &&
            t_.currentViewport.Height > 0 &&
            t_.currentViewport.Width < 16384)
        {
            D3D12_VIEWPORT hVp = t_.currentViewport;

            hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
            hVp.MaxDepth = reversedDepth ? 0.01f : 1.0f;

            // submit modified viewport ONCE
            oRSSetViewportsD3D12(_this, 1, &hVp);
            return;
        }
    }

        oRSSetViewportsD3D12(_this, NumViewports, pViewports);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookIASetVertexBuffersD3D12(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {

        t_.StartSlot = StartSlot;
        t_.numViews = NumViews;

        for (UINT i = 0; i < 16; ++i) {
            t_.Strides[i] = (pViews && i < NumViews)
                ? pViews[i].StrideInBytes
                : 0;
            //t_.currentGPUVAddress = pViews[0].BufferLocation;
        }

        if (pViews && NumViews > 0) {
            t_.CanonicalCount = BuildCanonicalStrides(
                pViews,
                NumViews,
                t_.CanonicalStrides,
                _countof(t_.CanonicalStrides)
            );

            if (t_.CanonicalCount > 0) {
                uint32_t h = HashStrides(
                    t_.CanonicalStrides,
                    t_.CanonicalCount
                );

                //
                t_.StrideHash = FoldToTwoDigits(h);
            }
            else {
                t_.StrideHash = 0;
            }
        }
        else {
            t_.StrideHash = 0;
        }
        
        return oIASetVertexBuffersD3D12(_this, StartSlot, NumViews, pViews);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookIASetIndexBufferD3D12(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView)
    {
        if (pView != nullptr) {
            t_.currentiSize = pView->SizeInBytes;
            t_.currentIndexFormat = pView->Format;
            t_.currentGPUIAddress = pView->BufferLocation;
        }

        return oIASetIndexBufferD3D12(dCommandList, pView);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookOMSetRenderTargetsD3D12(
        ID3D12GraphicsCommandList* dCommandList,
        UINT NumRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        BOOL RTsSingleHandleToDescriptorRange,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor) {

        if (dCommandList->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            t_.currentNumRTVs = NumRenderTargetDescriptors;
            //t_.currentRTsSingleHandle = RTsSingleHandleToDescriptorRange;
            /*
            // 1. Capture RTV Handles
            if (pRenderTargetDescriptors != nullptr && NumRenderTargetDescriptors > 0)
            {
                if (RTsSingleHandleToDescriptorRange) {
                    // Just store the first handle (the base of the range)
                    t_.currentRTVHandles[0] = pRenderTargetDescriptors[0];
                }
                else {
                    // Store the whole array (up to 8)
                    UINT count = (NumRenderTargetDescriptors > 8) ? 8 : NumRenderTargetDescriptors;
                    for (UINT i = 0; i < count; ++i) {
                        t_.currentRTVHandles[i] = pRenderTargetDescriptors[i];
                    }
                }
            }

            // 2. Capture DSV Handle
            if (pDepthStencilDescriptor != nullptr) {
                t_.currentDSVHandle = *pDepthStencilDescriptor;
                t_.hasDSV = true;
            }
            else {
                t_.hasDSV = false;
            }
            */
        }

        return oOMSetRenderTargetsD3D12(dCommandList, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        // 1. QUICK EXIT
        // Check RTVs first (simple member access) before calling GetType (vtable call)
        if (!_this || t_.currentNumRTVs == 0 || _this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // 2. IDENTIFICATION (Short-circuiting logic)
        // We use a local copy of strides to ensure consistency during this function call
        const UINT currentStrides = t_.StrideHash + t_.StartSlot;
        bool isModelDraw = false;

        if (currentStrides == countstride1 || currentStrides == countstride2 ||
            currentStrides == countstride3 || currentStrides == countstride4 ||
            t_.currentNumRTVs == countfindrendertarget)
        {
            isModelDraw = true;
        }
        else
        {
            // Only check RootSig if strides didn't match
            uint32_t rootSigID = (tlsCurrentCmdList == _this) ? tlsCurrentRootSigID : 0;
            if (rootSigID == countcurrentRootSigID || rootSigID == countcurrentRootSigID2)
            {
                isModelDraw = true;
            }
            else
            {
                // Only perform expensive Modulo if everything else failed
                uint8_t indexid = static_cast<uint8_t>((t_.currentGPUIAddress >> 12) % 100);
                if (indexid == countcurrentindexid || indexid == countcurrentindexid2 || indexid == countcurrentindexid3)
                {
                    isModelDraw = true;
                }
            }
        }

        if (isModelDraw)
        {
            // 3. FILTER LOGIC (Fail-Fast)
            // If any filter fails, we immediately jump to the original draw
            if (filternumViews && t_.numViews != countfilternumViews) goto skip;
            if (ignorenumViews && t_.numViews == countignorenumViews) goto skip;
            if (filternumViewports && t_.numViewports != countfilternumViewports) goto skip;
            if (ignorenumViewports && t_.numViewports == countignorenumViewports) goto skip;
            if (filterrendertarget && t_.currentNumRTVs != countfilterrendertarget) goto skip;
            if (ignorerendertarget && t_.currentNumRTVs == countignorerendertarget) goto skip;

            // Root Descriptor/Constant Filters
            if (filterRootDescriptor && (tls_cache.lastRDIndex != countfilterrootDescriptor && tls_cache.lastRDIndex != countfilterrootDescriptor2 && tls_cache.lastRDIndex != countfilterrootDescriptor3)) goto skip;
            if (ignoreRootDescriptor && (tls_cache.lastRDIndex == countignorerootDescriptor || tls_cache.lastRDIndex == countignorerootDescriptor2 || tls_cache.lastRDIndex == countignorerootDescriptor3)) goto skip;
            if (filterRootConstant && (tls_cache.lastCbvIndex != countfilterrootConstant && tls_cache.lastCbvIndex != countfilterrootConstant2 && tls_cache.lastCbvIndex != countfilterrootConstant3)) goto skip;
            if (ignoreRootConstant && (tls_cache.lastCbvIndex == countignorerootConstant || tls_cache.lastCbvIndex == countignorerootConstant2 || tls_cache.lastCbvIndex == countignorerootConstant3)) goto skip;

            // 4. VIEWPORT VALIDATION & EXECUTION
            // MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
            const D3D12_VIEWPORT originalVp = t_.currentViewport;

            if (originalVp.Width >= 128 && originalVp.Width < 16384 && originalVp.Height >= 128)
            {
                D3D12_VIEWPORT hVp = originalVp;
                hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
                hVp.MaxDepth = reversedDepth ? 0.1f : 1.0f;

                _this->RSSetViewports(1, &hVp);
                oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
                _this->RSSetViewports(1, &originalVp);
                return;
            }
        }

    skip:
        oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookExecuteIndirectD3D12(
        ID3D12GraphicsCommandList* _this,
        ID3D12CommandSignature* pCommandSignature,
        UINT MaxCommandCount,
        ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,
        ID3D12Resource* pCountBuffer,
        UINT64 CountBufferOffset)
    {
        // 1. QUICK EXIT
        // Check RTVs first (simple member access) before calling GetType (vtable call)
        if (!_this || t_.currentNumRTVs == 0 || _this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

        // 2. IDENTIFICATION (Short-circuiting logic)
        // We use a local copy of strides to ensure consistency during this function call
        const UINT currentStrides = t_.StrideHash + t_.StartSlot;
        //uint32_t shortID = static_cast<uint32_t>(MaxCommandCount % 100);
        bool isModelDraw = false;

        if (currentStrides == countstride1 || currentStrides == countstride2 ||
            currentStrides == countstride3 || currentStrides == countstride4 ||
            t_.currentNumRTVs == countfindrendertarget)
        {
            isModelDraw = true;
        }
        else
        {
            // Only check RootSig if strides didn't match
            uint32_t rootSigID = (tlsCurrentCmdList == _this) ? tlsCurrentRootSigID : 0;
            if (rootSigID == countcurrentRootSigID || rootSigID == countcurrentRootSigID2)
            {
                isModelDraw = true;
            }
            else
            {
                // Only perform expensive Modulo if everything else failed
                uint8_t indexid = static_cast<uint8_t>((t_.currentGPUIAddress >> 12) % 100);
                if (indexid == countcurrentindexid || indexid == countcurrentindexid2 || indexid == countcurrentindexid3)
                {
                    isModelDraw = true;
                }
            }
        }

        if (isModelDraw)
        {
            // 3. FILTER LOGIC (Fail-Fast)
            // If any filter fails, we immediately jump to the original draw
            if (filternumViews && t_.numViews != countfilternumViews) goto skip;
            if (ignorenumViews && t_.numViews == countignorenumViews) goto skip;
            if (filternumViewports && t_.numViewports != countfilternumViewports) goto skip;
            if (ignorenumViewports && t_.numViewports == countignorenumViewports) goto skip;
            if (filterrendertarget && t_.currentNumRTVs != countfilterrendertarget) goto skip;
            if (ignorerendertarget && t_.currentNumRTVs == countignorerendertarget) goto skip;

            // Root Descriptor/Constant Filters
            if (filterRootDescriptor && (tls_cache.lastRDIndex != countfilterrootDescriptor && tls_cache.lastRDIndex != countfilterrootDescriptor2 && tls_cache.lastRDIndex != countfilterrootDescriptor3)) goto skip;
            if (ignoreRootDescriptor && (tls_cache.lastRDIndex == countignorerootDescriptor || tls_cache.lastRDIndex == countignorerootDescriptor2 || tls_cache.lastRDIndex == countignorerootDescriptor3)) goto skip;
            if (filterRootConstant && (tls_cache.lastCbvIndex != countfilterrootConstant && tls_cache.lastCbvIndex != countfilterrootConstant2 && tls_cache.lastCbvIndex != countfilterrootConstant3)) goto skip;
            if (ignoreRootConstant && (tls_cache.lastCbvIndex == countignorerootConstant || tls_cache.lastCbvIndex == countignorerootConstant2 || tls_cache.lastCbvIndex == countignorerootConstant3)) goto skip;

            // 4. VIEWPORT VALIDATION & EXECUTION
            // MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
            const D3D12_VIEWPORT originalVp = t_.currentViewport;

            if (originalVp.Width >= 128 && originalVp.Width < 16384 && originalVp.Height >= 128)
            {
                D3D12_VIEWPORT hVp = originalVp;
                hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
                hVp.MaxDepth = reversedDepth ? 0.1f : 1.0f;

                _this->RSSetViewports(1, &hVp);
                oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
                _this->RSSetViewports(1, &originalVp);
                return;
            }
        }

    skip:
        oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDrawInstancedD3D12(ID3D12GraphicsCommandList* cmd, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
    {
        oDrawInstancedD3D12(cmd, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootConstantBufferViewD3D12(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

        // Synchronize cache if the engine switched command lists on this thread
        if (tls_cache.cmdListPtr != _this) {
            tls_cache.cmdListPtr = _this;
            // Optionally: Reset state if it's a new command list context
            //tls_cache.lastCbvIndex = UINT_MAX;
        }

        if(_this)
        {
            tls_cache.lastCbvIndex = RootParameterIndex;
            //t_.currentGPURAddress = BufferLocation;
        }

        return oSetGraphicsRootConstantBufferViewD3D12(_this, RootParameterIndex, BufferLocation);
    }

    //=========================================================================================================================//
    
    void STDMETHODCALLTYPE hookDispatchD3D12(ID3D12GraphicsCommandList* pList,UINT threadCountX, UINT threadCountY, UINT threadCountZ)
    {     
        oDispatchD3D12(pList, threadCountX, threadCountY, threadCountZ);
    }

    //=========================================================================================================================//
    
    void STDMETHODCALLTYPE hookDispatchMeshD3D12(ID3D12GraphicsCommandList6* cmd,UINT x, UINT y, UINT z)
    {
        oDispatchMeshD3D12(cmd, x, y, z);
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

    void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {

        if (tls_cache.cmdListPtr != dCommandList) {
            tls_cache.cmdListPtr = dCommandList;

            //tls_cache.lastRDIndex = UINT_MAX;

            if (BaseDescriptor.ptr != 0)
                t_.LastDescriptorBase = BaseDescriptor;
        }

        if (BaseDescriptor.ptr != 0)
        {
            tls_cache.lastRDIndex = RootParameterIndex;
        }

        return oSetGraphicsRootDescriptorTableD3D12(dCommandList, RootParameterIndex, BaseDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookResetD3D12(ID3D12GraphicsCommandList* _this, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState) {

        // Reset our cache for this specific command list context
        if (tls_cache.cmdListPtr == _this) {
            tls_cache.lastCbvIndex = UINT_MAX;
            tls_cache.lastRDIndex = UINT_MAX;
        }

        //CmdState* state = GetCmdState(_this);
        //state->NumRTVs = 0;

        return oResetD3D12(_this, pAllocator, pInitialState);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookCloseD3D12(ID3D12GraphicsCommandList* cl)
    {
        //if (cl) {
            //CmdState* state = GetCmdState(cl);
            //state->NumRTVs = 0;
        //}

        oCloseD3D12(cl);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetDescriptorHeapsD3D12(ID3D12GraphicsCommandList* cmdList, UINT NumHeaps, ID3D12DescriptorHeap* const* ppHeaps)
    {
        /*
        for (UINT i = 0; i < NumHeaps; i++) {
            D3D12_DESCRIPTOR_HEAP_DESC desc = ppHeaps[i]->GetDesc();
            if (desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            {
                g_GameSRVHeap = ppHeaps[i];
            }
            //if (desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
                //g_GameSamplerHeap = ppHeaps[i];
        }
        */
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
                param.Value = 1; // 1 = Visible //or 0xFFFF

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

//=========================================================================================================================//

// ============================================================
// overlay
// ============================================================

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
#define FRAME_COUNT 2

// ============================================================
// GLOBALS (SINGLE SOURCE OF TRUTH)
// ============================================================
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
#include <fstream>
#include <string>

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

HWND CreateOverlayWindow()
{
    WNDCLASSEX wc{ sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"DCompDX12Overlay";
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE,wc.lpszClassName,L"",WS_POPUP,0, 0, 100, 100,nullptr, nullptr, wc.hInstance, nullptr);
    //HWND hwnd = CreateWindowEx(WS_EX_TOPMOST |WS_EX_NOACTIVATE |WS_EX_TRANSPARENT |WS_EX_LAYERED,wc.lpszClassName,L"",WS_POPUP,0, 0, 100, 100,nullptr, nullptr, wc.hInstance, nullptr);//breaks mouse

    ShowWindow(hwnd, SW_SHOW);
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
        // When the menu is open, we generally want inputs ENABLED 
             // unless you specifically want to 'lock' it.
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowSize(ImVec2(480, 440), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(25, 25), ImGuiCond_FirstUseEver);

        ImGui::Begin("Wallhack", &g_showMenu, flags);
        //ImGui::Begin("Wallhack", nullptr, flags | ImGuiWindowFlags_NoFocusOnAppearing);

        const UINT minus_val = -1;
        const UINT min_val = 0;
        const UINT max_val = 100;
        ImGui::Text("Permanent Values");
        //ImGui::SliderScalar("Find Stridehash 1", ImGuiDataType_U32, &countstride1, &min_val, &max_val, "%u");
        ImGui::SliderInt("Find Stridehash 1", &countstride1, minus_val, max_val);
        ImGui::SliderInt("Find Stridehash 2", &countstride2, minus_val, max_val);
        ImGui::SliderInt("Find Stridehash 3", &countstride3, minus_val, max_val);
        ImGui::SliderInt("Find Stridehash 4", &countstride4, minus_val, max_val);
        ImGui::SliderInt("Find RenderTarget", &countfindrendertarget, minus_val, max_val);
        ImGui::SliderInt("Find IndexFormat+Vp", &countindexformat, minus_val, max_val);

        //temporary IDs below (changes after game restart)
        ImGui::Checkbox("Temporary IDs", &temporaryids);
        if (temporaryids)
        {
            ImGui::SliderInt("Find CurrentRootID", &countcurrentRootSigID, minus_val, max_val);
            ImGui::SliderInt("Find CurrentRootID2", &countcurrentRootSigID2, minus_val, max_val);
            ImGui::SliderInt("Find CurrentIndexID", &countcurrentindexid, minus_val, max_val);
            ImGui::SliderInt("Find CurrentIndexID2", &countcurrentindexid2, minus_val, max_val);
            ImGui::SliderInt("Find CurrentIndexID3", &countcurrentindexid3, minus_val, max_val);
            ImGui::SliderInt("Find CurrentIndexID4", &countcurrentindexid4, minus_val, max_val);
        }

        //Filter/Ignore
        ImGui::Checkbox("Filter RenderTarget", &filterrendertarget);
        if (filterrendertarget)
        {
            ImGui::SliderInt("RenderTarget", &countfilterrendertarget, minus_val, max_val);
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

        ImGui::Checkbox("Filter RootDescriptor", &filterRootDescriptor);
        if (filterRootDescriptor)
        {
            ImGui::SliderInt("RootDescriptor1", &countfilterrootDescriptor, minus_val, max_val);
            ImGui::SliderInt("RootDescriptor2", &countfilterrootDescriptor2, minus_val, max_val);
            ImGui::SliderInt("RootDescriptor3", &countfilterrootDescriptor3, minus_val, max_val);
        }

        ImGui::Checkbox("Filter RootConstant", &filterRootConstant);
        if (filterRootConstant)
        {
            ImGui::SliderInt("RootConstant1", &countfilterrootConstant, minus_val, max_val);
            ImGui::SliderInt("RootConstant2", &countfilterrootConstant2, minus_val, max_val);
            ImGui::SliderInt("RootConstant3", &countfilterrootConstant3, minus_val, max_val);
        }

        ImGui::Checkbox("Ignore RootDescriptor", &ignoreRootDescriptor);
        if (ignoreRootDescriptor)
        {
            ImGui::SliderInt("RootDescriptor 1", &countignorerootDescriptor, minus_val, max_val);
            ImGui::SliderInt("RootDescriptor 2", &countignorerootDescriptor2, minus_val, max_val);
            ImGui::SliderInt("RootDescriptor 3", &countignorerootDescriptor3, minus_val, max_val);
        }

        ImGui::Checkbox("Ignore RootConstant", &ignoreRootConstant);
        if (ignoreRootConstant)
        {
            ImGui::SliderInt("RootConstant 1", &countignorerootConstant, minus_val, max_val);
            ImGui::SliderInt("RootConstant 2", &countignorerootConstant2, minus_val, max_val);
            ImGui::SliderInt("RootConstant 3", &countignorerootConstant3, minus_val, max_val);
        }

        ImGui::Checkbox("Reverse Depth", &reversedDepth);
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

    g_swapchain->Present(1, 0); // Recommended: use 1 for smoother resizing

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
            if (GetModuleHandleA("d3d12.dll") ||
                GetModuleHandleA("dxgi.dll"))
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

    // Start backend watcher thread
    HANDLE hThread = CreateThread(
        nullptr,
        0,
        BackendWatcherThread,
        nullptr,
        0,
        nullptr
    );

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
            HANDLE thread = CreateThread(
                nullptr, 0,
                onAttach,
                nullptr,
                0,
                nullptr
            );
            if (thread) CloseHandle(thread);
            else Log("[DllMain] Failed to create hook thread: %d\n", GetLastError());
        }
        break;

    case DLL_PROCESS_DETACH:
        g_running = false; // Signal all threads to stop
        g_BackendInitialized = true; // Break the watcher loop

        Log("[DllMain] DLL_PROCESS_DETACH. Cleaning up.\n");

        // Give threads a moment to finish (don't use WaitForSingleObject in DllMain - it causes deadlocks)
        Sleep(100);

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
