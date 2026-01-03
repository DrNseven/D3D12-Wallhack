#include "stdafx.h"
/*
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace inputhook {
    static WNDPROC sOriginalWndProc = nullptr;

    // Cache the window handle to allow later removal of the hook
    // (stored in globals::mainWindow for cross-namespace access)

    void Init(HWND hWindow)
    {
        Log("[inputhook] Initializing input hook for window %p\n", hWindow);

        // Store window globally for later use during release
        globals::mainWindow = hWindow;

        sOriginalWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
        if (!sOriginalWndProc) {
            Log("[inputhook] Failed to set WndProc: %d\n", GetLastError());
        }
        else {
            Log("[inputhook] WndProc hook set. Original WndProc=%p\n", sOriginalWndProc);
        }
    }

    void Remove(HWND hWindow)
    {
        if (!sOriginalWndProc) {
            Log("[inputhook] WndProc hook already removed or was never set\n");
            return;
        }

        Log("[inputhook] Removing input hook for window %p\n", hWindow);
        if (SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)sOriginalWndProc) == 0) {
            Log("[inputhook] Failed to restore WndProc: %d\n", GetLastError());
        }
        else {
            Log("[inputhook] WndProc restored to %p\n", sOriginalWndProc);
        }

        // Clear cached values to prevent repeated removals
        sOriginalWndProc = nullptr;
        globals::mainWindow = nullptr;
    }

    LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (menuisOpen)
        {
            ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
            {
                switch (uMsg)
                {
                case WM_KEYUP:
                case WM_SYSKEYUP:
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                    return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
                default:
                    return TRUE;
                }
            }
        }

        return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
    }
}
*/