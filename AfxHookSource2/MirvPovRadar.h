#pragma once

#include <Windows.h>

class CEntityInstance;

CEntityInstance * GetFakePovRadarController();
int GetFakePovRadarControllerIndex();
bool IsFakePovRadarFrameContextActive();
bool ConsumeFakePovRadarFrameContextWasActive();
void SetFakePovRadarControllerIndex(int index);
void SetFakePovRadarAutoSync(bool enabled);
bool GetFakePovRadarAutoSync();

void MirvPov_ApplyRadarPatches(HMODULE clientDll);
void MirvPov_RemoveRadarPatches();
void MirvPov_RemoveShowAllNOP();
