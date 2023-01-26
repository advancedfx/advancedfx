#pragma once

#include "AfxConsole.h"

#include "CamPath.h"

class __declspec(novtable) IMirvCampath_Time abstract
{
public:
	virtual double GetTime() abstract = 0;
	virtual double GetCurTime() abstract = 0;
	virtual bool GetCurrentDemoTick(int& outTick) abstract = 0;
	virtual bool GetCurrentDemoTime(double& outDemoTime) abstract = 0;
	virtual bool GetDemoTickFromDemoTime(double curTime, double demoTime, int& outTick) abstract = 0;
	virtual bool GetDemoTimeFromClientTime(double curTime, double clientTime, double& outDemoTime) abstract = 0;
};

struct SMirvCameraValue
{
	double X;
	double Y;
	double Z;
	double Pitch;
	double Yaw;
	double Roll;
	double Fov;

	SMirvCameraValue(double x, double y, double z, double pitch, double yaw, double roll, double fov)
	{
		X = x;
		Y = y;
		Z = z;
		Pitch = pitch;
		Yaw = yaw;
		Roll = roll;
		Fov = fov;
	}
};

class __declspec(novtable) IMirvCampath_Camera abstract
{
public:
	virtual SMirvCameraValue GetCamera() abstract = 0;
};

class __declspec(novtable) IMirvCampath_Drawer abstract
{
public:
	virtual bool GetEnabled() abstract = 0;
	virtual void SetEnabled(bool value) abstract = 0;
	virtual bool GetDrawKeyframeAxis() abstract = 0;
	virtual void SetDrawKeyframeAxis(bool value) abstract = 0;
	virtual bool GetDrawKeyframeCam() abstract = 0;
	virtual void SetDrawKeyframeCam(bool value) abstract = 0;

	virtual float GetDrawKeyframeIndex() abstract = 0;
	virtual void SetDrawKeyframeIndex(float value) abstract = 0;
};

// <param name="drawer">Can be nullptr.</param>
void MirvCampath_ConCommand(advancedfx::ICommandArgs* args, advancedfx::Con_Printf_t conMessage, advancedfx::Con_Printf_t conWarning, CamPath* camPath, IMirvCampath_Time* mirvTime, IMirvCampath_Camera* mirvCamera, IMirvCampath_Drawer* mirvDrawer);

