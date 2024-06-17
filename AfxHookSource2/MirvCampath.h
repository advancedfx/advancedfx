#pragma once

#include "Globals.h"
#include "CampathDrawer.h"
#include "MirvInputEx.h"

#include "../shared/MirvCampath.h"
#include "../shared/MirvCamIO.h"

class CMirvCampath_Time : public IMirvCampath_Time
{
public:
	virtual double GetTime();
	virtual double GetCurTime();
	virtual bool GetCurrentDemoTick(int& outTick);
	virtual bool GetCurrentDemoTime(double& outDemoTime);
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick);
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime);
    virtual bool GetDemoTickFromClientTime(double curTime, double targetTime, int& outTick);
};

class CMirvCampath_Camera : public IMirvCampath_Camera
{
public:
	virtual SMirvCameraValue GetCamera();
};

class CMirvCampath_Drawer : public IMirvCampath_Drawer
{
public:
	virtual bool GetEnabled();
	virtual void SetEnabled(bool value);
	virtual bool GetDrawKeyframeAxis();
	virtual void SetDrawKeyframeAxis(bool value);
	virtual bool GetDrawKeyframeCam();
	virtual void SetDrawKeyframeCam(bool value);
	virtual float GetDrawKeyframeIndex();
	virtual void SetDrawKeyframeIndex(float value);
};

extern CamPath g_CamPath;
extern CMirvCampath_Time g_MirvCampath_Time;
extern CMirvCampath_Camera g_MirvCampath_Camera;
extern CMirvCampath_Drawer g_MirvCampath_Drawer;