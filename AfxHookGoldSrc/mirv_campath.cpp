#include "stdafx.h"

#include "cmdregister.h"
#include "MirvCommandArgs.h"
#include "filming.h"
#include "CampathDrawer.h"
#include "mirv_time.h"
#include "hooks/HookHw.h"
#include <shared/StringTools.h>
#include <shared/MirvCampath.h>

#include <stdio.h>
#include <stdarg.h>

class CMirvCampath_Time : public IMirvCampath_Time
{
public:
	virtual double GetTime() {
		return Mirv_GetDemoTimeOrClientTime();
	}
	virtual double GetCurTime() {
		return Mirv_GetDemoTimeOrClientTime();
	}
	virtual bool GetCurrentDemoTick(int& outTick) {
		return false;
	}
	virtual bool GetCurrentDemoTime(double& outDemoTime) {
		outDemoTime = Mirv_GetDemoTimeOrClientTime();
		return outDemoTime;
	}
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
		return false;
	}
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
		return false;
	}
    virtual bool GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick) {
        return false;
    }
} g_MirvCampath_Time;

class CMirvCampath_Camera : public IMirvCampath_Camera
{
public:
	virtual SMirvCameraValue GetCamera() {
		return SMirvCameraValue(
			g_Filming.LastCameraOrigin[0],
			g_Filming.LastCameraOrigin[1],
			g_Filming.LastCameraOrigin[2],
			g_Filming.LastCameraAngles[PITCH],
			g_Filming.LastCameraAngles[YAW],
			g_Filming.LastCameraAngles[ROLL],
			g_Filming.LastCameraFov
		);
	}
} g_MirvCampath_Camera;


void MirvCampath_Msg(const char* format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	pEngfuncs->Con_Printf("%s", buffer);
	va_end(args);
}

REGISTER_CMD_FUNC(campath)
{
	CMirvCommandArgs args(pEngfuncs);

	MirvCampath_ConCommand(&args, MirvCampath_Msg, MirvCampath_Msg, g_Filming.GetCamPath(), &g_MirvCampath_Time, &g_MirvCampath_Camera, &g_CampathDrawer);
}
