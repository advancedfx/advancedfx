#include "stdafx.h"

#include "ClientToolsCssV34.h"

#include "addresses.h"
#include "RenderView.h"

#include <cssV34/sdk_src/public/tier1/KeyValues.h>
#include <cssV34/sdk_src/public/tools/bonelist.h>

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

using namespace SOURCESDK::CSSV34;


SOURCESDK::CStudioHdr * g_cssv34_hdr = nullptr;
std::vector<SOURCESDK::matrix3x4_t> g_cssv34_BoneState;

typedef void *  (__fastcall * cssv34_C_BaseAnimating_RecordBones_t)(void* This, void* Edx, SOURCESDK::CStudioHdr *hdr);
cssv34_C_BaseAnimating_RecordBones_t True_cssv34_C_BaseAnimating_RecordBones = nullptr;
void * __fastcall My_cssv34_C_BaseAnimating_RecordBones(void* This, void* Edx, SOURCESDK::CStudioHdr *hdr) {
	g_cssv34_hdr =  hdr;
	SOURCESDK::matrix3x4_t *pBoneState = *(SOURCESDK::matrix3x4_t **)((unsigned char *)This + AFXADDR_GET(cssv34_client_C_BaseAnimating_m_BoneAccessor_m_pBones));
	if(g_cssv34_BoneState.size() < hdr->numbones()) g_cssv34_BoneState.resize(hdr->numbones());
	memcpy(&(g_cssv34_BoneState[0]),pBoneState,sizeof(SOURCESDK::matrix3x4_t) * hdr->numbones());
	void * result = True_cssv34_C_BaseAnimating_RecordBones(This, Edx, hdr);
	return result;
}

bool cssv34_C_BaseAnimating_RecordBones_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(cssv34_client_C_BaseAnimating_RecordBones))
	{
		LONG error = NO_ERROR;

		True_cssv34_C_BaseAnimating_RecordBones = (cssv34_C_BaseAnimating_RecordBones_t)AFXADDR_GET(cssv34_client_C_BaseAnimating_RecordBones);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_cssv34_C_BaseAnimating_RecordBones, My_cssv34_C_BaseAnimating_RecordBones);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CClientToolsCssV34 * CClientToolsCssV34::m_Instance = 0;

CClientToolsCssV34::CClientToolsCssV34(SOURCESDK::CSSV34::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsCssV34::~CClientToolsCssV34()
{
	m_Instance = 0;
}

void CClientToolsCssV34::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageCssV34(reinterpret_cast<SOURCESDK::CSSV34::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::CSSV34::KeyValues *>(msg));
}

void CClientToolsCssV34::OnPostToolMessageCssV34(SOURCESDK::CSSV34::HTOOLHANDLE hEntity, SOURCESDK::CSSV34::KeyValues * msg)
{
	if (!(hEntity != SOURCESDK::CSSV34::HTOOLHANDLE_INVALID && msg))
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
					for (SOURCESDK::CSSV34::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
						Tier0_Msg("%s,\n", subKey->GetName());
					Tier0_Msg("----\n");
				}

				if (SOURCESDK::CSSV34::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSSV34::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity")))
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
				SOURCESDK::CSSV34::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSSV34::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible))
				{
					// Entity not visible, avoid trash data:

					std::map<SOURCESDK::CSSV34::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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
					SOURCESDK::CSSV34::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSSV34::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
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
					SOURCESDK::CSSV34::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::CSSV34::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
					if (pBaseAnimatingRs && hasParentTransform)
					{
						WriteDictionary("baseanimating");
						//Write((int)pBaseAnimatingRs->m_nSkin);
						//Write((int)pBaseAnimatingRs->m_nBody);
						//Write((int)pBaseAnimatingRs->m_nSequence);
						WriteBones(g_cssv34_hdr, &(g_cssv34_BoneState[0]), parentTransform);
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
		std::map<SOURCESDK::CSSV34::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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

void CClientToolsCssV34::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

}

void CClientToolsCssV34::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}

void CClientToolsCssV34::EnableRecordingMode_set(bool value) {
	m_ClientTools->EnableRecordingMode(value);
}

bool CClientToolsCssV34::EnableRecordingMode_get() {
	return m_ClientTools->IsInRecordingMode();
}

void CClientToolsCssV34::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		if(!cssv34_C_BaseAnimating_RecordBones_Install())
		{
			Tier0_Warning("AFX: Failed to install IClientRenderable::SetupBones hook.\n");
		}

		for (std::map<SOURCESDK::CSSV34::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, true);
		}
	}
}

void CClientToolsCssV34::EndRecording()
{
	if (GetRecording())
	{
		for (std::map<SOURCESDK::CSSV34::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, false);
		}
	}

	CClientTools::EndRecording();
}

