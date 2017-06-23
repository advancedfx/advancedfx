#include "stdafx.h"

#include "DemoPlayer.h"

#include "../interface.h"

IDemoPlayer001 * g_DemoPlayer = 0;

void Hook_DemoPlayer(void * hModule)
{
	static bool firstRun = true;
	if(!firstRun) return;
	firstRun = false;

	CreateInterfaceFn demoPlayerCreateInterface = Sys_GetFactory((CSysModule *)hModule);

	if(0 != demoPlayerCreateInterface)
	{
		int returnCode = 0;
		g_DemoPlayer = (IDemoPlayer001 *)demoPlayerCreateInterface("demoplayer001", &returnCode);
	}
}
