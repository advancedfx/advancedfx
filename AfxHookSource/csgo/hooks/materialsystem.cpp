#include "stdafx.h"

#include "materialsystem.h"

#include "../../AfxInterop.h"

#include "addresses.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


typedef void*(__fastcall * csgo_CMaterialSystem_Lock_t)(void * This, void * Edx);

csgo_CMaterialSystem_Lock_t g_Org_csgo_CMaterialSystem_Lock;

void __fastcall My_csgo_CMaterialSystem_Lock(void* This, void* Edx) {

	AfxInterop::On_Materialysystem_FlushQueue();

	g_Org_csgo_CMaterialSystem_Lock(This, Edx);
}

bool Hook_csgo_Materialsystem(SOURCESDK::IMaterialSystem_csgo* materialSystem) {
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (materialSystem)
	{
		LONG error = NO_ERROR;

		g_Org_csgo_CMaterialSystem_Lock = (csgo_CMaterialSystem_Lock_t)((void**)(*(void***)materialSystem))[118];

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Org_csgo_CMaterialSystem_Lock, My_csgo_CMaterialSystem_Lock);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
