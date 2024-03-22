#include "stdafx.h"

#include "FovScaling.h"

#define _USE_MATH_DEFINES
#include <math.h>

FovScaling g_FovScaling = FovScaling_Default;

FovScaling GetFovScaling() {

	if (FovScaling_Default == g_FovScaling)
		return GetDefaultFovScaling(); // we need to eval this, since it can change dynamically

	return g_FovScaling;
}

void SetFovScaling(FovScaling fovScaling) {
	g_FovScaling = fovScaling;
}

const char * FovScalingToCString(FovScaling fovScaling)
{

	switch (fovScaling)
	{
	case FovScaling_Default:
		return "default";
	case FovScaling_None:
		return "none";
	case FovScaling_AlienSwarm:
		return "alienSwarm";
	case FovScaling_Sdk2013Restricted:
		return "sdk2013Restricted";
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
	else if (0 == _stricmp("sdk2013Restricted", value)) {
		fovScaling = FovScaling::FovScaling_Sdk2013Restricted;
		return true;
	}
	else if (0 == _stricmp("default", value)) {
		fovScaling = FovScaling::FovScaling_Default;
		return true;
	}

	return false;
}

double Apply_FovScaling(double width, double height, double fov, FovScaling fovScaling)
{
	switch (fovScaling)
	{
	case FovScaling_AlienSwarm:
		return AlienSwarm_FovScaling(width, height, fov);
	case FovScaling_Sdk2013Restricted:
		return Sdk2013Restricted_FovScaling(width, height, fov);
	default:
		return fov;
	}
}

double Apply_InverseFovScaling(double width, double height, double fov, FovScaling fovScaling)
{
	switch (fovScaling)
	{
	case FovScaling_AlienSwarm:
		return AlienSwarm_InverseFovScaling(width, height, fov);
	case FovScaling_Sdk2013Restricted:
		return Sdk2013Restricted_InverseFovScaling(width, height, fov);
	default:
		return fov;
	}
}

double Auto_FovScaling(double width, double height, double fov)
{
	return Apply_FovScaling(width, height, fov, GetFovScaling());
}

double Auto_InverseFovScaling(double width, double height, double fov)
{
	return Apply_InverseFovScaling(width, height, fov, GetFovScaling());
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

double Sdk2013Restricted_FovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	const double ratioMin = 1.85 / (4.0 / 3.0);
	if(ratio > ratioMin) ratio = ratioMin;
	double halfAngle = 0.5 * fov * (2.0 * M_PI / 360.0);
	double t = ratio * tan(halfAngle);
	return 2.0 * atan(t) / (2.0 * M_PI / 360.0);
}

double Sdk2013Restricted_InverseFovScaling(double width, double height, double fov)
{
	if (!height) return fov;

	double engineAspectRatio = width / height;
	double defaultAscpectRatio = 4.0 / 3.0;
	double ratio = engineAspectRatio / defaultAscpectRatio;
	const double ratioMin = 1.85 / (4.0 / 3.0);
	if(ratio > ratioMin) ratio = ratioMin;
	double t = tan(0.5 * fov * (2.0 * M_PI / 360.0));
	double halfAngle = atan(t / ratio);
	return 2.0 * halfAngle / (2.0 * M_PI / 360.0);
}

void Console_MirvFovScaling(advancedfx::ICommandArgs * args)
{
	int argC = args->ArgC();

	const char * arg0 = args->ArgV(0);

	if (2 <= argC) {

		const char * arg1 = args->ArgV(1);

		FovScaling fovScaling;

		if (CStringToFovScaling(arg1, fovScaling)) {
			SetFovScaling(fovScaling);
		}
		else advancedfx::Warning("Error: \"%s\" is not a valid fovScaling.\n", arg1);

		return;
	}

	advancedfx::Message(
		"%s default|none|alienSwarm|sdk2013Restricted - Set default fov scaling.\n"
		"Current: %s\n"
		"Default: %s\n"
		, arg0
		, FovScalingToCString(g_FovScaling)
		, FovScalingToCString(GetDefaultFovScaling())
	);
	return;
}
