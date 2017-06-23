#include "stdafx.h"

#include "Host_Frame.h"
#include <hl_addresses.h>
#include "../HookGameLoaded.h"

#include <shared/detours.h>

typedef void (*Host_Frame_t) (float time);

Host_Frame_t g_Old_HostFrame = 0;
double * g_phost_frametime = 0;
float g_Host_Frame_time = 0;
bool g_Host_Frame_Called = false;

void New_Host_Frame (float time)
{
	g_Host_Frame_Called = true;

	g_Host_Frame_time = time;
	g_Old_HostFrame(time);
}

void Hook_Host_Frame()
{
	if( !g_Old_HostFrame && 0 != HL_ADDR_GET(Host_Frame) )
	{
		g_phost_frametime = (double *)HL_ADDR_GET(host_frametime);
		g_Old_HostFrame = (Host_Frame_t) DetourApply((BYTE *)HL_ADDR_GET(Host_Frame), (BYTE *)New_Host_Frame, (int)HL_ADDR_GET(Host_Frame_DSZ));
	}
}
