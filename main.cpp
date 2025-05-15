///////////////////////////////
// D3D12 Wallhack 2025 by N7 //
///////////////////////////////

#include "main.h"

//=========================================================================================================================//

typedef void(STDMETHODCALLTYPE* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
ExecuteCommandLists oExecuteCommandLists = NULL;

typedef HRESULT(STDMETHODCALLTYPE* Present12) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
Present12 oPresent = NULL;

typedef void(STDMETHODCALLTYPE* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
DrawIndexedInstanced oDrawIndexedInstanced = NULL;

typedef void(STDMETHODCALLTYPE* DrawInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
DrawInstanced oDrawInstanced = NULL;

typedef void (STDMETHODCALLTYPE* RSSetViewportsFunc)(ID3D12GraphicsCommandList* dCommandList, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
RSSetViewportsFunc oRSSetViewports = nullptr;

typedef void (STDMETHODCALLTYPE* SetGraphicsRootSignature)(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature);
SetGraphicsRootSignature oSetGraphicsRootSignature = nullptr;

typedef void (STDMETHODCALLTYPE* SetGraphicsRootConstantBufferView)(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
SetGraphicsRootConstantBufferView oSetGraphicsRootConstantBufferView;

typedef void(STDMETHODCALLTYPE* OMSetRenderTargets)(ID3D12GraphicsCommandList* dCommandList, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
OMSetRenderTargets oOMSetRenderTargets = NULL;

typedef void(STDMETHODCALLTYPE* IASetVertexBuffers)(ID3D12GraphicsCommandList* dCommandList, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
IASetVertexBuffers oIASetVertexBuffers = NULL;

typedef void(STDMETHODCALLTYPE* IASetIndexBuffer)(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView);
IASetIndexBuffer oIASetIndexBuffer = NULL;

typedef HRESULT(STDMETHODCALLTYPE* CreateGraphicsPipelineState)(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState);
CreateGraphicsPipelineState oCreateGraphicsPipelineState = NULL;

typedef void(STDMETHODCALLTYPE* SetPipelineState)(ID3D12GraphicsCommandList* dCommandList, ID3D12PipelineState* pPipelineState);
SetPipelineState oSetPipelineState = NULL;

typedef void (STDMETHODCALLTYPE* CreateShaderResourceView_t)(ID3D12Device*,ID3D12Resource*,const D3D12_SHADER_RESOURCE_VIEW_DESC*,D3D12_CPU_DESCRIPTOR_HANDLE);
CreateShaderResourceView_t oCreateShaderResourceView = nullptr;

typedef void (STDMETHODCALLTYPE* SetDescriptorHeaps)(ID3D12GraphicsCommandList* dCommandList, UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps);
SetDescriptorHeaps oSetDescriptorHeaps;

typedef HRESULT(STDMETHODCALLTYPE* CreateCommittedResource)(
    ID3D12Device* pDevice,
    const D3D12_HEAP_DESC* pDesc,
    const D3D12_HEAP_PROPERTIES* pHeapProperties, 
    const D3D12_RESOURCE_DESC* pResourceDesc,    
    D3D12_RESOURCE_STATES InitialResourceState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID riidResource,
    void** ppvResource
    );
CreateCommittedResource oCreateCommittedResource = NULL;

typedef HRESULT(STDMETHODCALLTYPE* CreatePlacedResource)(
    ID3D12Device* pDevice,
    ID3D12Heap* pHeap,
    UINT64                    HeapOffset,
    const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES     InitialState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID                    riid,
    void** ppvResource
    );
CreatePlacedResource oCreatePlacedResource = NULL;

typedef void (STDMETHODCALLTYPE* SetGraphicsRootShaderResourceView)(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
SetGraphicsRootShaderResourceView oSetGraphicsRootShaderResourceView;

typedef void (STDMETHODCALLTYPE* SetComputeRootConstantBufferView)(
    ID3D12GraphicsCommandList* pCommandList,
    UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation
    );
SetComputeRootConstantBufferView oSetComputeRootConstantBufferView = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* CreateConstantBufferView)(ID3D12Device* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
CreateConstantBufferView oCreateConstantBufferView = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* ResolveQueryData)(ID3D12GraphicsCommandList* self,
    ID3D12QueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    ID3D12Resource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset);
ResolveQueryData oResolveQueryData = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* CopyBufferRegion)(
    ID3D12GraphicsCommandList* self,
    ID3D12Resource* pDstBuffer,
    UINT64 DstOffset,
    ID3D12Resource* pSrcBuffer,
    UINT64 SrcOffset,
    UINT64 NumBytes);
CopyBufferRegion oCopyBufferRegion = nullptr;

//=========================================================================================================================//

bool vkENDkeydown = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    if (ShowMenu) {
        ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam); //ImGui gets first crack at the message
    }
    
    switch (uMsg) {
    case WM_KEYDOWN: {
        switch (wParam) {

        case VK_INSERT: //insert key
            wallh = !wallh;
            break;

        case VK_OEM_COMMA: //, key
            countnum--;
            break;
        case VK_OEM_PERIOD: //. key
            countnum++;
            break;
        case '9':
            countnum = -1;
            break;
        case VK_END:
            vkENDkeydown = true;
            break;
        }
        break;
    }

    case WM_KEYUP: {
        if (wParam == VK_END) {
            vkENDkeydown = false; 
        }
        break;
    }
    }
    
    if (Process::WndProc) {
        return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (!commandQueue)
    commandQueue = queue;

	return oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

//=========================================================================================================================//

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    //present is not called in unity games

    return oPresent(pSwapChain, SyncInterval, Flags);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {

	static bool initialized = false;
	if (!initialized) {
		//get device
		HRESULT hr = dCommandList->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
		if (FAILED(hr)) {
			Log("Failed to get ID3D12Device from command list, HRESULT: 0x%08X", hr);
			return;
		}

        //load custom stuff
        //ID3D12RootSignature* rawRootSig = nullptr;
        //ID3D12PipelineState* rawPSO = nullptr;
        //bool initSuccess = InitGreenOverlayPipeline(pDevice.Get(), &rawRootSig, &rawPSO);
        //if (initSuccess && rawRootSig && rawPSO) {
            //g_rootSig = rawRootSig;
            //g_greenPSO = rawPSO;
        //}

        //color stuff
        CreateCustomConstantBuffer();

		initialized = true;
	}

    
    //rootIndex
    UINT rootIndex = UINT_MAX;
    if (dCommandList) {
        CommandListSpecificData retrievedData;
        UINT dataSize = sizeof(retrievedData); // Pass the size of our buffer

        HRESULT hr = dCommandList->GetPrivateData(MyCommandListPrivateDataGuid, &dataSize, &retrievedData);

        if (SUCCEEDED(hr)) {
            if (dataSize == sizeof(CommandListSpecificData)) {
                // Successfully retrieved the struct and the size matches
                rootIndex = retrievedData.lastCbvRootParameterIndex;
            }
            else {
                // Data was found, but the size doesn't match what we expect.
                // Log("Warning: Private data size mismatch for GUID. Expected %u, got %u.", sizeof(CommandListSpecificData), dataSize);
                rootIndex = UINT_MAX; // Or handle as an error
            }
        }
        else {
            // GetPrivateData failed (e.g., no data set for this GUID, or other error)
            // hr might be DXGI_ERROR_NOT_FOUND if no data was set.
            // Log("Failed to get private data or no data found. HRESULT: 0x%X", hr);
            rootIndex = UINT_MAX; // Ensure it's default
        }
    }
    

    // Retrieve state from TLS
    UINT NumViews = t_cLS.NumViews;
    UINT StartSlot = t_cLS.StartSlot;
    DXGI_FORMAT currentiFormat = t_cLS.currentIndexFormat;
    UINT currentiSize = t_cLS.currentiSize/2;
    // Sum all the current strides
    UINT currentStride = 0;
    for (int i = 0; i < 7; ++i) {
        currentStride += t_cLS.vertexStrides[i];
    }
    //include StartSlot for Unity
    Strides = currentStride + StartSlot;
    // For example, get the first buffer's size if needed
    UINT currentvSize = t_cLS.vertexBufferSizes[0]/2;
    //int twoDigitSize = getTwoDigitValue(currentiSize);
    int twoDigitSize = getTwoDigitValue(IndexCountPerInstance);
 

    //viewport
    float vpWidth = 0.0f;
    float vpHeight = 0.0f;
    {
        std::lock_guard<std::mutex> lock(gViewportMutex);
        auto it = gViewportMap.find(dCommandList);
        if (it != gViewportMap.end()) {
            vpWidth = it->second.Width;
            vpHeight = it->second.Height;
        }
    }
    

    /*
    //COLORING
    //For some ue5 games, add rootIndex and bruteforce colorOffset by using: colorOffset == countnum
    //1. apply color first (ue5), use rootIndex > 0 to avoid color flickering
    // Define the matrix
    DirectX::XMFLOAT4X4 identity;
    DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());

    // Define the color
    DirectX::XMFLOAT4 newColor = { 125.0f, 125.0f, 125.0f, 1.0f }; // Green

    // Define the offsets (example values; these must be accurate)
    size_t matrixOffset = 0;        // Identity matrix, bruteforce this too if colors not working
    size_t colorOffset = countnum;  // bruteforce this value to change colors 0-200?

    // Only proceed if buffer is mapped and offsets are valid
    if ((Strides == countnum || rootIndex == countnum || twoDigitSize == countnum) && (rootIndex))
        if (g_pMappedConstantBuffer &&
            matrixOffset + sizeof(identity) <= g_constantBufferSize &&
            colorOffset + sizeof(newColor) <= g_constantBufferSize) {

            // Copy identity matrix into buffer
            memcpy(g_pMappedConstantBuffer + matrixOffset, &identity, sizeof(identity));

            // Copy color into buffer
            memcpy(g_pMappedConstantBuffer + colorOffset, &newColor, sizeof(newColor));

            // Set GPU address
            D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = g_pCustomConstantBuffer->GetGPUVirtualAddress();

            // Bind the buffer
            dCommandList->SetGraphicsRootConstantBufferView(rootIndex, bufferAddress);
        }
    */
    

    //2. apply wallhack 
    if (Strides == countnum || rootIndex == countnum /* || twoDigitSize == countnum*/) {
    //if ((Strides == 40 || Strides == 48) && (rootIndex == countnum && IndexCountPerInstance > 9)) { //brute force models, hold . key
        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = vpWidth;
        viewport.Height = vpHeight;
        viewport.MinDepth = 0.9f; //or 1.0f
        viewport.MaxDepth = 1.0f;
        dCommandList->RSSetViewports(1, &viewport);

        oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

        //reset the viewport
        viewport.MinDepth = 0.0f;
        dCommandList->RSSetViewports(1, &viewport);
    }



    // Logger
    //1. use keys comma (,) and period (.) to cycle through textures
    //2. log current values by pressing END
    if (vkENDkeydown && (Strides == countnum || rootIndex == countnum || twoDigitSize == countnum)) {
        Log("countnum == %d && Strides == %d && rootIndex == %d && IndexCountPerInstance == %d && twoDigitSize == %d && currentiSize == %d && currentvSize == %d && currentiFormat == %d && StartIndexLocation == %d && NumViews == %d",
            countnum, Strides, rootIndex, IndexCountPerInstance, twoDigitSize, currentiSize, currentvSize, currentiFormat, StartIndexLocation, NumViews);
    }
    

    //This is classic wallhack, but doesn't work if game caches pipeline offline ect. (ue5)
    //Try to hijack games pipeline (unity)
    if (Strides == 9999 || rootIndex == 9999) { //brute force values
        // 1. Get the PSO that should be currently active for this command list
        {
            std::lock_guard<std::mutex> lock(commandListPSOMutex);
            auto it = currentCommandListPSO.find(dCommandList);
            if (it != currentCommandListPSO.end()) {
                originalPSO = it->second;
            }
        } // Release commandListPSOMutex lock

        // 2. Find the green variant for this PSO
        if (originalPSO) {
            std::lock_guard<std::mutex> lock(pipelineMutex);
            auto it = greenVariants.find(originalPSO);
            if (it != greenVariants.end()) {
                greenPSO = it->second;
            }
        } // Release pipelineMutex lock

        // 3. If we found a green variant, swap to it
        if (greenPSO) {
            oSetPipelineState(dCommandList, greenPSO); // Set the green PSO
            swapped = true;
            //Log("DrawIndexedInstanced (IndexCount: %u): Swapped to Green PSO %p (Original: %p)", IndexCountPerInstance, greenPSO, originalPSO);
        }
        else {
            //Log("DrawIndexedInstanced (IndexCount: %u): Target draw call, but Green PSO not found for Original PSO %p", IndexCountPerInstance, originalPSO);
        }

        // 4. Call the original draw function (with either original or green PSO active)
        //oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

        // 5. If we swapped, restore the original PSO (removes colors too)
        //if (swapped) {
            //oSetPipelineState(dCommandList, originalPSO); // Restore the original PSO
            // Log("DrawIndexedInstanced (IndexCount: %u): Restored Original PSO %p", IndexCountPerInstance, originalPSO);
        //}
    }

    /*
    //green custom shader test (bad)
    if(currentStride == countnum || rootIndex == countnum || twoDigitSize == countnum)
    {
        if(g_greenPSO)
        dCommandList->SetPipelineState(g_greenPSO.Get());

        dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //dCommandList->IASetVertexBuffers(0, 0, nullptr);
        //dCommandList->IASetIndexBuffer(nullptr);

        // Draw a fullscreen triangle using 3 vertices generated in the Vertex Shader
        dCommandList->DrawInstanced(3, 1, 0, 0);
        //return;
    }
    */

    /*
    //try to fook textures to green (bad)
    if(currentStride == countnum || rootIndex == countnum || twoDigitSize == countnum) {

        float mColor[] = { 0.0f, 1.0f, 0.0f, 0.0f }; // Green
        dCommandList->ClearRenderTargetView(g_RTVHandle, mColor, 0, nullptr);
        dCommandList->SetGraphicsRootConstantBufferView(?, 0);//countnum+13
    }
    */


	return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {

    return oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkRSSetViewports(ID3D12GraphicsCommandList* dCommandList,UINT NumViewports,const D3D12_VIEWPORT* pViewports) {

    if (NumViewports > 0 && pViewports) {
        std::lock_guard<std::mutex> lock(gViewportMutex);
        gViewportMap[dCommandList] = pViewports[0];  // assume single viewport
    }
    
    return oRSSetViewports(dCommandList, NumViewports, pViewports);  // Call original function
}

//=========================================================================================================================//

HRESULT STDMETHODCALLTYPE hkCreatePlacedResource(
    ID3D12Device* pDevice,
    ID3D12Heap* pHeap,
    UINT64                    HeapOffset,
    const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES     InitialState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID                    riid,
    void** ppvResource
) {

    HRESULT hr = oCreatePlacedResource(pDevice, pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

    return hr;
}

//=========================================================================================================================//

HRESULT STDMETHODCALLTYPE hkCreateCommittedResource(
    ID3D12Device* pDevice,
    const D3D12_HEAP_DESC* pDesc,
    const D3D12_HEAP_PROPERTIES* pHeapProperties,
    const D3D12_RESOURCE_DESC* pResourceDesc,
    D3D12_RESOURCE_STATES InitialResourceState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID riidResource,
    void** ppvResource
) {
    HRESULT hr = oCreateCommittedResource(pDevice, pDesc, pHeapProperties, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);

    /*
    if (SUCCEEDED(hr)) {
        g_pResource = *reinterpret_cast<ID3D12Resource**>(ppvResource);

        // Ensure it's a valid resource
        if (g_pResource != nullptr) {
            D3D12_RESOURCE_DESC resourceDesc = g_pResource->GetDesc();

            // Check if the resource is a buffer
            //if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && resourceDesc.Width >= 256) {
            if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
                goodresource = g_pResource;
                //Log("1. Buffer created with Width: %llu\n", resourceDesc.Width);
            }
        }
    }
    else {
        //Log("Error: *ppvResource is nullptr after CreateCommittedResource!\n");
    }
    */
    return hr;
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

    if (dCommandList) {
        CommandListSpecificData dataToStore;

        // Populate the data
        dataToStore.lastCbvRootParameterIndex = RootParameterIndex;

        // Store the struct. SetPrivateData makes a copy.
        HRESULT hrSet = dCommandList->SetPrivateData(MyCommandListPrivateDataGuid, sizeof(CommandListSpecificData), &dataToStore);
    }

    /*
    //try to log matrix
    if (goodresource && Strides == 9999) {
       D3D12_RESOURCE_DESC desc = goodresource->GetDesc();

       //Constant buffer size in bytes. Adjust as needed.
       ReadConstantBufferWithMapUnmap(goodresource, desc.Width);
    }
    */

    /*
    //maybe transformation matrix, projection or view
    INFO: Constant Buffer Data (Matrix):
    INFO: 0.000000 0.000000 0.999985 0.000000
    INFO: 0.000000 0.000000 1.000000 1.000000
    INFO: 1.000000 1.000000 -1.000000 1.000000
    INFO: 1.000000 -1.000000 1.000000 1.000000

    INFO: Constant Buffer Data (Matrix):
    INFO: 0.000000 0.000000 0.000000 -0.308071
    INFO: -0.173205 0.300300 1.000000 1.000000
    INFO: 1.000000 1.000000 0.000000 0.000000
    INFO: 0.000000 0.000000 0.000000 0.000000
    */

    return oSetGraphicsRootConstantBufferView(dCommandList, RootParameterIndex, BufferLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetVertexBuffers(ID3D12GraphicsCommandList* dCommandList, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {
    // Clear old values
    for (int i = 0; i < 7; ++i) {
        t_cLS.vertexStrides[i] = 0;
        t_cLS.vertexBufferSizes[i] = 0;
    }

    if (NumViews > 0 && pViews) {
        for (UINT i = 0; i < NumViews && i < 7; ++i) {
            if (pViews[i].StrideInBytes <= 120) { // Optional check
                t_cLS.vertexStrides[i] = pViews[i].StrideInBytes;
                t_cLS.vertexBufferSizes[i] = pViews[i].SizeInBytes;
            }
        }
        t_cLS.StartSlot = StartSlot;
        t_cLS.NumViews = NumViews;
    }

    return oIASetVertexBuffers(dCommandList, StartSlot, NumViews, pViews);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetIndexBuffer(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView)
{
    if (pView != nullptr) {
        t_cLS.currentIndexFormat = pView->Format;
        t_cLS.currentiSize = pView->SizeInBytes;
    }
    else {
        t_cLS.currentIndexFormat = DXGI_FORMAT_UNKNOWN;
    }
    
    return oIASetIndexBuffer(dCommandList, pView);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkOMSetRenderTargets(
    ID3D12GraphicsCommandList* dCommandList,
    UINT NumRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    BOOL RTsSingleHandleToDescriptorRange,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    if (pRenderTargetDescriptors && NumRenderTargetDescriptors > 0)
    {
        //g_RTVHandle = pRenderTargetDescriptors[0]; // Store the RTV
        g_RTVHandle = *pRenderTargetDescriptors; // Store the RTV
    }

    if (pDepthStencilDescriptor)
    {
        g_DSVHandle = *pDepthStencilDescriptor; // Store the DSV
    }

    return oOMSetRenderTargets(dCommandList, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
}

//=========================================================================================================================//

HRESULT STDMETHODCALLTYPE hkCreateGraphicsPipelineState(ID3D12Device* pDevice,const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc,REFIID riid,void** ppPipelineState)
{
    // Call the original function first
    HRESULT hr = oCreateGraphicsPipelineState(pDevice, pDesc, riid, ppPipelineState);
    if (FAILED(hr) || !ppPipelineState || !*ppPipelineState) {
        //Log("Original CreateGraphicsPipelineState failed or returned null PSO. HRESULT: 0x%X", hr);
        return hr;
    }

    ID3D12PipelineState* originalPSO = static_cast<ID3D12PipelineState*>(*ppPipelineState);
    //Log("Original PSO created: %p", originalPSO);

    // Now create a green-modified clone
    D3D12_GRAPHICS_PIPELINE_STATE_DESC newDesc = *pDesc;


    // 1. Modify Blend State (Green Color)
    if (newDesc.NumRenderTargets > 0) {
        // GREEN + ALPHA if alpha blending is important and enabled
        newDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_ALPHA;
        newDesc.BlendState.IndependentBlendEnable = TRUE;
    }
    else {
        //Log("Info: Original PSO has NumRenderTargets=0. Skipping BlendState modification.");
    }

    // 2. Modify Depth Stencil State
    // Check if a Depth/Stencil buffer format is actually specified. Modifying if none is specified is harmless but unnecessary.
    if (newDesc.DSVFormat != DXGI_FORMAT_UNKNOWN) {
        //Log("Modifying DepthStencilState for PSO %p (Original DepthEnable: %d)", originalPSO, newDesc.DepthStencilState.DepthEnable);
        newDesc.DepthStencilState.DepthEnable = FALSE;                 // Disable depth testing
        newDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // Set comparison func (wallhack)

        // Optional: Also disable depth writes if needed?
        // newDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    }
    else {
        //Log("Info: Original PSO has DSVFormat=UNKNOWN. Skipping DepthStencilState modification.");
    }
    // Note: Stencil settings (StencilEnable, etc.) remain unchanged from the original pDesc.


    ID3D12PipelineState* greenPSO = nullptr;
    // Use the same riid as the original call
    HRESULT greenHr = oCreateGraphicsPipelineState(pDevice, &newDesc, riid, reinterpret_cast<void**>(&greenPSO));

    if (SUCCEEDED(greenHr) && greenPSO) {
        std::lock_guard<std::mutex> lock(pipelineMutex);
        greenVariants[originalPSO] = greenPSO; // Store: original -> green
        //Log("Green variant PSO created (%p) for original PSO (%p)", greenPSO, originalPSO);
    }
    else {
        //Log("Failed to create green variant PSO for original PSO (%p). HRESULT: 0x%X", originalPSO, greenHr);
        if (greenPSO) { // If creation succeeded somehow but storing failed (unlikely with lock)
            greenPSO->Release(); // Avoid leak
        }
    }

    // Return the original PSO result
    return hr;
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetPipelineState(ID3D12GraphicsCommandList* dCommandList, ID3D12PipelineState* pPipelineState) {

    // Track the PSO the application wants to set for this command list
    if (pPipelineState) // Avoid storing nullptrs if the app does that
    {
        std::lock_guard<std::mutex> lock(commandListPSOMutex);
        currentCommandListPSO[dCommandList] = pPipelineState;
        // Log("Tracking PSO %p for CommandList %p", pPipelineState, dCommandList);
    }
    else {
        // Optionally remove the entry if PSO is set to null
        std::lock_guard<std::mutex> lock(commandListPSOMutex);
        currentCommandListPSO.erase(dCommandList);
        // Log("Untracking PSO for CommandList %p (set to NULL)", dCommandList);
    }

    return oSetPipelineState(dCommandList, pPipelineState); // Call the original function
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetComputeRootConstantBufferView(ID3D12GraphicsCommandList* pCommandList,UINT RootParameterIndex,D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
    
    return oSetComputeRootConstantBufferView(pCommandList, RootParameterIndex, BufferLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkCreateShaderResourceView(ID3D12Device* device,ID3D12Resource* resource,const D3D12_SHADER_RESOURCE_VIEW_DESC* desc,D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	return oCreateShaderResourceView(device, resource, desc, handle);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetDescriptorHeaps(ID3D12GraphicsCommandList* dCommandList,UINT NumDescriptorHeaps,ID3D12DescriptorHeap* const* ppDescriptorHeaps)
{
	return oSetDescriptorHeaps(dCommandList, NumDescriptorHeaps, ppDescriptorHeaps);
}

//=========================================================================================================================//

//NOT called in all games
HRESULT STDMETHODCALLTYPE hkCreateConstantBufferView(ID3D12Device* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {

    return oCreateConstantBufferView(pDevice, pDesc, DestDescriptor);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
	return oSetGraphicsRootShaderResourceView(dCommandList, RootParameterIndex, BufferLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootSignature(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature) {

    return oSetGraphicsRootSignature(dCommandList, pRootSignature);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkResolveQueryData(
    ID3D12GraphicsCommandList* self,
    ID3D12QueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    ID3D12Resource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset)
{
    //lognospam(3, "ResolveQueryData");

    if (Type == D3D12_QUERY_TYPE_OCCLUSION)
    {
        // Intercept occlusion queries. Instead of letting the GPU resolve the actual
        // visibility count, we manually write '1' (visible) into the destination
        // buffer for each query result on the CPU. This effectively disables
        // occlusion culling based on these queries.

        if (!pDestinationBuffer) {
            //Log("hkResolveQueryData: pDestinationBuffer is NULL for occlusion query!\n");
            // Cannot proceed, but also shouldn't call original as we intend to overwrite.
            // This path indicates an upstream problem if hit.
            return;
        }

        // Map the destination buffer for CPU writing
        void* mappedData = nullptr;
        // Specify zero range for read, as we are only writing.
        const D3D12_RANGE readRange = {};
        HRESULT hr = pDestinationBuffer->Map(0, &readRange, &mappedData);

        if (SUCCEEDED(hr))
        {
            // Calculate the starting address for writing query results
            // Occlusion results are UINT64.
            UINT64* writePtr = (UINT64*)((BYTE*)mappedData + AlignedDestinationBufferOffset);

            // Write '1' for each query result, forcing "visible" status.
            for (UINT i = 0; i < NumQueries; ++i) {
                writePtr[i] = 1;
            }

            // Specify the range written by the CPU. This is important for performance.
            const D3D12_RANGE writtenRange = {
                AlignedDestinationBufferOffset,
                AlignedDestinationBufferOffset + sizeof(UINT64) * NumQueries
            };
            pDestinationBuffer->Unmap(0, &writtenRange);

            // We have manually populated the buffer, so DO NOT call the original
            // ResolveQueryData, as that would overwrite our '1's with the actual
            // GPU results (or potentially cause issues if the buffer state isn't COPY_DEST).
            return;
        }
        else
        {
            // Log the error if mapping failed
            //Log("hkResolveQueryData: Failed to map destination buffer for occlusion query! HRESULT: 0x%X\n", hr);
            // If Map failed, we cannot write. Skipping the original call means the
            // buffer won't contain valid results, but calling it seems wrong too.
            // Returning here effectively skips the resolve.
            return;
        }
    }

    // For any query type other than occlusion, pass the call to the original function.
    oResolveQueryData(self, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
}

/*
void STDMETHODCALLTYPE hkResolveQueryData(
    ID3D12GraphicsCommandList* self,
    ID3D12QueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    ID3D12Resource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset)
{
    if (Type == D3D12_QUERY_TYPE_OCCLUSION) {
        // Map the destination buffer
        void* mappedData = nullptr;
        D3D12_RANGE readRange = {}; // We won't read from it
        HRESULT hr = pDestinationBuffer->Map(0, &readRange, &mappedData);

        if (SUCCEEDED(hr)) {
            UINT64* writePtr = (UINT64*)((BYTE*)mappedData + AlignedDestinationBufferOffset);
            for (UINT i = 0; i < NumQueries; ++i) {
                writePtr[i] = 1; // Force visibility
            }

            D3D12_RANGE writtenRange = { AlignedDestinationBufferOffset, AlignedDestinationBufferOffset + sizeof(UINT64) * NumQueries };
            pDestinationBuffer->Unmap(0, &writtenRange);
        }

        // Skip calling the original ResolveQueryData
        return;
    }

    // Call the original method for other query types
    oResolveQueryData(self, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
}
*/

//=========================================================================================================================//

void STDMETHODCALLTYPE hkCopyBufferRegion(
    ID3D12GraphicsCommandList* self,
    ID3D12Resource* pDstBuffer,
    UINT64 DstOffset,
    ID3D12Resource* pSrcBuffer,
    UINT64 SrcOffset,
    UINT64 NumBytes)
{
    /*
    //lognospam(10, "CopyBufferRegion");

    // Call original copy first
    oCopyBufferRegion(self, pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);

    if (!pDstBuffer) return;

    // Check if destination buffer is CPU-visible (UPLOAD)
    D3D12_HEAP_PROPERTIES heapProps;
    D3D12_HEAP_FLAGS heapFlags;
    if (SUCCEEDED(pDstBuffer->GetHeapProperties(&heapProps, &heapFlags)) &&
        heapProps.Type == D3D12_HEAP_TYPE_UPLOAD)
    {
        // Map and patch data to simulate visibility
        void* mappedData = nullptr;
        const D3D12_RANGE readRange = { (SIZE_T)DstOffset, (SIZE_T)(DstOffset + NumBytes) };
        if (SUCCEEDED(pDstBuffer->Map(0, &readRange, &mappedData)))
        {
            // Patch all copied UINT64 values to 1
            UINT64* data = (UINT64*)((BYTE*)mappedData + DstOffset);
            UINT64 count = NumBytes / sizeof(UINT64);

            for (UINT64 i = 0; i < count; ++i) {
                data[i] = 1;
            }

            const D3D12_RANGE writtenRange = {
                (SIZE_T)DstOffset,
                (SIZE_T)(DstOffset + NumBytes)
            };
            pDstBuffer->Unmap(0, &writtenRange);
        }
    }
    */
}

//=========================================================================================================================//

DWORD WINAPI MainThread(LPVOID lpParameter) {

	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

			Process::ID = GetCurrentProcessId();
			Process::Handle = GetCurrentProcess();
			Process::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX12::Init() == true) {
			//CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
			//CreateHook(140, (void**)&oPresent, hkPresent);
			CreateHook(85, (void**)&oDrawIndexedInstanced, hkDrawIndexedInstanced);
            //CreateHook(84, (void**)&oDrawInstanced, hkDrawInstanced);
            CreateHook(93, (void**)&oRSSetViewports, hkRSSetViewports);
            CreateHook(27, (void**)&oCreateCommittedResource, hkCreateCommittedResource);
            CreateHook(110, (void**)&oSetGraphicsRootConstantBufferView, hkSetGraphicsRootConstantBufferView);
			CreateHook(116, (void**)&oIASetVertexBuffers, hkIASetVertexBuffers);
			CreateHook(115, (void**)&oIASetIndexBuffer, hkIASetIndexBuffer);
            CreateHook(10, (void**)&oCreateGraphicsPipelineState, hkCreateGraphicsPipelineState);
            CreateHook(97, (void**)&oSetPipelineState, hkSetPipelineState);
            CreateHook(126, (void**)&oResolveQueryData, hkResolveQueryData);
            //CreateHook(118, (void**)&oOMSetRenderTargets, hkOMSetRenderTargets);
            //CreateHook(102, (void**)&oSetGraphicsRootSignature, hkSetGraphicsRootSignature);
            //CreateHook(39, (void**)&oCreateQueryHeap, hkCreateQueryHeap);
            //CreateHook(124, (void**)&oBeginQuery, hkBeginQuery);
            //CreateHook(125, (void**)&oEndQuery, hkEndQuery);
            //CreateHook(87, (void**)&oCopyBufferRegion, hkCopyBufferRegion);
            //CreateHook(109, (void**)&oSetComputeRootConstantBufferView, hkSetComputeRootConstantBufferView);
            //CreateHook(17, (void**)&oCreateConstantBufferView, hkCreateConstantBufferView);
			//CreateHook(18, (void**)&oCreateShaderResourceView, hkCreateShaderResourceView);
			//CreateHook(100, (void**)&oSetDescriptorHeaps, hkSetDescriptorHeaps);
            //CreateHook(29, (void**)&oCreatePlacedResource, hkCreatePlacedResource);
			//CreateHook(112, (void**)&oSetGraphicsRootShaderResourceView, hkSetGraphicsRootShaderResourceView);

            static bool wndproc_initialized = false;
            if (!wndproc_initialized) {
                Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));

                wndproc_initialized = true;
            }

			InitHook = true;
		}
	}
	return 0;
}

//=========================================================================================================================//

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		Process::Module = hModule;
		GetModuleFileNameA(hModule, dlldir, 320);
		for (size_t i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }
		CreateThread(0, 0, MainThread, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		FreeLibraryAndExitThread(hModule, TRUE);
		DisableAll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) { return CallNextHookEx(NULL, code, wParam, lParam); }

//=========================================================================================================================//

//D3D12 Methods Table:
//[0]   QueryInterface
//[1]   AddRef
//[2]   Release
//[3]   GetPrivateData
//[4]   SetPrivateData
//[5]   SetPrivateDataInterface
//[6]   SetName
//[7]   GetNodeCount
//[8]   CreateCommandQueue
//[9]   CreateCommandAllocator
//[10]  CreateGraphicsPipelineState
//[11]  CreateComputePipelineState
//[12]  CreateCommandList
//[13]  CheckFeatureSupport
//[14]  CreateDescriptorHeap
//[15]  GetDescriptorHandleIncrementSize
//[16]  CreateRootSignature
//[17]  CreateConstantBufferView
//[18]  CreateShaderResourceView
//[19]  CreateUnorderedAccessView
//[20]  CreateRenderTargetView
//[21]  CreateDepthStencilView
//[22]  CreateSampler
//[23]  CopyDescriptors
//[24]  CopyDescriptorsSimple
//[25]  GetResourceAllocationInfo
//[26]  GetCustomHeapProperties
//[27]  CreateCommittedResource
//[28]  CreateHeap
//[29]  CreatePlacedResource
//[30]  CreateReservedResource
//[31]  CreateSharedHandle
//[32]  OpenSharedHandle
//[33]  OpenSharedHandleByName
//[34]  MakeResident
//[35]  Evict
//[36]  CreateFence
//[37]  GetDeviceRemovedReason
//[38]  GetCopyableFootprints
//[39]  CreateQueryHeap
//[40]  SetStablePowerState
//[41]  CreateCommandSignature
//[42]  GetResourceTiling
//[43]  GetAdapterLuid
//[44]  QueryInterface
//[45]  AddRef
//[46]  Release
//[47]  GetPrivateData
//[48]  SetPrivateData
//[49]  SetPrivateDataInterface
//[50]  SetName
//[51]  GetDevice
//[52]  UpdateTileMappings
//[53]  CopyTileMappings
//[54]  ExecuteCommandLists
//[55]  SetMarker
//[56]  BeginEvent
//[57]  EndEvent
//[58]  Signal
//[59]  Wait
//[60]  GetTimestampFrequency
//[61]  GetClockCalibration
//[62]  GetDesc
//[63]  QueryInterface
//[64]  AddRef
//[65]  Release
//[66]  GetPrivateData
//[67]  SetPrivateData
//[68]  SetPrivateDataInterface
//[69]  SetName
//[70]  GetDevice
//[71]  Reset
//[72]  QueryInterface
//[73]  AddRef
//[74]  Release
//[75]  GetPrivateData
//[76]  SetPrivateData
//[77]  SetPrivateDataInterface
//[78]  SetName
//[79]  GetDevice
//[80]  GetType
//[81]  Close
//[82]  Reset
//[83]  ClearState
//[84]  DrawInstanced
//[85]  DrawIndexedInstanced
//[86]  Dispatch
//[87]  CopyBufferRegion
//[88]  CopyTextureRegion
//[89]  CopyResource
//[90]  CopyTiles
//[91]  ResolveSubresource
//[92]  IASetPrimitiveTopology
//[93]  RSSetViewports
//[94]  RSSetScissorRects
//[95]  OMSetBlendFactor
//[96]  OMSetStencilRef
//[97]  SetPipelineState
//[98]  ResourceBarrier
//[99]  ExecuteBundle
//[100] SetDescriptorHeaps
//[101] SetComputeRootSignature
//[102] SetGraphicsRootSignature
//[103] SetComputeRootDescriptorTable
//[104] SetGraphicsRootDescriptorTable
//[105] SetComputeRoot32BitConstant
//[106] SetGraphicsRoot32BitConstant
//[107] SetComputeRoot32BitConstants
//[108] SetGraphicsRoot32BitConstants
//[109] SetComputeRootConstantBufferView
//[110] SetGraphicsRootConstantBufferView
//[111] SetComputeRootShaderResourceView
//[112] SetGraphicsRootShaderResourceView
//[113] SetComputeRootUnorderedAccessView
//[114] SetGraphicsRootUnorderedAccessView
//[115] IASetIndexBuffer
//[116] IASetVertexBuffers
//[117] SOSetTargets
//[118] OMSetRenderTargets
//[119] ClearDepthStencilView
//[120] ClearRenderTargetView
//[121] ClearUnorderedAccessViewUint
//[122] ClearUnorderedAccessViewFloat
//[123] DiscardResource
//[124] BeginQuery
//[125] EndQuery
//[126] ResolveQueryData
//[127] SetPredication
//[128] SetMarker
//[129] BeginEvent
//[130] EndEvent
//[131] ExecuteIndirect
//[132] QueryInterface
//[133] AddRef
//[134] Release
//[135] SetPrivateData
//[136] SetPrivateDataInterface
//[137] GetPrivateData
//[138] GetParent
//[139] GetDevice
//[140] Present
//[141] GetBuffer
//[142] SetFullscreenState
//[143] GetFullscreenState
//[144] GetDesc
//[145] ResizeBuffers
//[146] ResizeTarget
//[147] GetContainingOutput
//[148] GetFrameStatistics
//[149] GetLastPresentCount
