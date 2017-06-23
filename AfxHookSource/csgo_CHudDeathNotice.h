#pragma once

#include "WrpConsole.h"

extern int csgo_CHudDeathNotice_HighLightId;
extern bool csgo_CHudDeathNotice_HighLightAssists;
extern int csgo_debug_CHudDeathNotice_FireGameEvent;

extern float csgo_CHudDeathNotice_nScrollInTime;
extern float csgo_CHudDeathNotice_nFadeOutTime;
extern float csgo_CHudDeathNotice_nNoticeLifeTime;
extern float csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod;

bool csgo_CHudDeathNotice_Install(void);

/// <param name="modTime">Negative value means to block, otherwise means time modifier override.</param>
void csgo_CHudDeathNotice_Block(char const * uidAttacker, char const * uidVictim, char const * uidAssister, float modTime);
void csgo_CHudDeathNotice_Block_List(void);
void csgo_CHudDeathNotice_Block_Clear(void);

void Console_csgo_CHudDeathNotice_Fake(char const * htmlString, bool bIsVictim, bool bIsKiller);
