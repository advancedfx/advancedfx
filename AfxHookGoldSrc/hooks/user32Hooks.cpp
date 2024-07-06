#include "stdafx.h"

#include "user32Hooks.h"

#include "../AfxSettings.h"
#include "../supportrender.h"
#include "../mirv_input.h"

#include "../shared/MirvInput.h"

#include "../../shared/AfxDetours.h"
#include <Windows.h>
#include "../../deps/release/Detours/src/detours.h"

void ErrorBox(char const * messageText);

HWND g_GameWindow = NULL;
bool g_GameWindowActive = false;
bool g_GameWindowUndocked = false;
DWORD g_OldWindowStyle;

WNDPROC g_NextWindProc;
static bool g_afxWindowProcSet = false;

LRESULT CALLBACK new_Afx_WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
			g_GameWindowActive = false;
		
		MirvInput_Get()->Supply_Focus(LOWORD(wParam) != 0);
		break;
	case WM_CHAR:
		if (MirvInput_Get()->Supply_CharEvent(wParam, lParam))
			return 0;
		break;
	case WM_KEYDOWN:
		if (MirvInput_Get()->Supply_KeyEvent(MirvInput::KS_DOWN, wParam, lParam))
			return 0;
		break;
	case WM_KEYUP:
		if (MirvInput_Get()->Supply_KeyEvent(MirvInput::KS_UP, wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
		if (MirvInput_Get()->Supply_MouseEvent(uMsg, wParam, lParam))
			return 0;
		break;
	case WM_MOUSEACTIVATE:
		if( !g_AfxSettings.FullScreen_get() && !g_GameWindowUndocked && !g_GameWindowActive )
		{
			// Client Windows won't recieve window activation events,
			// so we will fake them:
			g_GameWindowActive = true;

			LRESULT lr = g_NextWindProc(hWnd,uMsg,wParam,lParam); // pass on WM_MOUSEACTIVATE
			g_NextWindProc(hWnd, WM_ACTIVATEAPP, TRUE, NULL);
			g_NextWindProc(hWnd, WM_ACTIVATE, WA_ACTIVE, NULL);//lParam);

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
			g_NextWindProc(hWnd, WM_ACTIVATE, WA_INACTIVE, NULL);//lParam);
			g_NextWindProc(hWnd, WM_ACTIVATEAPP, FALSE, NULL);
			return g_NextWindProc(hWnd, uMsg,wParam,lParam); // PASS ON WM_KILLFOCUS
		}
		break;
	}

	return g_NextWindProc(hWnd, uMsg, wParam, lParam);
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
	if (NULL != hWndParent || 0 == HIWORD(lpClassName) || 0 != lstrcmpW(L"SDL_app",lpClassName) || 0 != lstrcmpW(L"",lpWindowName))
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

	g_NextWindProc = (WNDPROC)GetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC);
	//char t[100];
	//sprintf_s(t,"0x%08x",g_GameWindowProc);
	//MessageBox(0,t,"g_GameWIndowProc",MB_OK);

	// We can't set a new windowproc, this will get us an endless loop because SDL
	// is just fucked up, so we hook the SDL one:
	//SetWindowLongPtrW(g_GameWindow, GWLP_WNDPROC, (LONG)new_Afx_WindowProc);
	//g_afxWindowProcSet = true;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_NextWindProc, new_Afx_WindowProc);
	if(NO_ERROR != DetourTransactionCommit())
		ErrorBox("DetourTransactionCommit failed:\new_Afx_WindowProc");

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

		if (g_pSupportRender) {
			delete g_pSupportRender;
			g_pSupportRender = nullptr;
		}
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

BOOL WINAPI new_GetCursorPos(
	__out LPPOINT lpPoint
)
{
	BOOL result = GetCursorPos(lpPoint);

	//	if (AfxHookSource::Gui::OnGetCursorPos(lpPoint))
	//		return TRUE;

	MirvInput_Get()->Supply_GetCursorPos(lpPoint);

	return result;
}

BOOL WINAPI new_SetCursorPos(
	__in int X,
	__in int Y
)
{
	//	if (AfxHookSource::Gui::OnSetCursorPos(X, Y))
	//		return TRUE;

	BOOL result = SetCursorPos(X, Y);
	if(result) MirvInput_Get()->Supply_SetCursorPos(X, Y);
	return result;
}



CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)>* Get_Import_USER32_GetCursorPos_Internal() {
	static CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos);
	return &g_Import_USER32_GetCursorPos;
}
CAfxImportFuncHookBase* Get_Import_USER32_GetCursorPos() {
	return Get_Import_USER32_GetCursorPos_Internal();
}

CAfxImportFuncHook<BOOL(WINAPI*)(int, int)>* Get_Import_USER32_SetCursorPos_Internal() {
	static CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);
	return &g_Import_USER32_SetCursorPos;
}
CAfxImportFuncHookBase* Get_Import_USER32_SetCursorPos() {
	return Get_Import_USER32_SetCursorPos_Internal();
}

CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)>* Get_Import_client_USER32_GetCursorPos_Internal() {
	static CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_client_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos);
	return &g_Import_client_USER32_GetCursorPos;
}
CAfxImportFuncHookBase* Get_Import_client_USER32_GetCursorPos() {
	return Get_Import_client_USER32_GetCursorPos_Internal();
}

CAfxImportFuncHook<BOOL(WINAPI*)(int, int)>* Get_Import_client_USER32_SetCursorPos_Internal() {
	static CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_client_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);
	return &g_Import_client_USER32_SetCursorPos;
}
CAfxImportFuncHookBase* Get_Import_client_USER32_SetCursorPos() {
	return Get_Import_client_USER32_SetCursorPos_Internal();
}
