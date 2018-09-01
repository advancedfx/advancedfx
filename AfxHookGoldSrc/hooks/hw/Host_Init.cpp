#include "stdafx.h"

#include "Host_Init.h"
#include <hl_addresses.h>
#include "../HookGameLoaded.h"

#include <Windows.h>
#include <shared/Detours/src/detours.h>

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

bool Hook_Host_Init()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	LONG error = NO_ERROR;

	g_Old_Host_Init = (Host_Init_t)AFXADDR_GET(Host_Init);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_Old_Host_Init, New_Host_Init);
	error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		firstResult = false;
		ErrorBox("Interception failed:\nhw.dll:Host_Init");
	}
	
	return firstResult;
}
