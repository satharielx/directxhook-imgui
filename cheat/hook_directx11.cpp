#include <Windows.h>
#include <d3d11.h>
#include <mutex>
#include "imgui.h"
#include "imgui_impl_dx11.h"

#include <MinHook.h>

#include "hook_directx11.h"

#pragma comment(lib, "d3d11.lib")

const int menuKey = VK_F9;
const int shutdownKey = VK_DELETE;

#include "Menu.h"
#include "Input.h"
#include <iostream>

Menu menu = Menu();
Input input = Input();

//Main DirectX API Calls
typedef HRESULT(__stdcall *D3D11PresentHook) 
(IDXGISwapChain* swapChain, 
    UINT interval, 
    UINT flags);
typedef void(__stdcall *D3D11DrawIndexedHook) 
(ID3D11DeviceContext* context, 
    UINT indexCount, 
    UINT startIdx,
    INT vertexLocation);
typedef void(__stdcall *D3D11CreateQueryHook) 
(ID3D11Device* device, 
    const D3D11_QUERY_DESC *queryDescription, 
    ID3D11Query **query);
typedef void(__stdcall *D3D11PSSetShaderResourcesHook) 
(ID3D11DeviceContext* context,
    UINT slotBegin, 
    UINT views, 
    ID3D11ShaderResourceView *const *ShaderResourceViews);
typedef void(__stdcall *D3D11ClearRenderTargetViewHook) 
(ID3D11DeviceContext* context, 
    ID3D11RenderTargetView *renderTargetView, 
    const FLOAT RGBA[4]);

//Properties
static HWND WindowHandle = nullptr;
static HMODULE ModuleHandle = nullptr;
static ID3D11Device* DirectXDevice = nullptr;
static ID3D11DeviceContext* DirectXDeviceContext = nullptr;
static IDXGISwapChain* DirectXGraphicInterfaceSwapChain = nullptr;
static std::once_flag Intialized;

D3D11PresentHook presentHook = nullptr;
D3D11DrawIndexedHook indexedHook = nullptr;
D3D11CreateQueryHook queryHook = nullptr;
D3D11PSSetShaderResourcesHook shaderResourceHook = nullptr;
D3D11ClearRenderTargetViewHook targetViewHook = nullptr;

DWORD_PTR* swapChainTable = nullptr;
DWORD_PTR* deviceTable = nullptr;
DWORD_PTR* deviceContextTable = nullptr;

DIRECTX11_API void DirectXHook_Present(
    ID3D11Device *device, 
    ID3D11DeviceContext *context, 
    IDXGISwapChain *swapChain) {
    
    if (GetAsyncKeyState(menuKey) & 0x1) {
        if (menu.isAlreadyOpen) menu.isAlreadyOpen = false;
        else menu.isAlreadyOpen = true;
    }

    menu.InitializeAndShow();
}

HRESULT __stdcall PresentHook(
    IDXGISwapChain* swapChain, 
    UINT interval, 
    UINT flags) {

    std::call_once(Intialized, [&]() {
        swapChain->GetDevice(__uuidof(DirectXDevice),
            reinterpret_cast<void**>(&DirectXDevice));
        DirectXDevice->GetImmediateContext(&DirectXDeviceContext);
        ImGui_ImplDX11_Init(DirectXDevice, DirectXDeviceContext);
        input.Initialize(WindowHandle);
    });

    DirectXHook_Present(DirectXDevice, 
        DirectXDeviceContext, 
        DirectXGraphicInterfaceSwapChain);
    
    return presentHook(DirectXGraphicInterfaceSwapChain, 
        interval, flags);
}

void __stdcall CreateD3DQuery(
    ID3D11Device* device, 
    const D3D11_QUERY_DESC *queryDescription, 
    ID3D11Query **query) {

    if (queryDescription->Query == D3D11_QUERY_OCCLUSION) {
        D3D11_QUERY_DESC oqueryDesc = CD3D11_QUERY_DESC();
        (&oqueryDesc)->MiscFlags = queryDescription->MiscFlags;
        (&oqueryDesc)->Query = D3D11_QUERY_TIMESTAMP;
        return queryHook(device, &oqueryDesc, query);
    }

    return queryHook(device, queryDescription, query);
}

UINT startSlot;
D3D11_SHADER_RESOURCE_VIEW_DESC description;
void __stdcall SetShaderResources(ID3D11DeviceContext* ctx, 
    UINT sSlot, UINT nViews, 
    ID3D11ShaderResourceView *const *shaderResourceView) {
    
    startSlot = sSlot;
    for (UINT j = 0; j < nViews; j++) {
        ID3D11ShaderResourceView* shaderRView = shaderResourceView[j];
        if (shaderRView) {
            shaderRView->GetDesc(&description);
            if ((description.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) ||
                (description.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX)) continue;
        }
    }
    return shaderResourceHook(ctx, sSlot, nViews, shaderResourceView);
}

void __stdcall DrawIndexedHook(ID3D11DeviceContext* context,
    UINT count, UINT startIdx, INT vertexLocation) {
    return indexedHook(context, count, startIdx, vertexLocation);
}

void __stdcall ClearRenderTargetViewHook(ID3D11DeviceContext* ctx,
    ID3D11RenderTargetView* renderTargetView, const FLOAT ColorRGBA[4]) {
    return targetViewHook(ctx, renderTargetView, ColorRGBA);
}


DWORD __stdcall IntializeDX11Hook() {
    printf("[!][DirectX 11] Hello from DirectX API!\n");
    /*WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "Injected";
    windowClass.hIconSm = NULL;

    HWND window = CreateWindow(windowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 300, 300, NULL, NULL, windowClass.hInstance, NULL);

    RegisterClassEx(&windowClass);
    Sleep(1000);*/

    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
    D3D_FEATURE_LEVEL selectedLevel;
    DXGI_SWAP_CHAIN_DESC swapChainDescription;
    {
        ZeroMemory(&swapChainDescription, sizeof(swapChainDescription));
        swapChainDescription.BufferCount = 1;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swapChainDescription.OutputWindow = WindowHandle;
        swapChainDescription.SampleDesc.Count = 1;
       // swapChainDescription.SampleDesc.Quality = 0;
        swapChainDescription.Windowed = ((GetWindowLongPtr(WindowHandle, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
        swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        
        swapChainDescription.BufferDesc.Width = 1;
        swapChainDescription.BufferDesc.Height = 1;
        swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    }
    
    
    HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
        0, levels, 1, D3D11_SDK_VERSION,
        &swapChainDescription, &DirectXGraphicInterfaceSwapChain,
        &DirectXDevice, &selectedLevel, &DirectXDeviceContext);
    
    if (FAILED(result)) {
        //MessageBox(WindowHandle, "Failed to create d3d11 device and swapchain.", "Critical Error", MB_ICONERROR);
        printf("[!][DirectX 11] Failed to create d3d11 device and swapchain.");
        char c = 'c';
        std::cin >> c;
        return E_FAIL;
    }

    


    printf("[!][DirectX 11] Swap Chain Description set!\n");

    swapChainTable = (DWORD_PTR*)(DirectXGraphicInterfaceSwapChain);
    swapChainTable = (DWORD_PTR*)(swapChainTable[1]);

    deviceTable = (DWORD_PTR*)(DirectXDevice);
    deviceTable = (DWORD_PTR*)deviceTable[0];

    deviceContextTable = (DWORD_PTR*)(DirectXDeviceContext);
    deviceContextTable = (DWORD_PTR*)deviceContextTable[0];
    printf("[!][DirectX 11] Variable tables set!\n");

    if (MH_Initialize() != MH_OK) {
        printf("[!][DirectX 11] Cannot initialize MinHook");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_CreateHook((DWORD_PTR*)swapChainTable[8], PresentHook, reinterpret_cast<void**>(&presentHook)) != MH_OK) {
        printf("[!][DirectX 11] Cannot create hook with name 'PresentHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_EnableHook((DWORD_PTR*)swapChainTable[8]) != MH_OK) { 
        printf("[!][DirectX 11] Cannot enable hook with name 'PresentHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    
    }
    if (MH_CreateHook((DWORD_PTR*)deviceContextTable[12], DrawIndexedHook, reinterpret_cast<void**>(&indexedHook)) != MH_OK) {
        printf("[!][DirectX 11] Cannot create hook with name 'IndexHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_EnableHook((DWORD_PTR*)deviceContextTable[12]) != MH_OK) { 
        printf("[!][DirectX 11] Cannot enable hook with name 'IndexHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_CreateHook((DWORD_PTR*)deviceTable[24], CreateD3DQuery, reinterpret_cast<void**>(&queryHook)) != MH_OK) { 
        printf("[!][DirectX 11] Cannot create hook with name 'D3DQuery'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_EnableHook((DWORD_PTR*)deviceTable[24]) != MH_OK) { 
        printf("[!][DirectX 11] Cannot enable hook with name 'D3DQuery'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_CreateHook((DWORD_PTR*)deviceContextTable[8], SetShaderResources, reinterpret_cast<void**>(&shaderResourceHook)) != MH_OK) { 
        printf("[!][DirectX 11] Cannot create hook with name 'SetShaderResources'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_EnableHook((DWORD_PTR*)deviceContextTable[8]) != MH_OK) { 
        printf("[!][DirectX 11] Cannot enable hook with name 'SetShaderResources'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_CreateHook((DWORD_PTR*)swapChainTable[50], ClearRenderTargetViewHook, reinterpret_cast<void**>(&targetViewHook)) != MH_OK) { 
        printf("[!][DirectX 11] Cannot create hook with name 'ClearRenderTargetViewHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    if (MH_EnableHook((DWORD_PTR*)swapChainTable[50]) != MH_OK) { 
        printf("[!][DirectX 11] Cannot enable hook with name 'ClearRenderTargetViewHook'");
        char c = 'c';
        std::cin >> c;
        return 1; 
    }
    printf("[!][DirectX 11] Hooks are ready.!\n");
    DWORD oldprotection;
    VirtualProtect(presentHook, 2, PAGE_EXECUTE_READWRITE, &oldprotection);
    //ImGui::SetCurrentContext(ImGui::CreateContext());
    //ImGui_ImplDX11_Init(DirectXDevice, DirectXDeviceContext);
    
    do {
        
        /*if (GetAsyncKeyState(menuKey) & 0x1) {
            if (menu.isAlreadyOpen) menu.isAlreadyOpen = false;
            else menu.isAlreadyOpen = true;
            printf("[!][ImGui] Menu status: %s", menu.isAlreadyOpen ? "OPEN" : "CLOSED");
            
        }
        menu.InitializeAndShow();
        */
        
        Sleep(100);
    } while (!(GetAsyncKeyState(shutdownKey) & 0x1));

    DirectXDevice->Release();
    DirectXDeviceContext->Release();
    DirectXGraphicInterfaceSwapChain->Release();

    DirectXHook_Shutdown();
    input.Takedown(WindowHandle);

    Beep(200, 100);
    FreeLibraryAndExitThread(ModuleHandle, 0);
    return S_OK;
}

DIRECTX11_API void DirectXHook_Intialize(HMODULE moduleHandle, HWND windowHandle) {
    WindowHandle = windowHandle;
    ModuleHandle = moduleHandle;
    printf("[!] Initializing DirectX API...\n");
    IntializeDX11Hook();
}


DIRECTX11_API void DirectXHook_Shutdown() {
    if (MH_DisableHook(MH_ALL_HOOKS)) return;
    if (MH_Uninitialize()) return;
}