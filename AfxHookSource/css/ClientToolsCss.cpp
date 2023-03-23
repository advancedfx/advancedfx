#include "stdafx.h"

#include "ClientToolsCss.h"

#include "addresses.h"
#include "RenderView.h"

#include <css/sdk_src/public/tier1/KeyValues.h>

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

using namespace SOURCESDK::CSS;

typedef void* (__fastcall* css_C_BaseAnimating_RecordBones_t)(void* This, void* Edx, SOURCESDK::CStudioHdr* hdr, SOURCESDK::matrix3x4_t* pBoneState);
css_C_BaseAnimating_RecordBones_t True_css_C_BaseAnimating_RecordBones = nullptr;
void* __fastcall My_css_C_BaseAnimating_RecordBones(void* This, void* Edx, SOURCESDK::CStudioHdr* hdr, SOURCESDK::matrix3x4_t* pBoneState) {
	void* result = True_css_C_BaseAnimating_RecordBones(This, Edx, hdr, pBoneState);
	
	if(CClientTools * instance = CClientTools::Instance())
		instance->CaptureBones(hdr, pBoneState);

	return result;
}

bool css_C_BaseAnimating_RecordBones_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(css_client_C_BaseAnimating_RecordBones))
	{
		LONG error = NO_ERROR;

		True_css_C_BaseAnimating_RecordBones = (css_C_BaseAnimating_RecordBones_t)AFXADDR_GET(css_client_C_BaseAnimating_RecordBones);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_css_C_BaseAnimating_RecordBones, My_css_C_BaseAnimating_RecordBones);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CClientToolsCss * CClientToolsCss::m_Instance = 0;

CClientToolsCss::CClientToolsCss(SOURCESDK::CSS::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsCss::~CClientToolsCss()
{
	m_Instance = 0;
}

void CClientToolsCss::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageCss(reinterpret_cast<SOURCESDK::CSS::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::CSS::KeyValues *>(msg));
}

void CClientToolsCss::OnPostToolMessageCss(SOURCESDK::CSS::HTOOLHANDLE hEntity, SOURCESDK::CSS::KeyValues * msg)
{
	if (!(hEntity != SOURCESDK::CSS::HTOOLHANDLE_INVALID && msg))
		return;

	char const * msgName = msg->GetName();

	if (!strcmp("entity_state", msgName))
	{
		if (GetRecording())
		{
			char const * className = m_ClientTools->GetClassname(hEntity);
			if (!className) className = "[NULL]";

			if (0 != Debug_get())
			{
				if (2 <= Debug_get())
				{
					Tier0_Msg("-- %s (%i) --\n", className, hEntity);
					for (SOURCESDK::CSS::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
						Tier0_Msg("%s,\n", subKey->GetName());
					Tier0_Msg("----\n");
				}

				if (SOURCESDK::CSS::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSS::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity")))
				{
					Tier0_Msg("%i: %s: %s\n", hEntity, className, pBaseEntityRs->m_pModelName);
				}
			}

			bool isPlayer =
				false
				|| className && (
					!strcmp(className, "class C_CSPlayer")
					|| !strcmp(className, "class C_CSRagdoll")
					)
				;

			bool isWeapon =
				false
				|| className && (
					StringBeginsWith(className, "weapon_")
					|| !strcmp(className, "class C_BreakableProp")
					)
				;

			bool isProjectile =
				className && !strcmp(className, "grenade")
				;

			bool isViewModel =
				className && (
					!strcmp(className, "viewmodel")
					)
				;

			if (false
				|| RecordPlayers_get() && isPlayer
				|| RecordWeapons_get() && isWeapon
				|| RecordProjectiles_get() && isProjectile
				|| RecordViewModels_get() && isViewModel
				)
			{
				SOURCESDK::CSS::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSS::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible))
				{
					// Entity not visible, avoid trash data:

					std::map<SOURCESDK::CSS::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
					if (it != m_TrackedHandles.end() && it->second)
					{
						MarkHidden((int)(it->first));

						it->second = false;
					}

					return;
				}

				bool wasVisible = false;
				bool hasParentTransform = false;
				SOURCESDK::matrix3x4_t parentTransform;				

				WriteDictionary("entity_state");
				Write((int)hEntity);
				{
					SOURCESDK::CSS::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSS::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
					if (pBaseEntityRs)
					{
						wasVisible = pBaseEntityRs->m_bVisible;

						WriteDictionary("baseentity");
						//Write((float)pBaseEntityRs->m_flTime);
						WriteDictionary(pBaseEntityRs->m_pModelName);
						Write((bool)wasVisible);

						hasParentTransform = true;
						SOURCESDK::AngleMatrix(pBaseEntityRs->m_vecRenderAngles, pBaseEntityRs->m_vecRenderOrigin, parentTransform);
						WriteMatrix3x4(parentTransform);
					}
				}

				m_TrackedHandles[hEntity] = wasVisible;

				{
					SOURCESDK::CSS::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::CSS::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
					if (pBaseAnimatingRs && hasParentTransform)
					{
						WriteDictionary("baseanimating");
						//Write((int)pBaseAnimatingRs->m_nSkin);
						//Write((int)pBaseAnimatingRs->m_nBody);
						//Write((int)pBaseAnimatingRs->m_nSequence);
						WriteBones(nullptr != pBaseAnimatingRs->m_pBoneList, parentTransform);
					}
				}

				WriteDictionary("/");

				bool viewModel = 0 != msg->GetInt("viewmodel");

				Write((bool)viewModel);
			}
		}
	}
	else if (!strcmp("deleted", msgName))
	{
		std::map<SOURCESDK::CSS::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
		if (it != m_TrackedHandles.end())
		{
			if (GetRecording())
			{
				WriteDictionary("deleted");
				Write((int)(it->first));
			}

			m_TrackedHandles.erase(it);
		}
	}
	else if (!strcmp("created", msgName))
	{
		if (0 != Debug_get() && hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID)
		{
			const char * className = m_ClientTools->GetClassname(hEntity);
			if (!className) className = "[NULL]";

			Tier0_Msg("%i n/a: %s\n", hEntity, className);
		}

		if (hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID)// && m_ClientTools->ShouldRecord(hEntity))
		{
			m_TrackedHandles[hEntity] = false;
			if(GetRecording()) m_ClientTools->SetRecording(hEntity, true);
		}
	}
}

void CClientToolsCss::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

}

void CClientToolsCss::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}

void CClientToolsCss::EnableRecordingMode_set(bool value) {
	m_ClientTools->EnableRecordingMode(value);
}

bool CClientToolsCss::EnableRecordingMode_get() {
	return m_ClientTools->IsInRecordingMode();
}

void CClientToolsCss::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		if(!css_C_BaseAnimating_RecordBones_Install())
		{
			Tier0_Warning("AFX: Failed to install IClientRenderable::SetupBones hook.\n");
		}

		for (std::map<SOURCESDK::CSS::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, true);
		}
	}
}

void CClientToolsCss::EndRecording()
{
	if (GetRecording())
	{
		for (std::map<SOURCESDK::CSS::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, false);
		}
	}

	CClientTools::EndRecording();
}
