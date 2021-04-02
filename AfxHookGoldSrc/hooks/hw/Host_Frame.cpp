#include "stdafx.h"

#include "Host_Frame.h"
#include <hl_addresses.h>
#include "../HookGameLoaded.h"
#include "../../GameRecord.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


typedef void (*Host_Frame_t) (float time);

Host_Frame_t g_Old_HostFrame = 0;
double * g_phost_frametime = 0;
float g_Host_Frame_time = 0;
bool g_Host_Frame_Called = false;

void New_Host_Frame (float time)
{
	g_Host_Frame_Called = true;

	g_GameRecord.BeforeHostFrame();

	g_Host_Frame_time = time;
	g_Old_HostFrame(time);
}

bool Hook_Host_Frame()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	LONG error = NO_ERROR;

	g_phost_frametime = (double *)HL_ADDR_GET(host_frametime);
	g_Old_HostFrame = (Host_Frame_t)AFXADDR_GET(_Host_Frame);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_Old_HostFrame, New_Host_Frame);
	error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		firstResult = false;
		ErrorBox("Interception failed:\nhw.dll:Host_Frame");
	}

	return firstResult;
}