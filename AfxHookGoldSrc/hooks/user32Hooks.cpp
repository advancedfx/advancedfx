#include "stdafx.h"

#include "user32Hooks.h"

#include <shared/AfxDetours.h>

#include "../AfxSettings.h"
#include "../supportrender.h"

HWND g_GameWindow = NULL;
bool g_GameWindowActive = false;
WNDPROC g_GameWindowProc = NULL;
bool g_GameWindowUndocked = false;
DWORD g_OldWindowStyle;


LRESULT CALLBACK NewGameWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	// one could filter window unspecific messages here.

	if (hWnd==NULL || hWnd != g_GameWindow)
		// this is not the GameWindow we want to control
		return g_GameWindowProc(hWnd,uMsg,wParam,lParam);

	// filter GameWindow specific messages:

	switch (uMsg)
	{
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
			g_GameWindowActive = false;
		break;
	case WM_MOUSEACTIVATE:
		if( !g_AfxSettings.FullScreen_get() && !g_GameWindowUndocked && !g_GameWindowActive )
		{
			// Client Windows won't recieve window activation events,
			// so we will fake them:
			g_GameWindowActive = true;

			LRESULT lr = g_GameWindowProc(hWnd,uMsg,wParam,lParam); // pass on WM_MOUSEACTIVATE
			g_GameWindowProc(hWnd, WM_ACTIVATEAPP, TRUE, NULL);
			g_GameWindowProc(hWnd, WM_ACTIVATE, WA_ACTIVE, NULL);//lParam);

			// Don't let strange mods like Natural Selection mess with us:
			ShowCursor(TRUE);
			return lr;
		}
		// Don't let strange mods like Natural Selection mess with us:
		ShowCursor(TRUE);
		break;
	case WM_SETFOCUS:
		break;
	case WM_KILLFOCUS:
		if( !g_AfxSettings.FullScreen_get() && !g_GameWindowUndocked && g_GameWindowActive )
		{
			g_GameWindowActive = false;
			g_GameWindowProc(hWnd, WM_ACTIVATE, WA_INACTIVE, NULL);//lParam);
			g_GameWindowProc(hWnd, WM_ACTIVATEAPP, FALSE, NULL);
			return g_GameWindowProc(hWnd, uMsg,wParam,lParam); // PASS ON WM_KILLFOCUS
		}
		break;
	}

	return g_GameWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND APIENTRY NewCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
CAfxImportFuncHook<HWND(APIENTRY*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID)>* Get_Import_USER32_CreateWindowExW_Internal() {
	static CAfxImportFuncHook<HWND(APIENTRY*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID)> g_Import_USER32_CreateWindowExW("CreateWindowExW", NewCreateWindowExW);
	return &g_Import_USER32_CreateWindowExW;
}
CAfxImportFuncHookBase* Get_Import_USER32_CreateWindowExW() {
	return Get_Import_USER32_CreateWindowExW_Internal();
}
HWND APIENTRY NewCreateWindowExW(DWORD dwExStyle,LPCWSTR lpClassName,LPCWSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
	if (NULL != hWndParent || lstrcmpW(L"",lpWindowName))
		// it's not the window we want.
		return Get_Import_USER32_CreateWindowExW_Internal()->TrueFunc(dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);

	// it's the window we want.
	
	if(!g_AfxSettings.FullScreen_get())
	{
		// currently won't work:
		/*// modifiy some properities to our needs:
		dwStyle = WS_CHILD; // | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		dwExStyle = WS_EX_NOPARENTNOTIFY;
		hWndParent = g_AfxGoldSrcComClient.GetParentWindow();
		x = 0;
		y = 0;*/
	}
	
	g_GameWindow = Get_Import_USER32_CreateWindowExW_Internal()->TrueFunc( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );

	g_GameWindowProc = (WNDPROC)GetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC);
	//char t[100];
	//sprintf_s(t,"0x%08x",g_GameWindowProc);
	//MessageBox(0,t,"g_GameWIndowProc",MB_OK);

	// We can't set a new windowproc, this will get us an endless loop because SDL
	// is just fucked up, so we hook the SDL one:
	//SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG)NewGameWindowProc);
	//g_GameWindowProc = (WNDPROC)DetourApply((BYTE *)g_GameWindowProc, (BYTE *)NewGameWindowProc, 0x09);

	return g_GameWindow;
}

BOOL APIENTRY NewDestroyWindow(HWND hWnd);

CAfxImportFuncHook<BOOL(APIENTRY*)(HWND)>* Get_Import_USER32_DestroyWindow_Internal() {
	static CAfxImportFuncHook<BOOL(APIENTRY*)(HWND)> g_Import_USER32_DestroyWindow("DestroyWindow", NewDestroyWindow);
	return &g_Import_USER32_DestroyWindow;
}
CAfxImportFuncHookBase* Get_Import_USER32_DestroyWindow() {
	return Get_Import_USER32_DestroyWindow_Internal();
}
BOOL APIENTRY NewDestroyWindow(HWND hWnd)
{
	if (hWnd != NULL && hWnd == g_GameWindow)
	{
		// H-L main game window being destroyed

		g_GameWindow = NULL;

		if (g_pSupportRender) delete g_pSupportRender;
	}

	return Get_Import_USER32_DestroyWindow_Internal()->TrueFunc(hWnd);
}


void CloseGameWindow()
{
	if(0 != g_GameWindow)
	{
		SendMessage(g_GameWindow, WM_CLOSE, 0, 0);
	}
}

void RedockGameWindow()
{
	if(!g_AfxSettings.FullScreen_get() && g_GameWindowUndocked)
	{
		DWORD dwExStyle = GetWindowLong(g_GameWindow, GWL_EXSTYLE);

		// restore old style and parent (see SetParent() on MSDN2, why we do it in this order):
		SetWindowLongPtr( g_GameWindow, GWL_STYLE, g_OldWindowStyle);
		//SetParent( g_GameWindow, g_AfxGoldSrcComClient.GetParentWindow() );
		RECT windowRect = {0, 0, g_AfxSettings.Width_get(), g_AfxSettings.Height_get() };
		AdjustWindowRectEx(&windowRect, g_OldWindowStyle, FALSE, dwExStyle);
		SetWindowPos( g_GameWindow, HWND_NOTOPMOST, 0, 0, windowRect.right -windowRect.left, windowRect.bottom -windowRect.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW);

		g_GameWindowUndocked = false;
	}
}


void UndockGameWindowForCapture()
{
	if(!g_AfxSettings.FullScreen_get() && !g_GameWindowUndocked)
	{
		g_GameWindowUndocked = true;

		g_OldWindowStyle = GetWindowLongPtr(g_GameWindow, GWL_STYLE);
		DWORD dwExStyle = GetWindowLong(g_GameWindow, GWL_EXSTYLE);
		// set new parent and style (see SetParent() on MSDN2, why we do it in this order):
		SetParent( g_GameWindow, NULL );
		SetWindowLongPtr( g_GameWindow, GWL_STYLE, WS_POPUP );
		RECT windowRect = {0, 0, g_AfxSettings.Width_get(), g_AfxSettings.Height_get() };
		AdjustWindowRectEx(&windowRect, WS_POPUP, FALSE, dwExStyle);
		SetWindowPos( g_GameWindow, HWND_TOPMOST, 0, 0, windowRect.right -windowRect.left, windowRect.bottom -windowRect.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW);
	}
}
