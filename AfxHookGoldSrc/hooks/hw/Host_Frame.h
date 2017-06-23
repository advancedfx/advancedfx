#pragma once

// pointer on double host_frametime:
// this is the real frame time and can be much lower than g_Host_Frame_time
extern double * g_phost_frametime;

// this is the time since the last function call
// and can be much greater than *g_phost_frametime:
// extern float g_Host_Frame_time;

extern bool g_Host_Frame_Called;

void Hook_Host_Frame();
