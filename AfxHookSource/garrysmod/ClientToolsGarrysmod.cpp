#include "stdafx.h"

#include "ClientToolsGarrysmod.h"

#include "addresses.h"
#include "RenderView.h"

#include <garrysmod/sdk_src/public/tier1/KeyValues.h>

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

using namespace SOURCESDK::GARRYSMOD;

typedef void* (__fastcall* garrysmod_C_BaseAnimating_RecordBones_t)(void* This, void* unk1, void* unk2);
garrysmod_C_BaseAnimating_RecordBones_t True_garrysmod_C_BaseAnimating_RecordBones = nullptr;
void* __fastcall My_garrysmod_C_BaseAnimating_RecordBones(void* This, void* unk1, void* unk2) {
	void* result = True_garrysmod_C_BaseAnimating_RecordBones(This, unk1, unk2);

	auto pHdr = (SOURCESDK::CStudioHdr*)(*(u_char**)((u_char*)This + 0x16d8));
	auto pMatrix = *(SOURCESDK::matrix3x4_t**)((u_char*)This + 0x13F0);
	
	if (CClientTools* instance = CClientTools::Instance())
		instance->CaptureBones(pHdr, pMatrix);

	return result;
}

bool garrysmod_C_BaseAnimating_RecordBones_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(garrysmod_client_C_BaseAnimating_RecordBones))
	{
		LONG error = NO_ERROR;

		True_garrysmod_C_BaseAnimating_RecordBones = (garrysmod_C_BaseAnimating_RecordBones_t)AFXADDR_GET(garrysmod_client_C_BaseAnimating_RecordBones);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_garrysmod_C_BaseAnimating_RecordBones, My_garrysmod_C_BaseAnimating_RecordBones);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CClientToolsGarrysmod * CClientToolsGarrysmod::m_Instance = 0;

CClientToolsGarrysmod::CClientToolsGarrysmod(SOURCESDK::GARRYSMOD::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsGarrysmod::~CClientToolsGarrysmod()
{
	m_Instance = 0;
}

void CClientToolsGarrysmod::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageGarrysmod(reinterpret_cast<SOURCESDK::GARRYSMOD::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::GARRYSMOD::KeyValues *>(msg));
}

void CClientToolsGarrysmod::OnPostToolMessageGarrysmod(SOURCESDK::GARRYSMOD::HTOOLHANDLE hEntity, SOURCESDK::GARRYSMOD::KeyValues * msg)
{
	if (!(hEntity != SOURCESDK::GARRYSMOD::HTOOLHANDLE_INVALID && msg))
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
					for (SOURCESDK::GARRYSMOD::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
						Tier0_Msg("%s,\n", subKey->GetName());
					Tier0_Msg("----\n");
				}

				if (SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity")))
				{
					Tier0_Msg("%i: %s: %s\n", hEntity, className, pBaseEntityRs->m_pModelName);
				}
			}

			bool isPlayer =
				false
				|| className && (
					!strcmp(className, "player")
					|| !strcmp(className, "class C_HL2MPRagdoll")
					|| StringBeginsWith(className, "npc_")
					|| !strcmp(className, "class C_ClientRagdoll")
					)
				;

			bool isWeapon =
				false
				|| className && (
					StringBeginsWith(className, "weapon_")
					|| !strcmp(className, "class C_BreakableProp")
					|| !strcmp(className, "gmod_tool")
					)
				;

			bool isProjectile =
				false
				|| className && (
					!strcmp(className, "grenade")
					|| !strcmp(className, "rpg_missile")
					)
				;

			bool isViewModel =
				false
				|| className && (
					!strcmp(className, "viewmodel")
					|| !strcmp(className, "gmod_hands")
					)
				;

			// Props are a huge gameplay part of Garrysmod, but we seperate them for possible future AGR updates. Thinking about mirv_agr recordProps for AGR 7.
			// Props are often used like a projectile
			bool isProp =
				className && !strcmp(className, "prop_physics")
				;
			// Vehicles are recorded as Players for now, cause the player controls them instead of the character once entered.
			bool isVehicle =
				className && !strcmp(className, "prop_vehicle")
				;

			if (false
				|| RecordPlayers_get() && (isPlayer || isVehicle)
				|| RecordWeapons_get() && isWeapon
				|| RecordProjectiles_get() && (isProjectile || isProp)
				|| RecordViewModels_get() && isViewModel
				)
			{
				SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (!RecordInvisible_get() && !(pBaseEntityRs && pBaseEntityRs->m_bVisible))
				{
					// Entity not visible, avoid trash data:

					std::map<SOURCESDK::GARRYSMOD::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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
					SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::GARRYSMOD::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
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
					SOURCESDK::GARRYSMOD::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::GARRYSMOD::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
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
		std::map<SOURCESDK::GARRYSMOD::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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

void CClientToolsGarrysmod::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

}

void CClientToolsGarrysmod::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}

void CClientToolsGarrysmod::EnableRecordingMode_set(bool value) {
	m_ClientTools->EnableRecordingMode(value);
}

bool CClientToolsGarrysmod::EnableRecordingMode_get() {
	return m_ClientTools->IsInRecordingMode();
}

void CClientToolsGarrysmod::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		if(!garrysmod_C_BaseAnimating_RecordBones_Install())
		{
			Tier0_Warning("AFX: Failed to install IClientRenderable::SetupBones hook.\n");
		}

		for (std::map<SOURCESDK::GARRYSMOD::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, true);
		}
	}
}

void CClientToolsGarrysmod::EndRecording()
{
	if (GetRecording())
	{
		for (std::map<SOURCESDK::GARRYSMOD::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.begin(); it != m_TrackedHandles.end(); ++it)
		{
			m_ClientTools->SetRecording(it->first, false);
		}
	}

	CClientTools::EndRecording();
}
