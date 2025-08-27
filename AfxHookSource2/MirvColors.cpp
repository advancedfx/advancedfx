#include "MirvColors.h"
#include "ClientEntitySystem.h"
#include "SchemaSystem.h"
#include "MirvTime.h"

bool g_bHookedMirvColors = false;
extern u_char* g_pHudReticle_WashColor_T;
extern u_char* g_pHudReticle_WashColor_CT;

std::vector<AfxBasicColor> afxBasicColors = {
	{ "red",		{ 255, 0, 0, 255 }		},
	{ "green",		{ 0, 255, 0, 255 }		},
	{ "blue",		{ 0, 0, 255, 255 }		},
	{ "yellow",		{ 255, 255, 0, 255 }	},
	{ "cyan",		{ 0, 255, 255, 255 }	},
	{ "magenta",	{ 255, 0, 255, 255 }	},
	{ "white",		{ 255, 255, 255, 255 }	},
	{ "black",		{ 0, 0, 0, 255 }		},
	{ "90black",	{ 0, 0, 0, 230 }		},
	{ "75black",	{ 0, 0, 0, 191 }		},
	{ "50black",	{ 0, 0, 0, 128 }		},
	{ "25black",	{ 0, 0, 0, 64 }			},
	{ "10black",	{ 0, 0, 0, 25 }			},
	{ "transparent",{ 0, 0, 0, 0 }			},
};

const afxUtils::RGBA TdefaultColor = { 0xE0, 0xAF, 0x56, 0xFF };
const afxUtils::RGBA CTdefaultColor = { 0x72, 0x9B, 0xDD, 0xFF };
const afxUtils::RGBA TdefaultWashColor = { 0xEA, 0xBE, 0x54, 0xFF };
const afxUtils::RGBA CTdefaultWashColor = { 0x96, 0xC8, 0xFA, 0xFF };

struct ParticleData {
	float a1;
	float a2;
	float a3;
};

struct MyColor {
	afxUtils::RGBA value;
	afxUtils::RGBA defaultValue;
	std::string userValue = "";
	bool use = false;

	bool setColor(const char* arg) {
		if (nullptr == arg) return false;
		if (0 == _stricmp("default", arg))
		{
			use = false;
			value = defaultValue;
			return true;
		}

		for (int i = 0; i < 8; i++)
		{
			if (0 == _stricmp(afxBasicColors[i].name, arg))
			{
				use = true;
				userValue = arg;
				value = afxBasicColors[i].value;

				return true;
			}
		}
		return false;
	}

	bool setColor(advancedfx::ICommandArgs* args) {
		auto argc = args->ArgC();

		if (4 == argc)
		{
			std::string str = "";
			str.append(args->ArgV(1));
			str.append(" ");
			str.append(args->ArgV(2));
			str.append(" ");
			str.append(args->ArgV(3));

			use = true;
			userValue = str.c_str();
			value = { 
				(uint8_t)atoi(args->ArgV(1)),
				(uint8_t)atoi(args->ArgV(2)),
				(uint8_t)atoi(args->ArgV(3)),
				255
			};
			return true;
		}

		if (5 == argc)
		{
			std::string str = "";
			str.append(args->ArgV(1));
			str.append(" ");
			str.append(args->ArgV(2));
			str.append(" ");
			str.append(args->ArgV(3));
			str.append(" ");
			str.append(args->ArgV(4));

			use = true;
			userValue = str.c_str();
			value = { 
				(uint8_t)atoi(args->ArgV(1)),
				(uint8_t)atoi(args->ArgV(2)),
				(uint8_t)atoi(args->ArgV(3)),
				(uint8_t)atoi(args->ArgV(4))
			};
			return true;
		}

		return false;
	};
};

struct MirvColors {
	struct Glow {
		MyColor Tcolor = { TdefaultColor, TdefaultColor };
		MyColor CTcolor = { CTdefaultColor, CTdefaultColor };
	} glow;

	struct Trails {
		MyColor Tcolor = { TdefaultColor, TdefaultColor };
		MyColor CTcolor = { CTdefaultColor, CTdefaultColor };
		float width = 1.0f;
	} trails;

	struct Smokes {
		MyColor Tcolor = { { 0xB4, 0x81, 0x32, 0 }, { 0xB4, 0x81, 0x32, 0 } };
		MyColor CTcolor = { { 0x4B, 0x7F, 0x9B, 0 }, { 0x4B, 0x7F, 0x9B, 0 } };
	} smokes;

	struct TeamIdOverhead {
		MyColor Tcolor = { TdefaultWashColor, TdefaultWashColor };
		MyColor CTcolor = { CTdefaultWashColor, CTdefaultWashColor };
	} teamIdOverhead;
} g_MirvColors;

void SetHudReticleWashColorT(uint32_t value);
void SetHudReticleWashColorCT(uint32_t value);

void applyTeamIdOverheadColors () {
	{
		auto value = afxUtils::rgbaToHex(
			g_MirvColors.teamIdOverhead.CTcolor.use 
				? g_MirvColors.teamIdOverhead.CTcolor.value 
				: g_MirvColors.teamIdOverhead.CTcolor.defaultValue
			);

		SetHudReticleWashColorCT(value);
	}

	{
		auto value = afxUtils::rgbaToHex(
			g_MirvColors.teamIdOverhead.Tcolor.use 
				? g_MirvColors.teamIdOverhead.Tcolor.value 
				: g_MirvColors.teamIdOverhead.Tcolor.defaultValue
			);

		SetHudReticleWashColorT(value);
	}
}

typedef void (__fastcall *g_Original_setGlowColor_t)(u_char* glowProperty, uint32_t color);
g_Original_setGlowColor_t g_Original_setGlowColor = nullptr;

void __fastcall new_setGlowColor(u_char* glowProperty, uint32_t color) {

	// alpha is dynamic there
	if (
		(afxUtils::rgbaToHex(g_MirvColors.glow.Tcolor.defaultValue) & 0x00FFFFFF) == (color & 0x00FFFFFF)
	)
	{ 
		// there is some weird issue when its 0
		if (g_MirvColors.glow.Tcolor.use) {
			if (g_MirvColors.glow.Tcolor.value.r == 0)
				g_MirvColors.glow.Tcolor.value.r = 1;
			if (g_MirvColors.glow.Tcolor.value.g == 0)
				g_MirvColors.glow.Tcolor.value.g = 1;
			if (g_MirvColors.glow.Tcolor.value.b == 0)
				g_MirvColors.glow.Tcolor.value.b = 1;
			if (g_MirvColors.glow.Tcolor.value.a == 0)
				g_MirvColors.glow.Tcolor.value.a = 1;

			color = afxUtils::rgbaToHex(g_MirvColors.glow.Tcolor.value); 
		}
	} 
	else if (
		(afxUtils::rgbaToHex(g_MirvColors.glow.CTcolor.defaultValue) & 0x00FFFFFF) == (color & 0x00FFFFFF)
	) 
	{ 
		if (g_MirvColors.glow.CTcolor.use) {
			if (g_MirvColors.glow.CTcolor.value.r == 0)
				g_MirvColors.glow.CTcolor.value.r = 1;
			if (g_MirvColors.glow.CTcolor.value.g == 0)
				g_MirvColors.glow.CTcolor.value.g = 1;
			if (g_MirvColors.glow.CTcolor.value.b == 0)
				g_MirvColors.glow.CTcolor.value.b = 1;
			if (g_MirvColors.glow.CTcolor.value.a == 0)
				g_MirvColors.glow.CTcolor.value.a = 1;

			color = afxUtils::rgbaToHex(g_MirvColors.glow.CTcolor.value); 
		}
	}

	g_Original_setGlowColor(glowProperty, color);
}

typedef void (__fastcall *g_Original_setSmokeProps_t)(CEntityInstance* param_1);
g_Original_setSmokeProps_t g_Original_setSmokeProps = nullptr;

void __fastcall new_setSmokeProps(CEntityInstance* param_1) {
	auto team = param_1->GetTeam();
	auto smokeColor = (ParticleData*)(param_1 + g_clientDllOffsets.C_SmokeGrenadeProjectile.m_vSmokeColor);

	if (team == 2 && g_MirvColors.smokes.Tcolor.use)
	{
		smokeColor->a1 = (float)g_MirvColors.smokes.Tcolor.value.r;
		smokeColor->a2 = (float)g_MirvColors.smokes.Tcolor.value.g;
		smokeColor->a3 = (float)g_MirvColors.smokes.Tcolor.value.b;
	} 
	else if (team == 3 && g_MirvColors.smokes.CTcolor.use) 
	{
		smokeColor->a1 = (float)g_MirvColors.smokes.CTcolor.value.r;
		smokeColor->a2 = (float)g_MirvColors.smokes.CTcolor.value.g;
		smokeColor->a3 = (float)g_MirvColors.smokes.CTcolor.value.b;
	}

	g_Original_setSmokeProps(param_1);
}

typedef u_char* (__fastcall *g_Original_getParticleManager_t)(void);
g_Original_getParticleManager_t g_Original_getParticleManager = nullptr;

typedef int* (__fastcall *g_Original_createParticle_t)(u_char* CGameParticleManager, int* idx, const char* name, int param_4, int64_t param_5, int64_t param_6, int64_t param_7, int64_t param_8);
g_Original_createParticle_t g_Original_createParticle = nullptr;

typedef void (__fastcall *g_Original_updateParticle_t)(u_char* CGameParticleManager, int idx, int param_3, ParticleData* data, int param_5);
g_Original_updateParticle_t g_Original_updateParticle = nullptr;

typedef void (__fastcall *g_Original_drawStuff_t)(CEntityInstance* param_1, char param_2);
g_Original_drawStuff_t g_Original_drawStuff = nullptr;

void __fastcall new_drawStuff(CEntityInstance* param_1, char param_2) {
	auto name = param_1->GetClassName();
	auto team = param_1->GetTeam();

	SOURCESDK::CS2::Cvar_s * handle_show_xray = SOURCESDK::CS2::g_pCVar->GetCvar(SOURCESDK::CS2::g_pCVar->FindConVar("spec_show_xray", false).Get());
	auto canCreateGrenadeTrail = *(bool*)((u_char*)param_1 + g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_bCanCreateGrenadeTrail);

	if (handle_show_xray == nullptr || handle_show_xray->m_Value.m_bValue == false || canCreateGrenadeTrail == false) {
		g_Original_drawStuff(param_1, param_2);
		return;
	}

	if ( 
		strcmp(name, "smokegrenade_projectile"	) == 0 ||
		strcmp(name, "hegrenade_projectile"		) == 0 ||
		strcmp(name, "decoy_projectile"			) == 0 ||
		strcmp(name, "flashbang_projectile"		) == 0 ||
		strcmp(name, "molotov_projectile"		) == 0
	) {
		auto CGameParticleManager = g_Original_getParticleManager();
		bool useColor = false;
		ParticleData color;
		ParticleData data = { 
			4.0f, // seems to be lifetime, but there is weird issue when changing it
			g_MirvColors.trails.width,
			1.0f // alpha
		};

		if (team == 2 && g_MirvColors.trails.Tcolor.use) {
			useColor = true;
			color.a1 = (float)g_MirvColors.trails.Tcolor.value.r;
			color.a2 = (float)g_MirvColors.trails.Tcolor.value.g;
			color.a3 = (float)g_MirvColors.trails.Tcolor.value.b;
			data.a3 = g_MirvColors.trails.Tcolor.value.a / 255.0f;
		} else if (team == 3 && g_MirvColors.trails.CTcolor.use) {
			useColor = true;
			color.a1 = (float)g_MirvColors.trails.CTcolor.value.r;
			color.a2 = (float)g_MirvColors.trails.CTcolor.value.g;
			color.a3 = (float)g_MirvColors.trails.CTcolor.value.b;
			data.a3 = g_MirvColors.trails.CTcolor.value.a / 255.0f;
		}

		if (
			useColor &&
			*(int *)(param_1 + g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_nSnapshotTrajectoryEffectIndex) == -1
		) {
			*(float*)(param_1 + g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_flTrajectoryTrailEffectCreationTime) = g_MirvTime.curtime_get();

			int idx = 0;
			g_Original_createParticle(CGameParticleManager, &idx, "particles/entity/spectator_utility_trail.vpcf", 8, 0, 0, 0, 0);
			*(int *)(param_1 + g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_nSnapshotTrajectoryEffectIndex) = idx;

			g_Original_updateParticle(CGameParticleManager, idx, 0x10, &color, 0);
			g_Original_drawStuff(param_1, param_2);
		} else {
			g_Original_drawStuff(param_1, param_2);
			g_Original_updateParticle(CGameParticleManager, *(int *)(param_1 + g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_nSnapshotTrajectoryEffectIndex), 0x3, &data, 0);
		}

	} 
}

bool getAddresses(HMODULE clientDll) {
	// called with offset to m_Glow of C_BaseModelEntity as first argument
	size_t g_Original_setGlowColor_addr = getAddress(clientDll, "40 53 48 83 EC 20 48 8B D9 48 83 C1 40 39 11 ?? ?? 89 11 ?? ?? ?? ?? ?? 48 8B 4B 18 48 85 C9 ?? ?? 48 83");
	if(g_Original_setGlowColor_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	// can be found with offsets to m_vSmokeColor and m_vSmokeDetonationPos
	size_t g_Original_applySmokeProps_addr = getAddress(clientDll, "40 53 48 83 EC ?? 8B 91 ?? ?? ?? ?? 48 8B D9 85 D2 75");
	if(g_Original_applySmokeProps_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	// not very elegant way to get it // can be found close to functions below before calling them
	size_t g_Original_getParticleManager_addr = getAddress(clientDll, "48 8b 05 ?? ?? ?? ?? c3 cc cc cc cc cc cc cc cc 40 53 48 83 ec 20 90 48 8b d9 80 b9 18 01 00 00 00");
	if(g_Original_getParticleManager_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	// next three functions can be found with "particles/entity/spectator_utility_trail.vpcf" or with offsets to m_nSnapshotTrajectoryEffectIndex, etc.
	size_t g_Original_drawStuff_addr = getAddress(clientDll, "40 55 53 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 80 B9");
	if(g_Original_drawStuff_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	size_t g_Original_createParticle_addr = getAddress(clientDll, "4C 8B DC 53 48 81 EC ?? ?? ?? ?? F2 0F 10 05");
	if(g_Original_createParticle_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	size_t g_Original_updateParticle_addr = getAddress(clientDll, "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? F3 0F 10 1D ?? ?? ?? ?? 41 8B F8 8B DA 4C 8D 05");
	if(g_Original_updateParticle_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	g_Original_setGlowColor = (g_Original_setGlowColor_t)(g_Original_setGlowColor_addr);
	g_Original_setSmokeProps = (g_Original_setSmokeProps_t)(g_Original_applySmokeProps_addr);
	g_Original_getParticleManager = (g_Original_getParticleManager_t)(g_Original_getParticleManager_addr);
	g_Original_drawStuff = (g_Original_drawStuff_t)(g_Original_drawStuff_addr);
	g_Original_createParticle = (g_Original_createParticle_t)(g_Original_createParticle_addr);
	g_Original_updateParticle = (g_Original_updateParticle_t)(g_Original_updateParticle_addr);

	return true;
}

void HookMirvColors(HMODULE clientDll) {
	if (g_bHookedMirvColors) return;

	if (!getAddresses(clientDll)) return;

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Original_setGlowColor, new_setGlowColor);
	DetourAttach(&(PVOID&)g_Original_setSmokeProps, new_setSmokeProps);
	DetourAttach(&(PVOID&)g_Original_drawStuff, new_drawStuff);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour MirvColors functions.");
		return;
	}

	g_bHookedMirvColors  = true;
};

void mirvColors_Console(advancedfx::ICommandArgs* args) {
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("glow", arg1) || 0 == _stricmp("trails", arg1) || 0 == _stricmp("smokes", arg1) || 0 == _stricmp("teamid", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("t", arg2))
				{
					std::string value;

					if (0 == _stricmp("glow", arg1))
					{
						value = g_MirvColors.glow.Tcolor.use
							? g_MirvColors.glow.Tcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.glow.Tcolor.setColor(args->ArgV(3));
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.glow.Tcolor.setColor(&subArgs);
							return;
						}
					} 
					else if (0 == _stricmp("trails", arg1))
					{
						value = g_MirvColors.trails.Tcolor.use
							? g_MirvColors.trails.Tcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.trails.Tcolor.setColor(args->ArgV(3));
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.trails.Tcolor.setColor(&subArgs);
							return;
						}
					} 
					else if (0 == _stricmp("smokes", arg1))
					{
						value = g_MirvColors.smokes.Tcolor.use
							? g_MirvColors.smokes.Tcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.smokes.Tcolor.setColor(args->ArgV(3));
							return;
						}

						if (6 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.smokes.Tcolor.setColor(&subArgs);
							return;
						}
					}
					else if (0 == _stricmp("teamid", arg1))
					{
						value = g_MirvColors.teamIdOverhead.Tcolor.use
							? g_MirvColors.teamIdOverhead.Tcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.teamIdOverhead.Tcolor.setColor(args->ArgV(3));
							applyTeamIdOverheadColors();
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.teamIdOverhead.Tcolor.setColor(&subArgs);
							applyTeamIdOverheadColors();
							return;
						}
					}

					advancedfx::Message(
						"%s %s t <option> - Control T color.\n"
						"Current value: %s\n"
						, arg0, arg1
						, value.c_str()
					);

					return;
				}

				if (0 == _stricmp("ct", arg2))
				{
					std::string value;
					if (0 == _stricmp("glow", arg1))
					{
						value = g_MirvColors.glow.CTcolor.use
							? g_MirvColors.glow.CTcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.glow.CTcolor.setColor(args->ArgV(3));
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.glow.CTcolor.setColor(&subArgs);
							return;
						}
					} 
					else if (0 == _stricmp("trails", arg1))
					{
						value = g_MirvColors.trails.CTcolor.use
							? g_MirvColors.trails.CTcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.trails.CTcolor.setColor(args->ArgV(3));
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.trails.CTcolor.setColor(&subArgs);
							return;
						}
					} 
					else if (0 == _stricmp("smokes", arg1))
					{
						value = g_MirvColors.smokes.CTcolor.use
							? g_MirvColors.smokes.CTcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.smokes.CTcolor.setColor(args->ArgV(3));
							return;
						}

						if (6 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.smokes.CTcolor.setColor(&subArgs);
							return;
						}
					}
					else if (0 == _stricmp("teamid", arg1))
					{
						value = g_MirvColors.teamIdOverhead.CTcolor.use
							? g_MirvColors.teamIdOverhead.CTcolor.userValue
							: "default";

						if (4 == argc)
						{
							g_MirvColors.teamIdOverhead.CTcolor.setColor(args->ArgV(3));
							applyTeamIdOverheadColors();
							return;
						}

						if (7 == argc)
						{
							advancedfx::CSubCommandArgs subArgs(args, 3);
							g_MirvColors.teamIdOverhead.CTcolor.setColor(&subArgs);
							applyTeamIdOverheadColors();
							return;
						}
					}

					advancedfx::Message(
						"%s %s ct <option> - Control CT color.\n"
						"Current value: %s\n"
						, arg0, arg1
						, value.c_str()
					);
					return;
				}

				if (0 == _stricmp("width", arg2) && 0 == _stricmp("trails", arg1))
				{
					
					if (4 == argc)
					{
						g_MirvColors.trails.width = atof(args->ArgV(3));
						return;
					}

					advancedfx::Message(
						"%s %s width <f> - Set width of trails (Default: 1.0).\n"
						"Current value: %.1f\n"
						, arg0, arg1
						, g_MirvColors.trails.width
					);
					return;
				}
			}

			std::string colors = "";

			for (int i = 0; i < 8; i++)
			{
				auto color = afxBasicColors[i];
				colors.append(color.name);
				if (i < 7) colors.append(", ");
			}

			std::string options = 
				"Where <option> is one of:\n"
				"default - use default game color\n"
				"<color> - one of the default colors e.g. red\n";

			0 == _stricmp("smokes", arg1)
				? options.append("<0-255> <0-255> <0-255> - color in RGB format e.g. 255 0 0 \n")
				: options.append("<0-255> <0-255> <0-255> <0-255> - color in RGBA format e.g. 255 0 0 255\n");

			advancedfx::Message(
				"%s %s t <option> - Control T color.\n"
				"%s %s ct <option> - Control CT color.\n"
				, arg0, arg1
				, arg0, arg1
			);
			if (0 == _stricmp("trails", arg1)) advancedfx::Message(
				"%s %s width <f> - Set width of trails (Default: 1.0).\n"
				, arg0, arg1
			);

			advancedfx::Message(
				"%s\n"
				"Available colors:\n"
				"%s\n"
				, options.c_str(), colors.c_str()
			);
			return;
		}

	}

	advancedfx::Message(
		"%s glow [...] - Control glow colors.\n"
		"%s trails [...] - Control trails colors.\n"
		"%s smokes [...] - Control smokes colors.\n"
		"%s teamid [...] - Control team id overhead colors.\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
};

CON_COMMAND(mirv_colors, "Control some ingame colors.")
{
	mirvColors_Console(args);
};
