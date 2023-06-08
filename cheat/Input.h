#pragma once
 
class Input {
public:
    void Initialize(HWND windowHandle);
    void Takedown(HWND windowHandle);
};

static LRESULT APIENTRY WndProc(HWND windowHandle, UINT message, WPARAM wParameter, LPARAM lParamter);