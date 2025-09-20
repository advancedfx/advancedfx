#include "MirvCommands.h"
#include "ClientEntitySystem.h"

#include "../shared/StringTools.h"

#include "SceneSystem.h"
#include "SchemaSystem.h"

bool g_bHookedMirvCommands = false;

bool g_bNoFlashEnabled = false;

bool g_bEndOfMatchEnabled = true;

MirvGlow g_MirvGlow;
extern void deathMsgPlayers_PrintHelp_Console();

typedef void (__fastcall *g_Original_EOM_t)(u_char* param_1, uint64_t param_2);
g_Original_EOM_t g_Original_EOM = nullptr;

void __fastcall new_EOM(u_char* param_1, uint64_t param_2) {
	if (g_bEndOfMatchEnabled) return g_Original_EOM(param_1, param_2);
}

void mirvEndOfMatch_Console(advancedfx::ICommandArgs* args) {
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 == argc)
	{
		g_bEndOfMatchEnabled = 0 != atoi(args->ArgV(1));
		return;
	}

	advancedfx::Message(
		"%s <0|1> - Enable (1) / disable (0) end of match scene.\n"
		"Current value: %d\n"
		, arg0, g_bEndOfMatchEnabled
	);
}

CON_COMMAND(mirv_endofmatch, "Disables end of match scene.")
{
	mirvEndOfMatch_Console(args);
}

typedef void (__fastcall *g_Original_flashFunc_t)(u_char* param_1, u_char* param_2, float* param_3);
g_Original_flashFunc_t g_Original_flashFunc = nullptr;

void __fastcall new_flashFunc(u_char* param_1, u_char* param_2, float* param_3) {	
	if (g_bNoFlashEnabled) return;
	else return g_Original_flashFunc(param_1, param_2, param_3);
}

void mirvNoFlash_Console(advancedfx::ICommandArgs* args) {
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 == argc)
	{
		g_bNoFlashEnabled = 0 != atoi(args->ArgV(1));
		return;
	}

	advancedfx::Message(
		"%s <0|1> - Enable (1) / disable (0) no flash.\n"
		"Current value: %d\n"
		, arg0, g_bNoFlashEnabled 
	);

}

CON_COMMAND(mirv_noflash, "Disables flash overlay.")
{
	mirvNoFlash_Console(args);
}

///////////////////////////////////////////////////////////////////////////////

bool g_bMirvFovEnabled = false;
float g_fMirvFovValue = 90.0;
float g_bMirvFovHandleZoom = true;
float g_fMirvFovMinUnzoomedFov = 90.0;

extern float GetLastCameraFov();

bool MirvFovOverride(float &fov) {
	if(g_bMirvFovEnabled) {
		if(!g_bMirvFovHandleZoom || g_fMirvFovMinUnzoomedFov <= fov) {
			fov = g_fMirvFovValue;
			return true;
		}
	}
	return false;
}

CON_COMMAND(mirv_fov,"allows overriding FOV (Field Of View) of the camera")
{
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(!_stricmp(arg1,"default"))
		{
			g_bMirvFovEnabled = false;
			return;
		}
		else
		if(!_stricmp(arg1,"handleZoom"))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if(!_stricmp(arg2, "enabled"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						g_bMirvFovHandleZoom = 0 != atoi(arg3);
						return;
					}

					advancedfx::Message(
						"Usage:\n"
						"mirv_fov handleZoom enabled 0|1 - Enable (1), disable (0).\n"
						"Current value: %s\n",
						g_bMirvFovHandleZoom ? "1" : "0"
					);
					return;
				}
				else
				if(!_stricmp(arg2, "minUnzoomedFov"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						if(!_stricmp(arg3, "current"))
						{
							g_fMirvFovMinUnzoomedFov = GetLastCameraFov();
							return;
						}
						else if(StringIsEmpty(arg3) || !StringIsAlphas(arg3))
						{
							g_fMirvFovMinUnzoomedFov = atof(arg3);
							return;
						}
					}

					advancedfx::Message(
						"Usage:\n"
						"mirv_fov handleZoom minUnzoomedFov current - Set current fov as threshold.\n"
						"mirv_fov handleZoom minUnzoomedFov <f> - Set floating point value <f> as threshold.\n"
						"Current value: %f\n",
						g_fMirvFovMinUnzoomedFov
					);
					return;
				}
			}

			advancedfx::Message(
				"Usage:\n"
				"mirv_fov handleZoom enabled [...] - Whether to enable zoom handling (if enabled mirv_fov is only active if it's not below minUnzoomedFov (not zoomed)).\n"
				"mirv_fov handleZoom minUnzoomedFov [...] - Zoom detection threshold.\n"
			);
			return;
		}
		if(StringIsEmpty(arg1) || !StringIsAlphas(arg1))
		{
			g_bMirvFovEnabled = true;
			g_fMirvFovValue = atof(arg1);
			return;
		}
	}

	advancedfx::Message(
		"Usage:\n"
		"mirv_fov <f> - Override fov with given floating point value <f>.\n"
		"mirv_fov default - Revert to the game's default behaviour.\n"
		"mirv_fov handleZoom [...] - Handle zooming (e.g. AWP in CS:GO).\n"
	);
	{
		advancedfx::Message("Current value: ");

		if(!g_bMirvFovEnabled)
			advancedfx::Message("default (currently: %f)\n", GetLastCameraFov());
		else
			advancedfx::Message("%f\n", g_fMirvFovValue);
	}
}

bool shouldGlowProjectile(const char* className, int team) {
	if (team != 2 && team != 3) return true;

	bool shouldGlow = true;
	std::string teamStr = team == 2 ? "t" : "ct";

	if (!_stricmp(className, "smokegrenade_projectile")) {
		shouldGlow = g_MirvGlow.nades["smokes"][teamStr];

	} else if (!_stricmp(className, "hegrenade_projectile")) {
		shouldGlow = g_MirvGlow.nades["grenades"][teamStr];

	} else if (!_stricmp(className, "flashbang_projectile")) {
		shouldGlow = g_MirvGlow.nades["flashbangs"][teamStr];

	} else if (!_stricmp(className, "molotov_projectile")) {
		shouldGlow = g_MirvGlow.nades["molotovs"][teamStr];

	} else if (!_stricmp(className, "decoy_projectile")) {
		shouldGlow = g_MirvGlow.nades["decoys"][teamStr];
	}

	return shouldGlow;
}

typedef void (__fastcall *g_Original_setGlowProps_t)(u_char* glowProperty, int param_2, float param_3);
g_Original_setGlowProps_t g_Original_setGlowProps = nullptr;

void new_setGlowProps (u_char* glowProperty, int param_2, float param_3) {
	SOURCESDK::CS2::Cvar_s * handle_show_xray = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("spec_show_xray", false).Get());

	if (handle_show_xray != nullptr && handle_show_xray->m_Value.m_bValue == true) {
		auto shouldGlow = true;
		CEntityInstance* resolvedPlayerPawn = 0;

		auto ent = (CEntityInstance*)(glowProperty - g_clientDllOffsets.C_BaseModelEntity.m_Glow);
		auto name = ent->GetClassName();
		auto team = ent->GetTeam();
		auto handle = ent->GetHandle();

		if (handle.IsValid() && g_MirvGlow.entities.find(handle.ToInt()) != g_MirvGlow.entities.end()) {
			shouldGlow = g_MirvGlow.entities[handle.ToInt()];
		} else if (StringEndsWith(name, "_projectile") && (team == 2 || team == 3)) {
			shouldGlow = shouldGlowProjectile(name, team);
		} else if (ent->IsPlayerPawn()) {
			resolvedPlayerPawn = ent;
		} else {
			auto ownerHandle = SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(uint32_t*)((u_char*)ent + g_clientDllOffsets.C_BaseEntity.m_hOwnerEntity));
			if (ownerHandle.IsValid()) {
				auto ownerEnt = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,ownerHandle.GetEntryIndex());
				if (0 != ownerEnt && ownerEnt->IsPlayerPawn()) {
					resolvedPlayerPawn = ownerEnt;
				}
			} else {
				auto sceneNode = *(u_char**)(ent + g_clientDllOffsets.C_BaseEntity.m_pGameSceneNode);
				auto parentNode = *(u_char**)(sceneNode + g_clientDllOffsets.CGameSceneNode.m_pParent);
				if (0 != parentNode) {
					auto parentEnt = *(CEntityInstance**)(parentNode + g_clientDllOffsets.CGameSceneNode.m_pOwner);
					if (0 != parentEnt && parentEnt->IsPlayerPawn()) {
						resolvedPlayerPawn = parentEnt;
					}
				};
			}
		}

		if (0 != resolvedPlayerPawn) {
			auto controllerHandle = resolvedPlayerPawn->GetPlayerControllerHandle();
			if (controllerHandle.IsValid()) {
				auto controller = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,controllerHandle.GetEntryIndex());
				auto steamId = controller->GetSteamId();

				if (g_MirvGlow.players.find(steamId) != g_MirvGlow.players.end()) shouldGlow = g_MirvGlow.players[steamId];
			}
		}

		if (!shouldGlow) return g_Original_setGlowProps(glowProperty, 0, param_3);
	}

	return g_Original_setGlowProps(glowProperty, param_2, param_3);
}

typedef bool (__fastcall * org_shouldGlow_t)(u_char* glowProperty);
org_shouldGlow_t org_shouldGlow = nullptr;

bool new_shouldGlow(u_char* glowProperty) {
	auto result = org_shouldGlow(glowProperty);
	// see where this function is called, in the block where result of this fn is true
	// if (*(longlong **)(param_1 + 0x18) != (longlong *)0x0) {
    //   lVar7 = (**(code **)(**(longlong **)(param_1 + 0x18) + 0x1a8))();
    // }
	if (0 == *(u_char**)(glowProperty + 0x18)) return result;

	auto ent = (CEntityInstance*)(glowProperty - g_clientDllOffsets.C_BaseModelEntity.m_Glow);
	auto handle = ent->GetHandle();

	if (handle.IsValid() && g_MirvGlow.entities.find(handle.ToInt()) != g_MirvGlow.entities.end()) {
		result = g_MirvGlow.entities[handle.ToInt()];
	}

	return result;
}

CON_COMMAND(mirv_glow, "Manage glow drawing.")
{
    int argC = args->ArgC();
    const char * arg0 = args->ArgV(0);

    if (2 <= argC) {
        const char * arg1 = args->ArgV(1);
        if (!_stricmp("projectiles", arg1)) {
            if (3 <= argC) {
                auto arg2 = afxUtils::stringToLowerCase(args->ArgV(2));

				if (g_MirvGlow.nades.find(arg2) != g_MirvGlow.nades.end()) {
					if (4 <= argC) {
						auto arg3 = afxUtils::stringToLowerCase(args->ArgV(3));
						auto nade = g_MirvGlow.nades[arg2];
						if (nade.find(arg3) != nade.end()) {
							if (5 <= argC) {
								g_MirvGlow.nades[arg2][arg3] = 0 != atoi(args->ArgV(4));
								return;
							}
							advancedfx::Message(
								"%s %s %s %s 0|1\n"
								"Current Value: %s\n"
								, arg0, arg1, arg2.c_str(), arg3.c_str(), nade[arg3] ? "1" : "0"
							);
							return;
						}
					}

					advancedfx::Message(
						"%s %s %s T  0|1 - Enable (1) / disable (0) glow for T %s.\n"
						"%s %s %s CT 0|1 - Enable (1) / disable (0) glow for CT %s.\n"
						, arg0, arg1, arg2.c_str(), arg1
						, arg0, arg1, arg2.c_str(), arg1
					);
					return;
				}
            }

			advancedfx::Message(
				"%s %s smokes     T  0|1 - Enable (1) / disable (0) glow for T smokes.\n"
				"%s %s smokes     CT 0|1 - Enable (1) / disable (0) glow for CT smokes.\n"
				"%s %s grenades   T  0|1 - Enable (1) / disable (0) glow for T grenades.\n"
				"%s %s grenades   CT 0|1 - Enable (1) / disable (0) glow for CT grenades.\n"
				"%s %s flashbangs T  0|1 - Enable (1) / disable (0) glow for T flashbangs.\n"
				"%s %s flashbangs CT 0|1 - Enable (1) / disable (0) glow for CT flashbangs.\n"
				"%s %s molotovs   T  0|1 - Enable (1) / disable (0) glow for T molotovs.\n"
				"%s %s molotovs   CT 0|1 - Enable (1) / disable (0) glow for CT molotovs.\n"
				"%s %s decoys     T  0|1 - Enable (1) / disable (0) glow for T decoys.\n"
				"%s %s decoys     CT 0|1 - Enable (1) / disable (0) glow for CT decoys.\n"
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
			);
			return;
        } else if (!_stricmp("players", arg1)) {
            if (3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if (!_stricmp("set", arg2) && 5 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;
                    g_MirvGlow.players[strtoull(arg3,nullptr,10)] = 0 != atoi(args->ArgV(4));
                    return;
                }
                else if (!_stricmp("remove", arg2) && 4 <= argC) {
                    const char * arg3 = args->ArgV(3);
                    if(StringIBeginsWith(arg3,"x")) arg3++;                    
                    g_MirvGlow.players.erase(strtoull(arg3,nullptr,10));
                    return;
                }
                else if (!_stricmp("print", arg2)) {
                    for(auto it = g_MirvGlow.players.begin(); it != g_MirvGlow.players.end(); it++) {
                        advancedfx::Message("x%llu: %i\n",it->first,it->second ? 1 : 0);
                    }
                    return;
                }                
                else if (!_stricmp("help", arg2)) {
                    deathMsgPlayers_PrintHelp_Console();
					return;
				}
            }
			advancedfx::Message(
				"%s %s set x<ullXuid> 0|1 - Enable (1) / disable (0) glow for given player.\n"
				"%s %s remove x<ullXuid> - Remove entry from list.\n"
				"%s %s print - Print entries.\n"
				"%s %s help - Print players info.\n"
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
			);
            return;
        } else if (!_stricmp("entities", arg1)) {
            if (3 <= argC) {
                const char * arg2 = args->ArgV(2);
                if (!_stricmp("set", arg2) && 5 <= argC) {
                    auto arg3 = atoi(args->ArgV(3));
                    g_MirvGlow.entities[arg3] = 0 != atoi(args->ArgV(4));
                    return;
                }
                else if (!_stricmp("remove", arg2) && 4 <= argC) {
                    auto arg3 = atoi(args->ArgV(3));
                    g_MirvGlow.entities.erase(arg3);
                    return;
                }
                else if (!_stricmp("print", arg2)) {
                    for(auto it = g_MirvGlow.entities.begin(); it != g_MirvGlow.entities.end(); it++) {
                        advancedfx::Message("%i: %i\n",it->first,it->second ? 1 : 0);
                    }
                    return;
                }                
            }
			advancedfx::Message(
				"%s %s set <iHandle> 0|1 - Enable (1) / disable (0) glow for given entity.\n"
				"%s %s remove <iHandle> - Remove entry from list.\n"
				"%s %s print - Print entries.\n"
				"Tip: use mirv_listentities to get handles.\n"
				, arg0, arg1
				, arg0, arg1
				, arg0, arg1
			);
            return;
        }
    }

    advancedfx::Message(
        "%s projectiles [...] - Control glow per projectiles.\n"
		"%s players [...] - Control glow per player.\n"
		"%s entities [...] - Control glow per entity.\n"
        , arg0
        , arg0
        , arg0
    );
}

// we probably need addresses.cpp at this point
typedef void* (__fastcall * ForceUpdateSkybox_t)(void* This);
extern ForceUpdateSkybox_t org_ForceUpdateSkybox;

bool getAddressesFromClient(HMODULE clientDll) {
	// can be found with offsets to m_flFlashScreenshotAlpha, m_flFlashDuration, m_flFlashMaxAlpha, etc. 
	// In this function values being assigned to all these offsets at once
	size_t g_Original_flashFunc_addr = getAddress(clientDll, "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 0F 29 74 24 ?? 33 C9");
	if(g_Original_flashFunc_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	// called in func with 'cs_win_panel_match' in the end in if/else statement
	// in func itself it starts with 'if (*(char *)(param_1 + 8) == '\0')'
	size_t g_Original_EOM_addr = getAddress(clientDll, "40 56 41 55 48 83 EC ?? 80 79");
	if(g_Original_EOM_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	// See where spec_show_xray is checked, has offsets to glowProperty
	// Also called first in 234th vtable function for C_LightEntity and other 100+ entities
	size_t g_Original_setGlowProps_addr = getAddress(clientDll, "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B 05 ?? ?? ?? ?? 48 8B D9 F3 0F 10 41");
	if (g_Original_setGlowProps_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	g_Original_flashFunc = (g_Original_flashFunc_t)(g_Original_flashFunc_addr);
	g_Original_EOM = (g_Original_EOM_t)(g_Original_EOM_addr);
	g_Original_setGlowProps = (g_Original_setGlowProps_t)(g_Original_setGlowProps_addr);

	// C_BaseModelEntity vtable 234th, then go to second function call, there go to first function call
	// this function should return m_bGlowing of CGlowProperty

	if (auto addr = getAddress(clientDll, "E8 ?? ?? ?? ?? 33 DB 84 C0 0F 84 ?? ?? ?? ?? 48")) {
		org_shouldGlow = (org_shouldGlow_t)(addr + 5 + *(int32_t*)(addr + 1));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__)); 

   // Has offset to material of skybox (other members too), pCSceneSystem and it's function to update skybox.
   //
   // Could be found if you see callstack for function of vtable for CSceneSystem (currently 39th)
   // To trigger update just skip demo backwards or load level.
   //
   // Also it's referenced in 10th vtable function of C_EnvSky (we are getting it from there)
   //
   // 1801c02cb 48  8b  0d       MOV        RCX, qword ptr [DAT_182025518] // CSceneSystem
   //           46  52  e6  01
   // 1801c02d2 48  8d  55  30   LEA        RDX => local_res8, [RBP + 0x30]
   // 1801c02d6 48  8b  01       MOV        RAX, qword ptr [RCX]
   // 1801c02d9 4c  8b  88       MOV        R9, qword ptr [RAX + 0x138] // 39th function of CSceneSystem
   //           38  01  00  00
   // 1801c02e0 48  8b  87       MOV        RAX, qword ptr [RDI + 0xec0] // offset to m_hSkyMaterial of C_EnvSky
   //           c0  0e  00  00
   // 1801c02e7 48  89  45  30   MOV        qword ptr [RBP + local_res8], RAX
   // 1801c02eb 41  ff  d1       CALL       R9

	if (auto addr = getAddress(clientDll, "33 DB 48 8D 05 ?? ?? ?? ?? 48 8B CF 48 89 44 24 ??")) {
		auto offset = *(int32_t*)(addr + 5);
		org_ForceUpdateSkybox = (ForceUpdateSkybox_t)(addr + 2 + 7 + offset);
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));

	return true;
}

void HookMirvCommands(HMODULE clientDll) {
	if (g_bHookedMirvCommands) return;

	if (!getAddressesFromClient(clientDll)) return;

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Original_flashFunc, new_flashFunc);
	DetourAttach(&(PVOID&)g_Original_EOM, new_EOM);
	DetourAttach(&(PVOID&)g_Original_setGlowProps, new_setGlowProps);
	DetourAttach(&(PVOID&)org_shouldGlow, new_shouldGlow);
	DetourAttach(&(PVOID&)org_ForceUpdateSkybox, new_ForceUpdateSkybox);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour MirvCommands functions.");
		return;
	}

	g_bHookedMirvCommands = true;
};

