#include "MirvTime.h"
#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"

typedef void * Cs2Gloabls_t;
extern Cs2Gloabls_t g_pGlobals;

extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;

CMirvTime g_MirvTime;
//DemoPausedData g_DemoPausedData;

float CMirvTime::curtime_get(void)
{
	/*if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedCurtime;
	}*/

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals + 12*4) : 0;
}

int CMirvTime::framecount_get(void)
{
	return g_pGlobals ? *(int *)((unsigned char *)g_pGlobals + 1*4) : 0;
}

float CMirvTime::frametime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +2*4) : 0;
}

float CMirvTime::absoluteframetime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +3*4) : 0;
}

float CMirvTime::interval_per_tick_get(void)
{
	const int default_value = 64;
//	if(nullptr == g_pGlobals) return default_value;
//	int value = *(int *)((unsigned char *)g_pGlobals +4*4);
//	if(value <= 1) value = default_value; // In menu it's 1.
//	return 1.0f / value;
	return 1.0f / default_value;
}

float CMirvTime::interpolation_amount_get(void)
{
	/*if(g_DemoPausedData.IsPaused) {
		return g_DemoPausedData.FirstPausedInterpolationAmount;
	}*/

	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +14) : 0;
}

bool CMirvTime::GetCurrentDemoTick(int& outTick) {
	if(g_pEngineToClient) {
		if(auto pDemoFile = g_pEngineToClient->GetDemoFile()) {
			outTick = pDemoFile->GetDemoTick();
			return true;
		}
	}
	return false;
}

bool CMirvTime::GetCurrentDemoTime(double& outDemoTime) {
	int tick;
	if(GetCurrentDemoTick(tick)) {
		outDemoTime = (tick + interpolation_amount_get()) * (double)interval_per_tick_get();
		return true;
	}

	return false;
}
