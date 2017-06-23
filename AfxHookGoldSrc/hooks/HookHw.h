#pragma once

#include <windows.h>

extern struct cl_enginefuncs_s * pEngfuncs;
extern struct engine_studio_api_s * pEngStudio;
extern struct playermove_s * ppmove;

/// <summary> Installs hw.dll hooks </summary>
void HookHw(HMODULE hHw);
