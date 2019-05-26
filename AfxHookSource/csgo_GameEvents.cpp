#include "stdafx.h"

#include "csgo_GameEvents.h"

#include "addresses.h"
#include "WrpVEngineClient.h"
#include "WrpGlobals.h"
#include "RenderView.h"
#include "csgo/ClientToolsCsgo.h"

#include <Windows.h>
#include <shared/Detours/src/detours.h>

#include <ctime>

extern WrpVEngineClient * g_VEngineClient;

typedef bool(__fastcall * csgo_CGameEventManager_FireEventIntern_t)(void * This, void * edx, SOURCESDK::CSGO::IGameEvent *event, bool bServerOnly, bool bClientOnly);
typedef void * (__cdecl * csgo_dynamic_cast_t)(void * from, void * unk1, void * baseClassRtti, void * classRtti, void * unk2);

csgo_CGameEventManager_FireEventIntern_t True_csgo_CGameEventManager_FireEventIntern;
void * g_csgo_RTTI_IGameEvent;
void * g_csgo_RTTI_CGameEvent;
csgo_dynamic_cast_t csgo_dynamic_cast;


enum CsgoGameEventKeyType
{
	CsgoGameEventKeyType_Local = 0,
	CsgoGameEventKeyType_CString = 1,
	CsgoGameEventKeyType_Float = 2,
	CsgoGameEventKeyType_Long = 3,
	CsgoGameEventKeyType_Short = 4,
	CsgoGameEventKeyType_Byte = 5,
	CsgoGameEventKeyType_Bool = 6,
	CsgoGameEventKeyType_Uint64 = 7
};

void CAfxGameEventListenerSerialzer::FireHandledEvent(SOURCESDK::CSGO::CGameEvent * gameEvent)
{
	if (!BeginSerialize()) return;

	if (SOURCESDK::CSGO::CGameEventDescriptor * descriptor = gameEvent->m_pDescriptor)
	{
		int eventId = descriptor->eventid;

		if (m_KnownEventIds.end() != m_KnownEventIds.find(eventId))
		{
			WriteLong(eventId);
		}
		else
		{
			WriteLong(0);
			WriteLong(eventId);
			WriteCString(gameEvent->GetName());

			if (descriptor->keys)
			{
				if (SOURCESDK::CSGO::KeyValues *key = descriptor->keys->GetFirstSubKey())
				{
					while (key)
					{
						int type = key->GetInt();

						switch (type)
						{
						case CsgoGameEventKeyType_CString:
						case CsgoGameEventKeyType_Float:
						case CsgoGameEventKeyType_Long:
						case CsgoGameEventKeyType_Short:
						case CsgoGameEventKeyType_Byte:
						case CsgoGameEventKeyType_Bool:
						case CsgoGameEventKeyType_Uint64:
							{
								WriteBoolean(true);
								WriteCString(key->GetName());
								WriteLong(type);
							}
							break;
						default:
							break;
						}

						key = key->GetNextKey();
					}
				}
			}

			WriteBoolean(false);
		}

		if (TransmitClientTime)
		{
			float curTime = 0;

			if (WrpGlobals * globals = g_Hook_VClient_RenderView.GetGlobals())
			{
				curTime = globals->curtime_get();
			}

			WriteFloat(curTime);
		}
		if (TransmitTick)
		{
			int tick = 0;

			if (WrpVEngineClientDemoInfoEx * demoInfo = g_VEngineClient->GetDemoInfoEx())
			{
				tick = demoInfo->GetDemoPlaybackTick();
			}

			WriteLong(tick);
		}
		if (TransmitSystemTime)
		{
			std::time_t result = std::time(nullptr);

			WriteUInt64((unsigned __int64)result);
		}

		if (descriptor->keys)
		{
			if (SOURCESDK::CSGO::KeyValues *key = descriptor->keys->GetFirstSubKey())
			{
				const char * eventName = gameEvent->GetName();

				while (key)
				{
					const char * keyName = key->GetName();
					int type = key->GetInt();

					switch (type)
					{
					case CsgoGameEventKeyType_CString:
						WriteCString(gameEvent->GetString(keyName));
						break;
					case CsgoGameEventKeyType_Float:
						WriteFloat(gameEvent->GetFloat(keyName));
						break;
					case CsgoGameEventKeyType_Long:
						WriteLong(gameEvent->GetInt(keyName));
						break;
					case CsgoGameEventKeyType_Short:
						WriteShort(gameEvent->GetInt(keyName));
						break;
					case CsgoGameEventKeyType_Byte:
						WriteByte(gameEvent->GetInt(keyName));
						break;
					case CsgoGameEventKeyType_Bool:
						WriteBoolean(gameEvent->GetBool(keyName));
						break;
					case CsgoGameEventKeyType_Uint64:
						WriteUInt64(gameEvent->GetUint64(keyName));
						break;
						break;
					default:
						break;
					}

					auto it1 = m_Enrichments.find(eventName);
					if (it1 != m_Enrichments.end())
					{
						auto it2 = it1->second.find(keyName);
						if (it2 != it1->second.end())
						{
							DWORD enrichmentType = it2->second.Type;

							if (enrichmentType & CEnrichment::Type_UseridWithSteamId)
							{
								int userId = gameEvent->GetInt(keyName);
								unsigned __int64 xuid = 0;

								if (g_VEngineClient && CClientToolsCsgo::Instance())
								{
									if (SOURCESDK::IVEngineClient_014_csgo * pEngineCsgo = g_VEngineClient->GetVEngineClient_csgo())
									{
										int entnum = pEngineCsgo->GetPlayerForUserID(userId);

										if(SOURCESDK::g_Entitylist_csgo)
										{
											if (SOURCESDK::IClientNetworkable_csgo * networkable = SOURCESDK::g_Entitylist_csgo->GetClientNetworkable(entnum))
											{
												SOURCESDK::player_info_t_csgo pInfo;

												if (pEngineCsgo->GetPlayerInfo(networkable->entindex(), &pInfo))
												{
													xuid = pInfo.xuid;
												}
											}
										}
									}
								}

								WriteUInt64(xuid);
							}
							if (enrichmentType & CEnrichment::Type_EntnumWithOrigin)
							{
								int entnum = gameEvent->GetInt(keyName);
								SOURCESDK::Vector value;
								value.x = 0;
								value.y = 0;
								value.z = 0;
								if (SOURCESDK::g_Entitylist_csgo)
								{
									if (SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(entnum))
									{
										if (SOURCESDK::C_BaseEntity_csgo * be = ce->GetBaseEntity())
										{
											value = be->GetAbsOrigin();
										}
									}
								}

								WriteFloat(value.x);
								WriteFloat(value.y);
								WriteFloat(value.z);
							}
							if (enrichmentType & CEnrichment::Type_EntnumWithAngles)
							{
								int entnum = gameEvent->GetInt(keyName);
								SOURCESDK::QAngle value;
								value.x = 0;
								value.y = 0;
								value.z = 0;

								if (SOURCESDK::g_Entitylist_csgo)
								{
									if (SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(entnum))
									{
										if (SOURCESDK::C_BaseEntity_csgo * be = ce->GetBaseEntity())
										{
											value = be->GetAbsAngles();
										}
									}
								}

								WriteFloat(value.x);
								WriteFloat(value.y);
								WriteFloat(value.z);
							}
							if (enrichmentType & CEnrichment::Type_UseridWithEyePosition)
							{
								int userid = gameEvent->GetInt(keyName);
								SOURCESDK::Vector value;
								value.x = 0;
								value.y = 0;
								value.z = 0;

								if (g_VEngineClient && CClientToolsCsgo::Instance())
								{
									if (SOURCESDK::IVEngineClient_014_csgo * pEngineCsgo = g_VEngineClient->GetVEngineClient_csgo())
									{
										int entnum = pEngineCsgo->GetPlayerForUserID(userid);

										if (SOURCESDK::g_Entitylist_csgo)
										{
											if (SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(entnum))
											{
												if (SOURCESDK::C_BaseEntity_csgo * be = ce->GetBaseEntity())
												{
													value = be->EyePosition();
												}
											}
										}
									}
								}

								WriteFloat(value.x);
								WriteFloat(value.y);
								WriteFloat(value.z);
							}
							if (enrichmentType & CEnrichment::Type_UseridWithEyeAngels)
							{
								int userid = gameEvent->GetInt(keyName);
								SOURCESDK::QAngle value;
								value.x = 0;
								value.y = 0;
								value.z = 0;

								if (g_VEngineClient && CClientToolsCsgo::Instance())
								{
									if (SOURCESDK::IVEngineClient_014_csgo * pEngineCsgo = g_VEngineClient->GetVEngineClient_csgo())
									{
										int entnum = pEngineCsgo->GetPlayerForUserID(userid);

										if (SOURCESDK::g_Entitylist_csgo)
										{
											if (SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(entnum))
											{
												if (SOURCESDK::C_BaseEntity_csgo * be = ce->GetBaseEntity())
												{
													value = be->EyeAngles();
												}
											}
										}
									}
								}

								WriteFloat(value.x);
								WriteFloat(value.y);
								WriteFloat(value.z);
							}
						}
					}

					key = key->GetNextKey();
				}
			}
		}
	}

	EndSerialize();
}

bool Hook_csgo_GameEvents(void);

void CAfxGameEvents::FireEvent(SOURCESDK::CSGO::CGameEvent * gameEvent)
{
	for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
	{
		(*it)->FireEvent(gameEvent);
	}
}

void CAfxGameEvents::AddListener(IAfxGameEventListener * listener)
{
	if (!Hook_csgo_GameEvents())
	{
		Tier0_Warning("AFXERROR: Hook_csgo_GameEvents failed.\n");
	}

	if (m_Listeners.end() == m_Listeners.find(listener))
		m_Listeners.emplace(listener);
}

CAfxGameEvents g_AfxGameEvents;

bool __fastcall My_csgo_CGameEventManager_FireEventIntern(void * This, void * edx, SOURCESDK::CSGO::IGameEvent *event, bool bServerOnly, bool bClientOnly)
{
	if (event && bClientOnly)
	{
		SOURCESDK::CSGO::CGameEvent * gameEvent = (SOURCESDK::CSGO::CGameEvent *)(csgo_dynamic_cast(event, 0, g_csgo_RTTI_IGameEvent, g_csgo_RTTI_CGameEvent, 0));
		g_AfxGameEvents.FireEvent(gameEvent);
	}

	return True_csgo_CGameEventManager_FireEventIntern(This, edx, event, bServerOnly, bClientOnly);
}

bool Hook_csgo_GameEvents(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CGameEventManger_FireEventIntern)
		&& AFXADDR_GET(csgo_dynamic_cast)
		&& AFXADDR_GET(csgo_RTTI_CGameEvent)
		&& AFXADDR_GET(csgo_RTTI_IGameEvent))
	{
		LONG error = NO_ERROR;

		csgo_dynamic_cast = (csgo_dynamic_cast_t)AFXADDR_GET(csgo_dynamic_cast);
		g_csgo_RTTI_IGameEvent = (void *)AFXADDR_GET(csgo_RTTI_IGameEvent);
		g_csgo_RTTI_CGameEvent = (void *)AFXADDR_GET(csgo_RTTI_CGameEvent);
		True_csgo_CGameEventManager_FireEventIntern = (csgo_CGameEventManager_FireEventIntern_t)AFXADDR_GET(csgo_CGameEventManger_FireEventIntern);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_csgo_CGameEventManager_FireEventIntern, My_csgo_CGameEventManager_FireEventIntern);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

