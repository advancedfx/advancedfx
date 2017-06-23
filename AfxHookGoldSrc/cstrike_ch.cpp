#include "stdafx.h"

#include <hlsdk.h>
#include "hooks/HookHw.h"
#include "hooks/client/cstrike/CrossHairFix.h"
#include "cmdregister.h"


REGISTER_DEBUGCMD_FUNC(cstrike_ch_fps)
{
	bool bShowHelp = true;

	if(1 < pEngfuncs->Cmd_Argc() )
	{
		double fVal = atof(pEngfuncs->Cmd_Argv(1) );

		Cstrike_CrossHair_Fps_set(fVal);

		bShowHelp = false;
	}

	if (bShowHelp)
	{
		pEngfuncs->Con_Printf(
			"Usage: " DEBUG_PREFIX "cstrike_ch_fps <fpsToLookLike> (0.0 = disable fix)\n"
			"Current value: %f\n",
			Cstrike_CrossHair_Fps_get()
		);
	}
}
