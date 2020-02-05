#include "stdafx.h"


#include "Mod_LeafPvs.h"

#include <hl_addresses.h>

#include <hlsdk.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

//
// Mod_LeafPVS WH related stuff:
//

// Hints:

// 01d51d90 R_RenderView (found by searching for "%3ifps %3i ms %4i wpoly %4i epoly\n")
// 
// 01d51c60 R_RenderScene (modified) (usually has pretty static offset from R_RenderView)
// 
// 01d51cb3 e8b8f0ffff      call    launcher!CreateInterface+0x94f981 (01d50d70)
// 01d51cb8 e883efffff      call    launcher!CreateInterface+0x94f851 (01d50c40)
// 01d51cbd e82ef5ffff      call    launcher!CreateInterface+0x94fe01 (01d511f0) R_SetupGL
// 01d51cc2 e8d9320000      call    launcher!CreateInterface+0x953bb1 (01d54fa0) <-- R_MarkLeaves
// 
// 01d54fa0 R_MarkLeafs (Valve modification):
// 
// (see Q1 source for more help):
// 
// 01edcdb4 --> r_novis.value
//
// R_MarkLeaves leads to Mod_LeafPVS

bool g_Mod_LeafPvs_NoVis = false;

typedef byte * (*Mod_LeafPVS_t)(mleaf_t *leaf, model_t *model);
Mod_LeafPVS_t g_Old_Mod_LeafPVS;

byte * New_Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if(g_Mod_LeafPvs_NoVis)
		return g_Old_Mod_LeafPVS( model->leafs, model);

	return g_Old_Mod_LeafPVS( leaf, model);
}

bool Hook_Mod_LeafPvs()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(Mod_LeafPVS) != NULL)
	{
		LONG error = NO_ERROR;

		g_Old_Mod_LeafPVS = (Mod_LeafPVS_t)AFXADDR_GET(Mod_LeafPVS);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_Mod_LeafPVS, New_Mod_LeafPVS);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_Mod_LeafPvs");
		}
	}
	else
		firstResult = false;

	return firstResult;
}

//
