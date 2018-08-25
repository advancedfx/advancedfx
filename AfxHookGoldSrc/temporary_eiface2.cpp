#include <wrect.h>
#include <cl_dll.h>
#include <cdll_int.h>
#include "cmdregister.h"

extern cl_enginefuncs_s *pEngfuncs;

extern void HLeiface_install();
extern bool HLeiface_test(int iReceiver, int iSender, bool bListen);
extern float HLeiface_gettime();
extern void HLeiface_setmodel(int i, const char *m);
extern void HLeiface_test2(char *szs);

REGISTER_DEBUGCMD_FUNC(test_eiface)
{
	HLeiface_install();

	char *szs;

	HLeiface_test2(szs);
	pEngfuncs->Con_Printf("GameDir: %s\n",szs);
}

REGISTER_DEBUGCMD_FUNC(test_setlisten)
{
	if (pEngfuncs->Cmd_Argc() != 4)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "voice_setlisten RecivePlayerEntIndex SendPlayerEntIndex 0|1\n");
		return;
	}

	HLeiface_install(); // make sure eiface is installed

	int iReceiver = atoi(pEngfuncs->Cmd_Argv(1));
	int iSender = atoi(pEngfuncs->Cmd_Argv(2));
	bool bListen = ( 0 != atoi(pEngfuncs->Cmd_Argv(3)) );

	pEngfuncs->Con_Printf("Returned %s\n",
		HLeiface_test(iReceiver,iSender,bListen) ? "True" : "False"
	);
}

REGISTER_DEBUGCMD_FUNC(test_timings)
{
	HLeiface_install();

	pEngfuncs->Con_Printf("EngineTime: %f\nClientTime: %f\n",HLeiface_gettime(),pEngfuncs->GetClientTime());
}

REGISTER_DEBUGCMD_FUNC(test_setmodel)
{
	if (pEngfuncs->Cmd_Argc() != 3)
	{
		pEngfuncs->Con_Printf("Useage: " PREFIX "fx_setmodel EntityIndex ModelString\n");
		return;
	}

	HLeiface_install();
	HLeiface_setmodel(atoi(pEngfuncs->Cmd_Argv(1)),pEngfuncs->Cmd_Argv(2));
}

