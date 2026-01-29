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
    DrawInstancedFn         oDrawInstancedD3D12 = nullptr;
    DispatchFn              oDispatchD3D12 = nullptr;
    SetGraphicsRootConstantBufferViewFn oSetGraphicsRootConstantBufferViewD3D12 = nullptr;
    SetDescriptorHeapsFn    oSetDescriptorHeapsD3D12 = nullptr;
    SetGraphicsRootDescriptorTableFn oSetGraphicsRootDescriptorTableD3D12 = nullptr;
    SetComputeRootDescriptorTableFn oSetComputeRootDescriptorTableD3D12 = nullptr;
    OMSetRenderTargetsFn    oOMSetRenderTargetsD3D12 = nullptr;
    ResolveQueryDataFn      oResolveQueryDataD3D12 = nullptr;
    ExecuteIndirectFn       oExecuteIndirectD3D12 = nullptr;
    SetGraphicsRootSignatureFn oSetGraphicsRootSignatureD3D12 = nullptr;
    ClearUnorderedAccessViewUintFn oClearUnorderedAccessViewUintD3D12 = nullptr;
    ResetFn oResetD3D12 = nullptr;
    CloseFn oCloseD3D12 = nullptr;
    IASetIndexBufferFn      oIASetIndexBufferD3D12 = nullptr;
    SetPipelineStateFn      oSetPipelineStateD3D12 = nullptr;
    IASetPrimitiveTopologyFn oIASetPrimitiveTopologyD3D12 = nullptr;
    SetPredicationFn        oSetPredicationD3D12 = nullptr;
    BeginQueryFn            oBeginQueryD3D12 = nullptr;
    EndQueryFn              oEndQueryD3D12 = nullptr;
    //if unresolved external symbol d3d12hook::function, add fix here


    static ID3D12Device*            gDevice = nullptr;
    static ID3D12CommandQueue*      gCommandQueue = nullptr;
    static ID3D12DescriptorHeap*    gHeapRTV = nullptr;
    static ID3D12DescriptorHeap*    gHeapSRV = nullptr;
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

    void STDMETHODCALLTYPE hookSetPipelineStateD3D12(ID3D12GraphicsCommandList* _this,ID3D12PipelineState* pso)
    {
        // cache for fast filtering
        t_.currentPSO = pso;

        return oSetPipelineStateD3D12(_this, pso);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookIASetPrimitiveTopologyD3D12(ID3D12GraphicsCommandList* _this, D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
    {
        t_.currentTopology = PrimitiveTopology;
      
        return oIASetPrimitiveTopologyD3D12(_this, PrimitiveTopology);
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
        /*
        //for fucked up games, where it draws in unknown function (doesn't really happen)
        if (countindexformat > 0 && t_.currentNumRTVs > 0) {
            UINT currentindexformat = t_.currentIndexFormat + t_.numViewports;

            if (currentindexformat == countindexformat &&
                t_.currentViewport.Width > 128 &&
                t_.currentViewport.Height > 128 &&
                t_.currentViewport.Width < 16384)
            {
                D3D12_VIEWPORT hVp = t_.currentViewport;

                hVp.MinDepth = reversedDepth ? 0.0f : 0.9f;
                hVp.MaxDepth = reversedDepth ? 0.1f : 1.0f;

                // submit modified viewport ONCE
                oRSSetViewportsD3D12(_this, 1, &hVp);
                return;
            }
        }
        */
        oRSSetViewportsD3D12(_this, NumViewports, pViewports);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookIASetVertexBuffersD3D12(ID3D12GraphicsCommandList* _this,UINT StartSlot,UINT NumViews,const D3D12_VERTEX_BUFFER_VIEW* pViews)
    {
        // 1. Reset current state
        t_.StartSlot = StartSlot;
        t_.numViews = NumViews;
        //t_.cachedStrideSum = 0;

        // Clear old data
        for (int i = 0; i < 16; ++i) {
            t_.Strides[i] = 0;
            //t_.vertexBufferSizes[i] = 0;
        }

        // 2. Process views
        if (NumViews > 0 && pViews) {
            uint32_t strideData[16] = {};
            UINT validCount = (NumViews > 16) ? 16 : NumViews;

            for (UINT i = 0; i < validCount; ++i) {
                // Filter out clearly invalid strides or huge raw buffers
                if (pViews[i].StrideInBytes > 0 && pViews[i].StrideInBytes <= 999) {
                    t_.Strides[i] = pViews[i].StrideInBytes;
                    //t_.vertexBufferSizes[i] = pViews[i].SizeInBytes;

                    strideData[i] = pViews[i].StrideInBytes;
                    //t_.cachedStrideSum += pViews[i].StrideInBytes;
                }
            }

            // 3. Generate a High-Precision Hash
            // Use the full 32-bit result for internal logic to prevent flickering
            t_.StrideHash = fastStrideHash(strideData, validCount);
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

        // Filter only Direct Command Lists (standard rendering)
        if (dCommandList->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            t_.currentNumRTVs = NumRenderTargetDescriptors;

            // 1. Capture DSV State (important performance filter)
            if (pDepthStencilDescriptor != nullptr && pDepthStencilDescriptor->ptr != 0) {
                t_.currentDSVHandle = *pDepthStencilDescriptor;
                t_.hasDSV = true;
            }
            else {
                t_.hasDSV = false;
                t_.currentDSVHandle.ptr = 0;
            }
            /*
            // 2. Capture RTV Handles (Optional, if you need to know WHERE it renders)
            if (pRenderTargetDescriptors != nullptr && NumRenderTargetDescriptors > 0)
            {
                if (RTsSingleHandleToDescriptorRange) {
                    t_.currentRTVHandles[0] = pRenderTargetDescriptors[0];
                }
                else {
                    UINT count = (NumRenderTargetDescriptors > 8) ? 8 : NumRenderTargetDescriptors;
                    for (UINT i = 0; i < count; ++i) {
                        t_.currentRTVHandles[i] = pRenderTargetDescriptors[i];
                    }
                }
            }
            */
        }

        return oOMSetRenderTargetsD3D12(dCommandList, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        // --- LEVEL 1: REGISTER-ONLY CHECKS (ZERO COST) ---
        // These use values already in CPU registers (passed as arguments).
        // We check InstanceCount and IndexCount first because they require NO memory lookups.
        if (!_this || InstanceCount == 0 || InstanceCount > 5 || IndexCountPerInstance < 100)
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // --- LEVEL 2: COMMAND LIST TYPE ---
        // GetType() is a virtual call. Fast, but slightly more expensive than a register check.
        if (_this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // --- LEVEL 3: CACHED STATE CHECKS (LOW COST) ---
        // These look up your thread_local 't_' struct. 
        // Filter 3 & 4 combined: If no RTV or no Depth, it's UI/Post-FX/Shadows.
        if (t_.currentNumRTVs == 999 || !t_.hasDSV) //sadly t_.currentNumRTVs == 0 can be models, usually big performance boost if we filter 0 out
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // --- LEVEL 4: VIEWPORT CHECKS (MEDIUM COST) ---
        // Floating point comparisons and slightly deeper memory offset.
        // Filters out reflection probes, tiny icons, and sub-renders.
        if (t_.currentViewport.Width < 500.0f)
        {
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        // --- LEVEL 5
        if (t_.currentTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

        // --- LEVEL 6
        if (!t_.currentPSO) //Guard against t_.currentPSO == nullptr
            return oDrawIndexedInstancedD3D12(_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

        ID3D12PipelineState* currentPSO = t_.currentPSO;
        PSOStats* stats = nullptr;
        // 1. Find existing PSO (linear scan)
        for (UINT i = 0; i < psoCount; ++i)
        {
            if (psoStats[i].pso == currentPSO)
            {
                stats = &psoStats[i];

                // --- MOVE-TO-FRONT HEURISTIC ---
                if (i > 0)
                {
                    PSOStats tmp = psoStats[i];
                    psoStats[i] = psoStats[0];
                    psoStats[0] = tmp;

                    stats = &psoStats[0];
                }
                break;
            }
        }
        // 2. Create new entry if not found
        if (!stats)
        {
            if (psoCount < _countof(psoStats))
            {
                stats = &psoStats[psoCount++];
                stats->pso = currentPSO;
                stats->maxIndexCount = 0;
            }
        }
        // 3. Update heuristic + filter
        if (stats)
        {
            if (IndexCountPerInstance > stats->maxIndexCount)
                stats->maxIndexCount = IndexCountPerInstance;

            // Skip heavy logic for tiny meshes
            if (stats->maxIndexCount < 300)
            {
                return oDrawIndexedInstancedD3D12(
                    _this,
                    IndexCountPerInstance,
                    InstanceCount,
                    StartIndexLocation,
                    BaseVertexLocation,
                    StartInstanceLocation
                );
            }
        }


        // 2. IDENTIFICATION
        const UINT currentStrides = t_.StrideHash + t_.StartSlot;

        bool isModelDraw = false;

        if (currentStrides == countstride1 || currentStrides == countstride2 ||
            currentStrides == countstride3 || currentStrides == countstride4 ||
            tls_cache.lastRDTindex == countrootdescriptor || tls_cache.lastRCBVindex == countconstantbuffer || //tls_cache.lastCRDTindex == countcomputerootdescriptor ||
            t_.currentNumRTVs == countfindrendertarget|| IndexCountPerInstance / 500 == countIndexCount)
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
            // 3. FILTER LOGIC
            // If any filter fails, we immediately jump to the original draw
            if (filternumViews && t_.numViews != countfilternumViews) goto skip;
            if (ignorenumViews && t_.numViews == countignorenumViews) goto skip;
            if (filternumViewports && t_.numViewports != countfilternumViewports) goto skip;
            if (ignorenumViewports && t_.numViewports == countignorenumViewports) goto skip;
            if (filterrendertarget && t_.currentNumRTVs != countfilterrendertarget) goto skip;
            if (ignorerendertarget && t_.currentNumRTVs == countignorerendertarget) goto skip;

            // Root Descriptor/Constant Filters
            if (filterRootDescriptor && (tls_cache.lastRDTindex != countfilterrootDescriptor && tls_cache.lastRDTindex != countfilterrootDescriptor2 && tls_cache.lastRDTindex != countfilterrootDescriptor3)) goto skip;
            if (ignoreRootDescriptor && (tls_cache.lastRDTindex == countignorerootDescriptor || tls_cache.lastRDTindex == countignorerootDescriptor2 || tls_cache.lastRDTindex == countignorerootDescriptor3)) goto skip;
            if (filterRootConstant && (tls_cache.lastRCBVindex != countfilterrootConstant && tls_cache.lastRCBVindex != countfilterrootConstant2 && tls_cache.lastRCBVindex != countfilterrootConstant3)) goto skip;
            if (ignoreRootConstant && (tls_cache.lastRCBVindex == countignorerootConstant || tls_cache.lastRCBVindex == countignorerootConstant2 || tls_cache.lastRCBVindex == countignorerootConstant3)) goto skip;

            // 4. VIEWPORT EXECUTION, MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
            const D3D12_VIEWPORT originalVp = t_.currentViewport;

            if (originalVp.Width >= 128 && originalVp.Width < 16384)
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
        // 1. Mandatory Exits
        if (!_this || MaxCommandCount == 0)
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // 2. Command List Type
        if (_this->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // 3. State Filters (The most important ones here!)
        // These ensure we aren't looking at UI, Shadows, or Post-Processing
        // Be careful with NumRTVs == 0 (Good for perfromance but RISKY)
        // If the game uses a Depth - Pre - Pass(common in Warzone, Halo, Battlefield), the players are drawn to the Depth buffer first with 0 RTVs.
        // If you filter this, you will miss them
        if (t_.currentNumRTVs == 999 || !t_.hasDSV || t_.currentViewport.Width < 500.0f) //sadly t_.currentNumRTVs == 0 can be models, usually big performance boost if we filter 0 out
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // 4. Batch Size Filter
        // Environment meshes (grass, etc) are usually drawn in large batches.
        // Players/Characters are usually drawn in small batches or single calls.
        if (MaxCommandCount > 50) //?
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // 5.
        if (t_.currentTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // 6.
        if (!t_.currentPSO)
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

        // PSO Logic (Simplified for ExecuteIndirect)
        PSOStats* stats = nullptr;
        for (UINT i = 0; i < psoCount; ++i) {
            if (psoStats[i].pso == t_.currentPSO) {
                stats = &psoStats[i];
                break;
            }
        }

        // 6. Only filter if we are CERTAIN this PSO is for tiny objects (UI)
        // We learned this certainty from the DrawIndexedInstanced hook!
        if (stats && stats->maxIndexCount > 0 && stats->maxIndexCount < 100) {
            return oExecuteIndirectD3D12(_this, pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
        }

        // 7. Signature Pointer Filter (Optional but recommended)
        // if (pCommandSignature == knownComputeSignature) return ...;

        
        // 2. IDENTIFICATION
        const UINT currentStrides = t_.StrideHash + t_.StartSlot;
        //uint8_t shortCount = static_cast<uint8_t>((MaxCommandCount >> 12) % 100);
        bool isModelDraw = false;

        if (currentStrides == countstride1 || currentStrides == countstride2 ||
            currentStrides == countstride3 || currentStrides == countstride4 ||
            tls_cache.lastRDTindex == countrootdescriptor || tls_cache.lastRCBVindex == countconstantbuffer || //tls_cache.lastCRDTindex == countcomputerootdescriptor ||
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
            // 3. FILTER LOGIC
            // If any filter fails, we immediately jump to the original draw
            if (filternumViews && t_.numViews != countfilternumViews) goto skip;
            if (ignorenumViews && t_.numViews == countignorenumViews) goto skip;
            if (filternumViewports && t_.numViewports != countfilternumViewports) goto skip;
            if (ignorenumViewports && t_.numViewports == countignorenumViewports) goto skip;
            if (filterrendertarget && t_.currentNumRTVs != countfilterrendertarget) goto skip;
            if (ignorerendertarget && t_.currentNumRTVs == countignorerendertarget) goto skip;

            // Root Descriptor/Constant Filters
            if (filterRootDescriptor && (tls_cache.lastRDTindex != countfilterrootDescriptor && tls_cache.lastRDTindex != countfilterrootDescriptor2 && tls_cache.lastRDTindex != countfilterrootDescriptor3)) goto skip;
            if (ignoreRootDescriptor && (tls_cache.lastRDTindex == countignorerootDescriptor || tls_cache.lastRDTindex == countignorerootDescriptor2 || tls_cache.lastRDTindex == countignorerootDescriptor3)) goto skip;
            if (filterRootConstant && (tls_cache.lastRCBVindex != countfilterrootConstant && tls_cache.lastRCBVindex != countfilterrootConstant2 && tls_cache.lastRCBVindex != countfilterrootConstant3)) goto skip;
            if (ignoreRootConstant && (tls_cache.lastRCBVindex == countignorerootConstant || tls_cache.lastRCBVindex == countignorerootConstant2 || tls_cache.lastRCBVindex == countignorerootConstant3)) goto skip;

            // 4. VIEWPORT EXECUTION, MUST BE A COPY, NOT A REFERENCE, to prevent flickering/state corruption
            const D3D12_VIEWPORT originalVp = t_.currentViewport;

            if (originalVp.Width >= 128 && originalVp.Width < 16384)
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

    void STDMETHODCALLTYPE hookDrawInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
    {
       
        oDrawInstancedD3D12(_this, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootConstantBufferViewD3D12(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

        // 1. Sync the command list context
        if (tls_cache.cmdListPtr != _this) {
            tls_cache.cmdListPtr = _this;
            // Reset to a sentinel value because we don't know the state of this new list
            tls_cache.lastRCBVindex = UINT_MAX;
        }

        // 2. Only update the cache if the address is valid
        if (BufferLocation != 0) {
            tls_cache.lastRCBVindex = RootParameterIndex;
            // tls_cache.currentGPURAddress = BufferLocation; // Useful if you need to track address ranges
        }

        return oSetGraphicsRootConstantBufferViewD3D12(_this, RootParameterIndex, BufferLocation);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {

        // 1. Update the command list context if it switched
        if (tls_cache.cmdListPtr != dCommandList) {
            tls_cache.cmdListPtr = dCommandList;
            tls_cache.lastRDTindex = UINT_MAX; // Reset index for the new list
        }

        // 2. Only cache and pass through if it's a valid handle
        // or if you specifically want to track the 'null' state.
        if (BaseDescriptor.ptr != 0) {
            if (tls_cache.lastRDTindex != RootParameterIndex) {
                tls_cache.lastRDTindex = RootParameterIndex;
            }
        }

        return oSetGraphicsRootDescriptorTableD3D12(dCommandList, RootParameterIndex, BaseDescriptor);
    }

    //=========================================================================================================================//

    //Claim: The Solution for CPU Culling :
    //You need to hook ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable and look for the constant buffers that pass culling parameters to the GPU, 
    //or find the engine's internal flags (CVars) in memory and flip them to 0.

    void STDMETHODCALLTYPE hookSetComputeRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList,UINT RootParameterIndex,D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
    {
        // 1. Sync the command list context
        if (tls_cache.cmdListPtr != dCommandList) {
            tls_cache.cmdListPtr = dCommandList;
            // It's safer to reset all indices when the command list switches
            tls_cache.lastCRDTindex = UINT_MAX;
        }

        // 2. Access the .ptr member for the null check
        if (BaseDescriptor.ptr != 0) {
            tls_cache.lastCRDTindex = RootParameterIndex;
        }

        return oSetComputeRootDescriptorTableD3D12(dCommandList, RootParameterIndex, BaseDescriptor);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookSetGraphicsRootSignatureD3D12(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature) {

        if (!dCommandList || !pRootSignature) 
            return oSetGraphicsRootSignatureD3D12(dCommandList, pRootSignature);

        // 1. Always sync the command list pointer
        tls_cache.cmdListPtr = dCommandList;

        // 2. INVALIDATE the cached indices
        // Because the new signature might have different bindings at these indices
        tls_cache.lastRDTindex = UINT_MAX;
        tls_cache.lastRCBVindex = UINT_MAX;
        //tls_cache.lastCRDTindex = UINT_MAX;

        // 3. Optional: Cache the root signature itself if you need to check types later
        //tls_cache.currentRootSig = pRootSignature;

        if ((countcurrentindexid > 0 || countcurrentindexid2 > 0 || countcurrentindexid3 > 0) && (dCommandList && pRootSignature)) {
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

        // ALWAYS sync the pointer
        tls_cache.cmdListPtr = _this;

        // ALWAYS reset the indices
        // A Reset() command list has no state. Stale cache = bugs.
        tls_cache.lastRCBVindex = UINT_MAX;
        tls_cache.lastRDTindex = UINT_MAX;
        //tls_cache.lastCRDTindex = UINT_MAX;

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
    
    //occlusion culling try 1 
    void STDMETHODCALLTYPE hookResolveQueryDataD3D12(
        ID3D12GraphicsCommandList* self,
        ID3D12QueryHeap* pQueryHeap,
        D3D12_QUERY_TYPE Type,
        UINT StartIndex,
        UINT NumQueries,
        ID3D12Resource* pDestinationBuffer,
        UINT64 AlignedDestinationBufferOffset)
    {
        
        // Filter: Only handle occlusion-related queries
        if (DisableOcclusionCulling && (Type == D3D12_QUERY_TYPE_OCCLUSION || Type == D3D12_QUERY_TYPE_BINARY_OCCLUSION)) {

            // WriteBufferImmediate is not allowed in Bundles
            if (self->GetType() != D3D12_COMMAND_LIST_TYPE_BUNDLE) {

                ID3D12GraphicsCommandList2* cl2 = nullptr;
                if (SUCCEEDED(self->QueryInterface(IID_PPV_ARGS(&cl2)))) {

                    UINT64 visibleValue = 1ull; // Default to 1

                    if (Type == D3D12_QUERY_TYPE_OCCLUSION) {
                        // Standard occlusion: use a high pixel count to pass engine thresholds
                        visibleValue = 0xFFFFFFFFull;
                    }
                    else {
                        // Binary occlusion: strictly 1
                        visibleValue = 1ull;
                    }

                    const UINT batchSize = 32;
                    D3D12_WRITEBUFFERIMMEDIATE_PARAMETER params[batchSize];
                    UINT64 gpuAddrBase = pDestinationBuffer->GetGPUVirtualAddress() + AlignedDestinationBufferOffset;

                    for (UINT i = 0; i < NumQueries; i += batchSize) {
                        UINT currentBatchCount = (NumQueries - i) < batchSize ? (NumQueries - i) : batchSize;

                        for (UINT j = 0; j < currentBatchCount; ++j) {
                            params[j].Dest = gpuAddrBase + ((UINT64(i) + j) * sizeof(UINT64));
                            params[j].Value = visibleValue;
                        }

                        cl2->WriteBufferImmediate(currentBatchCount, params, nullptr);
                    }

                    cl2->Release();
                    return; // Successfully spoofed, skip original call
                }
            }
        }
        
        oResolveQueryDataD3D12(self, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
    }
    
    //=========================================================================================================================//

    //occlusion culling try 2 (most games do not call this, so it's useless)
    void STDMETHODCALLTYPE hookSetPredicationD3D12(ID3D12GraphicsCommandList* self, ID3D12Resource* pBuffer, UINT64 AlignedBufferOffset, D3D12_PREDICATION_OP Operation)
    {
        //if (pBuffer != nullptr)
        //{
            // Disable predication -> draw everything
            //oSetPredicationD3D12(self, nullptr, 0, Operation);
            //return;
        //}

        oSetPredicationD3D12(self, pBuffer, AlignedBufferOffset, Operation);
    }

    //=========================================================================================================================//

    //occlusion culling try 3 (can cause black screen)
    HRESULT STDMETHODCALLTYPE hookMapD3D12(ID3D12Resource* self, UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData)
    {
        HRESULT hr = oMapD3D12(self, Subresource, pReadRange, ppData);
        /*
        // 1. Basic Safety
        if (FAILED(hr) || !ppData || !*ppData)
            return hr;

        // 2. Resource/Heap Validation
        D3D12_RESOURCE_DESC desc = self->GetDesc();
        if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            return hr;

        D3D12_HEAP_PROPERTIES heapProps;
        if (FAILED(self->GetHeapProperties(&heapProps, nullptr)))
            return hr;

        if (heapProps.Type != D3D12_HEAP_TYPE_READBACK)
            return hr;

        // 3. Determine actual range to process
        SIZE_T begin = 0;
        SIZE_T end = (SIZE_T)desc.Width;

        if (pReadRange != nullptr) {
            // If End <= Begin, the application is not reading data. Skip to avoid sync issues.
            if (pReadRange->End <= pReadRange->Begin)
                return hr;

            begin = pReadRange->Begin;
            end = min((SIZE_T)desc.Width, pReadRange->End);
        }

        SIZE_T size = end - begin;

        // 4. Final Heuristic Check
        // - Must be at least 8 bytes
        // - Must be 8-byte aligned (to prevent CPU crashes on dereference)
        // - Max 64KB (Adjusted from 32KB, still very safe for occlusion but covers more queries)
        if (size >= 8 && (size % 8 == 0) && (begin % 8 == 0) && size <= 65536)
        {
            uint64_t* dataPtr = (uint64_t*)((uint8_t*)(*ppData) + begin);
            size_t count = size / 8;

            for (size_t i = 0; i < count; ++i) {
                // ONLY modify if 0. 
                // This is the primary protection against black screens.
                if (dataPtr[i] == 0) {
                    dataPtr[i] = 1;
                }
            }
        }
        */
        return hr;
    }

    //=========================================================================================================================//
    
    //Warning, this doesn't work, can have the opposite effect
    void STDMETHODCALLTYPE hookBeginQueryD3D12(
        ID3D12GraphicsCommandList* self,
        ID3D12QueryHeap* pQueryHeap,
        D3D12_QUERY_TYPE Type,
        UINT Index)
    {
        //if (Type == D3D12_QUERY_TYPE_OCCLUSION || Type == D3D12_QUERY_TYPE_BINARY_OCCLUSION) 
        //{ return; }

        oBeginQueryD3D12(self, pQueryHeap, Type, Index);
    }

    void STDMETHODCALLTYPE hookEndQueryD3D12(
        ID3D12GraphicsCommandList* self,
        ID3D12QueryHeap* pQueryHeap,
        D3D12_QUERY_TYPE Type,
        UINT Index)
    {
        //if (Type == D3D12_QUERY_TYPE_OCCLUSION || Type == D3D12_QUERY_TYPE_BINARY_OCCLUSION) 
        //{ return; }

        oEndQueryD3D12(self, pQueryHeap, Type, Index);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookClearUnorderedAccessViewUintD3D12(
        ID3D12GraphicsCommandList* cl,
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
        ID3D12Resource* resource,
        const UINT values[4],
        UINT numRects,
        const D3D12_RECT* rects)
    {
        oClearUnorderedAccessViewUintD3D12(cl, gpuHandle, cpuHandle, resource, values, numRects, rects);
    }

    //=========================================================================================================================//

    void STDMETHODCALLTYPE hookDispatchD3D12(ID3D12GraphicsCommandList* cl, UINT x, UINT y, UINT z)
    {
        oDispatchD3D12(cl, x, y, z);
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


