#include "stdafx.h"

#include "csgo_CRendering3dView.h"

#include "addresses.h"
#include "AfxStreams.h"

#include <shared/Detours/src/detours.h>

typedef void(__fastcall * CRendering3dView_DrawTranslucentRenderables_t)(void * This, void * edx, bool bInSkybox, bool bShadowDepth);

CRendering3dView_DrawTranslucentRenderables_t True_CRendering3dView_DrawTranslucentRenderables;

void __fastcall My_CRendering3dView_DrawTranslucentRenderables(void * This, void * edx, bool bInSkybox, bool bShadowDepth)
{
	g_AfxStreams.On_DrawTranslucentRenderables(bInSkybox, bShadowDepth, false);

	True_CRendering3dView_DrawTranslucentRenderables(This, edx, bInSkybox, bShadowDepth);

	g_AfxStreams.On_DrawTranslucentRenderables(bInSkybox, bShadowDepth, true);
}

bool csgo_CRendering3dView_Install()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;
	
	if (AFXADDR_GET(csgo_CRendering3dView_DrawTranslucentRenderables))
	{
		LONG error = NO_ERROR;

		True_CRendering3dView_DrawTranslucentRenderables = (CRendering3dView_DrawTranslucentRenderables_t)AFXADDR_GET(csgo_CRendering3dView_DrawTranslucentRenderables);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_CRendering3dView_DrawTranslucentRenderables, My_CRendering3dView_DrawTranslucentRenderables);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
