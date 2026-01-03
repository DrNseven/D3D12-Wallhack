    //menu
    bool menuisOpen = false;
    UINT countstride1 = 0;
    UINT countstride2 = 0;
    UINT countstride3 = 0;
    UINT countcurrentRootSigID = 0;
    UINT countcurrentRootSigID2 = 0;
    bool reversedDepth = false;
    

    void menuInit() {
        // Begin frame UI setup
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = menuisOpen;

        if (!menuisOpen) {
            return;
        }

        // Style setup (one-time)
        static bool styled = false;
        if (!styled) {
            ImGui::StyleColorsDark();
            ImVec4* colors = ImGui::GetStyle().Colors;
            // Custom color palette
            colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.8f);
            colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 0.8f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.8f);
            colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.4f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.0f);
            styled = true;
            //Log("[menu] Style applied.\n");
        }

        // Window flags
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowSize(ImVec2(480, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(25, 25), ImGuiCond_FirstUseEver);

        ImGui::Begin("ImGui Menu", &menuisOpen, flags);

                const UINT min_val = 0;
                const UINT max_val = 100;
                ImGui::Text("Wallhack:");
                ImGui::SliderScalar("Find Stridehash 1", ImGuiDataType_U32, &countstride1, &min_val, &max_val, "%u");
                ImGui::SliderScalar("Find Stridehash 2", ImGuiDataType_U32, &countstride2, &min_val, &max_val, "%u");
                ImGui::SliderScalar("Find Stridehash 3", ImGuiDataType_U32, &countstride3, &min_val, &max_val, "%u");
                ImGui::SliderScalar("Find currentRootID", ImGuiDataType_U32, &countcurrentRootSigID, &min_val, &max_val, "%u");
                ImGui::SliderScalar("Find currentRootID2", ImGuiDataType_U32, &countcurrentRootSigID2, &min_val, &max_val, "%u");
                ImGui::Checkbox("Reverse Depth", &reversedDepth);
                //ImGui::Checkbox("Color", &colors);

        ImGui::End();
    }

//=======================================================================================//

    using SetCursorPos_t = BOOL(WINAPI*)(int, int);
    static SetCursorPos_t oSetCursorPos = nullptr;
    BOOL WINAPI hookSetCursorPos(int x, int y) {
        return menuisOpen ? TRUE : oSetCursorPos(x, y);
    }

    using ClipCursor_t = BOOL(WINAPI*)(const RECT*);
    static ClipCursor_t oClipCursor = nullptr;
    BOOL WINAPI hookClipCursor(const RECT* rect) {
        return menuisOpen ? TRUE : oClipCursor(rect);
    }

    namespace mousehooks {
        void Init() {
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32)
                return;

            if (auto addr = GetProcAddress(user32, "SetCursorPos")) {
                if (MH_CreateHook(addr, reinterpret_cast<LPVOID>(hookSetCursorPos), reinterpret_cast<LPVOID*>(&oSetCursorPos)) == MH_OK) {
                    MH_EnableHook(addr);
                    Log("[mousehooks] Hooked SetCursorPos@%p\n", addr);
                }
            }

            if (auto addr = GetProcAddress(user32, "ClipCursor")) {
                if (MH_CreateHook(addr, reinterpret_cast<LPVOID>(hookClipCursor), reinterpret_cast<LPVOID*>(&oClipCursor)) == MH_OK) {
                    MH_EnableHook(addr);
                    Log("[mousehooks] Hooked ClipCursor@%p\n", addr);
                }
            }
        }

        void Remove() {
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if (!user32)
                return;

            if (auto addr = GetProcAddress(user32, "SetCursorPos")) {
                MH_DisableHook(addr);
                MH_RemoveHook(addr);
            }

            if (auto addr = GetProcAddress(user32, "ClipCursor")) {
                MH_DisableHook(addr);
                MH_RemoveHook(addr);
            }
        }
    }

    //=======================================================================================//

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

    //=======================================================================================//

    ID3D12DescriptorHeap* g_GameSRVHeap = nullptr;

    //=======================================================================================//
    
    #include <shared_mutex>
    #include <unordered_map>

    std::shared_mutex rootSigMutex;
    uint32_t nextRuntimeSigID = 1;
    std::unordered_map<ID3D12RootSignature*, uint32_t> rootSigToID;

    // Thread-local storage: Each thread tracks its own active command list and ID
    thread_local ID3D12GraphicsCommandList* tlsCurrentCmdList = nullptr;
    thread_local uint32_t tlsCurrentRootSigID = 0;
