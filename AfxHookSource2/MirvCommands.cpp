#include "MirvCommands.h"

#include "../shared/StringTools.h"

#include "SceneSystem.h"

bool g_bHookedMirvCommands = false;

bool g_bNoFlashEnabled = false;

bool g_bEndOfMatchEnabled = true;

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

	g_Original_flashFunc = (g_Original_flashFunc_t)(g_Original_flashFunc_addr);
	g_Original_EOM = (g_Original_EOM_t)(g_Original_EOM_addr);

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

	// if (auto addr = getAddress(clientDll, "33 DB 48 8D 05 ?? ?? ?? ?? 48 8B CF 48 89 44 24 ??")) {
	// 	auto offset = *(int32_t*)(addr + 5);
	// 	org_ForceUpdateSkybox = (ForceUpdateSkybox_t)(addr + 2 + 7 + offset);
	// } else ErrorBox(MkErrStr(__FILE__, __LINE__));

	return true;
}

void HookMirvCommands(HMODULE clientDll) {
	if (g_bHookedMirvCommands) return;

	if (!getAddressesFromClient(clientDll)) return;

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Original_flashFunc, new_flashFunc);
	DetourAttach(&(PVOID&)g_Original_EOM, new_EOM);
	// DetourAttach(&(PVOID&)org_ForceUpdateSkybox, new_ForceUpdateSkybox);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour MirvCommands functions.");
		return;
	}

	g_bHookedMirvCommands = true;
};

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
