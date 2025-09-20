#pragma once

#include "Globals.h"

void HookSceneSystem(HMODULE sceneSystemDll);
void HookMaterialSystem(HMODULE materialSystemDll);

void* new_ForceUpdateSkybox(void* This);

class CMaterial2
{
public:
	virtual const char* GetName() = 0;
	virtual const char* GetShareName() = 0;
};

struct GetMaterialsArrayResult {
	uint64_t count;
	CMaterial2*** pArrMaterials;
	uint64_t unk;
};

class CResourceSystem {
public:
	CMaterial2** PreCache(const char* name);
	void GetMaterials(GetMaterialsArrayResult* out);
};

extern CResourceSystem* g_pCResourceSystem;
