#pragma once
#include "dxgi.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

class Graphics {
    struct WndProc {
        static LRESULT thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static inline WNDPROC func;

    };

    struct CreateD3DAndSwapChain {
        static inline IDXGISwapChain* swapChain = nullptr;
        static inline ID3D11Device* device = nullptr;
        static inline ID3D11DeviceContext* context = nullptr;
        static void thunk();
        static inline REL::Relocation<decltype(thunk)> func;
        static void Install();
    };

    struct DrawHook {
        static void thunk(std::uint32_t a_timer);
        static inline REL::Relocation<decltype(thunk)> func;
        static void Install();
    };
    static inline std::vector<std::function<void()>> drawFunctions;
    static float GetResolutionScale();

public:
    static void Register(std::function<void()> drawFunction);
    static void Render();
    static void Install();
};


ImVec2 WorldToScreenLoc(RE::NiPoint3 position);
