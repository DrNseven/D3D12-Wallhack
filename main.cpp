/////////////////////////
// D3D12 Wallhack 2025 //
/////////////////////////

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

//=========================================================================================================================//

bool vkENDkeydown = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //Log("1");
    if (ShowMenu) {
        ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam); //ImGui gets first crack at the message
    }
    
    switch (uMsg) {
    case WM_KEYDOWN: {
        switch (wParam) {
        case 'N':
            countnum--;
            break;
        case 'M':
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

HRESULT APIENTRY hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {

    //draw menu here

	return oPresent(pSwapChain, SyncInterval, Flags);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {

	mIndexCount = IndexCountPerInstance;
    
	static bool initialized = false;
	if (!initialized) {
		//get device here if you want to use it
		HRESULT hr = dCommandList->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
		if (FAILED(hr)) {
			Log("Failed to get ID3D12Device from command list, HRESULT: 0x%08X", hr);
			return;
		}

        //load custom stuff
        CreateRootSignature(pDevice.Get());
        CreateGreenPipelineState(pDevice.Get(), dCommandList);
        //CreateRedPipelineState(pDevice.Get(), dCommandList);

		initialized = true;
	}

    
    //iSize
    UINT currentiSize = 0;
    DXGI_FORMAT currentiFormat = DXGI_FORMAT_UNKNOWN;
    {
        std::lock_guard<std::mutex> lock(iSizesMutex);

        // Retrieve SizeInBytes
        auto it = iSizes.find(dCommandList);
        if (it != iSizes.end()) {
            currentiSize = it->second;
        }
        else {
            currentiSize = 0;
            //Or maybe return
        }

        // Retrieve Format
        auto itFormat = iFormat.find(dCommandList);
        if (itFormat != iFormat.end()) {
            currentiFormat = static_cast<DXGI_FORMAT>(itFormat->second);
        }
    }
    //Usage: if (currentiSize == x && currentiFormat == DXGI_FORMAT_R16_UINT) {

    twoDigitiSize = getTwoDigitValue(currentiSize);


    //Stride
    UINT Stride0 = 0;
    UINT Stride1 = 0;
    UINT Stride2 = 0;
    {
        std::lock_guard<std::mutex> lock(vStrideMutex);
        auto it = vStride.find(dCommandList);
        if (it != vStride.end()) {
            Stride0 = it->second[0]; // 0 = stride of slot 0
            Stride1 = it->second[1];
            Stride2 = it->second[2];
        }
    }

    //rootsignature
    UINT currentRootSigID = 0;
    {
        std::lock_guard<std::mutex> lock(rootSigMutex);
        if (rootSignatures.find(dCommandList) != rootSignatures.end()) {
            ID3D12RootSignature* currentRootSig = rootSignatures[dCommandList];
            if (rootSigIDs.find(currentRootSig) != rootSigIDs.end()) {
                currentRootSigID = rootSigIDs[currentRootSig];
            }
        }
    }


    //log bruteforced values by pressing VK_END
    if (vkENDkeydown && (twoDigitiSize == countnum || currentRootSigID == countnum)) {
        Log("countnum == %d && mIndexCount == %d && Stride0 == %d && Stride1 == %d && Stride2 == %d && gRootParameterIndex == %d && twoDigitiSize == %d currentiSize == %d && currentiFormat == %d && currentRootSigID == %d",
            countnum, mIndexCount, Stride0, Stride1, Stride2, gRootParameterIndex, twoDigitiSize, currentiSize, currentiFormat, currentRootSigID);
    }
  
    //wallhack
    if(twoDigitiSize == countnum || currentRootSigID == countnum) {
        SetDepthRange(0.95f, dCommandList);
        oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        ResetDepthRange(dCommandList);
        //return; //or erase texture
    }


    /*
    //meh
    if (gRootParameterIndex == 6 && vStride[0] == 40) {

        float mColor[] = { 0.0f, 1.0f, 0.0f, 0.0f }; // Green
        dCommandList->ClearRenderTargetView(g_RTVHandle, mColor, 0, nullptr);
        dCommandList->SetGraphicsRootConstantBufferView(countnum+13, 0);//1- = crash
    }
    */

    /*
    //meh
    if (gRootParameterIndex == 6 && vStride[0] == 40|| gRootParameterIndex == 8 && mIndexCount == 11532) {
        if (g_GreenPSO) {
            dCommandList->SetPipelineState(g_GreenPSO.Get());  // Set custom pipeline state
            CreateConstantBuffer(countnum+13, pDevice.Get(), dCommandList);//5,4 //1- = crash
            //return;
        }
    }
    */

	return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {

    //mVertexCount = VertexCountPerInstance;

    return oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkRSSetViewports(ID3D12GraphicsCommandList* dCommandList,UINT NumViewports,const D3D12_VIEWPORT* pViewports) {

    if (NumViewports > 0) {
        gpV = *pViewports;
    }

    return oRSSetViewports(dCommandList, NumViewports, pViewports);  // Call original function
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootSignature(ID3D12GraphicsCommandList* dCommandList, ID3D12RootSignature* pRootSignature) {
    if (dCommandList && pRootSignature) {
        std::lock_guard<std::mutex> lock(rootSigMutex);

        // If root signature is not yet assigned an ID, give it one
        if (rootSigIDs.find(pRootSignature) == rootSigIDs.end()) {
            rootSigIDs[pRootSignature] = nextRootSigID++;
        }

        // Store the root signature for this command list
        rootSignatures[dCommandList] = pRootSignature;
    }
    return oSetGraphicsRootSignature(dCommandList, pRootSignature);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {

    //gRootParameterIndex = RootParameterIndex;
    //gBufferLocation = BufferLocation;

    return oSetGraphicsRootConstantBufferView(dCommandList, RootParameterIndex, BufferLocation);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetVertexBuffers(ID3D12GraphicsCommandList* dCommandList, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {

    if (!dCommandList || !pViews || NumViews == 0) {
        return;
    }
    
    //Stride
    std::lock_guard<std::mutex> lock(vStrideMutex);

    // Access or initialize stride array for this command list
    auto it = vStride.find(dCommandList);
    if (it == vStride.end()) {
        // Command list not found, insert a new entry with a default-initialized array.
        it = vStride.emplace(dCommandList, std::array<UINT, MAX_VERTEX_BUFFER_SLOTS>{}).first;
    }
    auto& strideArray = it->second;

    for (UINT i = 0; i < NumViews; ++i) {
        if (StartSlot + i < MAX_VERTEX_BUFFER_SLOTS) {
            strideArray[StartSlot + i] = pViews[i].StrideInBytes;
        }
    }

    return oIASetVertexBuffers(dCommandList, StartSlot, NumViews, pViews);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkIASetIndexBuffer(ID3D12GraphicsCommandList* dCommandList, const D3D12_INDEX_BUFFER_VIEW* pView)
{
    //iSize
    if (pView != nullptr && dCommandList != nullptr) // Add a null check for dCommandList
    {
        std::lock_guard<std::mutex> lock(iSizesMutex); // Thread safety
        iSizes[dCommandList] = pView->SizeInBytes;
        iFormat[dCommandList] = pView->Format;
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
        g_RTVHandle = *pRenderTargetDescriptors; // Store the RTV
    }

    if (pDepthStencilDescriptor)
    {
        g_DSVHandle = *pDepthStencilDescriptor; // Store the DSV
    }

    return oOMSetRenderTargets(dCommandList, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
}

//=========================================================================================================================//

void STDMETHODCALLTYPE hkSetPipelineState(ID3D12GraphicsCommandList* dCommandList, ID3D12PipelineState* pPipelineState) {

    if (pPipelineState != nullptr)
    {
        g_CurrentPSO = pPipelineState;
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

void STDMETHODCALLTYPE hkCreateCommittedResource(
    ID3D12Device* pDevice,
    const D3D12_HEAP_DESC* pDesc,
    const D3D12_HEAP_PROPERTIES* pHeapProperties,
    const D3D12_RESOURCE_DESC* pResourceDesc,
    D3D12_RESOURCE_STATES InitialResourceState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID riidResource,
    void** ppvResource)
{
    oCreateCommittedResource(pDevice, pDesc, pHeapProperties, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
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

//This function is NOT called in all games
HRESULT STDMETHODCALLTYPE hkCreateConstantBufferView(ID3D12Device* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {

    return oCreateConstantBufferView(pDevice, pDesc, DestDescriptor);
}

//=========================================================================================================================//

//not called
void STDMETHODCALLTYPE hkSetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* dCommandList, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
	return oSetGraphicsRootShaderResourceView(dCommandList, RootParameterIndex, BufferLocation);
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
			CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
			CreateHook(140, (void**)&oPresent, hkPresent);
			CreateHook(85, (void**)&oDrawIndexedInstanced, hkDrawIndexedInstanced);
            CreateHook(84, (void**)&oDrawInstanced, hkDrawInstanced);
            CreateHook(93, (void**)&oRSSetViewports, hkRSSetViewports);
            CreateHook(110, (void**)&oSetGraphicsRootConstantBufferView, hkSetGraphicsRootConstantBufferView);
			CreateHook(116, (void**)&oIASetVertexBuffers, hkIASetVertexBuffers);
			CreateHook(115, (void**)&oIASetIndexBuffer, hkIASetIndexBuffer);
            CreateHook(118, (void**)&oOMSetRenderTargets, hkOMSetRenderTargets);
            CreateHook(97, (void**)&oSetPipelineState, hkSetPipelineState);
            CreateHook(102, (void**)&oSetGraphicsRootSignature, hkSetGraphicsRootSignature);
            //CreateHook(39, (void**)&oCreateQueryHeap, hkCreateQueryHeap);
            //CreateHook(124, (void**)&oBeginQuery, hkBeginQuery);
            //CreateHook(125, (void**)&oEndQuery, hkEndQuery);
            //CreateHook(126, (void**)&oResolveQueryData, hkResolveQueryData);
            //CreateHook(109, (void**)&oSetComputeRootConstantBufferView, hkSetComputeRootConstantBufferView);
            //CreateHook(17, (void**)&oCreateConstantBufferView, hkCreateConstantBufferView);
			//CreateHook(18, (void**)&oCreateShaderResourceView, hkCreateShaderResourceView);
			//CreateHook(100, (void**)&oSetDescriptorHeaps, hkSetDescriptorHeaps);	    
			//CreateHook(27, (void**)&oCreateCommittedResource, hkCreateCommittedResource);
            //CreateHook(29, (void**)&oCreatePlacedResource, hkCreatePlacedResource);
			//CreateHook(112, (void**)&oSetGraphicsRootShaderResourceView, hkSetGraphicsRootShaderResourceView);

            static bool wndproc_initialized = false;
            if (!wndproc_initialized) {
                //Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
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