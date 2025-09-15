#pragma once

#include "Globals.h"

void HookSceneSystem(HMODULE sceneSystemDll);

void* new_ForceUpdateSkybox(void* This);

class CMaterial2
{
public:
	virtual const char* GetName() = 0;
	virtual const char* GetShareName() = 0;
};

class CResourceSystem {
public:
	CMaterial2** PreCache(const char* name);
};

extern CResourceSystem* g_pCResourceSystem;
