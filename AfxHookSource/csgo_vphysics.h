#pragma once

bool Hook_csgo_vphsyics_frametime_lowerlimit(void);

void csgo_vphysics_SetMaxFps(double value);
double csgo_vphysics_GetMaxFps(void);