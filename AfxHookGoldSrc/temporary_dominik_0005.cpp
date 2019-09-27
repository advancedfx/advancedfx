#include "stdafx.h"

#include "windows.h" // we need access to virtualprotect etc.

#include <hlsdk.h>

#include "cmdregister.h"

#include <shared/AfxDetours.h>

#include <hooks/HookHw.h>
#include "hooks/user32Hooks.h"
#include "hooks/DemoPlayer/DemoPlayer.h"

#include "filming.h"
#include "hl_addresses.h"

typedef void (*Unk_RenderView_t) (void);

Unk_RenderView_t g_Old_Unk_RenderView = 0;

void New_Unk_RenderView (void) {
	g_Filming.FullClear();
	g_Old_Unk_RenderView();
	g_Filming.DoCanDebugCapture();
}


REGISTER_DEBUGCMD_FUNC(tst_debugcapture)
{
	g_Filming.EnableDebugCapture(true);

/*	if(!g_Old_Unk_RenderView)
	{
		g_Old_Unk_RenderView = (Unk_RenderView_t) DetourApply((BYTE *)((DWORD)HL_ADDR_GET(hwDll)+0xBFC30), (BYTE *)New_Unk_RenderView, (int)0x09);
	}
*/
}

REGISTER_DEBUGCMD_FUNC(tst_undock)
{
	UndockGameWindowForCapture();
}

REGISTER_DEBUGCMD_FUNC(tst_dock)
{
	RedockGameWindow();
}


REGISTER_DEBUGCMD_FUNC(tst_info)
{
	DWORD dw,dw2,dw3;
	
	dw = (DWORD)pEngfuncs->pEfxAPI->R_DecalShoot;
	dw2 = (DWORD)&(pEngfuncs->pEfxAPI) - (DWORD)pEngfuncs;
	dw3 =(DWORD)&(pEngfuncs->pEfxAPI->R_DecalShoot) - (DWORD)pEngfuncs->pEfxAPI;
	pEngfuncs->Con_Printf("pEngfuncs->pEfxAPI->R_DecalShoot = 0x%08x (ofs1: 0x%08x, ofs2: 0x%08x)\n", dw, dw2, dw3);
	dw = (DWORD)pEngfuncs->pfnAddCommand;
	dw2 = (DWORD)&(pEngfuncs->pfnAddCommand) - (DWORD)pEngfuncs;
	pEngfuncs->Con_Printf("pEngfuncs->pfnAddCommand = 0x%08x (ofs1: 0x%08x)\n", dw ,dw2);
}

REGISTER_DEBUGCMD_FUNC(tst_demotime)
{
	double demoTime = g_DemoPlayer->GetDemoTime();

	pEngfuncs->Con_Printf("DemoTime: %f\n", demoTime);
}

REGISTER_DEBUGCMD_FUNC(tst_isspectateonly)
{
	pEngfuncs->Con_Printf(" pEngfuncs->IsSpectateOnly: %s\n", pEngfuncs->IsSpectateOnly() ? "true" : "false");
}

REGISTER_DEBUGCMD_FUNC(debug_cmdline)
{
	LPSTR cmdLine = GetCommandLine();
	pEngfuncs->Con_Printf("GetCommandLine()==\"%s\"", cmdLine);
}
