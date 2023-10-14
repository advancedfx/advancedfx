#pragma once

#include <Windows.h>
#include <shared/AfxDetours.h>

extern HWND g_GameWindow;
extern int g_Height;
extern int g_Width;

extern CAfxImportFuncHookBase* Get_Import_USER32_CreateWindowExW();
extern CAfxImportFuncHookBase* Get_Import_USER32_DestroyWindow();
extern CAfxImportFuncHookBase* Get_Import_USER32_GetCursorPos();
extern CAfxImportFuncHookBase* Get_Import_USER32_SetCursorPos();
extern CAfxImportFuncHookBase* Get_Import_client_USER32_GetCursorPos();
extern CAfxImportFuncHookBase* Get_Import_client_USER32_SetCursorPos();

void CloseGameWindow();
void RedockGameWindow();
void UndockGameWindowForCapture();
