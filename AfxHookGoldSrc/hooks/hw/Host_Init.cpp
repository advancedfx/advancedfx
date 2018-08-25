#include "stdafx.h"

#include "Host_Init.h"
#include <hl_addresses.h>
#include "../HookGameLoaded.h"

#include <shared/detours.h>

struct quakeparms_t;

typedef void (*Host_Init_t)(quakeparms_t *parms);

Host_Init_t g_Old_Host_Init = 0;


void New_Host_Init(quakeparms_t *parms)
{
	static bool firstRun = true;

	g_Old_Host_Init(parms);

	if(firstRun)
	{
		firstRun = false;
		HookGameLoaded();
	}
}

void Hook_Host_Init()
{
	if( !g_Old_Host_Init && 0 != HL_ADDR_GET(Host_Init) )
	{
		g_Old_Host_Init = (Host_Init_t) DetourApply((BYTE *)HL_ADDR_GET(Host_Init), (BYTE *)New_Host_Init, (int)HL_ADDR_GET(Host_Init_DSZ));
	}
}
