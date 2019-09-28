#include "stdafx.h"

#include "csgo_CCSGameMovement.h"

#include "addresses.h"

#include <shared/AfxDetours.h>

bool g_Enable_csgo_CCSGameMovement_DuckFix = false;


//:009
typedef void(__fastcall * csgo_CCSGameMovement_DuckShit_t)(void *This, void* Edx);

csgo_CCSGameMovement_DuckShit_t detoured_csgo_CCSGameMovement_DuckShit;

void __fastcall csgo_CCSGameMovement_DuckShit(void* This, void* Edx)
{
	if (!g_Enable_csgo_CCSGameMovement_DuckFix)
		detoured_csgo_CCSGameMovement_DuckShit(This, Edx);
}

bool Hook_csgo_CCSGameMovement_DuckFix(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CCSGameMovement_vtable))
	{
		DWORD * vtable = (DWORD *)(AFXADDR_GET(csgo_CCSGameMovement_vtable));

		AfxDetourPtr((PVOID *)&(vtable[57]), csgo_CCSGameMovement_DuckShit, (PVOID *)&detoured_csgo_CCSGameMovement_DuckShit);

		firstResult = true;
	}

	return firstResult;
}
