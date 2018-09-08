#include "stdafx.h"

#include "R_DrawParticles.h"

#include <hl_addresses.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>

bool g_In_R_DrawViewModel = false;

typedef void (*R_DrawViewModel_t) (void);
R_DrawViewModel_t g_Old_R_DrawViewModel = 0;

void New_R_DrawViewModel(void)
{
	g_In_R_DrawViewModel = true;
	g_Old_R_DrawViewModel();
	g_In_R_DrawViewModel = false;
}

bool Hook_R_DrawViewModel()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(R_DrawViewModel))
	{
		LONG error = NO_ERROR;

		g_Old_R_DrawViewModel = (R_DrawViewModel_t)AFXADDR_GET(R_DrawViewModel);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_R_DrawViewModel, New_R_DrawViewModel);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_DrawViewModel");
		}
	}
	else
		firstResult = false;

	return firstResult;
}
