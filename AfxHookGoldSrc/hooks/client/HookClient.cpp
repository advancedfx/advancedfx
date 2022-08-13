#include "stdafx.h"

#include "HookClient.h"

#include "../HookHw.h"

#include "cstrike/CrossHairFix.h"
#include "cstrike/SpectatorFix.h"

void HookClient()
{
	static bool firstRun = true;
	if(!firstRun) return;
	firstRun = false;

	//
	// detect game:

	const char *gamedir = pEngfuncs->pfnGetGameDirectory();
	
	if(!gamedir) return;
	
	if(0 == _stricmp("cstrike",gamedir))
	{
		Hook_Cstrike_CrossHair_Fix();
		Hook_Cstrike_Spectator_Fix();
	}
}
