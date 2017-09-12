#include "stdafx.h"

#include "ClientToolsCssV34.h"

#include "addresses.h"
#include "RenderView.h"

#include <cssV34/sdk_src/public/tier1/KeyValues.h>
#include <cssV34/sdk_src/public/tools/bonelist.h>

#include <shared/StringTools.h>

using namespace SOURCESDK::CSSV34;

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
	if (!GetRecording())
		return;

	if (!(hEntity != SOURCESDK::CSSV34::HTOOLHANDLE_INVALID) && msg)
		return;

	char const * msgName = msg->GetName();

	if (!strcmp("entity_state", msgName))
	{
		char const * className = m_ClientTools->GetClassname(hEntity);

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
			|| className && !strcmp(className, "class C_CSRagdoll")
			;

		bool isWeapon =
			false
			|| className && (
				!strcmp(className, "weaponworldmodel")
				// cannot allow this for now, import plugins will cause model spam the way they work currently: // || !strcmp(className, "class C_PlayerAddonModel")
				)
			;

		bool isProjectile =
			className && StringEndsWith(className, "Projectile")
			;

		bool isViewModel =
			className && (
				!strcmp(className, "predicted_viewmodel")
				|| !strcmp(className, "class C_ViewmodelAttachmentModel")
				)
			;

		if (false
			|| RecordPlayers_get() && isPlayer
			|| RecordWeapons_get() && isWeapon
			|| RecordProjectiles_get() && isProjectile
			)
		{
			SOURCESDK::CSSV34::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSSV34::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

			if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible))
			{
				// Entity not visible, avoid trash data:

				std::map<SOURCESDK::CSSV34::HTOOLHANDLE,bool>::iterator it = m_TrackedHandles.find(hEntity);
				if (it != m_TrackedHandles.end() && it->second)
				{
					WriteDictionary("afxHidden");
					Write((int)(it->first));
					Write((float)g_Hook_VClient_RenderView.GetGlobals()->curtime_get());

					it->second = false;
				}

				return;
			}

			bool wasVisible = false;

			WriteDictionary("entity_state");
			Write((int)hEntity);
			{
				SOURCESDK::CSSV34::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSSV34::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
				if (pBaseEntityRs)
				{
					WriteDictionary("baseentity");
					Write((float)pBaseEntityRs->m_flTime);
					WriteDictionary(pBaseEntityRs->m_pModelName);
					Write((bool)pBaseEntityRs->m_bVisible);
					Write(pBaseEntityRs->m_vecRenderOrigin);
					Write(pBaseEntityRs->m_vecRenderAngles);

					wasVisible = pBaseEntityRs->m_bVisible;
				}
			}

			m_TrackedHandles[hEntity] = wasVisible;

			{
				SOURCESDK::CSSV34::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::CSSV34::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
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

			bool viewModel = 0 != msg->GetInt("viewmodel");

			Write((bool)viewModel);
		}
	}
	else
		if (!strcmp("deleted", msgName))
		{
			std::map<SOURCESDK::CSSV34::HTOOLHANDLE,bool>::iterator it = m_TrackedHandles.find(hEntity);
			if (it != m_TrackedHandles.end())
			{
				WriteDictionary("deleted");
				Write((int)(it->first));
				Write((float)g_Hook_VClient_RenderView.GetGlobals()->curtime_get());

				m_TrackedHandles.erase(it);
			}
		}
}

void CClientToolsCssV34::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

	if (!GetRecording())
		return;

	for (EntitySearchResult ent = m_ClientTools->FirstEntity(); 0 != ent; ent = m_ClientTools->NextEntity(ent))
	{
		SOURCESDK::CSSV34::HTOOLHANDLE hEnt = m_ClientTools->AttachToEntity(ent);

		if (hEnt != SOURCESDK::CSSV34::HTOOLHANDLE_INVALID)// && m_ClientTools->ShouldRecord(hEnt))
		{
			m_ClientTools->SetRecording(hEnt, true);
		}

		// never detach, the ToolsSystem does that already when the entity is removed:
		// m_ClientTools->DetachFromEntity(ent);
	}

}

void CClientToolsCssV34::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}

void CClientToolsCssV34::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		m_ClientTools->EnableRecordingMode(true);
	}
}

void CClientToolsCssV34::EndRecording()
{
	if (GetRecording())
	{
		m_ClientTools->EnableRecordingMode(false);
	}

	CClientTools::EndRecording();
}

void CClientToolsCssV34::Write(SOURCESDK::CSSV34::CBoneList const * value)
{
	Write((int)value->m_nBones);

	for (int i = 0; i < value->m_nBones; ++i)
	{
		Write(value->m_vecPos[i]);
		Write(value->m_quatRot[i]);
	}
}
