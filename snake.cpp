#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include "headers/BaseWindow.h"
#include "headers/SnakeWindow.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Winmm.lib")

#include <iostream>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    SnakeWindow win;
	
    if (!win.Create(L"Snake", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    win.forceRender();

    // Run the message loop.

    MSG msg = { };
	while (TRUE)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				return 0;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
            win.forceRender();
		}
    }
    return 0;
}