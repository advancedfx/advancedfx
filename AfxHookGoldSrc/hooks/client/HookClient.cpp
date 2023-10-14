#include "stdafx.h"

#include "HookClient.h"

#include "../HookHw.h"
#include "../user32Hooks.h"

#include "cstrike/CrossHairFix.h"
#include "cstrike/SpectatorFix.h"
#include "cstrike/ViewmodelAnimationFix.h"


CAfxImportDllHook g_Import_client_USER32("USER32.dll", CAfxImportDllHooks({
	Get_Import_client_USER32_GetCursorPos(),
	Get_Import_client_USER32_SetCursorPos(),
	}));

CAfxImportsHook g_Import_client(CAfxImportsHooks({
	&g_Import_client_USER32
	}));


void HookClient(void* hModule)
{
	static bool firstRun = true;
	if(!firstRun) return;
	firstRun = false;

	g_Import_client.Apply((HMODULE)hModule);

	if (!*Get_Import_client_USER32_GetCursorPos()->GetTrueFunc()) { /* This is broken e.g. in dmc and dod (yes they broke m_rawinput 0 mouse there haha): MessageBox(0, "Interception failed:\nclient.dll:user32.dll!GetCursorPos", "MDT_ERROR", MB_OK | MB_ICONHAND); */ }
	if (!*Get_Import_client_USER32_SetCursorPos()->GetTrueFunc()) { /* This is broken e.g. in dmc and dod (yes they broke m_rawinput 0 mouse there haha): MessageBox(0, "Interception failed:\nclient.dll:user32.dll!SetCursorPos", "MDT_ERROR", MB_OK | MB_ICONHAND); */ }

	//
	// detect game:

	const char *gamedir = pEngfuncs->pfnGetGameDirectory();
	
	if(!gamedir) return;
	
	if(0 == _stricmp("cstrike",gamedir))
	{
		Hook_Cstrike_CrossHair_Fix();
		Hook_Cstrike_Spectator_Fix();
		Hook_Cstrike_Viewmodel_Animation_Fix();
	}
}
