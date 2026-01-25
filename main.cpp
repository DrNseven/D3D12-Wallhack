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

// headers
#include "main.h" //hooks, helper functions
#include "overlay.h" //overlay, config, dllmain

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

    HRESULT STDMETHODCALLTYPE hookResizeBuffersD3D12(IDXGISwapChain3* pSwapChain,UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags)
    {
        return oResizeBuffersD3D12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookRSSetViewportsD3D12(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports) {
        gInitialized = true; //ready, do not rehook

        if (!pViewports || NumViewports == 0) {
            oRSSetViewportsD3D12(_this, NumViewports, pViewports);
            return;
        }

        if (NumViewports > 0 && pViewports) {
            t_.currentViewport = pViewports[0]; // cache viewport
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
                ? pViews[i].StrideInBytes: 0;
            //t_.currentGPUVAddress = pViews[0].BufferLocation;
        }

        if (pViews && NumViews > 0) {
            t_.CanonicalCount = BuildCanonicalStrides(pViews,NumViews,t_.CanonicalStrides,_countof(t_.CanonicalStrides)
            );

            if (t_.CanonicalCount > 0) {
                uint32_t h = HashStrides(t_.CanonicalStrides,t_.CanonicalCount
                );

                //hash
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
        if (!_this || t_.currentNumRTVs == 0 || _this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // 2. IDENTIFICATION (Short-circuiting logic)
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

            // 4. VIEWPORT EXECUTION, MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
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

    void STDMETHODCALLTYPE hookExecuteIndirectD3D12(ID3D12GraphicsCommandList* _this,ID3D12CommandSignature* pCommandSignature,UINT MaxCommandCount,ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,ID3D12Resource* pCountBuffer,UINT64 CountBufferOffset)
    {
        // 1. QUICK EXIT
        if (!_this || t_.currentNumRTVs == 0 || _this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

        // 2. IDENTIFICATION (Short-circuiting logic)
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

            // 4. VIEWPORT EXECUTION, MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
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
        //same as above, but games almost never draw models here
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
        // We use a GPU command to write '1' directly into the destination buffer. This avoids the CPU-GPU sync bottleneck.

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
    //Check for Binary Occlusion: Some games expect 0 or 1 (Binary), while others expect the actual number of samples passed (Occlusion). Writing a very large number like 0xFFFF 
    //can sometimes be more effective for standard occlusion to ensure the game treats it as "fully visible."
    //Handle Resource Barriers: If you use GPU commands to overwrite the data, you must ensure the pDestinationBuffer is in the D3D12_RESOURCE_STATE_COPY_DEST or COMMON state.
    //Performance: WriteBufferImmediate is excellent, but if NumQueries is large (e.g., 500+), it’s better to have a small "source" buffer containing many 1s and use CopyBufferRegion.

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDispatchD3D12(ID3D12GraphicsCommandList* pList, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
    {
        oDispatchD3D12(pList, threadCountX, threadCountY, threadCountZ);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDispatchMeshD3D12(ID3D12GraphicsCommandList6* cmd, UINT x, UINT y, UINT z)
    {
        oDispatchMeshD3D12(cmd, x, y, z);
    }

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


