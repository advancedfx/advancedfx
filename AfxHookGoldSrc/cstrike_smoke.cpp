#include "stdafx.h"

#include <hlsdk.h>

#include "cmdregister.h"
#include "hl_addresses.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

extern cl_enginefuncs_s *pEngfuncs;

typedef void ( *pfnEvent_t )( struct event_args_s *args );

pfnEvent_t detoured_cstrike_EN_CreateSmoke = NULL;

bool g_cstrike_EN_CreateSmoke_Block = false;
bool g_cstrike_EN_CreateSmoke_UserCall = false;


void touring_cstrike_EN_CreateSmoke( struct event_args_s *args ) {
	if(g_cstrike_EN_CreateSmoke_UserCall || !g_cstrike_EN_CreateSmoke_Block)
		detoured_cstrike_EN_CreateSmoke(args);

	//pEngfuncs->Con_DPrintf("touring_cstrike_EN_CreateSmoke\n");
}


bool InstallHook_cstrike_EN_CreateSmoke()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	static bool bFirstRun = true;

	if (AFXADDR_GET(cstrike_EV_CreateSmoke))
	{
		LONG error = NO_ERROR;

		detoured_cstrike_EN_CreateSmoke = (pfnEvent_t)AFXADDR_GET(cstrike_EV_CreateSmoke);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)detoured_cstrike_EN_CreateSmoke, touring_cstrike_EN_CreateSmoke);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			//ErrorBox("Interception failed:\InstallHook_cstrike_EN_CreateSmoke()");
		}
	}
	else
		firstResult = false;


	return firstResult;
}


	//void		(*pfnPlaybackEvent)			( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

void FakeSmoke(float x, float y, float z, float pitch, float yaw, float roll) {
	event_args_s args;

	args.flags    = 0;
	args.entindex = 0;

	args.origin[0] = x;
	args.origin[1] = y;
	args.origin[2] = z;

	args.angles[0] = pitch;
	args.angles[1] = yaw;
	args.angles[2] = roll;

	args.velocity[0] = 0;
	args.velocity[1] = 0;
	args.velocity[2] = 0;

	args.ducking = 0;

	args.fparam1 = 0;
	args.fparam2 = 0;

	args.iparam1 = 0;
	args.iparam2 = 1; // ?

	args.bparam1 = 0; // ?
	args.bparam2 = 0;

	g_cstrike_EN_CreateSmoke_UserCall = true;
	touring_cstrike_EN_CreateSmoke(&args);
	g_cstrike_EN_CreateSmoke_UserCall = false;
}


REGISTER_CMD_FUNC(cstrike_smoke)
{
	if(!InstallHook_cstrike_EN_CreateSmoke()) {
		pEngfuncs->Con_Printf("AfxError: Could not install hook.\n");
		return;
	}

	int argc = pEngfuncs->Cmd_Argc();

	if(2 <= argc) {
		char * subcmd = pEngfuncs->Cmd_Argv(1);

		if(!_stricmp("block", subcmd) && 2 == argc) {
			pEngfuncs->Con_Printf( "Block is %s.\n", g_cstrike_EN_CreateSmoke_Block ? "enabled" : "disabled");
			return;
		}
		else if(!_stricmp("block", subcmd) && 3 == argc) {
			g_cstrike_EN_CreateSmoke_Block = 1 == atoi(pEngfuncs->Cmd_Argv(2));

			return;
		}
		else
		if(!_stricmp("create", subcmd) && 8 == argc) {
			FakeSmoke(
				(float)atof(pEngfuncs->Cmd_Argv(2)),
				(float)atof(pEngfuncs->Cmd_Argv(3)),
				(float)atof(pEngfuncs->Cmd_Argv(4)),
				(float)atof(pEngfuncs->Cmd_Argv(5)),
				(float)atof(pEngfuncs->Cmd_Argv(6)),
				(float)atof(pEngfuncs->Cmd_Argv(7))
			);

			return;
		}
	}

	pEngfuncs->Con_Printf(
		PREFIX "cstrike_smoke block 0|1\n"
		PREFIX "cstrike_smoke create <x> <y> <z> <pitch> <yaw> <roll>\n"
	);
	return;
}
