#include "stdafx.h"

#include "staticpropmgr.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "AfxStreams.h"

#include <shared/detours.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>


struct RenderableInstance_t;

typedef int(__fastcall * csgo_CStaticProp_IClientRenderable_DrawModel_t)(SOURCESDK::IClientRenderable_csgo * This, void * Edx, int flags, const RenderableInstance_t &instance);

csgo_CStaticProp_IClientRenderable_DrawModel_t Truecsgo_CStaticProp_IClientRenderable_DrawModel;

int __fastcall Mycsgo_CStaticProp_IClientRenderable_DrawModel(SOURCESDK::IClientRenderable_csgo * This, void * Edx, int flags, const RenderableInstance_t &instance)
{
	if (flags) g_AfxStreams.SetClientRenderable(This);

	int result = Truecsgo_CStaticProp_IClientRenderable_DrawModel(This, Edx, flags, instance);

	if (flags) g_AfxStreams.SetClientRenderable(nullptr);

	return result;
}

bool Hook_csgo_CStaticProp_IClientRenderable_DrawModel(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (
		AFXADDR_GET(csgo_CStaticProp_IClientEntity_vtable)
		)
	{
		LONG error = NO_ERROR;

		void **vtable = (void **)AFXADDR_GET(csgo_CStaticProp_IClientEntity_vtable);

		Truecsgo_CStaticProp_IClientRenderable_DrawModel = (csgo_CStaticProp_IClientRenderable_DrawModel_t)(vtable[9]);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_CStaticProp_IClientRenderable_DrawModel, Mycsgo_CStaticProp_IClientRenderable_DrawModel);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
