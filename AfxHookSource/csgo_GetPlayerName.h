#pragma once

extern bool csgo_debug_GetPlayerName;

bool csgo_GetPlayerName_Install(void);

void csgo_GetPlayerName_Replace(char const * uidPlayer, char const * newName);
void csgo_GetPlayerName_Replace_List(void);
void csgo_GetPlayerName_Replace_Delete(char const * uidPlayer);
void csgo_GetPlayerName_Replace_Clear(void);
