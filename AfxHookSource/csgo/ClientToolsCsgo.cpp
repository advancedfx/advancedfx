#include "stdafx.h"

#include "ClientToolsCsgo.h"

#include "csgo_net_chan.h"

#include "addresses.h"
#include "RenderView.h"
#include "WrpVEngineClient.h"

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

extern WrpVEngineClient* g_VEngineClient;

using namespace SOURCESDK::CSGO;

void Call_CBaseAnimating_GetToolRecordingState(SOURCESDK::C_BaseEntity_csgo * object, SOURCESDK::CSGO::KeyValues * msg)
{
	if (g_AfxAddr_csgo_C_BaseAnimating_vtable)
	{
		void * functionPtr = ((void **)g_AfxAddr_csgo_C_BaseAnimating_vtable)[102];

		__asm
		{
			mov ecx, msg
			push ecx
			mov ecx, object
			call functionPtr
		}
	}
}

typedef void (__fastcall * csgo_C_CSPLayer_UpdateClientSideAnimation_t)(void * This, void * Edx);

csgo_C_CSPLayer_UpdateClientSideAnimation_t csgo_TrueC_CSPLayer_UpdateClientSideAnimation = nullptr;

void __fastcall csgo_MyC_CSPLayer_UpdateClientSideAnimation(void* This, void* Edx)
{
	/*
	if (g_i_MirvPov)
	{
		SOURCESDK::C_BasePlayer_csgo* csPlayer = reinterpret_cast<SOURCESDK::C_BasePlayer_csgo*>(This);
		if (csPlayer->entindex() == g_i_MirvPov)
		{
			SOURCESDK::QAngle angles = csPlayer->EyeAngles();
			g_VEngineClient->SetViewAngles(angles);
			Tier0_Msg("%i: %f %f %f\n", csPlayer->entindex(), angles.x, angles.y, angles.z);

			unsigned char* p_m_IsLocalPlayer = (unsigned char*)This + 0x3624;
			unsigned char oldValue = *p_m_IsLocalPlayer;

			*p_m_IsLocalPlayer = 0;

			csgo_TrueC_CSPLayer_UpdateClientSideAnimation(This, Edx);

			*p_m_IsLocalPlayer = oldValue;

			angles = csPlayer->EyeAngles();
			//g_VEngineClient->SetViewAngles(angles);
			Tier0_Msg("%i: %f %f %f\n", csPlayer->entindex(), angles.x, angles.y, angles.z);

			return;
		}
	}
	*/
	if (CClientToolsCsgo::Instance() && CClientToolsCsgo::Instance()->GetRecording() && CClientToolsCsgo::Instance()->RecordViewModels_get())
	{
		// When recording viewmodels we make the C_CSPlayers think that they are the local player (so they will make their viewmodel and attachments) =)

		unsigned char* p_m_IsLocalPlayer = (unsigned char*)This + 0x3624;
		unsigned char oldValue = *p_m_IsLocalPlayer;

		*p_m_IsLocalPlayer = 1;

		csgo_TrueC_CSPLayer_UpdateClientSideAnimation(This, Edx);

		*p_m_IsLocalPlayer = oldValue;
	}
	else
	{
		csgo_TrueC_CSPLayer_UpdateClientSideAnimation(This, Edx);
	}
}


bool csgo_C_CSPlayer_UpdateClientSideAnimation_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CNetChan_ProcessMessages))
	{
		LONG error = NO_ERROR;

		csgo_TrueC_CSPLayer_UpdateClientSideAnimation = (csgo_C_CSPLayer_UpdateClientSideAnimation_t)AFXADDR_GET(csgo_C_CSPlayer_UpdateClientSideAnimation);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)csgo_TrueC_CSPLayer_UpdateClientSideAnimation, csgo_MyC_CSPLayer_UpdateClientSideAnimation);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

CClientToolsCsgo * CClientToolsCsgo::m_Instance = 0;

CClientToolsCsgo::CClientToolsCsgo(SOURCESDK::CSGO::IClientTools * clientTools)
	: CClientTools()
	, m_ClientTools(clientTools)
{
	m_Instance = this;
}

CClientToolsCsgo::~CClientToolsCsgo()
{
	m_Instance = 0;
}

void CClientToolsCsgo::OnPostToolMessage(void * hEntity, void * msg)
{
	CClientTools::OnPostToolMessage(hEntity, msg);

	OnPostToolMessageCsgo(reinterpret_cast<SOURCESDK::CSGO::HTOOLHANDLE>(hEntity), reinterpret_cast<SOURCESDK::CSGO::KeyValues *>(msg));
}

bool CClientToolsCsgo::IsViewmodel(SOURCESDK::CSGO::HTOOLHANDLE hEntity)
{
	char const * className;

	return
		hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID
		&& (className = m_ClientTools->GetClassname(hEntity))
		&& (
			!strcmp(className, "predicted_viewmodel")
			|| !strcmp(className, "class C_ViewmodelAttachmentModel")
		)
	;
}

void CClientToolsCsgo::OnPostToolMessageCsgo(SOURCESDK::CSGO::HTOOLHANDLE hEntity, SOURCESDK::CSGO::KeyValues * msg)
{
	if (!(hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID && msg))
		return;

	char const * msgName = msg->GetName();

	if (!strcmp("entity_state", msgName))
	{
		if (GetRecording())
		{
			char const * className = m_ClientTools->GetClassname(hEntity);
			if (nullptr == className) className = "[NULL]";

			if (2 <= Debug_get())
			{
				Tier0_Msg("-- %s (%i) --\n", className, hEntity);
				for (SOURCESDK::CSGO::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
					Tier0_Msg("%s,\n", subKey->GetName());
				Tier0_Msg("----\n");
			}

			SOURCESDK::CSGO::EntitySearchResult ent = m_ClientTools->GetEntity(hEntity);

			SOURCESDK::C_BaseEntity_csgo * be = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(ent);
			SOURCESDK::IClientEntity_csgo * ce = be ? be->GetIClientEntity() : 0;

			bool isPlayer =
				m_ClientTools->IsPlayer(ent)
				|| m_ClientTools->IsRagdoll(ent)
				|| className && !strcmp(className, "class C_CSRagdoll")
				;

			bool isWeaponWorldModel = className && 0 == strcmp(className, "weaponworldmodel");

			bool isWeapon =
				m_ClientTools->IsWeapon(ent)
				|| className && (
					isWeaponWorldModel
					|| !strcmp(className, "class C_PlayerAddonModel")
					|| !strcmp(className, "class C_PlantedC4")
					)
				;

			bool isProjectile =
				className && StringEndsWith(className, "Projectile")
				;

			bool isPredictedViewmodel = 0 == strcmp(className, "predicted_viewmodel");
			bool isViewmodelAttachment = 0 == strcmp(className, "class C_ViewmodelAttachmentModel");

			bool isViewModel = className && (isPredictedViewmodel || isViewmodelAttachment);

			bool isRecordedViewModel = false;
			if (isViewModel && RecordViewModels_get())
			{
				if (-1 == RecordViewModels_get())
					isRecordedViewModel = true;
				else
				{
					int weaponIdx;

					if (isViewmodelAttachment && SOURCESDK::g_Entitylist_csgo && be)
					{
						CBaseHandle viewModelEntBaseHandle = be->AfxGetMoveParentHandle();
						weaponIdx = m_ClientTools->GetOwningWeaponEntIndex(viewModelEntBaseHandle.GetEntryIndex());
					}
					else
						weaponIdx = m_ClientTools->GetOwningWeaponEntIndex(m_ClientTools->GetEntIndex(ent));

					HTOOLHANDLE weaponHandle = m_ClientTools->GetToolHandleForEntityByIndex(weaponIdx);
					if(EntitySearchResult weaponEnt = m_ClientTools->GetEntity(weaponHandle))
					{
						if (EntitySearchResult ownerEnt = m_ClientTools->GetOwnerEntity(weaponEnt))
						{
							isRecordedViewModel = RecordViewModels_get() == m_ClientTools->GetEntIndex(ownerEnt);
						}
					}
				}
			}

			if (ce
				&& (
					RecordPlayers_get() && isPlayer
					|| RecordWeapons_get() && isWeapon
					|| RecordProjectiles_get() && isProjectile
					|| isRecordedViewModel
					)
				)
			{
				// This code is only relevant for weapon entities that can have multiple models depending on their state:
				if (m_ClientTools->IsWeapon(ent))
				{
					if (be && -1 != AFXADDR_GET(csgo_C_BaseCombatWeapon_m_iState))
					{

						SOURCESDK::C_BaseCombatWeapon_csgo * weapon = reinterpret_cast<SOURCESDK::C_BaseCombatWeapon_csgo *>(ent);
						int state = *(int *)((char const *)weapon + AFXADDR_GET(csgo_C_BaseCombatWeapon_m_iState));

						if (SOURCESDK_CSGO_WEAPON_NOT_CARRIED != state)
						{
							// Weapon not on ground, mark as invisible (if visible) and then ignore:

							std::map<SOURCESDK::CSGO::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
							if (it != m_TrackedHandles.end() && it->second)
							{
								MarkHidden((int)(it->first));

								it->second = false;
							}

							return;
						}
					}
					else
						return;
				}

				if (!msg->GetPtr("baseentity") && isWeapon)
				{
					// Fix up broken overloaded C_BaseCombatWeapon code as good as we can:

					/*
					int* p_m_nModelIndex = -1 != AFXADDR_GET(csgo_C_BaseEntity_ofs_m_nModelIndex) ? (int *)((char *)be + AFXADDR_GET(csgo_C_BaseEntity_ofs_m_nModelIndex)): nullptr;
					int* p_m_iWorldModelIndex = -1 != AFXADDR_GET(csgo_C_BaseEntity_ofs_m_iWorldModelIndex) ? (int*)((char*)be + AFXADDR_GET(csgo_C_BaseEntity_ofs_m_iWorldModelIndex)) : nullptr;
					bool hasModelIndex = p_m_nModelIndex && p_m_iWorldModelIndex;
					bool modelIndexChanged = false;
					int oldModelIndex;

					if (hasModelIndex)
					{
						oldModelIndex = *p_m_nModelIndex;
						if (*p_m_iWorldModelIndex != oldModelIndex)
						{
							modelIndexChanged = true;
							*p_m_nModelIndex = *p_m_iWorldModelIndex;
							be->ValidateModelIndex();
						}
					}
					*/

					Call_CBaseAnimating_GetToolRecordingState(be, msg);

					// Btw.: We don't need to call CBaseAnimating::CleanupToolRecordingState afterwards, because that code is still fully functional / function not overloaded.

					/*
					if (modelIndexChanged)
					{
						*p_m_nModelIndex = oldModelIndex;
						be->ValidateModelIndex();
					}
					*/
				}

				SOURCESDK::CSGO::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSGO::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));

				if (pBaseEntityRs && !(pBaseEntityRs->m_bVisible || IsViewmodel(hEntity)) && (
					!RecordInvisible_get()
					|| isWeaponWorldModel && be && be->AfxGetMoveParentHandle() == SOURCESDK_CSGO_INVALID_EHANDLE_INDEX
				)) {	
					std::map<SOURCESDK::CSGO::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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
					SOURCESDK::CSGO::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSGO::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
					if (pBaseEntityRs)
					{
						wasVisible = pBaseEntityRs->m_bVisible || IsViewmodel(hEntity);

						WriteDictionary("baseentity");
						//Write((float)pBaseEntityRs->m_flTime);
						WriteDictionary(pBaseEntityRs->m_pModelName);
						Write((bool)wasVisible);
						Write(pBaseEntityRs->m_vecRenderOrigin);
						Write(pBaseEntityRs->m_vecRenderAngles);
					}
				}

				m_TrackedHandles[hEntity] = wasVisible;

				{
					SOURCESDK::CSGO::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::CSGO::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
					if (pBaseAnimatingRs)
					{
						WriteDictionary("baseanimating");
						//Write((int)pBaseAnimatingRs->m_nSkin);
						//Write((int)pBaseAnimatingRs->m_nBody);
						//Write((int)pBaseAnimatingRs->m_nSequence);
						Write((bool)(0 != pBaseAnimatingRs->m_pBoneList));
						if (pBaseAnimatingRs->m_pBoneList)
						{
							bool hasError = false;
							Write(pBaseAnimatingRs->m_pBoneList);
						}
					}
				}

				if(RecordPlayerCameras_get() != 0 && (RecordPlayerCameras_get() == -1 || be && RecordPlayerCameras_get() == be->entindex()))
				{
					struct SOURCESDK::CSGO::CameraRecordingState_t* pCameraRecordingState = (struct SOURCESDK::CSGO::CameraRecordingState_t*)(msg->GetPtr("camera"));
					if (pCameraRecordingState)
					{
						WriteDictionary("camera");
						Write((bool)pCameraRecordingState->m_bThirdPerson);
						Write(pCameraRecordingState->m_vecEyePosition);
						Write(pCameraRecordingState->m_vecEyeAngles);
						Write((float)ScaleFov(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, (float)pCameraRecordingState->m_flFOV));
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
		std::map<SOURCESDK::CSGO::HTOOLHANDLE, bool>::iterator it = m_TrackedHandles.find(hEntity);
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
			SOURCESDK::CSGO::EntitySearchResult ent = m_ClientTools->GetEntity(hEntity);
			SOURCESDK::C_BaseEntity_csgo * be = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(ent);

			const char * className = m_ClientTools->GetClassname(hEntity);
			if (!className) className = "";

			Tier0_Msg("%i (%i): %s\n", hEntity, be ? be->GetRefEHandle().ToInt() : SOURCESDK_CSGO_INVALID_EHANDLE_INDEX, className);
		}

		if (hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID && m_ClientTools->ShouldRecord(hEntity))
		{
			m_ClientTools->SetRecording(hEntity, true);
		}
	}
}

void CClientToolsCsgo::OnBeforeFrameRenderStart(void)
{
	CClientTools::OnBeforeFrameRenderStart();

	if (GetRecording() && RecordViewModels_get())
	{
		int numRecordAbles = m_ClientTools->GetNumRecordables();
		for (int i = 0; i < numRecordAbles; ++i)
		{
			HTOOLHANDLE hEntity = m_ClientTools->GetRecordable(i);

			if (m_ClientTools->ShouldRecord(hEntity))
			{
				EntitySearchResult ent = m_ClientTools->GetEntity(hEntity);

				if (m_ClientTools->IsPlayer(ent))
				{
					SOURCESDK::C_BasePlayer_csgo* player = reinterpret_cast<SOURCESDK::C_BasePlayer_csgo*>(ent);

					if (-1 == RecordViewModels_get() || player->entindex() == RecordViewModels_get())
					{
						SOURCESDK::Vector eyeOrigin;
						SOURCESDK::QAngle eyeAngles;
						float fov;
						float zNear = 0;
						float zFar = 1;

						player->CalcView(eyeOrigin, eyeAngles, zNear, zFar, fov);
						player->CalcViewModelView(eyeOrigin, eyeAngles);
					}
				}
			}
		}
	}
}


void CClientToolsCsgo::OnAfterFrameRenderEnd(void)
{

	CClientTools::OnAfterFrameRenderEnd();
}


void CClientToolsCsgo::StartRecording(wchar_t const * fileName)
{
	CClientTools::StartRecording(fileName);

	if (GetRecording())
	{
		m_ClientTools->EnableRecordingMode(true);

		if (RecordViewModels_get())
		{
			if (nullptr == csgo_TrueC_CSPLayer_UpdateClientSideAnimation)
			{
				if (!csgo_C_CSPlayer_UpdateClientSideAnimation_Install())
				{
					Tier0_Warning("AFX: Missing address for C_CSPlayer::UpdateClientSideAnimation.\n");
				}
			}
		}
	}
}

void CClientToolsCsgo::EndRecording()
{
	if (GetRecording())
	{
		m_ClientTools->EnableRecordingMode(false);
	}

	CClientTools::EndRecording();
}

float CClientToolsCsgo::ScaleFov(int width, int height, float fov)
{
	return (float)AlienSwarm_FovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, g_Hook_VClient_RenderView.LastCameraFov);
}

void CClientToolsCsgo::Write(SOURCESDK::CSGO::CBoneList const * value)
{
	Write((int)value->m_nBones);

	for (int i = 0; i < value->m_nBones; ++i)
	{
		if (0 != Debug_get() && (
			std::isnan(value->m_vecPos[i].x) || std::isnan(value->m_vecPos[i].y) || std::isnan(value->m_vecPos[i].z)
			|| std::isnan(value->m_quatRot[i].w) || std::isnan(value->m_quatRot[i].x) || std::isnan(value->m_quatRot[i].y) || std::isnan(value->m_quatRot[i].z)
		)) {
			// This error condition happens with bugged weaponworldmodels that have no movementowner a lot.

			Tier0_Warning("ClientToolsCsgo::Write(SOURCESDK::CSGO::CBoneList const * value): %i: (%f, %f, %f) (%f, %f, %f, %f)\n"
				, i
				, value->m_vecPos[i].x
				, value->m_vecPos[i].y
				, value->m_vecPos[i].z
				, value->m_quatRot[i].w
				, value->m_quatRot[i].x
				, value->m_quatRot[i].y
				, value->m_quatRot[i].z
			);
		}

		Write(value->m_vecPos[i]);
		Write(value->m_quatRot[i]);
	}
}

void CClientToolsCsgo::DebugEntIndex(int index)
{
	if (!m_ClientTools)
		return;

	SOURCESDK::CSGO::HTOOLHANDLE hHandle = m_ClientTools->GetToolHandleForEntityByIndex(index);

	if (SOURCESDK::CSGO::HTOOLHANDLE_INVALID == hHandle)
	{
		Tier0_Msg("Invalid tool handle\n");
		return;
	}

	SOURCESDK::CSGO::EntitySearchResult sResult = m_ClientTools->GetEntity(hHandle);

	if (!sResult)
	{
		Tier0_Msg("Invalid search result\n");
		return;
	}

	Tier0_Msg(
		"ShouldRecord: %i\n"
		"IsPlayer: %i\n"
		"IsCombatCharacter: %i\n"
		"IsNPC: %i\n"
		"IsRagdoll: %i\n"
		"IsViewModel: %i\n"
		"IsViewModelOrAttachment: %i\n"
		"IsWeapon: %i\n"
		"IsSprite: %i\n"
		"IsProp: %i\n"
		"IsBrush: %i\n"
		, m_ClientTools->ShouldRecord(hHandle) ? 1 : 0
		, m_ClientTools->IsPlayer(sResult) ? 1 : 0
		, m_ClientTools->IsCombatCharacter(sResult) ? 1 : 0
		, m_ClientTools->IsNPC(sResult) ? 1 : 0
		, m_ClientTools->IsRagdoll(sResult) ? 1 : 0
		, m_ClientTools->IsViewModel(sResult) ? 1 : 0
		, m_ClientTools->IsViewModelOrAttachment(sResult) ? 1 : 0
		, m_ClientTools->IsWeapon(sResult) ? 1 : 0
		, m_ClientTools->IsSprite(sResult) ? 1 : 0
		, m_ClientTools->IsProp(sResult) ? 1 : 0
		, m_ClientTools->IsBrush(sResult) ? 1 : 0
	);

	SOURCESDK::Vector vec = m_ClientTools->GetAbsOrigin(hHandle);

	Tier0_Msg("GetAbsOrigin: %f %f %f\n", vec.x, vec.y, vec.z);

	SOURCESDK::QAngle ang = m_ClientTools->GetAbsAngles(hHandle);

	Tier0_Msg("GetAbsAngles: %f %f %f\n", ang.x, ang.y, ang.z);
}
