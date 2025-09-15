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

struct MyColor {
	afxUtils::RGBA value;
	afxUtils::RGBA defaultValue;
	std::string userValue = "";
	bool use = false;

	bool setColor(const char* arg);
	bool setColor(advancedfx::ICommandArgs* args); 
};
