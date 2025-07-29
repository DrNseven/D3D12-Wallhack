///////////////////////////////
// D3D12 Wallhack 2025 by N7 //
///////////////////////////////

//project -> properties -> advanced -> Character Set -> Use Multi-Byte Character Set

#include "main.h"

//=========================================================================================================================//

typedef void(STDMETHODCALLTYPE* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
DrawIndexedInstanced oDrawIndexedInstanced = NULL;

typedef void (STDMETHODCALLTYPE* RSSetViewportsFunc)(ID3D12GraphicsCommandList* dCommandList, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
RSSetViewportsFunc oRSSetViewports = nullptr;

typedef void (STDMETHODCALLTYPE* SetGraphicsRootConstantBufferView)(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
SetGraphicsRootConstantBufferView oSetGraphicsRootConstantBufferView;

typedef void(STDMETHODCALLTYPE* IASetVertexBuffers)(ID3D12GraphicsCommandList* dCommandList, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
IASetVertexBuffers oIASetVertexBuffers = NULL;

typedef void(STDMETHODCALLTYPE* IASetIndexBuffer)(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView);
IASetIndexBuffer oIASetIndexBuffer = NULL;

typedef void (STDMETHODCALLTYPE* SetGraphicsRootDescriptorTable)(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
SetGraphicsRootDescriptorTable oSetGraphicsRootDescriptorTable;

typedef HRESULT(STDMETHODCALLTYPE* ResolveQueryData)(ID3D12GraphicsCommandList* self,
    ID3D12QueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    ID3D12Resource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset);
ResolveQueryData oResolveQueryData = nullptr;

//=========================================================================================================================//

bool vkENDkeydown = false;

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    switch (uMsg) {
    case WM_KEYDOWN: {
        switch (wParam) {

        case VK_INSERT: //insert key
            wallhack = !wallhack;
            break;

        case VK_HOME: //home key
            colors = !colors;
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
bool initialized = false;

void STDMETHODCALLTYPE hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
    
    // One time initialization
    if (!initialized && IndexCountPerInstance > 0 && InstanceCount > 0) {
        HRESULT hr = dCommandList->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
        if (FAILED(hr)) {
            Log("GetDevice failed: 0x%08X", hr);
            return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        if (CreateCustomConstantBuffer()) {
            initialized = true;
            //Log("Constant buffer initialized");
        }
        else {
            //Log("Failed to create constant buffer");
        }
    }


    // Read cached values from TLS
    UINT rootIndex = t_.lastCbvRootParameterIndex;
    UINT Strides = t_.cachedStrideSum + t_.StartSlot;
    //int twoDigitSize = getTwoDigitValue(IndexCountPerInstance);
    float vpWidth = t_.currentViewport.Width;
    float vpHeight = t_.currentViewport.Height;

    
    // Colors
    if(colors) //disabled by default
    if ((Strides == countnum) && rootIndex != UINT_MAX)
    {
        DirectX::XMFLOAT4 newColor = { 1.0f, 1.0f, 1.0f, 1.0f }; 
        //DirectX::XMFLOAT3 newColor = { 1.0f, 0.0f, 1.0f };

        // Multiply by 16 to step through 16-byte aligned addresses
        size_t colorOffset = countnum * 16; //bruteforce this value to find colors

        if (g_pMappedConstantBuffer &&
            colorOffset + sizeof(newColor) <= g_constantBufferSize)
        {
            // To prevent writing every frame, only write when the offset changes
            static size_t lastColorOffset = SIZE_MAX;
            if (lastColorOffset != colorOffset)
            {
                // Nuke the buffer to remove all old animation/transform data (optional)
                //memset(g_pMappedConstantBuffer, 0, g_constantBufferSize);

                // Write the color at the offset
                memcpy(g_pMappedConstantBuffer + colorOffset, &newColor, sizeof(newColor));

                lastColorOffset = colorOffset;
            }

            D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = g_pCustomConstantBuffer->GetGPUVirtualAddress();
            dCommandList->SetGraphicsRootConstantBufferView(rootIndex, bufferAddress);
        }
    }


    // Wallhack
    if (wallhack)
    if (Strides == countnum /*|| twoDigitSize == countnum*/) { //Strides is used for bruteforcing, replace later with something similar bel
    //if ((t_.Strides[0] == 12 && t_.Strides[1] == 8 && t_.Strides[2] == 4) || (t_.Strides[0] == 12 && t_.Strides[1] == 8 && t_.Strides[2] == 8)) { //modelrec example

        D3D12_VIEWPORT vp = { 0, 0, vpWidth, vpHeight, 0.9f, 1.0f };
        dCommandList->RSSetViewports(1, &vp);
        oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        vp.MinDepth = 0.0f;
        dCommandList->RSSetViewports(1, &vp);
    }


    
    // Logger
    //1. use keys comma (,) and period (.) to cycle through textures
    //2. log current values by pressing END
    //if (vkENDkeydown && Strides == countnum) { //bruteforce strides example
    if (vkENDkeydown && Strides == countnum) { //log killing floor 3 models
        Log("countnum == %d && Strides == %d && t_.Strides[0] == %d && t_.Strides[1] == %d && t_.Strides[2] == %d && t_.Strides[3] == %d && t_.Strides[4] == %d && t_.Strides[5] == %d && t_.Strides[6] == %d && t_.currentiSize == %d && IndexCountPerInstance == %d && rootIndex == %d && t_.StartSlot == %d && t_.vertexBufferSizes[0] == %d && t_.vertexBufferSizes[1] == %d && t_.vertexBufferSizes[2] == %d && t_.vertexBufferSizes[3] == %d",
            countnum, Strides, t_.Strides[0], t_.Strides[1], t_.Strides[2], t_.Strides[3], t_.Strides[4], t_.Strides[5], t_.Strides[6], t_.currentiSize, IndexCountPerInstance, rootIndex, t_.StartSlot, t_.vertexBufferSizes[0], t_.vertexBufferSizes[1], t_.vertexBufferSizes[2], t_.vertexBufferSizes[3]);
    }
    

	return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkRSSetViewports(ID3D12GraphicsCommandList* dCommandList,UINT NumViewports,const D3D12_VIEWPORT* pViewports) {

    if (NumViewports > 0 && pViewports) {
        t_.currentViewport = pViewports[0];  // fast copy to thread-local
    }
    
    return oRSSetViewports(dCommandList, NumViewports, pViewports);  // Call original function
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

    t_.lastCbvRootParameterIndex = RootParameterIndex;

    return oSetGraphicsRootConstantBufferView(dCommandList, RootParameterIndex, BufferLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetVertexBuffers(ID3D12GraphicsCommandList* dCommandList, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {

    // Reset stride and size data
    for (int i = 0; i < 7; ++i) {
        t_.Strides[i] = 0;
        t_.vertexBufferSizes[i] = 0;
    }

    t_.cachedStrideSum = 0;
    t_.StartSlot = StartSlot;

    uint32_t strideData[7] = {};

    if (NumViews > 0 && pViews) {
        for (UINT i = 0; i < NumViews && i < 7; ++i) {
            if (pViews[i].StrideInBytes <= 120) {
                t_.Strides[i] = pViews[i].StrideInBytes;
                strideData[i] = pViews[i].StrideInBytes;
                t_.vertexBufferSizes[i] = pViews[i].SizeInBytes;
            }
        }

        // Only hash meaningful data
        t_.cachedStrideSum = fastStrideHash(strideData, NumViews);
    }

    /*
    // old
    for (int i = 0; i < 7; ++i) {
        t_.Strides[i] = 0;
        t_.vertexBufferSizes[i] = 0;
    }

    t_.cachedStrideSum = 0;

    if (NumViews > 0 && pViews) {
        for (UINT i = 0; i < NumViews && i < 7; ++i) {
            if (pViews[i].StrideInBytes <= 120) { // sanity check
                t_.Strides[i] = pViews[i].StrideInBytes;
                t_.vertexBufferSizes[i] = pViews[i].SizeInBytes;
                t_.cachedStrideSum += pViews[i].StrideInBytes;
            }
        }
        t_.StartSlot = StartSlot;
    }
    */
    return oIASetVertexBuffers(dCommandList, StartSlot, NumViews, pViews);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetIndexBuffer(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView)
{
    if (pView != nullptr) {
        t_.currentiSize = pView->SizeInBytes;
        t_.currentIndexFormat = pView->Format;
    }
    else {
        t_.currentIndexFormat = DXGI_FORMAT_UNKNOWN;
    }
    
    return oIASetIndexBuffer(dCommandList, pView);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {

    t_.lastCbvRootParameterIndex2 = RootParameterIndex;

    return oSetGraphicsRootDescriptorTable(dCommandList, RootParameterIndex, BaseDescriptor);
}

//=========================================================================================================================//

//occlusion exploit will not work in all games
void STDMETHODCALLTYPE hkResolveQueryData(
    ID3D12GraphicsCommandList* self,
    ID3D12QueryHeap* pQueryHeap,
    D3D12_QUERY_TYPE Type,
    UINT StartIndex,
    UINT NumQueries,
    ID3D12Resource* pDestinationBuffer,
    UINT64 AlignedDestinationBufferOffset)
{
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
            CreateHook(85, (void**)&oDrawIndexedInstanced, hkDrawIndexedInstanced);
            CreateHook(93, (void**)&oRSSetViewports, hkRSSetViewports);
            CreateHook(110, (void**)&oSetGraphicsRootConstantBufferView, hkSetGraphicsRootConstantBufferView);
            CreateHook(116, (void**)&oIASetVertexBuffers, hkIASetVertexBuffers);
            CreateHook(115, (void**)&oIASetIndexBuffer, hkIASetIndexBuffer);
            CreateHook(104, (void**)&oSetGraphicsRootDescriptorTable, hkSetGraphicsRootDescriptorTable);
            CreateHook(126, (void**)&oResolveQueryData, hkResolveQueryData); //disable if not needed

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
