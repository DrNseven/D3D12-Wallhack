#pragma once

namespace globals {
        extern HMODULE mainModule;
        extern HWND mainWindow;
        extern int uninjectKey;
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
        void SetDebugLogging(bool enable);
}

namespace hooks {
        extern void Init();
}

namespace inputhook {
        extern void Init(HWND hWindow);
        extern void Remove(HWND hWindow);
        static LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

namespace mousehooks {
        void Init();
        void Remove();
}

namespace d3d12hook {
        typedef HRESULT(STDMETHODCALLTYPE* PresentD3D12)(IDXGISwapChain3 * pSwapChain, UINT SyncInterval, UINT Flags);
        extern PresentD3D12 oPresentD3D12;

        typedef HRESULT(STDMETHODCALLTYPE* Present1Fn)(IDXGISwapChain3 * pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams);
        extern Present1Fn   oPresent1D3D12;

	    typedef void(STDMETHODCALLTYPE* ExecuteCommandListsFn)(ID3D12CommandQueue * _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
        extern ExecuteCommandListsFn oExecuteCommandListsD3D12;

        typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffersFn)(IDXGISwapChain3* pSwapChain,UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags);
        extern ResizeBuffersFn oResizeBuffersD3D12;

        typedef void (STDMETHODCALLTYPE* RSSetViewportsFn)(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
        extern RSSetViewportsFn oRSSetViewportsD3D12;

        typedef void(STDMETHODCALLTYPE* IASetVertexBuffersFn)(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
        extern IASetVertexBuffersFn oIASetVertexBuffersD3D12;

        typedef void(STDMETHODCALLTYPE* DrawIndexedInstancedFn)(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
        extern DrawIndexedInstancedFn oDrawIndexedInstancedD3D12;

        //1
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


        extern long __fastcall hookPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
        extern long __fastcall hookPresent1D3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pParams);
        extern void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
        extern HRESULT STDMETHODCALLTYPE hookResizeBuffersD3D12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
        extern void STDMETHODCALLTYPE hookRSSetViewportsD3D12(ID3D12GraphicsCommandList* _this, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
        extern void STDMETHODCALLTYPE hookIASetVertexBuffersD3D12(ID3D12GraphicsCommandList* _this, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
        extern void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* _this, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
        extern void STDMETHODCALLTYPE hookSetGraphicsRootConstantBufferViewD3D12(ID3D12GraphicsCommandList* _this, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);//4
        extern void STDMETHODCALLTYPE hookSetDescriptorHeapsD3D12(ID3D12GraphicsCommandList* cmdList, UINT NumHeaps, ID3D12DescriptorHeap* const* ppHeaps);
        extern void STDMETHODCALLTYPE hookSetGraphicsRootDescriptorTableD3D12(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
        extern void STDMETHODCALLTYPE hookOMSetRenderTargetsD3D12(ID3D12GraphicsCommandList* dCommandList, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
        extern void STDMETHODCALLTYPE hookResolveQueryDataD3D12(ID3D12GraphicsCommandList* self,ID3D12QueryHeap* pQueryHeap,D3D12_QUERY_TYPE Type,UINT StartIndex,UINT NumQueries,ID3D12Resource* pDestinationBuffer,UINT64 AlignedDestinationBufferOffset);
        extern void STDMETHODCALLTYPE hookSetGraphicsRootSignatureD3D12(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature);
        extern void STDMETHODCALLTYPE hookResetD3D12(ID3D12GraphicsCommandList* _this, ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState);
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

