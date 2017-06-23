#pragma once

extern float g_csgo_mystique_annimation_factor;

void Enable_csgo_PlayerAnimStateFix_set(int value);
int Enable_csgo_PlayerAnimStateFix_get(void);

bool Hook_csgo_PlayerAnimStateFix(void);
