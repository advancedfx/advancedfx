#include "stdafx.h"

#include "CL_Disconnect.h"
#include <hl_addresses.h>
#include "../../filming.h"
#include <shared/detours.h>

typedef void (*CL_Disconnect_t) (void);

CL_Disconnect_t g_Old_CL_Disconnect = 0;

void New_CL_Disconnect (void)
{
	g_Filming.On_CL_Disconnect();

	g_Old_CL_Disconnect();
}

void Hook_CL_Disconnect()
{
	if( !g_Old_CL_Disconnect && 0 != HL_ADDR_GET(CL_Disconnect) )
	{
		g_Old_CL_Disconnect = (CL_Disconnect_t) DetourApply((BYTE *)HL_ADDR_GET(CL_Disconnect), (BYTE *)New_CL_Disconnect, (int)HL_ADDR_GET(CL_Disconnect_DSZ));
	}
}
