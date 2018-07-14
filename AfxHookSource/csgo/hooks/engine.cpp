#include "stdafx.h"

#include "engine.h"

#include "../../addresses.h"
#include "SourceInterfaces.h"
#include "../../WrpConsole.h"

#include <shared/detours.h>

typedef void(__fastcall * Engine_ToggleDebugger_t)(void * This, void * edx);

Engine_ToggleDebugger_t g_Engine_ToggleDebugger = nullptr;
void * g_Engine_ToggleDebugger_This = nullptr;

void * Detoured_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall;

void __declspec(naked) Touring_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall(void) {
	
	__asm push eax

	__asm mov eax, [esp + 0x08]
	__asm mov g_Engine_ToggleDebugger_This, eax

	__asm mov eax, [esp + 0x0C]
	__asm mov g_Engine_ToggleDebugger, eax

	__asm pop eax

	__asm jmp [Detoured_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall]
}


bool EngineHooks_Install()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (
		AFXADDR_GET(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall)
		)
	{
		Detoured_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall = DetourApply((BYTE *)AFXADDR_GET(csgo_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall), (BYTE *)Touring_engine_RegisterForUnhandledEvent_ToggleDebugger_BeforeCall, 0x6);

		firstResult = true;
	}

	return firstResult;
}

extern SOURCESDK::CSGO::vgui::ISurface *g_pVGuiSurface_csgo;

bool EngineHooks_PanoramaDebuggerActive()
{
	if(g_Engine_ToggleDebugger_This) return 0 != *((BYTE *)g_Engine_ToggleDebugger_This + 0x1d);
	
	return false;
}


void OnPanoramaDebuggerOpened();


void OnPanoramaDebuggerClosed();

CON_COMMAND(mirv_panorama_toggledebugger, "Toggle Panorama UI debugger") {
	if (!EngineHooks_Install() || nullptr == g_Engine_ToggleDebugger_This || nullptr == g_pVGuiSurface_csgo)
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return;
	}

	g_Engine_ToggleDebugger(g_Engine_ToggleDebugger_This, 0);

	if (EngineHooks_PanoramaDebuggerActive())
		OnPanoramaDebuggerOpened();
	else
		OnPanoramaDebuggerClosed();
}
