#include "stdafx.h"

namespace mousehooks { void Init(); void Remove(); }

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
            hooks::Init();
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

    while (!g_BackendInitialized)
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

    mousehooks::Init();

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

static DWORD WINAPI UninjectThread(LPVOID)
{
    Log("[Uninject] Starting\n");

    ReleaseActiveBackend();

    mousehooks::Remove();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    Log("[Uninject] Unloading module\n");
    FreeLibraryAndExitThread(globals::mainModule, 0);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
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
        Log("[DllMain] DLL_PROCESS_DETACH. Releasing hooks and uninitializing MinHook.\n");
        switch (globals::activeBackend) {
        case globals::Backend::DX12:
            d3d12hook::release();
            break;
        default:
            break;
        }
        mousehooks::Remove();
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) { return CallNextHookEx(NULL, code, wParam, lParam); }

//extern "C" __declspec(dllexport) LRESULT CALLBACK NextHook(int nCode, WPARAM wParam, LPARAM lParam)
//{
    //return CallNextHookEx(nullptr, nCode, wParam, lParam);
//}

