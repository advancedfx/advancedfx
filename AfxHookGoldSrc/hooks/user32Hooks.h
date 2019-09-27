#pragma once

#include <Windows.h>
#include <shared/AfxDetours.h>

extern HWND g_GameWindow;
extern int g_Height;
extern int g_Width;

extern CAfxImportFuncHookBase* g_pImport_USER32_CreateWindowExW;
extern CAfxImportFuncHookBase* g_pImport_USER32_DestroyWindow;

void CloseGameWindow();
void RedockGameWindow();
void UndockGameWindowForCapture();
