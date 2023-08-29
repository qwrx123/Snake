#include "../headers/SnakeWindow.h"

#include "../headers/BaseWindow.h"

#include <Windowsx.h>
#include <windows.h>
#include <D2d1.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Winmm.lib")

SnakeWindow::SnakeWindow()
{

}

SnakeWindow::~SnakeWindow()
{
	delete myScene;
}

LRESULT SnakeWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
    case WM_PAINT:
    {
		myScene->render();
        return 0;
    }
	case WM_SIZE:
	{
		RECT rc;

		GetClientRect(m_hwnd, &rc);

		myScene->resize(rc);

		return 0;
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 300;
		lpMMI->ptMinTrackSize.y = 300;
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		D2D1_POINT_2F clickPoint{(float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)};
		myScene->onCapture(clickPoint);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		D2D1_POINT_2F clickPoint{(float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)};
		myScene->onClick(clickPoint);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		D2D1_POINT_2F movePoint{(float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)};
		myScene->onMove(movePoint);
		return 0;
	}
	case WM_CHAR:
	{
		myScene->onKey((wchar_t)wParam);
		return 0;
	}
	case WM_KEYDOWN:
	{
		myScene->keyDown((wchar_t)wParam);
		return 0;
	}
	case WM_KEYUP:
	{
		myScene->keyUp((wchar_t)wParam);
		return 0;
	}
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

bool SnakeWindow::forceRender()
{
    return true;
}

PCWSTR  SnakeWindow::ClassName() const { return L"Snake"; }

BOOL SnakeWindow::Create(
    PCWSTR lpWindowName,
    DWORD dwStyle,
    DWORD dwExStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu
    ) 
{
    bool returnBool = BaseWindow::Create(
        lpWindowName,
        dwStyle,
    	dwExStyle,
    	x,
    	y,
    	nWidth,
    	nHeight,
    	hWndParent,
    	hMenu
    );
	
	myScene = new scene(m_hwnd);

	return returnBool;
}