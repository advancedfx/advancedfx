#pragma once

#include "Globals.h"
#include <windows.h>
#include "../deps/release/Detours/src/detours.h"

void HookMirvCommands(HMODULE clientDll);
