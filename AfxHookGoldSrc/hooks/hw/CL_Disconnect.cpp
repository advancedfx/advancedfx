#include "stdafx.h"

#include "CL_Disconnect.h"
#include <hl_addresses.h>
#include "../../filming.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

typedef void (*CL_Disconnect_t) (void);

CL_Disconnect_t g_Old_CL_Disconnect = 0;

void New_CL_Disconnect (void)
{
	g_Filming.On_CL_Disconnect();

	g_Old_CL_Disconnect();
}

bool Hook_CL_Disconnect()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	LONG error = NO_ERROR;

	g_Old_CL_Disconnect = (CL_Disconnect_t)AFXADDR_GET(CL_Disconnect);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_Old_CL_Disconnect, New_CL_Disconnect);
	error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		firstResult = false;
		ErrorBox("Interception failed:\nhw.dll:CL_Disconnect");
	}

	return firstResult;
}
