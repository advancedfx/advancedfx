#include "Globals.h"

Cs2Gloabls_t g_pGlobals = nullptr;
SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient = nullptr;
SOURCESDK::CS2::IGameUIService * g_pGameUIService = nullptr;
DemoPausedData g_DemoPausedData; 

float curtime_get(void)
{
	if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedCurtime;
	}

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals + 13*4) : 0;
}

int framecount_get(void)
{
	return g_pGlobals ? *(int *)((unsigned char *)g_pGlobals + 1*4) : 0;
}

float frametime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +2*4) : 0;
}

float absoluteframetime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +3*4) : 0;
}

float interval_per_tick_get(void)
{
	return g_pGlobals ? 1.0f / *(int *)((unsigned char *)g_pGlobals +4*4) : 1.0f/64;
}

float interpolation_amount_get(void)
{
	if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedInterpolationAmount;
	}

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +15*4) : 0;
}