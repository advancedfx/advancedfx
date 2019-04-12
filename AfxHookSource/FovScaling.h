#pragma once

#include "WrpConsole.h"

double Auto_FovScaling(double width, double height, double fov);

double Auto_InverseFovScaling(double width, double height, double fov);

double AlienSwarm_FovScaling(double width, double height, double fov);

double AlienSwarm_InverseFovScaling(double width, double height, double fov);

void Console_MirvFovScaling(IWrpCommandArgs * args);
