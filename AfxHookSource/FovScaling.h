#pragma once

#include "WrpConsole.h"

enum FovScaling {
	FovScaling_Uninitalized,
	FovScaling_None,
	FovScaling_AlienSwarm
};

FovScaling GetFovScaling();

double Apply_FovScaling(double width, double height, double fov, FovScaling fovScaling);

double Apply_InverseFovScaling(double width, double height, double fov, FovScaling fovScaling);

double Auto_FovScaling(double width, double height, double fov);

double Auto_InverseFovScaling(double width, double height, double fov);

double AlienSwarm_FovScaling(double width, double height, double fov);

double AlienSwarm_InverseFovScaling(double width, double height, double fov);

void Console_MirvFovScaling(IWrpCommandArgs * args);
