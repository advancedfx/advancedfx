#include "stdafx.h"

#include "csgo_CViewRender.h"

#include "AfxStreams.h"
#include "addresses.h"
#include <shared/AfxDetours.h>

static bool g_InRenderView_VGui_DrawHud = false;

int __stdcall DoOnRenderView_VGui_DrawHud_In(void)
{
	g_InRenderView_VGui_DrawHud = true;
	g_AfxStreams.OnDrawingHudBegin();

	return 0;
}

void __stdcall DoOnRenderView_VGui_DrawHud_Out(void)
{
	if (g_InRenderView_VGui_DrawHud)
	{
		g_InRenderView_VGui_DrawHud = false;
		g_AfxStreams.OnDrawingHudEnd();
	}
}

void * detoured_csgo_CViewRender_RenderView_VGui_DrawHud_In;

void * detoured_csgo_CViewRender_RenderView_VGui_DrawHud_Out;

void __declspec(naked) touring_csgo_CViewRender_RenderView_VGui_DrawHud_In(void)
{
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call DoOnRenderView_VGui_DrawHud_In
	__asm test eax, 1
	__asm pop edx
	__asm pop ecx
	__asm pop eax

	__asm jz __continue
	__asm jmp detoured_csgo_CViewRender_RenderView_VGui_DrawHud_Out

	__asm __continue:
	__asm jmp detoured_csgo_CViewRender_RenderView_VGui_DrawHud_In
}

void __declspec(naked) touring_csgo_CViewRender_RenderView_VGui_DrawHud_Out(void)
{
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call DoOnRenderView_VGui_DrawHud_Out
	__asm pop edx
	__asm pop ecx
	__asm pop eax

	__asm jmp detoured_csgo_CViewRender_RenderView_VGui_DrawHud_Out
}

/*
CCSViewRender_Render_t detoured_CCSViewRender_Render;

void __stdcall touring_CCSViewRender_Render(void * this_ptr, const SOURCESDK::vrect_t_csgo * rect)
{
	g_AfxStreams.OnRender(detoured_CCSViewRender_Render, this_ptr, rect);
	//detoured_CCSViewRender_Render(this_ptr, rect);
}
*/

CCSViewRender_RenderView_t detoured_CCSViewRender_RenderView;

bool g_csgo_SmokeOverlay_OnCompareAlphaBeforeDraw_FirstCall;
float g_csgo_OldSmokeOverlayAlphaFactor;
float g_csgo_NewSmokeOverlayAlphaFactor;
float g_csgo_AfxSmokeOverlayAlphaMod = 1.0f;

void __fastcall touring_CCSViewRender_RenderView(void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static bool isRootCall = true;

	if (isRootCall)
	{
		isRootCall = false;

		g_csgo_SmokeOverlay_OnCompareAlphaBeforeDraw_FirstCall = true;

		float * smokeOverlayAlphaFactor = (float *)((const char *)This + 0x588);

		g_csgo_OldSmokeOverlayAlphaFactor = *smokeOverlayAlphaFactor;

		g_AfxStreams.OnRenderView(detoured_CCSViewRender_RenderView, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, g_csgo_AfxSmokeOverlayAlphaMod);

		*smokeOverlayAlphaFactor = g_csgo_NewSmokeOverlayAlphaFactor;

		isRootCall = true;
	}
}

void * detoured_csgo_CViewRender_RenderSmokeOverlay_OnLoadOldAlpha;
void touring_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw(void);

void __declspec(naked) touring_csgo_CViewRender_RenderSmokeOverlay_OnLoadOldAlpha(void)
{
	// restore old value:
	__asm push eax
	__asm mov eax, g_csgo_OldSmokeOverlayAlphaFactor
	__asm mov dword ptr[edi + 588h], eax
	__asm pop eax

	__asm jmp detoured_csgo_CViewRender_RenderSmokeOverlay_OnLoadOldAlpha
}

void * detoured_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw;

void __declspec(naked) touring_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw(void)
{
	__asm test g_csgo_SmokeOverlay_OnCompareAlphaBeforeDraw_FirstCall, 0xff
	__asm je __continue

	__asm mov g_csgo_SmokeOverlay_OnCompareAlphaBeforeDraw_FirstCall, 0

	// store new old value:
	__asm push eax
	__asm mov eax, dword ptr[edi + 588h]
	__asm mov g_csgo_NewSmokeOverlayAlphaFactor, eax
	__asm pop eax

	__asm __continue:

	// calculate target value (xmm1 is free temporary register that we don't need to save):
	__asm movss xmm1, dword ptr[edi + 588h]
	__asm mulss xmm1, g_csgo_AfxSmokeOverlayAlphaMod
	__asm movss dword ptr [edi + 588h], xmm1

	__asm jmp detoured_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw
}

bool csgo_CViewRender_Install(void)
{
	static bool firstResult = true;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_CViewRender_RenderView_VGui_DrawHud_In) && AFXADDR_GET(csgo_CViewRender_RenderView_VGui_DrawHud_Out))
	{
		{
			detoured_csgo_CViewRender_RenderView_VGui_DrawHud_In = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CViewRender_RenderView_VGui_DrawHud_In), (BYTE *)touring_csgo_CViewRender_RenderView_VGui_DrawHud_In, 0x5);

			// fix-up code relocation:
			DWORD * pCalladdr = (DWORD *)((BYTE *)detoured_csgo_CViewRender_RenderView_VGui_DrawHud_In + 0x1);
			*pCalladdr = *pCalladdr - ((DWORD)detoured_csgo_CViewRender_RenderView_VGui_DrawHud_In - AFXADDR_GET(csgo_CViewRender_RenderView_VGui_DrawHud_In));
		}
		{
			detoured_csgo_CViewRender_RenderView_VGui_DrawHud_Out = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CViewRender_RenderView_VGui_DrawHud_Out), (BYTE *)touring_csgo_CViewRender_RenderView_VGui_DrawHud_Out, 0x5);
		}
	}
	else
		firstResult = false;

	if (AFXADDR_GET(csgo_CCSViewRender_vtable)
		&& AFXADDR_GET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha)
		&& AFXADDR_GET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw)
		&& AFXADDR_GET(csgo_client_CCSViewRender_RenderView_vtable_index))
	{
		int * vtable = (int*)AFXADDR_GET(csgo_CCSViewRender_vtable);

		//DetourIfacePtr((DWORD *)&(vtable[AFXADDR_GET(csgo_client_CCSViewRender_RenderView_vtable_index)-1]), touring_CCSViewRender_Render, (DetourIfacePtr_fn &)detoured_CCSViewRender_Render);
		AfxDetourPtr((PVOID *)&(vtable[AFXADDR_GET(csgo_client_CCSViewRender_RenderView_vtable_index)]), touring_CCSViewRender_RenderView, (PVOID *)&detoured_CCSViewRender_RenderView);

		detoured_csgo_CViewRender_RenderSmokeOverlay_OnLoadOldAlpha = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CCSViewRender_RenderSmokeOverlay_OnLoadOldAlpha), (BYTE *)touring_csgo_CViewRender_RenderSmokeOverlay_OnLoadOldAlpha, 8);	
		detoured_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CCSViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw), (BYTE *)touring_csgo_CViewRender_RenderSmokeOverlay_OnCompareAlphaBeforeDraw, 7);
	}
	else
		firstResult = false;

	return firstResult;
}
