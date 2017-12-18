#pragma once

#include <d3d9.h>

namespace AfxHookSource {
namespace Gui {

//bool SetActive(bool value);

/// <returns>Whether the event was already handled or not.</returns>
bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam);
bool OnSetCursorPos(__in int X, __in int Y);
bool OnGetCursorPos(__out LPPOINT lpPoint);
bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result);
bool OnSetCapture(HWND hWnd, HWND & result);
bool OnReleaseCapture();
bool OnGameFrameStart();

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device);
void On_Direct3DDevice9_Shutdown();

void On_Direct3DDevice9_EndScene();
void On_Direct3DDevice9_Present(bool deviceLost);
void On_Direct3DDevice9_Reset_Before();
void On_Direct3DDevice9_Reset_After();


} // namespace Gui {
} // namespace AfxHookSource {

