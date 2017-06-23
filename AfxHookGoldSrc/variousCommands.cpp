#include "stdafx.h"

#include "cmdregister.h"
#include "hooks/HookHw.h"
#include <shared/hooks/gameOverlayRenderer.h>

REGISTER_CMD_FUNC(gameoverlay)
{
	int argc = pEngfuncs->Cmd_Argc();

	if(3 <= argc)
	{
		const char * arg1 = pEngfuncs->Cmd_Argv(1);

		if(0 == _stricmp("enable", arg1))
		{
			bool value = 0 != atoi(pEngfuncs->Cmd_Argv(2));
			pEngfuncs->Con_Printf(
				"%s %s.\n",
				value ? "Enable" : "Disable",
				GameOverlay_Enable(value) ? "OK" : "FAILED"
			);
			return;
		}
	}

	pEngfuncs->Con_Printf(
		"Usage:\n"
		PREFIX "gameoverlay enable 0|1 - Disable/Enable the GameOverlay (will only do s.th. useful when it was enabled initally).\n"
	);
}
