#pragma once

extern bool g_csgo_S_StartSound_Debug;

bool csgo_S_StartSound_Install(void);

void csgo_S_StartSound_Block_Add(char const * szMask);
void csgo_S_StartSound_Block_Print(void);
void csgo_S_StartSound_Block_Remove(int index);
void csgo_S_StartSound_Block_Clear(void);
