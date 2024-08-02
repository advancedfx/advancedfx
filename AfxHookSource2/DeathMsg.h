#pragma once

#include <windows.h>

void HookDeathMsg(HMODULE clientDll);
void HookPanorama(HMODULE panoramaDll);

struct currentGameCamera {
	double origin[3];
	double angles[3];
	float time;
};

extern currentGameCamera g_CurrentGameCamera;

namespace CS2 {
// https://github.com/a2x/cs2-dumper/blob/main/output/client.dll.hpp
	namespace C_CSGameRulesProxy {
		constexpr std::ptrdiff_t m_pGameRules = 0x548; // C_CSGameRules*
	}
	namespace C_CSGameRules {
		constexpr std::ptrdiff_t m_gamePhase = 0x80; // int32
		constexpr std::ptrdiff_t m_nOvertimePlaying = 0x8C; // int32
	}
	namespace C_BaseEntity {
		constexpr std::ptrdiff_t m_iTeamNum = 0x3C3; // uint8
	}
	namespace CBasePlayerController {
		constexpr std::ptrdiff_t m_iszPlayerName = 0x630; // char[128]
		constexpr std::ptrdiff_t m_steamID = 0x6B8; // uint64
	}
	namespace CCSPlayerController {
		constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x740; // CUtlString
	}
// https://github.com/danielkrupinski/Osiris
	namespace PanoramaUIPanel {
		constexpr std::ptrdiff_t getAttributeString = 0x8b8;
		constexpr std::ptrdiff_t setAttributeString = 0x8e8;
		constexpr std::ptrdiff_t children = 0x28;
		constexpr std::ptrdiff_t panelId = 0x10;
		constexpr std::ptrdiff_t panelFlags = 0x11c;
		constexpr std::ptrdiff_t k_EPanelFlag_HasOwnLayoutFile = 0x40;
	}

	namespace PanoramaUIEngine {
// panorama.dll
// void UndefinedFunction_180097150(undefined8 param_1,undefined4 param_2,undefined8 param_3) // offset is for this one
// {
//   FUN_180095df0(param_2,param_3); 
//	 this is function to make symbols, it has C:\\buildworker\\csgo_rel_win64\\build\\src\\public\\tier0\\utlsymboltable.h string
//   return;
// }
// vtable offset for UIEngine
		constexpr std::ptrdiff_t makeSymbol = 0x398;
	}
};