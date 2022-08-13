#include "stdafx.h"

// See also: /doc/notes_goldsrc/debug_cstrike_PM_CatagorizePosition.txt

#include "SpectatorFix.h"

#include <windows.h>

#include <shared/AfxDetours.h>
#include <hlsdk.h>
#include "../../../hl_addresses.h"

#include "../../HookHw.h"
#include "../../hw/Host_Frame.h"

#include <gl/gl.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

typedef int (__cdecl*UnkCstrikeSpectatorFn_t)();
UnkCstrikeSpectatorFn_t g_pfnSpectatorFix_Hooked_Func = NULL;

int __cdecl SpectatorFix_Hooking_Func()
{
	if (pEngfuncs->pfnGetCvarFloat("mirv_cstrike_spectator_fix") == 0.0f)
		g_pfnSpectatorFix_Hooked_Func();

	return 0;
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
