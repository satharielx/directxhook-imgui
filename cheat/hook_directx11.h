#pragma once

#ifndef DIRECTX11_INCLUDED
#define DIRECTX11_INCLUDED
#endif // !DIRECTX11_INCLUDED
#define DIRECTX11_API

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

extern DIRECTX11_API void DirectXHook_Present(ID3D11Device* device, ID3D11DeviceContext* deviceContext, IDXGISwapChain* deviceSwapChain);

DIRECTX11_API void DirectXHook_Intialize(HMODULE module, HWND hWnd);

DIRECTX11_API void DirectXHook_Shutdown();