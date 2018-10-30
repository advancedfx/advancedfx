#include "stdafx.h"

#include "engine.h"

#include "../../addresses.h"
#include "SourceInterfaces.h"
#include "../../WrpConsole.h"

#include <shared/detours.h>

using namespace SOURCESDK::CSGO;
using namespace SOURCESDK::CSGO::panorama;

extern CGameUIFuncs * g_pGameUIFuncs;
extern CPanoramaUIEngine * g_pPanoramaUIEngine;
extern CPanoramaUIClient * g_pPanoramaUIClient;

CTopLevelWindowSource2 * panoramaDebuggerTopLevelWindow = 0;
HWND panoramaDebuggerHwnd = 0;

LRESULT CALLBACK PanoramaDebuggerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

CON_COMMAND(mirv_panorama_toggledebugger, "Toggle Panorama UI debugger") {
	if (!(false && g_pGameUIFuncs && g_pPanoramaUIEngine && g_pPanoramaUIClient))
	{
		Tier0_Warning("Error: Missing interfacess.\n");
		return;
	}

	static CDebugger * debugger = nullptr;

	if (nullptr == debugger) {

		/*
		WNDCLASSEXA wndClass = {
			sizeof(WNDCLASSEXA),
			CS_OWNDC | CS_DBLCLKS,
			PanoramaDebuggerWndProc,
			0,
			0,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			"ValvePanDebugger"
		};

		RECT windowRect = { 0, 0, 600, 400 };

		DWORD wndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;

		ATOM atomWndClass = RegisterClassExA(&wndClass);

		AdjustWindowRectEx(&windowRect, wndStyle, FALSE, 0);

		panoramaDebuggerHwnd = CreateWindowExA(0, wndClass.lpszClassName, "Panorama Debugger", wndStyle, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.left, NULL, NULL, NULL, NULL);
		*/
		CGameUIFuncs_SomeShit_t * someShit = g_pGameUIFuncs->GetSomeShit();
		CUIEngineSource2 * uiEngineSource = g_pPanoramaUIEngine->GetUIEngineSoruce2();
		panoramaDebuggerTopLevelWindow = uiEngineSource->CreateTopLevelWindow(0, 0, 600, 400, 0, 0, 0, 1, "PanoramaDebugger", someShit);

		//ShowWindowAsync(panoramaDebuggerHwnd, SW_SHOWNORMAL);

		debugger = g_pPanoramaUIClient->CreateDebugger(panoramaDebuggerTopLevelWindow, "Debugger");

	}

	Tier0_Msg("Debugger=0x%08x\n", debugger);
}
