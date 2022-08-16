#include "stdafx.h"

// See also: /doc/notes_goldsrc/debug_cstrike_PM_CatagorizePosition.txt

#include "SpectatorFix.h"

#include <windows.h>
#include <gl/gl.h>
#include <shared/AfxDetours.h>
#include <hlsdk.h>
#include <deps/release/Detours/src/detours.h>

#include "../../../hl_addresses.h"
#include "../../HookHw.h"
#include "../../hw/Host_Frame.h"
#include "../../../cmdregister.h"

REGISTER_CVAR(cstrike_spectator_fix, "1", 0);

typedef void (__cdecl*UnkCstrikeSpectatorFn_t)();
UnkCstrikeSpectatorFn_t g_pfnSpectatorFix_Hooked_Func = NULL;

extern playermove_s* ppmove;

void __cdecl SpectatorFix_Hooking_Func()
{
	if ((ppmove->spectator || ppmove->iuser1 > 0) && cstrike_spectator_fix->value != 0) {
		ppmove->onground = -1;
		ppmove->waterlevel = 0;
		ppmove->watertype = CONTENTS_EMPTY;
		return;
	}

	g_pfnSpectatorFix_Hooked_Func();
}

bool Hook_Cstrike_Spectator_Fix()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	BYTE* addrFn = (BYTE*)HL_ADDR_GET(cstrike_PM_CatagorizePositionFn);

	if (!(addrFn))
	{
		firstResult = false;
	}
	else
	{
		LONG error = NO_ERROR;

		g_pfnSpectatorFix_Hooked_Func = (UnkCstrikeSpectatorFn_t)addrFn;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_pfnSpectatorFix_Hooked_Func, SpectatorFix_Hooking_Func);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_Cstrike_Spectator_Fix()");
		}
	}

	return firstResult;
}
