#pragma once
//#pragma comment(lib, "minhook/lib/libMinHook.x64.lib")

#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")

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

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#include "minhook/include/MinHook.h"

#include "namespaces.h"

#include <fstream>
inline void Log(const char* fmt, ...) {
    if (!globals::enableDebugLog) {
        return;
    }
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

// Thread-local cache
thread_local struct {
    UINT lastCbvRootParameterIndex = UINT_MAX;
    UINT lastCbvRootParameterIndex2 = UINT_MAX;
    UINT StartSlot = 0;
    UINT Strides[16] = { 0 };
    UINT vertexBufferSizes[16] = { 0 };
    UINT cachedStrideSum = 0;
    UINT StrideHash = 0;
    D3D12_VIEWPORT currentViewport = {};
    UINT numViewports = 0;
    UINT currentiSize = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE LastDescriptorBase;
    UINT currentNumRTVs = 0;
} t_;