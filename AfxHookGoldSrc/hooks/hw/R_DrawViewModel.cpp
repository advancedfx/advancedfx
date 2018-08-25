#include "stdafx.h"

#include "R_DrawParticles.h"

#include <hl_addresses.h>

#include <shared/detours.h>

bool g_In_R_DrawViewModel = false;

typedef void (*R_DrawViewModel_t) (void);
R_DrawViewModel_t g_Old_R_DrawViewModel = 0;

void New_R_DrawViewModel(void)
{
	g_In_R_DrawViewModel = true;
	g_Old_R_DrawViewModel();
	g_In_R_DrawViewModel = false;
}

void Hook_R_DrawViewModel()
{
	if( !g_Old_R_DrawViewModel && 0 != HL_ADDR_GET(R_DrawViewModel) )
	{
		g_Old_R_DrawViewModel = (R_DrawViewModel_t) DetourApply((BYTE *)HL_ADDR_GET(R_DrawViewModel), (BYTE *)New_R_DrawViewModel, (int)HL_ADDR_GET(DTOURSZ_R_DrawViewModel));
	}
}
