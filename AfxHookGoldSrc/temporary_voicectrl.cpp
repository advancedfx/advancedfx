//
// File        : voicectrl.cpp
// Started     : 2007-12-07T20:30Z
// Project     : Mirv Demo Tool
// Authors     : Dominik Tugend
// Description : For Blocking a specific player's voice in demo mode
//

#include "wrect.h"
#include "cl_dll.h"
#include "cdll_int.h"
#include "r_efx.h"
#include "com_model.h"
#include "r_studioint.h"
#include "pm_defs.h"
#include "cvardef.h"
#include "entity_types.h"

#include "cmdregister.h"

extern cl_enginefuncs_s *pEngfuncs;

// >>voice_common.h:

#define VOICE_MAX_PLAYERS		32	// (todo: this should just be set to MAX_CLIENTS).
#define VOICE_MAX_PLAYERS_DW	((VOICE_MAX_PLAYERS / 32) + !!(VOICE_MAX_PLAYERS & 31))

// <<voice_common.h:

// most informations taken from game_shared\voice_gamemgr.cpp

REGISTER_CMD_FUNC(voice_block)
{
	/*if (pEngfuncs->Cmd_Argc() != 4)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "matte_setcolourf <red: 0.0-1.0> <green: 0.0-1.0> <blue: 0.0-1.0>\n");
		return;
	}

	float flRed = (float) atof(pEngfuncs->Cmd_Argv(1));
	float flGreen = (float) atof(pEngfuncs->Cmd_Argv(2));
	float flBlue = (float) atof(pEngfuncs->Cmd_Argv(3));

	_mirv_matte_setcolorf(flRed, flBlue, flGreen);*/

	MESSAGE_BEGIN(MSG_ONE, m_msgPlayerVoiceMask, NULL, pPlayer->pev);
	int dw;
	for(dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{
		WRITE_LONG(gameRulesMask.GetDWord(dw));
		WRITE_LONG(g_BanMasks[iClient].GetDWord(dw));
	}
	MESSAGE_END();
}

