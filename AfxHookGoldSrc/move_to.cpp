#include "stdafx.h"

#include <shared/AfxDetours.h>
#include "hl_addresses.h"
#include "hooks/HookHw.h"
#include "hooks/hw/ClientFunctions.h"

#include "cmdregister.h"

#include <hlsdk.h>
#include <deps/release/halflife/common/ref_params.h>

typedef void (*HUD_PlayerMove_t)( struct playermove_s *ppmove, int server );

HUD_PlayerMove_t g_Old_Hud_PlayerMove;

struct {
	float angles[3];
	float origin[3];
	bool setAngles;
	bool setOrigin;
} g_Move;


void New_Hud_PlayerMove( struct playermove_s *ppmove, int server )
{
	g_Old_Hud_PlayerMove(ppmove, server);

	if(g_Move.setAngles)
	{
		g_Move.setAngles = false;

		ppmove->angles[0] = g_Move.angles[0];
		ppmove->angles[1] = g_Move.angles[1];
		ppmove->angles[2] = g_Move.angles[2];
	}

	if(g_Move.setOrigin)
	{
		g_Move.setOrigin = false;

		ppmove->origin[0] = g_Move.origin[0];
		ppmove->origin[1] = g_Move.origin[1];
		ppmove->origin[2] = g_Move.origin[2];
	}
}


void Hook_Hud_PalyerMove(void)
{
	static bool firstRun=true;
	if(!firstRun) return;
	firstRun = false;

	g_Move.setAngles = false;
	g_Move.setOrigin = false;

	g_Old_Hud_PlayerMove = (HUD_PlayerMove_t)GetClientFunction(CFTE_HUD_PlayerMove);
	ReplaceClientFunction(CFTE_HUD_PlayerMove, (void *)&New_Hud_PlayerMove);
}


REGISTER_DEBUGCMD_FUNC(moveto)
{
	bool showHelp = true;
	int argc = pEngfuncs->Cmd_Argc();

	Hook_Hud_PalyerMove();

	if(4 == argc || 7 == argc)
	{
		showHelp = false;

		g_Move.origin[0] = (float)atof(pEngfuncs->Cmd_Argv(1));
		g_Move.origin[1] = (float)atof(pEngfuncs->Cmd_Argv(2));
		g_Move.origin[2] = (float)atof(pEngfuncs->Cmd_Argv(3));
		g_Move.setOrigin = true;

		if(7 == argc)
		{
			g_Move.angles[0] = (float)atof(pEngfuncs->Cmd_Argv(4));
			g_Move.angles[1] = (float)atof(pEngfuncs->Cmd_Argv(5));
			g_Move.angles[2] = (float)atof(pEngfuncs->Cmd_Argv(6));
			g_Move.setAngles = true;

			pEngfuncs->SetViewAngles(g_Move.angles);
		}

	}

	if(showHelp)
	{
		pEngfuncs->Con_Printf("Usage: " DEBUG_PREFIX "simorg_set x y z [ang0 ang1 ang2]\n");
	}


}
