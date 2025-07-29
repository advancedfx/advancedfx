#pragma once

/*struct DemoPausedData {
	bool IsPaused = false;
	float FirstPausedCurtime = 0.0f;
	float FirstPausedInterpolationAmount = 0.0f;
};

extern DemoPausedData g_DemoPausedData;*/

class CMirvTime
{
public:
	float curtime_get(void);
	int framecount_get(void);
	float frametime_get(void);
	float absoluteframetime_get(void);
	float interval_per_tick_get(void);
	float interpolation_amount_get(void);

	bool GetCurrentDemoTick(int& outTick);
	bool GetCurrentDemoTime(double& outDemoTime);
};

extern CMirvTime g_MirvTime;
