#pragma once

#include "Globals.h"
#include <windows.h>
#include "../deps/release/Detours/src/detours.h"

void HookMirvColors(HMODULE clientDll);

struct AfxBasicColor {
	const char* name;
	afxUtils::RGBA value;
};

extern std::vector<AfxBasicColor> afxBasicColors;
