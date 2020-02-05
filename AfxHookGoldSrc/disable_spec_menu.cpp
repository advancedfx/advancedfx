#include "stdafx.h"

#include "hooks/HookHw.h"
#include "cmdregister.h"
#include "hl_addresses.h"
#include "hlsdk.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

float g_OldHud_draw_value;
bool g_DisableSpecMenu = false;

typedef int (__fastcall *TeamFortressViewport_UpdateSpectatorPanel_t)(void * This, void * edx);

TeamFortressViewport_UpdateSpectatorPanel_t g_Old_TeamFortressViewport_UpdateSpectatorPanel;

void __fastcall New_TeamFortressViewport_UpdateSpectatorPanel(void * This, void * edx)
{
	if(!g_DisableSpecMenu)
	{
		g_Old_TeamFortressViewport_UpdateSpectatorPanel(This, edx);
		return;
	}

	g_OldHud_draw_value = pEngfuncs->pfnGetCvarFloat("hud_draw");
	
	pEngfuncs->Cvar_SetValue("hud_draw", 0.0f);

	g_Old_TeamFortressViewport_UpdateSpectatorPanel(This, edx);

	pEngfuncs->Cvar_SetValue("hud_draw", g_OldHud_draw_value);
}

bool Hook_TeamFortressViewport_UpdateSpectatorPanel_tfc(void)
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(tfc_TeamFortressViewport_UpdateSpecatorPanel))
	{
		LONG error = NO_ERROR;

		g_Old_TeamFortressViewport_UpdateSpectatorPanel = (TeamFortressViewport_UpdateSpectatorPanel_t)AFXADDR_GET(tfc_TeamFortressViewport_UpdateSpecatorPanel);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_TeamFortressViewport_UpdateSpectatorPanel, New_TeamFortressViewport_UpdateSpectatorPanel);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
		}
	}
	else
		firstResult = false;

	return firstResult;
}

bool Hook_TeamFortressViewport_UpdateSpectatorPanel_valve(void)
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(valve_TeamFortressViewport_UpdateSpecatorPanel))
	{
		LONG error = NO_ERROR;

		g_Old_TeamFortressViewport_UpdateSpectatorPanel = (TeamFortressViewport_UpdateSpectatorPanel_t)AFXADDR_GET(valve_TeamFortressViewport_UpdateSpecatorPanel);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_TeamFortressViewport_UpdateSpectatorPanel, New_TeamFortressViewport_UpdateSpectatorPanel);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
		}
	}
	else
		firstResult = false;

	return firstResult;
}

bool Hook_TeamFortressViewport_UpdateSpectatorPanel(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	const char *gameDir = pEngfuncs->pfnGetGameDirectory();
	
	if(gameDir)
	{	
		if(0 == _stricmp("tfc",gameDir))
		{
			firstResult = Hook_TeamFortressViewport_UpdateSpectatorPanel_tfc();
		}
		else if(0 == _stricmp("valve",gameDir))
		{
			firstResult = Hook_TeamFortressViewport_UpdateSpectatorPanel_valve();
		}
	}

	return firstResult;
}

REGISTER_CMD_FUNC(disable_specmenu)
{
	if(!Hook_TeamFortressViewport_UpdateSpectatorPanel())
	{
		pEngfuncs->Con_Printf(
			"Error: Hook not installed.\n"
			"Maybe your modification \"%s\" is not supported?\n",
			pEngfuncs->pfnGetGameDirectory()
		);
		return;
	}

	int argc = pEngfuncs->Cmd_Argc();

	if(2 == argc)
	{
		g_DisableSpecMenu = 0 != atoi(pEngfuncs->Cmd_Argv(1));
		return;
	}

	pEngfuncs->Con_Printf(
		PREFIX "disable_specmenu 0|1 (currently: %i)\n",
		g_DisableSpecMenu ? 1 : 0
	);
}