#include "stdafx.h"

#include "hooks/HookHw.h"
#include "hooks/hw/ClientFunctions.h"
#include "cmdregister.h"
#include <deps/release/halflife/common/const.h>

bool g_bForcePlayersSolid = false;

typedef void (*V_CalcRefdef_t)( struct ref_params_s *pparams );

V_CalcRefdef_t g_Old_V_CalcRefdef;

void New_V_CalcRefdef( struct ref_params_s *pparams )
{
	if(g_bForcePlayersSolid)
	{
		for (int i = 0; i <= pEngfuncs->GetMaxClients(); i++)
		{
			cl_entity_t *e = pEngfuncs->GetEntityByIndex(i);
			if (e && e->player)
			{
				e->curstate.solid = SOLID_BSP;
			}
		}
	}

	g_Old_V_CalcRefdef(pparams);
}

void Hook_V_CalcRefdef(void)
{
	static bool firstRun=true;
	if(!firstRun) return;
	firstRun = false;

	g_Old_V_CalcRefdef = (V_CalcRefdef_t)GetClientFunction(CFTE_V_CalcRefdef);
	ReplaceClientFunction(CFTE_V_CalcRefdef, (void *)&New_V_CalcRefdef);
}


REGISTER_DEBUGCMD_FUNC(force_players_solid)
{
	bool showHelp = true;
	int argc = pEngfuncs->Cmd_Argc();

	Hook_V_CalcRefdef();

	if(2 == argc)
	{
		showHelp = false;

		g_bForcePlayersSolid = 0 != atoi(pEngfuncs->Cmd_Argv(1));
	}

	if(showHelp)
	{
		pEngfuncs->Con_Printf("Usage: " DEBUG_PREFIX "force_players_solid 0|1\n");
	}
}
