#pragma once

#include "Globals.h"
#include <windows.h>
#include "../deps/release/Detours/src/detours.h"

void HookMirvCommands(HMODULE clientDll);

bool shouldGlowProjectile(const char* className, int team);

struct MirvGlow {
	std::map<uint64_t,bool> players;
	std::map<int32_t,bool> entities;

	std::map<std::string, std::map<std::string, bool>> nades {
		{ "smokes",     { {"ct", true}, {"t", true} } },
		{ "grenades",   { {"ct", true}, {"t", true} } },
		{ "flashbangs", { {"ct", true}, {"t", true} } },
		{ "molotovs",   { {"ct", true}, {"t", true} } },
		{ "decoys",     { {"ct", true}, {"t", true} } }
	};
};

extern MirvGlow g_MirvGlow;
