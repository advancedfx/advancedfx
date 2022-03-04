#include "stdafx.h"

#include "csgo_view.h"

#include "addresses.h"
#include "AfxStreams.h"

#include <shared/AfxDetours.h>


SOURCESDK::IViewRender_csgo * GetView_csgo(void)
{
	SOURCESDK::IViewRender_csgo ** pView = (SOURCESDK::IViewRender_csgo **)AFXADDR_GET(csgo_view);

	if(pView)
		return *pView;

	return 0;
}


typedef bool (__fastcall * csgo_CViewRender_ShouldForceNoVis_t)(void* This);

csgo_CViewRender_ShouldForceNoVis_t detoured_csgo_CViewRender_ShouldForceNoVis;

bool g_csgo_CViewRender_ShouldForceNoVis_OverrideEnable = false;
bool g_csgo_CViewRender_ShouldForceNoVis_OverrideValue;

bool __fastcall csgo_CViewRender_ShouldForceNoVis(void* This)
{
	bool orgValue = detoured_csgo_CViewRender_ShouldForceNoVis(This);

	return g_AfxStreams.OnViewRenderShouldForceNoVis(orgValue);
}

bool Hook_csgo_CViewRender_ShouldForceNoVis(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	SOURCESDK::IViewRender_csgo * view = GetView_csgo();

	if (view && AFXADDR_GET(csgo_CViewRender_ShouldForceNoVis_vtable_index))
	{
		int * vtable = *(int**)view;

		AfxDetourPtr((PVOID *)&(vtable[AFXADDR_GET(csgo_CViewRender_ShouldForceNoVis_vtable_index)]), csgo_CViewRender_ShouldForceNoVis, (PVOID*)&detoured_csgo_CViewRender_ShouldForceNoVis);

		firstResult = true;
	}

	return firstResult;
}
