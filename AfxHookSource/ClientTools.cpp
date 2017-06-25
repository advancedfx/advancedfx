#include "stdafx.h"

#include "ClientTools.h"

#include "addresses.h"
#include "RenderView.h"
#include "CamIO.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <iostream>
#include <fstream>

using namespace SOURCESDK::CSGO;

ClientTools g_ClientTools;


typedef void (*C_BaseEntity_ToolRecordEntities_t)(void);

C_BaseEntity_ToolRecordEntities_t detoured_C_BaseEntity_ToolRecordEntities;

void touring_C_BaseEntity_ToolRecordEntities(void)
{
	g_ClientTools.OnC_BaseEntity_ToolRecordEntities();

	detoured_C_BaseEntity_ToolRecordEntities();
}

void Call_CBaseAnimating_GetToolRecordingState(SOURCESDK::C_BaseEntity_csgo * object, SOURCESDK::CSGO::KeyValues * msg)
{
	if (g_AfxAddr_csgo_C_BaseAnimating_vtable)
	{
		void * functionPtr = ((void **)g_AfxAddr_csgo_C_BaseAnimating_vtable)[102];

		__asm
		{
			mov ecx, msg
			push msg
			mov ecx, object
			call functionPtr
		}
	}
}

bool Hook_C_BaseEntity_ToolRecordEnties(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_BaseEntity_ToolRecordEnties))
	{
		detoured_C_BaseEntity_ToolRecordEntities = (C_BaseEntity_ToolRecordEntities_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_C_BaseEntity_ToolRecordEnties), (BYTE *)touring_C_BaseEntity_ToolRecordEntities, (int)AFXADDR_GET(csgo_C_BaseEntity_ToolRecordEnties_DSZ));
		firstResult = true;
	}

	return firstResult;
}

ClientTools::ClientTools()
: m_Recording(false)
, m_ClientTools(0)
{
}

void ClientTools::SetClientTools(SOURCESDK::CSGO::IClientTools * clientTools)
{
	m_ClientTools = clientTools;
}

/*
char * int2xml(rapidxml::xml_document<> & doc, int value)
{
	return doc.allocate_string(std::to_string(value).c_str());
}

char * bool2xml(rapidxml::xml_document<> & doc, bool value)
{
	return int2xml(doc, value ? (int)1 : 0);
}*/

void ClientTools::OnPostToolMessage(SOURCESDK::CSGO::HTOOLHANDLE hEntity, SOURCESDK::CSGO::KeyValues * msg)
{
	if (!(m_Recording && m_File))
		return;

	if (!(m_ClientTools && (hEntity != SOURCESDK::CSGO::HTOOLHANDLE_INVALID)  && msg))
		return;

	char const * msgName = msg->GetName();

	if (!strcmp("entity_state", msgName))
	{
		SOURCESDK::CSGO::EntitySearchResult ent = m_ClientTools->GetEntity(hEntity);

		SOURCESDK::C_BaseEntity_csgo * be = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(ent);
		SOURCESDK::IClientEntity_csgo * ce = be ? be->GetIClientEntity() : 0;

		char const * className = be ? be->GetClassname() : 0;

		if (ce
			&& (
				m_RecordPlayers && (
					m_ClientTools->IsPlayer(ent)
					|| m_ClientTools->IsRagdoll(ent)
					|| className && !strcmp(className, "class C_CSRagdoll")
				)
				|| m_RecordWeapons && (
					m_ClientTools->IsWeapon(ent)
					|| className && !strcmp(className ,"weaponworldmodel")
				)
			)
		)
		{
			if (m_ClientTools->IsWeapon(ent))
			{
				if (be && -1 != AFXADDR_GET(csgo_C_BaseCombatWeapon_m_iState))
				{

					SOURCESDK::C_BaseCombatWeapon_csgo * weapon = reinterpret_cast<SOURCESDK::C_BaseCombatWeapon_csgo *>(ent);
					int state = *(int *)((char const *)weapon + AFXADDR_GET(csgo_C_BaseCombatWeapon_m_iState));

					if (SOURCESDK_CSGO_WEAPON_NOT_CARRIED != state)
					{
						// TODO: Weapon not on ground, delete if was present, so it can be marked as invisible.

						std::map<SOURCESDK::CSGO::HTOOLHANDLE, int>::iterator it = m_TrackedHandles.find(hEntity);
						if (it != m_TrackedHandles.end())
						{
							WriteDictionary("deleted");
							Write((int)(it->second));
							Write((float)g_Hook_VClient_RenderView.GetGlobals()->curtime_get());

							m_TrackedHandles.erase(it);
						}

						return;
					}
				}
				else
					return;
			}

			int handle = ce->GetRefEHandle().ToInt();

			m_TrackedHandles[hEntity] = handle;

			if (!msg->GetPtr("baseentity") && m_ClientTools->IsWeapon(ent))
			{
				// Fix up broken overloaded C_BaseCombatWeapon code as good as we can:
				Call_CBaseAnimating_GetToolRecordingState(be, msg);

				// Btw.: We don't need to call CBaseAnimating::CleanupToolRecordingState afterwards, because that code is still fully functional / function not overloaded.
			}
			/*
			{
				Tier0_Msg("-- %s @0x%08x--\n", className, be);
				for (SOURCESDK::CSGO::KeyValues * subKey = msg->GetFirstSubKey(); 0 != subKey; subKey = subKey->GetNextKey())
				Tier0_Msg("%s,\n", subKey->GetName());

				Tier0_Msg("----\n");
			}
			*/

			WriteDictionary("entity_state");
			Write((int)handle);
			{
				SOURCESDK::CSGO::BaseEntityRecordingState_t * pBaseEntityRs = (SOURCESDK::CSGO::BaseEntityRecordingState_t *)(msg->GetPtr("baseentity"));
				if (pBaseEntityRs)
				{
					WriteDictionary("baseentity");
					Write((float)pBaseEntityRs->m_flTime);
					WriteDictionary(pBaseEntityRs->m_pModelName);
					Write((bool)pBaseEntityRs->m_bVisible);
					Write(pBaseEntityRs->m_vecRenderOrigin);
					Write(pBaseEntityRs->m_vecRenderAngles);
				}
			}

			{
				SOURCESDK::CSGO::BaseAnimatingRecordingState_t * pBaseAnimatingRs = (SOURCESDK::CSGO::BaseAnimatingRecordingState_t *)(msg->GetPtr("baseanimating"));
				if (pBaseAnimatingRs)
				{
					WriteDictionary("baseanimating");
					Write((int)pBaseAnimatingRs->m_nSkin);
					Write((int)pBaseAnimatingRs->m_nBody);
					Write((int)pBaseAnimatingRs->m_nSequence);
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
	else
	if (!strcmp("deleted", msgName))
	{
		// We cannot read the handle from the entity, because by this time it has been invalidated already.
		// so we'll use the cachend handle instead:

		std::map<SOURCESDK::CSGO::HTOOLHANDLE, int>::iterator it = m_TrackedHandles.find(hEntity);
		if(it != m_TrackedHandles.end())
		{
			WriteDictionary("deleted");
			Write((int)(it->second));
			Write((float)g_Hook_VClient_RenderView.GetGlobals()->curtime_get());

			m_TrackedHandles.erase(it);
		}
	}
}

void ClientTools::OnC_BaseEntity_ToolRecordEntities(void)
{
	UpdateRecording();

	if(m_RecordCamrea)
	{
		WriteDictionary("afxCam");
		Write((float)g_Hook_VClient_RenderView.GetGlobals()->curtime_get());
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[0]);
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[1]);
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[0]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[1]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[2]);
		Write((float)AlienSwarm_FovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, g_Hook_VClient_RenderView.LastCameraFov));
	}
}

bool ClientTools::GetRecording(void)
{
	return m_Recording;
}

void ClientTools::StartRecording(wchar_t const * fileName)
{
	EndRecording();

	m_Recording = true;

	Dictionary_Clear();
	m_File = 0;

	_wfopen_s(&m_File, fileName, L"wb");

	if (m_File)
	{
		fputs("afxGameRecord", m_File);
		fputc('\0', m_File);
		int version = 1;
		fwrite(&version, sizeof(version), 1, m_File);
	}
	else
		Tier0_Warning("ERROR opening file \"%s\" for writing.\n", fileName);

	if (m_ClientTools)
	{
		m_ClientTools->EnableRecordingMode(true);
	}
	else
	{
		Tier0_Warning("ERROR: Missing ClientTools dependency.\n");
	}

	UpdateRecording();
}

void ClientTools::EndRecording()
{
	if (!m_Recording)
		return;

	if (m_ClientTools)
	{
		m_ClientTools->EnableRecordingMode(false);
	}

	if (m_File)
	{
		fclose(m_File);
	}

	Dictionary_Clear();

	m_Recording = false;
}

void ClientTools::UpdateRecording()
{
	if (!m_Recording)
		return;

	if (!m_ClientTools)
		return;

	for(EntitySearchResult ent = m_ClientTools->FirstEntity(); 0 != ent; ent = m_ClientTools->NextEntity(ent))
	{
		SOURCESDK::CSGO::HTOOLHANDLE hEnt = m_ClientTools->AttachToEntity(ent);

		if (hEnt != SOURCESDK::CSGO::HTOOLHANDLE_INVALID && m_ClientTools->ShouldRecord(hEnt))
		{
			m_ClientTools->SetRecording(hEnt, true);
		}

		// never detach, the ToolsSystem does that already when the entity is removed:
		// m_ClientTools->DetachFromEntity(ent);
	}
}

void ClientTools::WriteDictionary(char const * value)
{
	int idx = Dictionary_Get(value);

	Write(idx);

	if (-1 == idx)
	{
		Write(value);
	}
}

void ClientTools::Write(bool value)
{
	fwrite(&value, sizeof(value), 1, m_File);
}

void ClientTools::Write(int value)
{
	fwrite(&value, sizeof(value), 1, m_File);
}

void ClientTools::Write(float value)
{
	fwrite(&value, sizeof(value), 1, m_File);
}

void ClientTools::Write(double value)
{
	fwrite(&value, sizeof(value), 1, m_File);
}

void ClientTools::Write(char const * value)
{
	fputs(value, m_File);
	fputc('\0', m_File);
}

void ClientTools::Write(SOURCESDK::Vector const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
}

void ClientTools::Write(SOURCESDK::QAngle const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
}

void ClientTools::Write	(SOURCESDK::CSGO::CBoneList const * value)
{
	Write((int)value->m_nBones);

	for (int i = 0; i < value->m_nBones; ++i)
	{
		Write(value->m_vecPos[i]);
		Write(value->m_quatRot[i]);
	}
}

void ClientTools::Write(SOURCESDK::Quaternion const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
	Write((float)value.w);
}

void ClientTools::DebugEntIndex(int index)
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
