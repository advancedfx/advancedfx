#pragma once

#include <Windows.h>

void MirvPovHud_ApplyPatches(HMODULE clientDll);
void MirvPovHud_RemovePatches();
void MirvPovHud_UpdateSeekDetection(int curTick);
bool MirvPovHud_ShouldSuppressFrame();
