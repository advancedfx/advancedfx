#include "stdafx.h"

#include "ClientToolsTf2.h"

#include "addresses.h"
#include "RenderView.h"

#include <tf2/sdk_src/public/tier1/KeyValues.h>
#include <tf2/sdk_src/public/tools/bonelist.h>

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>



SOURCESDK::CStudioHdr * g_tf2_hdr = nullptr;
std::vector<SOURCESDK::matrix3x4_t> g_tf2_BoneState;

typedef void *  (__fastcall * tf2_C_BaseAnimating_RecordBones_t)(void * This, void* Edx, SOURCESDK::CStudioHdr *hdr, SOURCESDK::matrix3x4_t *pBoneState );
tf2_C_BaseAnimating_RecordBones_t True_tf2_C_BaseAnimating_RecordBones = nullptr;
void * __fastcall My_tf2_C_BaseAnimating_RecordBones(void * This, void* Edx, SOURCESDK::CStudioHdr *hdr, SOURCESDK::matrix3x4_t *pBoneState ) {
	void * result = True_tf2_C_BaseAnimating_RecordBones(This, Edx, hdr, pBoneState);
	g_tf2_hdr =  hdr;
	if(g_tf2_BoneState.size() < hdr->numbones()) g_tf2_BoneState.resize(hdr->numbones());
	memcpy(&(g_tf2_BoneState[0]),pBoneState,sizeof(SOURCESDK::matrix3x4_t) * hdr->numbones());
	return result;
}

bool tf2_C_BaseAnimating_RecordBones_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(tf2_client_C_BaseAnimating_RecordBones))
	{
		LONG error = NO_ERROR;

		True_tf2_C_BaseAnimating_RecordBones = (tf2_C_BaseAnimating_RecordBones_t)AFXADDR_GET(tf2_client_C_BaseAnimating_RecordBones);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_tf2_C_BaseAnimating_RecordBones, My_tf2_C_BaseAnimating_RecordBones);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CClientToolsTf2 * CClientToolsTf2::m_Instance = 0;

CClientToolsTf2::CClientToolsTf2(SOURCESDK::TF2::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsTf2::~CClientToolsTf2()
{
	m_Instance = 0;
}

void CClientToolsTf2::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageTf2(reinterpret_cast<SOURCESDK::TF2::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::TF2::KeyValues *>(msg));
}

void CClientToolsTf2::OnPostToolMessageTf2(SOURCESDK::TF2::HTOOLHANDLE hEntity, SOURCESDK::TF2::KeyValues * msg)
{
	if (!(hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID && msg))
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
					for (SOURCESDK::TF2::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
						Tier0_Msg("%s,\n", subKey->GetName());
					Tier0_Msg("----\n");
				}

				if (SOURCESDK::TF2::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::TF2::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity")))
				{
					Tier0_Msg("%i: %s: %s\n", hEntity, className, pBaseEntityRs->m_pModelName);
				}
			}

			bool isPlayer =
				false
				|| className && (
					!strcmp(className, "class C_TFPlayer")
					|| !strcmp(className, "class C_TFRagdoll")
					)
				;

			bool isWeapon =
				false
				|| className && (
					StringBeginsWith(className, "tf_weapon_")
					|| !strcmp(className, "grenade")
					)
				;

			bool isProjectile =
				false
				|| className && StringBeginsWith(className, "class C_TFProjectile_")
				;

			bool isViewModel =
				false
				|| className && 0 == strcmp(className, "viewmodel")
				|| className && 0 == strcmp(className, "class C_ViewmodelAttachmentModel")
				;

			if (false
				|| RecordPlayers_get() && isPlayer
				|| RecordWeapons_get() && isWeapon
				|| RecordProjectiles_get() && isProjectile
				|| RecordViewModels_get() && isViewModel
				)
			{

				SOURCESDK::TF2::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::TF2::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible || isViewModel))
				{
					// Entity not visible, avoid trash data:

					std::map<SOURCESDK::TF2::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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
					SOURCESDK::TF2::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::TF2::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
					if (pBaseEntityRs)
					{
						wasVisible = pBaseEntityRs->m_bVisible || isViewModel;

						WriteDictionary("baseentity");
						//Write((float)pBaseEntityRs->m_flTime);
						WriteDictionary(pBaseEntityRs->m_pModelName);
						Write((bool)wasVisible);

						hasParentTransform = true;
						SOURCESDK::AngleMatrix(pBaseEntityRs->m_vecRenderAngles, pBaseEntityRs->m_vecRenderOrigin, parentTransform);
						WriteMatrix3x4(parentTransform);
					}
				}

				m_TrackedHandles[hEntity] = true;

				{
					SOURCESDK::TF2::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::TF2::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
					if (pBaseAnimatingRs && hasParentTransform)
					{
						WriteDictionary("baseanimating");
						//Write((int)pBaseAnimatingRs->m_nSkin);
						//Write((int)pBaseAnimatingRs->m_nBody);
						//Write((int)pBaseAnimatingRs->m_nSequence);
						WriteBones(g_tf2_hdr, &(g_tf2_BoneState[0]), parentTransform);
					}
				}

				WriteDictionary("/");

				bool viewModel = msg->GetBool("viewmodel");

				Write((bool)viewModel);
			}
		}
	}
	else if (!strcmp("deleted", msgName))
	{
		std::map<SOURCESDK::TF2::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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

		if (hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID && m_ClientTools->ShouldRecord(hEntity))
		{
			m_TrackedHandles[hEntity] = false;
			if (GetRecording()) m_ClientTools->SetRecording(hEntity, true);
		}
	}
}

void CClientToolsTf2::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

}

void CClientToolsTf2::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}


void CClientToolsTf2::EnableRecordingMode_set(bool value) {
	m_ClientTools->EnableRecordingMode(value);
}

bool CClientToolsTf2::EnableRecordingMode_get() {
	return m_ClientTools->IsInRecordingMode();
}

void CClientToolsTf2::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		if(!tf2_C_BaseAnimating_RecordBones_Install())
		{
			Tier0_Warning("AFX: Failed to install IClientRenderable::SetupBones hook.\n");
		}

		for (std::map<SOURCESDK::TF2::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, true);
		}
	}
}

void CClientToolsTf2::EndRecording()
{
	if (GetRecording())
	{
		for (std::map<SOURCESDK::TF2::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, false);
		}
	}

	CClientTools::EndRecording();
}

float CClientToolsTf2::ScaleFov(int width, int height, float fov)
{
	return (float)AlienSwarm_FovScaling(width, height, fov);
}
