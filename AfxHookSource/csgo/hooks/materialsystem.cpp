#include "stdafx.h"

#include "materialsystem.h"

#include "../../AfxInterop.h"

#include "addresses.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


typedef void(__fastcall * csgo_CMatQueuedRenderContext_EndQueue_t)(void * This, void * Edx, bool bCallQueued);

csgo_CMatQueuedRenderContext_EndQueue_t g_Org_csgo_CMatQueuedRenderContext_EndQueue;

void __fastcall My_csgo_CMatQueuedRenderContext_EndQueue(void* This, void* Edx, bool bCallQueued) {

	AfxInterop::On_CMatQueuedRenderContext_EndQueue(bCallQueued);

	g_Org_csgo_CMatQueuedRenderContext_EndQueue(This, Edx, bCallQueued);
}

bool Hook_Materialsystem_CMatQueuedRenderContext_EndFrame() {
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_BaseEntity_ShouldInterpolate))
	{
		LONG error = NO_ERROR;

		g_Org_csgo_CMatQueuedRenderContext_EndQueue = (csgo_CMatQueuedRenderContext_EndQueue_t)AFXADDR_GET(csgo_materialsystem_CMatQueuedRenderContext_EndQueue);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Org_csgo_CMatQueuedRenderContext_EndQueue, My_csgo_CMatQueuedRenderContext_EndQueue);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}