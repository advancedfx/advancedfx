#include "stdafx.h"

#include "hooks/hw/R_PolyBlend.h"

#include "cmdregister.h"

#include "hooks/HookHw.h"

REGISTER_CMD_FUNC(fx_noblend)
{
	if( 2 == pEngfuncs->Cmd_Argc() )
	{
		int i = atoi(pEngfuncs->Cmd_Argv(1));
		g_R_PolyBlend_Block = i == 1;
	} else {
		pEngfuncs->Con_Printf("Usage:\n" PREFIX "fx_noblend 0/1 = normal/block blends\n");
	}
}

