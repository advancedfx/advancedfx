#pragma once

#include "Globals.h"

#include "../shared/MirvInput.h"

#define _USE_MATH_DEFINES
#include <math.h>

class MirvInputEx : private IMirvInputDependencies
{
public:
	MirvInputEx();

	~MirvInputEx();

	MirvInput * m_MirvInput;

	double LastCameraOrigin[3];
	double LastCameraAngles[3];
	double LastCameraFov;

	double GameCameraOrigin[3];
	double GameCameraAngles[3];
	double GameCameraFov;

	double LastFrameTime;

	int LastWidth;
	int LastHeight;

private:
	virtual bool GetSuspendMirvInput() override;

	virtual void GetLastCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override;

	virtual void GetGameCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) override;

	virtual double GetInverseScaledFov(double fov) override;

private:
	double ScaleFovInverse(double width, double height, double fov);
};

extern MirvInputEx g_MirvInputEx;