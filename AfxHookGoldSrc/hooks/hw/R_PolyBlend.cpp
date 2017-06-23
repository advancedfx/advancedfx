#include "stdafx.h"

#include "R_PolyBlend.h"

#include <shared/detours.h>

#include <hl_addresses.h>

#include <hlsdk.h>



//	R_PolyBlend hook (usefull for flashhack etc.)
//

typedef void (*R_PolyBlend_t) (void);
R_PolyBlend_t g_Old_R_PolyBlend = 0;

bool g_R_PolyBlend_Block = false;

void New_R_PolyBlend (void)
{
	if( !g_R_PolyBlend_Block ) g_Old_R_PolyBlend();
}

void Hook_R_PolyBlend()
{
	if (!g_Old_R_PolyBlend && (HL_ADDR_GET(R_PolyBlend)!=NULL))
			g_Old_R_PolyBlend = (R_PolyBlend_t) DetourApply((BYTE *)HL_ADDR_GET(R_PolyBlend), (BYTE *)New_R_PolyBlend, (int)HL_ADDR_GET(DTOURSZ_R_PolyBlend));
}


