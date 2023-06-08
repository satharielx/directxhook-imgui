// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "atlstr.h"

#include <d3d11.h>
#include <mutex>

#include "hook_directx11.h"


#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "d3d11.lib")

#pragma comment (lib, "user32.lib")

std::string WindowTitle = "Discord";
HWND mainWindow;
std::string keyword = "League of Legends (TM) Client";
static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    int length = GetWindowTextLength(hWnd);
    char* buffer = new char[length + 1];
    GetWindowText(hWnd, buffer, length + 1);

    std::string windowTitle(buffer);
    delete[] buffer;

    
    if (IsWindowVisible(hWnd) && length != 0) {
        std::cout << hWnd << ":  " << windowTitle << std::endl;
        if (windowTitle.find(keyword) != std::string::npos) {
            mainWindow = hWnd;
            WindowTitle = windowTitle;
        }
    }
    return TRUE;
}

DWORD WINAPI MainThread(HMODULE moduleHandle) {
    system("chcp 1251");
    MessageBox(NULL, "Injected!", "Injection complete!", MB_OK);
    Sleep(1000);
    AllocConsole();
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    printf("[!] Made by Sathariel\n");
    std::cout << "[!] Injection process with thread handle " << moduleHandle << " completed!" << "\n";
    std::cout << "[!] Listing all windows open.." << "\n";
    
    EnumWindows(enumWindowCallback, NULL);
    if (mainWindow != nullptr) {
        printf("Window Handle found! Handle: 0x%1X, Title: %s\n", mainWindow, WindowTitle.c_str());
        DirectXHook_Intialize(moduleHandle, mainWindow);
    }
    
    

    return S_OK;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            
            break;
    }
    return TRUE;
}

