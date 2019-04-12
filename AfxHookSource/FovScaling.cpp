#include "stdafx.h"

#include "FovScaling.h"

#include "addresses.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"

#define _USE_MATH_DEFINES
#include <math.h>


enum FovScaling {
	FovScaling_Uninitalized,
	FovScaling_None,
	FovScaling_AlienSwarm
};

FovScaling GetDefaultFovScaling() {
	switch (g_SourceSdkVer)
	{
	case SourceSdkVer_CSGO:
	case SourceSdkVer_TF2:
	case SourceSdkVer_SWARM:
	case SourceSdkVer_L4D2:
		return FovScaling_AlienSwarm;
	default:
		return FovScaling_None;
	}
}

FovScaling g_FovScaling = FovScaling_Uninitalized;

FovScaling GetFovScaling() {

	if (FovScaling_Uninitalized == g_FovScaling)
		g_FovScaling = GetDefaultFovScaling();

	return g_FovScaling;
}

void SetFovScaling(FovScaling fovScaling) {
	g_FovScaling = fovScaling;
}

const char * FovScalingToCString(FovScaling fovScaling)
{

	switch (fovScaling)
	{
	case FovScaling_None:
		return "none";
	case FovScaling_AlienSwarm:
		return "alienSwarm";
	}

	return "[unknown]";
}

bool CStringToFovScaling(const char * value, FovScaling & fovScaling) {

	if (0 == _stricmp("none", value)) {
		fovScaling = FovScaling::FovScaling_None;
		return true;
	}
	else if (0 == _stricmp("alienSwarm", value)) {
		fovScaling = FovScaling::FovScaling_AlienSwarm;
		return true;
	}
	else if (0 == _stricmp("default", value)) {
		fovScaling = GetDefaultFovScaling();
		return true;
	}

	return false;
}


double Auto_FovScaling(double width, double height, double fov)
{
	switch (GetFovScaling())
	{
	case FovScaling_AlienSwarm:
		return AlienSwarm_FovScaling(width, height, fov);
	default:
		return fov;
	}
}


double Auto_InverseFovScaling(double width, double height, double fov)
{
	switch (GetFovScaling())
	{
	case FovScaling_AlienSwarm:
		return AlienSwarm_InverseFovScaling(width, height, fov);
	default:
		return fov;
	}
}


double AlienSwarm_FovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	double halfAngle = 0.5 * fov * (2.0 * M_PI / 360.0);
	double t = ratio * tan(halfAngle);
	return 2.0 * atan(t) / (2.0 * M_PI / 360.0);
}

double AlienSwarm_InverseFovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	double t = tan(0.5 * fov * (2.0 * M_PI / 360.0));
	double halfAngle = atan(t / ratio);
	return 2.0 * halfAngle / (2.0 * M_PI / 360.0);
}

void Console_MirvFovScaling(IWrpCommandArgs * args)
{
	int argC = args->ArgC();

	const char * arg0 = args->ArgV(0);

	if (2 <= argC) {

		const char * arg1 = args->ArgV(1);

		FovScaling fovScaling;

		if (CStringToFovScaling(arg1, fovScaling)) {
			SetFovScaling(fovScaling);
		}
		else Tier0_Warning("Error: \"%s\" is not a valid fovScaling.\n", arg1);

		return;
	}

	Tier0_Msg(
		"%s default|none|alienSwarm - Set default fov scaling.\n"
		"Current: %s\n"
		"Default: %s\n"
		, arg0
		, FovScalingToCString(GetFovScaling())
		, FovScalingToCString(GetDefaultFovScaling())
	);
	return;
}

