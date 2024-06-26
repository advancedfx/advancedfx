#include "ViewModel.h"
#include "WrpConsole.h"
#include "Globals.h"

#include "../shared/binutils.h"

#include <cstddef>
#include "../deps/release/Detours/src/detours.h"

class MirvViewmodel 
{
	bool m_bHooked = false;
	bool m_Enabled = false;
public:
	int m_leftHanded = 0;
	float m_OffsetX = 0.0f, m_OffsetY = 0.0f, m_OffsetZ = 0.0f;
	float m_Fov = 68.0f;
	bool m_enabledX = false, m_enabledY = false, m_enabledZ = false, m_enabledFOV = false, m_enabledLeftHanded = false;
	void setViewmodel
	(
		float x, float y, float z, float fov, int leftHanded, 
		bool setX = false, bool setY = false, bool setZ = false, bool setFOV = false, bool setLeftHanded = false
	)
	{
		if (leftHanded != 0 && leftHanded != 1) leftHanded = 0;
		if(setX) m_OffsetX = x;
		if(setY) m_OffsetY = y;
		if(setZ) m_OffsetZ = z;
		if(setFOV) m_Fov = fov;
		if(setLeftHanded) m_leftHanded = leftHanded;

		m_enabledX = setX;
		m_enabledY = setY;
		m_enabledZ = setZ;
		m_enabledFOV = setFOV;
		m_enabledLeftHanded = setLeftHanded;
	
	};

	void setEnabled(bool enabled) {
		m_Enabled = enabled;
	};
	bool isEnabled() {
		return m_Enabled;
	};

	bool isHooked() {
		return m_bHooked;
	};

	void setHooked(bool hooked) {
		m_bHooked = hooked;
	};

};

MirvViewmodel g_MirvViewmodel;

typedef void(__fastcall* OriginalViewmodelFuncType)(void* param_1, void* viewmodel_offset, void* fov);
OriginalViewmodelFuncType g_OriginalViewmodelFunc = nullptr;

void __fastcall setViewmodel(void* param_1, void* viewmodel_offset, void* fov)
{
    g_OriginalViewmodelFunc(param_1, viewmodel_offset, fov);

    if (viewmodel_offset != nullptr && g_MirvViewmodel.isEnabled()) {
        float* offsets = reinterpret_cast<float*>(viewmodel_offset);
        if (g_MirvViewmodel.m_enabledX) offsets[0] = g_MirvViewmodel.m_OffsetX; // x 
        if (g_MirvViewmodel.m_enabledY) offsets[1] = g_MirvViewmodel.m_OffsetY; // y
        if (g_MirvViewmodel.m_enabledZ) offsets[2] = g_MirvViewmodel.m_OffsetZ; // z

		float* fovVal = reinterpret_cast<float*>(fov);
		if (g_MirvViewmodel.m_enabledFOV) fovVal[0] = g_MirvViewmodel.m_Fov;
    }
};

typedef bool(__fastcall* OriginalHandFuncType)(int64_t param_1);
OriginalHandFuncType g_OriginalHandFunc = nullptr;

bool __fastcall setHand(int64_t param_1)
{
	bool res = g_OriginalHandFunc(param_1);

	if (g_MirvViewmodel.isEnabled() && g_MirvViewmodel.m_enabledLeftHanded) {
		return g_MirvViewmodel.m_leftHanded;
	}

	return res;
	
};

size_t getAddress(HMODULE clientDll, char const* pattern)
{

	Afx::BinUtils::ImageSectionsReader sections((HMODULE)clientDll);
	Afx::BinUtils::MemRange textRange = sections.GetMemRange();
	Afx::BinUtils::MemRange result = FindPatternString(textRange, pattern);
	if (result.IsEmpty()) {
		advancedfx::Warning("Could not find address for pattern: %s\n", pattern);
		return 0;
	} else {
		return result.Start;
	}
};

void HookViewmodel(HMODULE clientDll)
{

	if (g_MirvViewmodel.isHooked()) return;

	// This function references the cvars viewmodel_offset_x, viewmodel_offset_y, viewmodel_offset_z, viewmodel_offset_fov (usually 3rd reference).
	size_t viewmodelAddr = getAddress(clientDll, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 49 8B E8 48 8B DA 48 8B F1"); // 0x51d8a0 26.06.24
	if (0 == viewmodelAddr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return;
	}
	// vtable byte offset 0xa98 for "C_CSGO_TeamPreviewModel" class (4th from end.).
	// This function is called right after the first call to viewmodelAddr function.
	size_t handAddr = getAddress(clientDll, "40 56 48 83 EC 20 80 B9 79 21 00 00 00 48 8B F1 ?? ?? 32 C0 48 83 C4 20 5E ?? 48"); // 0x526700 26.06.24
	if (0 == handAddr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return;
	}

	void* targetViewmodelFunc = reinterpret_cast<void*>(viewmodelAddr); 
	bool* targetHandFunc = reinterpret_cast<bool*>(handAddr);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    g_OriginalViewmodelFunc = reinterpret_cast<OriginalViewmodelFuncType>(targetViewmodelFunc);
	g_OriginalHandFunc = reinterpret_cast<OriginalHandFuncType>(targetHandFunc);

    DetourAttach(&(PVOID&)g_OriginalViewmodelFunc, setViewmodel);
	DetourAttach(&(PVOID&)g_OriginalHandFunc, setHand);
	if(NO_ERROR != DetourTransactionCommit()) ErrorBox("Failed to detour Viewmodel functions.");

	g_MirvViewmodel.setHooked(true);
};

void ViewModel_Console(advancedfx::ICommandArgs* args)
{
	int argc = args->ArgC();
	if (argc > 1) {
		char const* subcmd = args->ArgV(1);
		if (0 == _stricmp("enabled", subcmd) ) {
			if (3 == argc) {
				g_MirvViewmodel.setEnabled(0 != atoi(args->ArgV(2)));
				return;
			};
			advancedfx::Message(
				"%s enabled 0|1 - enable (1) / disable (0) custom viewmodel.\n"
				"Current value: %s\n"
				, args->ArgV(0)
				, g_MirvViewmodel.isEnabled() ? "1 (enabled)" : "0 (disabled)"
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
					atoi(args->ArgV(6)),
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
				"Set viewmodel. Use * to indicate to not change."
				"Hand: 0 = Right Handed, 1 = Left Handed\n"
				"Current value: OffsetX: %.1f, OffsetY: %.1f, OffsetZ: %.1f, FOV: %.1f, Hand: %i\n"
				"Example:\n"
				"%s set 2 2.5 -2 68 0\n"
				, args->ArgV(0)
				, g_MirvViewmodel.m_OffsetX
				, g_MirvViewmodel.m_OffsetY
				, g_MirvViewmodel.m_OffsetZ
				, g_MirvViewmodel.m_Fov
				, g_MirvViewmodel.m_leftHanded
				, args->ArgV(0)
			);
		};
	} else {
		advancedfx::Message("%s enabled <0|1> - enable (1) / disable (0) the viewmodel\n", args->ArgV(0));
		advancedfx::Message("%s set <OffsetX|*> <OffsetY|*> <OffsetZ|*> <FOV|*> <Hand|*>\n", args->ArgV(0));
		advancedfx::Message("Set viewmodel. Use * to indicate passthrough, which means value will depend on engine.\n");
		advancedfx::Message("Hand: 0 = Right Handed, 1 = Left Handed\n");
		advancedfx::Message("\n");
		advancedfx::Message("Example 1 - set custom viewmodel\n");
		advancedfx::Message("%s set 2 2.5 -2 68 0\n", args->ArgV(0));
		advancedfx::Message("%s enabled 1\n", args->ArgV(0));
		advancedfx::Message("\n");
		advancedfx::Message("Example 2 - set custom viewmodel partially\n");
		advancedfx::Message("%s set 2 2.5 -2 68 *\n", args->ArgV(0));
		advancedfx::Message("%s enabled 1\n", args->ArgV(0));
		advancedfx::Message("Note the * in the end, it means passthrough, which means in this case right/left hand state will depend on engine.\n");
	};
};

CON_COMMAND(mirv_viewmodel, "Set custom viewmodel")
{
	ViewModel_Console(args);
};
