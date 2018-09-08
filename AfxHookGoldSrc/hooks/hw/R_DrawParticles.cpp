#include "stdafx.h"

#include "R_DrawParticles.h"

#include <hl_addresses.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>

bool g_In_R_DrawParticles = false;

typedef void (*R_DrawParticles_t) (void);
R_DrawParticles_t g_Old_R_DrawParticles = 0;

void New_R_DrawParticles (void)
{
	g_In_R_DrawParticles = true;
	g_Old_R_DrawParticles();
	g_In_R_DrawParticles = false;
}

bool Hook_R_DrawParticles()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(R_DrawParticles))
	{
		LONG error = NO_ERROR;

		g_Old_R_DrawParticles = (R_DrawParticles_t)AFXADDR_GET(R_DrawParticles);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_R_DrawParticles, New_R_DrawParticles);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_DrawParticles");
		}
	}
	else
		firstResult = false;

	return firstResult;
}
