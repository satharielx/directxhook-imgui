#include <windows.h>
#include <string>
#include <imgui.h>
#include "imgui_impl_dx11.h"
#include <dxgi.h>
#include <d3d11.h>

#include "Input.h"

using namespace ImGui;

WNDPROC windowProcess;

void Input::Initialize(HWND windowHandle) {
    windowProcess = (WNDPROC)SetWindowLongPtr(windowHandle, 
        GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
}

void Input::Takedown(HWND windowHandle) {
    SetWindowLongPtr(windowHandle, GWLP_WNDPROC, (LONG_PTR)windowProcess);
}

LRESULT APIENTRY WndProc(HWND windowHandle, UINT messageRecieved, WPARAM wParamater, LPARAM lParameter) {
    switch (messageRecieved) {
    case WM_LBUTTONDOWN:
        GetIO().MouseDown[0] = true;
        return DefWindowProc(windowHandle, messageRecieved, 
            wParamater, lParameter);
        break;
   
    case WM_LBUTTONUP:
        GetIO().MouseDown[0] = false;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);

    case WM_RBUTTONDOWN:
        GetIO().MouseDown[1] = true;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);
        break;

    case WM_RBUTTONUP:
        GetIO().MouseDown[1] = false;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);

    case WM_MBUTTONDOWN:
        GetIO().MouseDown[2] = true;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);
        break;

    case WM_MBUTTONUP:
        GetIO().MouseDown[2] = false;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);

    case WM_MOUSEWHEEL:
        GetIO().MouseWheel += GET_WHEEL_DELTA_WPARAM(wParamater) > 0 ?
            +1.0f : -1.0f;
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);
        break;

    case WM_MOUSEMOVE:
        GetIO().MousePos.x = (signed short)(lParameter);
        GetIO().MousePos.y = (signed short)(lParameter >> 16);
        return DefWindowProc(windowHandle, messageRecieved,
            wParamater, lParameter);
        break;
    }
    return CallWindowProc(windowProcess, windowHandle, messageRecieved, wParamater, lParameter);
}