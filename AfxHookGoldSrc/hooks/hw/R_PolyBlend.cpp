#include "stdafx.h"

#include "R_PolyBlend.h"

#include <hl_addresses.h>

#include <hlsdk.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>

//	R_PolyBlend hook (usefull for flashhack etc.)
//

typedef void (*R_PolyBlend_t) (void);
R_PolyBlend_t g_Old_R_PolyBlend = 0;

bool g_R_PolyBlend_Block = false;

void New_R_PolyBlend (void)
{
	if( !g_R_PolyBlend_Block ) g_Old_R_PolyBlend();
}

bool Hook_R_PolyBlend()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(R_PolyBlend))
	{
		LONG error = NO_ERROR;

		g_Old_R_PolyBlend = (R_PolyBlend_t)AFXADDR_GET(R_PolyBlend);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_R_PolyBlend, New_R_PolyBlend);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_PolyBlend");
		}
	}
	else
		firstResult = false;
	
	return firstResult;
}


