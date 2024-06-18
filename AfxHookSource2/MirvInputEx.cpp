#include "Globals.h"
#include "MirvInputEx.h"

#include "../shared/MirvInput.h"

#define _USE_MATH_DEFINES
#include <math.h>

MirvInputEx g_MirvInputEx;

MirvInputEx::MirvInputEx() {
	LastWidth = 1920;
	LastHeight = 1080;

	LastCameraOrigin[0] = 0.0;
	LastCameraOrigin[1] = 0.0;
	LastCameraOrigin[2] = 0.0;
	LastCameraAngles[0] = 0.0;
	LastCameraAngles[1] = 0.0;
	LastCameraAngles[2] = 0.0;
	LastCameraFov = 90.0;

	GameCameraOrigin[0] = 0.0;
	GameCameraOrigin[1] = 0.0;
	GameCameraOrigin[2] = 0.0;
	GameCameraAngles[0] = 0.0;
	GameCameraAngles[1] = 0.0;
	GameCameraAngles[2] = 0.0;
	GameCameraFov = 90.0;

	LastFrameTime = 0;

	m_MirvInput = new MirvInput(this);
}

MirvInputEx::~MirvInputEx() {
	delete m_MirvInput;
}


bool MirvInputEx::GetSuspendMirvInput() {
	return g_pGameUIService && g_pGameUIService->Con_IsVisible();
}

void MirvInputEx::GetLastCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) {
	x = LastCameraOrigin[0];
	y = LastCameraOrigin[1];
	z = LastCameraOrigin[2];
	rX = LastCameraAngles[0];
	rY = LastCameraAngles[1];
	rZ = LastCameraAngles[2];
	fov = LastCameraFov;
}

void MirvInputEx::GetGameCameraData(double & x, double & y, double & z, double & rX, double & rY, double & rZ, double & fov) {
	x = GameCameraOrigin[0];
	y = GameCameraOrigin[1];
	z = GameCameraOrigin[2];
	rX = GameCameraAngles[0];
	rY = GameCameraAngles[1];
	rZ = GameCameraAngles[2];
	fov = GameCameraFov;
}

double MirvInputEx::GetInverseScaledFov(double fov) {
	return ScaleFovInverse(LastWidth, LastHeight, fov);
}

double MirvInputEx::ScaleFovInverse(double width, double height, double fov) {
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	double t = tan(0.5 * fov * (2.0 * M_PI / 360.0));
	double halfAngle = atan(t / ratio);
	return 2.0 * halfAngle / (2.0 * M_PI / 360.0);
}

