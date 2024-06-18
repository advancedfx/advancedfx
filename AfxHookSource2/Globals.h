#pragma once

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameuiservice.h"

struct DemoPausedData {
	bool IsPaused = false;
	float FirstPausedCurtime = 0.0f;
	float FirstPausedInterpolationAmount = 0.0f;
};

typedef void * Cs2Gloabls_t;
extern Cs2Gloabls_t g_pGlobals;
extern SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient;
extern SOURCESDK::CS2::IGameUIService * g_pGameUIService;
extern DemoPausedData g_DemoPausedData; 

float curtime_get(void);
int framecount_get(void);
float frametime_get(void);
float absoluteframetime_get(void);
float interval_per_tick_get(void);
float interpolation_amount_get(void);