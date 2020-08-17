#include "stdafx.h"

#include "ClientToolsMom.h"

#include "addresses.h"
#include "RenderView.h"

#include <tf2/sdk_src/public/tier1/KeyValues.h>
#include <tf2/sdk_src/public/tools/bonelist.h>

#include <shared/StringTools.h>

CClientToolsMom* CClientToolsMom::m_Instance = 0;

CClientToolsMom::CClientToolsMom(SOURCESDK::TF2::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsMom::~CClientToolsMom()
{
	m_Instance = 0;
}

void CClientToolsMom::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageTf2(reinterpret_cast<SOURCESDK::TF2::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::TF2::KeyValues *>(msg));
}

void CClientToolsMom::OnPostToolMessageTf2(SOURCESDK::TF2::HTOOLHANDLE hEntity, SOURCESDK::TF2::KeyValues * msg)
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
					0 == strcmp(className, "class C_MomentumPlayer")
					)
				;

			bool isWeapon =
				false
				|| className && (
					StringBeginsWith(className, "mom_weapon_")
					|| 0 == strcmp(className, "grenade")
					)
				;

			bool isProjectile =
				false
				|| className && (
					StringEndsWith(className, "Projectile")
					)
				;

			bool isViewModel =
				false
				|| className && (
					0 == strcmp(className, "viewmodel")
					)
				;

			if(false
				|| RecordPlayers_get() && isPlayer
				|| RecordWeapons_get() && isWeapon
				|| RecordProjectiles_get() && isProjectile
				|| RecordViewModels_get() && isViewModel
				)
			{

				SOURCESDK::TF2::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::TF2::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible))
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

				WriteDictionary("entity_state");
				Write((int)hEntity);
				{
					SOURCESDK::TF2::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::TF2::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
					if (pBaseEntityRs)
					{
						WriteDictionary("baseentity");
						//Write((float)pBaseEntityRs->m_flTime);
						WriteDictionary(pBaseEntityRs->m_pModelName);
						Write((bool)pBaseEntityRs->m_bVisible);
						Write(pBaseEntityRs->m_vecRenderOrigin);
						Write(pBaseEntityRs->m_vecRenderAngles);

						wasVisible = pBaseEntityRs->m_bVisible;
					}
				}

				m_TrackedHandles[hEntity] = true;

				{
					SOURCESDK::TF2::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::TF2::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
					if (pBaseAnimatingRs)
					{
						WriteDictionary("baseanimating");
						//Write((int)pBaseAnimatingRs->m_nSkin);
						//Write((int)pBaseAnimatingRs->m_nBody);
						//Write((int)pBaseAnimatingRs->m_nSequence);
						Write((bool)(0 != pBaseAnimatingRs->m_pBoneList));
						if (pBaseAnimatingRs->m_pBoneList)
						{
							Write(pBaseAnimatingRs->m_pBoneList);
						}
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

void CClientToolsMom::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

}

void CClientToolsMom::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}


void CClientToolsMom::EnableRecordingMode_set(bool value) {
	m_ClientTools->EnableRecordingMode(value);
}

bool CClientToolsMom::EnableRecordingMode_get() {
	return m_ClientTools->IsInRecordingMode();
}

void CClientToolsMom::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		for (std::map<SOURCESDK::TF2::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, true);
		}
	}
}

void CClientToolsMom::EndRecording()
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

float CClientToolsMom::ScaleFov(int width, int height, float fov)
{
	return (float)AlienSwarm_FovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, g_Hook_VClient_RenderView.LastCameraFov);
}

void CClientToolsMom::Write(SOURCESDK::TF2::CBoneList const * value)
{
	Write((int)value->m_nBones);

	for (int i = 0; i < value->m_nBones; ++i)
	{
		Write(value->m_vecPos[i]);
		Write(value->m_quatRot[i]);
	}
}
