#include "stdafx.h"

#include "c_baseanimating.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "AfxStreams.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


struct RenderableInstance_t;

typedef int(__fastcall * csgo_C_BaseAnimating_IClientRenderable_DrawModel_t)(SOURCESDK::IClientRenderable_csgo * This, void * Edx, int flags, const RenderableInstance_t &instance);

csgo_C_BaseAnimating_IClientRenderable_DrawModel_t Truecsgo_C_BaseAnimating_IClientRenderable_DrawModel;

int __fastcall Mycsgo_C_BaseAnimating_IClientRenderable_DrawModel(SOURCESDK::IClientRenderable_csgo * This, void * Edx, int flags, const RenderableInstance_t &instance)
{
	if(flags) g_AfxStreams.SetClientRenderable(This);

	int result = Truecsgo_C_BaseAnimating_IClientRenderable_DrawModel(This, Edx, flags, instance);

	if (flags) g_AfxStreams.SetClientRenderable(nullptr);

	return result;
}

bool Hook_csgo_C_BaseAnimating_IClientRenderable_DrawModel(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (
		AFXADDR_GET(csgo_C_BaseAnimating_IClientEntity_vtable)
		)
	{
		LONG error = NO_ERROR;

		void **vtable = (void **)AFXADDR_GET(csgo_C_BaseAnimating_IClientEntity_vtable);

		Truecsgo_C_BaseAnimating_IClientRenderable_DrawModel = (csgo_C_BaseAnimating_IClientRenderable_DrawModel_t)(vtable[9]);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_C_BaseAnimating_IClientRenderable_DrawModel, Mycsgo_C_BaseAnimating_IClientRenderable_DrawModel);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
