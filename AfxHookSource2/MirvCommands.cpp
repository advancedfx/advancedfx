#include "MirvCommands.h"

bool g_bHookedMirvCommands = false;

bool g_bNoFlashEnabled = false;

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

bool getAddressesFromClient(HMODULE clientDll) {
	// can be found with offsets to m_flFlashScreenshotAlpha, m_flFlashDuration, m_flFlashMaxAlpha, etc. 
	// In this function values being assigned to all these offsets at once
	size_t g_Original_flashFunc_addr = getAddress(clientDll, "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 0F 29 74 24 ?? 33 FF");
	if(g_Original_flashFunc_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return false;
	}

	g_Original_flashFunc = (g_Original_flashFunc_t)(g_Original_flashFunc_addr);

	return true;
}

void HookMirvCommands(HMODULE clientDll) {
	if (g_bHookedMirvCommands) return;

	if (!getAddressesFromClient(clientDll)) return;

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Original_flashFunc, new_flashFunc);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour MirvCommands functions.");
		return;
	}

	g_bHookedMirvCommands = true;
};
