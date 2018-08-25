#include "stdafx.h"

#include "HltvFix.h"

#include <shared/detours.h>

#include "hooks/HookHw.h"
#include "hooks/hw/ClientFunctions.h"

#include "cmdregister.h"
#include "hl_addresses.h"

#include <hlsdk.h>

REGISTER_DEBUGCVAR(fixforcehltv, "1", 0);
REGISTER_DEBUGCVAR(force_thirdperson, "0", 0);

bool g_FixForceHltvEnabled = false;

bool g_InClEmitEntities = false;

typedef void (*CL_EmitEntities_t)(void);
CL_EmitEntities_t g_Old_CL_EmitEntities;

void New_CL_EmitEntities(void)
{
	g_InClEmitEntities = true;

	g_Old_CL_EmitEntities();

	g_InClEmitEntities = false;
}


typedef int (*CL_IsThirdPerson_t)( void );
CL_IsThirdPerson_t g_OldClientCL_IsThirdPerson;


//	bool bNotInEye = ppmove->iuser1 != 4;
//	int iwatched = ppmove->iuser2;

int NewClientCL_IsThirdPerson( void )
{
	if(force_thirdperson->value)
		return 1 == force_thirdperson->value ? 1 : 0;

	if(g_FixForceHltvEnabled && g_InClEmitEntities && fixforcehltv->value)
		return 1;

	return g_OldClientCL_IsThirdPerson();
}

xcommand_t OldClientCmdDemForceHltv = NULL;

void InstallHltvFix(void)
{
	static bool firstRun=true;
	if(!firstRun) return;
	firstRun = false;

	g_Old_CL_EmitEntities = (CL_EmitEntities_t)DetourApply((BYTE *)HL_ADDR_GET(CL_EmitEntities), (BYTE *)New_CL_EmitEntities, (int)HL_ADDR_GET(CL_EmitEntities_DSZ));

	g_OldClientCL_IsThirdPerson = (CL_IsThirdPerson_t)GetClientFunction(CFTE_CL_IsThirdPerson);
	ReplaceClientFunction(CFTE_CL_IsThirdPerson, (void *)&NewClientCL_IsThirdPerson);
}


void NewClientCmdDemForceHltv(void)
{
	char *ptmp="";
	if (pEngfuncs->Cmd_Argc()>=1) ptmp=pEngfuncs->Cmd_Argv(1);
	g_FixForceHltvEnabled = (0 != atof(ptmp));

	if(g_FixForceHltvEnabled)
	{
		InstallHltvFix();
	}

	OldClientCmdDemForceHltv();
}

// << dem_forcehltv fix
