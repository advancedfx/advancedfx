#include "ViewModel.h"
#include "Globals.h"

#include <cstddef>
#include "../deps/release/Detours/src/detours.h"

struct MirvViewmodel 
{
	bool enabled = false;
	bool hooked = false;
	bool leftHandedValue = false;
	float offsetX = 0.0f, offsetY = 0.0f, offsetZ = 0.0f;
	float fovValue = 68.0f;
	bool enabledX = false, enabledY = false, enabledZ = false, enabledFOV = false, enabledLeftHanded = false;

	void setViewmodel
	(
		float x, float y, float z, float pFov, bool leftHanded, 
		bool setX = false, bool setY = false, bool setZ = false, bool setFOV = false, bool setLeftHanded = false
	)
	{
		if(setX) offsetX = x;
		if(setY) offsetY = y;
		if(setZ) offsetZ = z;
		if(setFOV) fovValue = pFov;
		if(setLeftHanded) leftHandedValue = leftHanded;

		enabledX = setX;
		enabledY = setY;
		enabledZ = setZ;
		enabledFOV = setFOV;
		enabledLeftHanded = setLeftHanded;
	
	};
} g_MirvViewmodel;

typedef void(__fastcall *g_OriginalViewmodelFunc_t)(void* param_1, float* pViewmodelOffsets, float* pFov);
g_OriginalViewmodelFunc_t g_OriginalViewmodelFunc = nullptr;

void __fastcall setViewmodel(void* param_1, float* pViewmodelOffsets, float* pFov)
{
    g_OriginalViewmodelFunc(param_1, pViewmodelOffsets, pFov);

    if (pViewmodelOffsets != nullptr && g_MirvViewmodel.enabled) {
        if (g_MirvViewmodel.enabledX) pViewmodelOffsets[0] = g_MirvViewmodel.offsetX; // x 
        if (g_MirvViewmodel.enabledY) pViewmodelOffsets[1] = g_MirvViewmodel.offsetY; // y
        if (g_MirvViewmodel.enabledZ) pViewmodelOffsets[2] = g_MirvViewmodel.offsetZ; // z
		if (g_MirvViewmodel.enabledFOV) pFov[0] = g_MirvViewmodel.fovValue;
    }
};

typedef bool(__fastcall *g_OriginalHandFunc_t)(int64_t param_1);
g_OriginalHandFunc_t g_OriginalHandFunc = nullptr;

bool __fastcall setHand(int64_t param_1)
{
	bool res = g_OriginalHandFunc(param_1);

	if (g_MirvViewmodel.enabled && g_MirvViewmodel.enabledLeftHanded) {
		return g_MirvViewmodel.leftHandedValue;
	}

	return res;
};

void HookViewmodel(HMODULE clientDll)
{
	if (g_MirvViewmodel.hooked) return;

	// This function references the cvars viewmodel_offset_x, viewmodel_offset_y, viewmodel_offset_z, viewmodel_offset_fov (usually 3rd reference).
	// can be also found with 00 00 88 42 (68.0 in float), which is max limit for fov
	// 
	size_t viewmodelAddr = getAddress(clientDll, "48 89 5C 24 18 55 56 57 41 56 41 57 48 83 EC 20 49 8B E8 48"); 
	if (0 == viewmodelAddr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	//TODO
		return;
	}
	// vtable byte offset 0xac8 for "C_CSGO_TeamPreviewModel" class (4th from end.).
	// This function is called right after the first call to viewmodelAddr function.
	size_t handAddr = getAddress(clientDll, "40 56 48 83 EC 20 80 B9 99 22 00 00 00 48 8B F1 ?? ?? 32 C0 48 83 C4 20 5E ?? 48 8B");
	if (0 == handAddr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return;
	}

    g_OriginalViewmodelFunc = (g_OriginalViewmodelFunc_t)(viewmodelAddr);
	g_OriginalHandFunc = (g_OriginalHandFunc_t)(handAddr);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)g_OriginalViewmodelFunc, setViewmodel);
	DetourAttach(&(PVOID&)g_OriginalHandFunc, setHand);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour Viewmodel functions.");
		return;
	};

	g_MirvViewmodel.hooked = true;
};

void ViewModel_Console(advancedfx::ICommandArgs* args)
{
	int argc = args->ArgC();
	const auto cmd = args->ArgV(0);

	if (argc > 1) {
		char const* subcmd = args->ArgV(1);
		if (0 == _stricmp("enabled", subcmd) ) {
			if (3 == argc) {
				g_MirvViewmodel.enabled = 0 != atoi(args->ArgV(2));
				return;
			};
			advancedfx::Message(
				"%s enabled 0|1 - enable (1) / disable (0) custom viewmodel.\n"
				"Current value: %s\n"
				, cmd
				, g_MirvViewmodel.enabled ? "1 (enabled)" : "0 (disabled)"
			);
		} else
		if (0 == _stricmp("set", subcmd)) {
			if (argc == 7) {
				bool setX = 0 != strcmp("*", args->ArgV(2));
				bool setY = 0 != strcmp("*", args->ArgV(3));
				bool setZ = 0 != strcmp("*", args->ArgV(4));
				bool setFOV = 0 != strcmp("*", args->ArgV(5));
				bool setLeftHanded = 0 != strcmp("*", args->ArgV(6));
				g_MirvViewmodel.setViewmodel(
					atof(args->ArgV(2)),
					atof(args->ArgV(3)),
					atof(args->ArgV(4)),
					atof(args->ArgV(5)),
					atoi(args->ArgV(6)) == 0 ? false : true,
					setX,
					setY,
					setZ,
					setFOV,
					setLeftHanded
				);
				return;
			};
			advancedfx::Message(
				"%s set <OffsetX|*> <OffsetY|*> <OffsetZ|*> <FOV|*> <Hand|*>\n"
				"Set viewmodel. Use * to indicate to not change (passthrough).\n"
				"Hand: 0 = Right Handed, 1 = Left Handed\n"
				"Current value: OffsetX: %.1f, OffsetY: %.1f, OffsetZ: %.1f, FOV: %.1f, Hand: %i\n"
				"Example:\n"
				"%s set 2 2.5 -2 68 0\n"
				, cmd
				, g_MirvViewmodel.offsetX
				, g_MirvViewmodel.offsetY
				, g_MirvViewmodel.offsetZ
				, g_MirvViewmodel.fovValue
				, g_MirvViewmodel.leftHandedValue
				, cmd
			);
		};
	} else {
		advancedfx::Message(
			"%s enabled <0|1> - enable (1) / disable (0) custom viewmodel\n"
			"%s set <OffsetX|*> <OffsetY|*> <OffsetZ|*> <FOV|*> <Hand|*>\n"
			"Set viewmodel. Use * to indicate passthrough, which means value will depend on engine.\n"
			"Hand: 0 = Right Handed, 1 = Left Handed\n"
			"\n"
			"Example 1 - set custom viewmodel\n"
			"%s set 2 2.5 -2 68 0\n"
			"%s enabled 1\n"
			"\n"
			"Example 2 - set custom viewmodel partially\n"
			"%s set 2 2.5 -2 68 *\n"
			"%s enabled 1\n"
			"Note the * in the end, it means in this case right/left hand state will depend on engine.\n"
			, cmd, cmd, cmd, cmd, cmd, cmd	
		);
	};
};

CON_COMMAND(mirv_viewmodel, "Set custom viewmodel")
{
	ViewModel_Console(args);
};
