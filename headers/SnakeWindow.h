#pragma once
#ifndef UNICODE
#define UNICODE
#endif 
#include "BaseWindow.h"
#include "scene.h"
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Winmm.lib")

class SnakeWindow : public BaseWindow<SnakeWindow>
{
public:
    SnakeWindow();
    ~SnakeWindow();
    
    BOOL Create(
		PCWSTR lpWindowName,
    	DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
        );

    bool forceRender();

    PCWSTR  ClassName() const;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	scene* myScene;
};

