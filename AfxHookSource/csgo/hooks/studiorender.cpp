#include "stdafx.h"

#include "studiorender.h"

#include "../../addresses.h"
#include "../../WrpConsole.h"

#include "AfxStreams.h"

#include <shared/detours.h>

// Draws the model
typedef void (__stdcall * csgo_CStudioRender_DrawModel_t)(void * This, SOURCESDK::CSGO::DrawModelResults_t *pResults, const SOURCESDK::CSGO::DrawModelInfo_t& info,
	SOURCESDK::matrix3x4_t *pBoneToWorld, float *pFlexWeights, float *pFlexDelayedWeights, const SOURCESDK::Vector &modelOrigin, int flags);

csgo_CStudioRender_DrawModel_t Detoured_csgo_CStudioRender_DrawModel;

void __stdcall Toruing_csgo_CStudioRender_DrawModel(void * This, SOURCESDK::CSGO::DrawModelResults_t *pResults, const SOURCESDK::CSGO::DrawModelInfo_t& info,
	SOURCESDK::matrix3x4_t *pBoneToWorld, float *pFlexWeights, float *pFlexDelayedWeights, const SOURCESDK::Vector &modelOrigin, int flags)
{
	g_AfxStreams.SetClientRenderable(reinterpret_cast<SOURCESDK::IClientRenderable_csgo *>(info.m_pClientEntity));

	Detoured_csgo_CStudioRender_DrawModel(This, pResults, info, pBoneToWorld, pFlexWeights, pFlexDelayedWeights, modelOrigin, flags);

	g_AfxStreams.SetClientRenderable(nullptr);
}

// Methods related to static prop rendering
typedef void (__stdcall * csgo_CStudioRender_DrawModelStaticProp_t)(void * This, const SOURCESDK::CSGO::DrawModelInfo_t& drawInfo, const SOURCESDK::matrix3x4_t &modelToWorld, int flags);

csgo_CStudioRender_DrawModelStaticProp_t Detoured_csgo_CStudioRender_DrawModelStaticProp;

void __stdcall Touring_csgo_CStudioRender_DrawModelStaticProp(void * This, const SOURCESDK::CSGO::DrawModelInfo_t& drawInfo, const SOURCESDK::matrix3x4_t &modelToWorld, int flags)
{
	g_AfxStreams.SetClientRenderable(reinterpret_cast<SOURCESDK::IClientRenderable_csgo *>(drawInfo.m_pClientEntity));

	Detoured_csgo_CStudioRender_DrawModelStaticProp(This, drawInfo, modelToWorld, flags);

	g_AfxStreams.SetClientRenderable(nullptr);
}

bool StudioHooks_Install(SOURCESDK::CSGO::IStudioRender * pStudioRender)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (pStudioRender)
	{
		void ** vtable = *(void ***)pStudioRender;

		DetourIfacePtr((DWORD *)&(vtable[29]), Toruing_csgo_CStudioRender_DrawModel, (DetourIfacePtr_fn &)Detoured_csgo_CStudioRender_DrawModel);
		DetourIfacePtr((DWORD *)&(vtable[30]), Touring_csgo_CStudioRender_DrawModelStaticProp, (DetourIfacePtr_fn &)Detoured_csgo_CStudioRender_DrawModelStaticProp);

		firstResult = true;
	}

	return firstResult;
}