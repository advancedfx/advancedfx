#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "RenderView.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <list>
#include <sstream>

/*

sub_1011CD40

void CHudBaseDeathNotice::FireGameEvent_UnkDoNotice( IGameEvent *event )

event->GetInt("attacker",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetInt("userid",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo
event->GetInt("assister",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetString("weapon",0)
event->GetInt("headshot",0)
event->GetInt("penetrated",0)
event->GetInt("dominated",0)
event->GetInt("revenge",0)

- SpawnTime
- Lifetime
- LifetimeMod
".?AVCUIPanel@panorama@@" (2nd ref) [282] UnkSetFloatProp(word propId, float value)


*/

extern WrpVEngineClient * g_VEngineClient;

typedef void csgo_CHudBaseDeathNotice_t;

typedef void (__stdcall *csgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice_t)(csgo_CHudBaseDeathNotice_t * This, SOURCESDK::CSGO::IGameEvent * event);

csgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice_t Realcsgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice = 0;

csgo_CHudBaseDeathNotice_t * csgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice_This = 0;

enum DeathMsgIdMatchMode
{
	DMBM_EQUAL,
	DMBM_EXCEPT,
	DMBM_ANY
};

struct DeathMsgId
{
	union {
		int userId;
		unsigned long long xuid;
	} Id;
	enum {
		Id_UserId,
		Id_Xuid
	} Mode;

	DeathMsgId()
	{
		Mode = Id_UserId;
		Id.userId = 0;
	}

	DeathMsgId(int userId)
	{
		Mode = Id_UserId;
		Id.userId = userId;

	}
	DeathMsgId(unsigned long long xuid)
	{
		Mode = Id_Xuid;
		Id.xuid = xuid;
	}

	void operator =(const int userId)
	{
		Mode = Id_UserId;
		Id.userId = userId;
	}

	void operator =(const unsigned long long xuid)
	{
		Mode = Id_Xuid;
		Id.xuid = xuid;
	}

	void operator =(char const * consoleValue)
	{
		if (!consoleValue)
			return;

		if (StringBeginsWith(consoleValue, "x"))
		{
			unsigned long long val = strtoull(consoleValue + 1, 0, 10);
			this->operator=(val);
		}
		else
		{
			this->operator=((int)atoi(consoleValue));
		}
	}

	void Console_Print()
	{
		if (Mode == Id_Xuid)
		{
			std::ostringstream oss;

			oss << Id.xuid;

			Tier0_Msg("x%s", oss.str().c_str());
		}
		else
		{
			Tier0_Msg("%i", Id.userId);
		}
	}

	bool EqualsUserId(int userId)
	{
		if (Mode == Id_UserId) return userId == Id.userId;

		if (userId < 1)
			return false;

		if (g_VEngineClient)
		{
			if (SOURCESDK::IVEngineClient_014_csgo * pEngine = g_VEngineClient->GetVEngineClient_csgo())
			{
				SOURCESDK::player_info_t_csgo pInfo;
				return (pEngine->GetPlayerInfo(pEngine->GetPlayerForUserID(userId), &pInfo) && pInfo.xuid == Id.xuid);
			}
		}

		return false;
	}
};


struct DeathMsgFilterEntry
{
	struct PlayerEntry
	{
		DeathMsgIdMatchMode mode;
		DeathMsgId id;

		bool useName;
		std::string name;

		bool isLocal;
	};

	struct IntEntry
	{
		bool use;
		int value;
	};

	struct FloatEntry
	{
		bool use;
		float value;
	};

	PlayerEntry attacker;
	PlayerEntry victim;
	PlayerEntry assister;

	IntEntry headshot;

	IntEntry penetrated;

	IntEntry dominated;

	IntEntry revenge;

	FloatEntry lifetime;

	FloatEntry lifetimeMod;

	bool block;

	bool lastRule;

	void Console_Print()
	{

	}

	void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{

		}

		Tier0_Msg(
			"%s attacker [...]\n"
			"%s victim [...]\n"
			"%s assister [...]\n"
			"%s headshot [...]\n"
			"%s penetrated [...]\n"
			"%s dominated [...]\n"
			"%s revenge [...]\n"
			"%s lifetime [...]\n"
			"%s lifetimeMod [...]\n"
			"%s block [...]\n"
			"%s lastRule [...]\n"
		);
	}
};

static std::list<DeathMsgFilterEntry> deathMessageFilter;

class MyGameEventWrapper : SOURCESDK::CSGO::IGameEvent
{
public:
	MyGameEventWrapper(SOURCESDK::CSGO::IGameEvent * event)
	{

	}

private:

	SOURCESDK::CSGO::IGameEvent * m_Event;
}

void __stdcall Mycsgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice(csgo_CHudBaseDeathNotice_t * This, SOURCESDK::CSGO::IGameEvent * event)
{
	csgo_CHudBaseDeathNotice_FireGameEvent_UnkDoNotice_This = This;

	static bool firstRun = true;
	if(firstRun)
	{
		firstRun = false;
		org_CHudDeathNotice_nScrollInTime = *(float *)((BYTE *)this_ptr+0x5c);
		org_CHudDeathNotice_nFadeOutTime = *(float *)((BYTE *)this_ptr+0x58);
		org_CHudDeathNotice_nNoticeLifeTime = *(float *)((BYTE *)this_ptr+0x54);
		org_CHudDeathNotice_nLocalPlayerLifeTimeMod = *(float *)((BYTE *)this_ptr+0x64);
	}

	*(float *)((BYTE *)this_ptr+0x5c) = 0 <= csgo_CHudDeathNotice_nScrollInTime ? csgo_CHudDeathNotice_nScrollInTime : org_CHudDeathNotice_nScrollInTime;
	*(float *)((BYTE *)this_ptr+0x58) = 0 <= csgo_CHudDeathNotice_nFadeOutTime ? csgo_CHudDeathNotice_nFadeOutTime : org_CHudDeathNotice_nFadeOutTime;
	*(float *)((BYTE *)this_ptr+0x54) = 0 <= csgo_CHudDeathNotice_nNoticeLifeTime ? csgo_CHudDeathNotice_nNoticeLifeTime : org_CHudDeathNotice_nNoticeLifeTime;
	*(float *)((BYTE *)this_ptr+0x64) = 0 <= csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod ? csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod : org_CHudDeathNotice_nLocalPlayerLifeTimeMod;
	
	int uidAttacker = event->GetInt("attacker");
	int uidVictim = event->GetInt("userid");
	int uidAssister = event->GetInt("assister");
	bool blocked = false;

	csgo_CHudDeathNotice_HighLightId_matchedAttacker = csgo_CHudDeathNotice_HighLightId.EqualsUserId(uidAttacker);
	csgo_CHudDeathNotice_HighLightId_matchedVictim = csgo_CHudDeathNotice_HighLightId.EqualsUserId(uidVictim);
	csgo_CHudDeathNotice_HighLightId_matchedAssister = csgo_CHudDeathNotice_HighLightId.EqualsUserId(uidAssister);

	if(0 < csgo_debug_CHudDeathNotice_FireGameEvent)
	{
		Tier0_Msg("CHudDeathNotice::FireGameEvent: uidAttaker=%i, uidVictim=%i, uidAssister=%i\n", uidAttacker, uidVictim, uidAssister);

		if(2 <= csgo_debug_CHudDeathNotice_FireGameEvent)
			Tier0_Msg(
				"org_scrollInTime=%f,org_fadeOutTime=%f,org_noticeLifeTime=%f,org_localPlayerLifeTimeMod=%f\n",
				org_CHudDeathNotice_nScrollInTime,
				org_CHudDeathNotice_nFadeOutTime,
				org_CHudDeathNotice_nNoticeLifeTime,
				org_CHudDeathNotice_nLocalPlayerLifeTimeMod
			);
	}

	csgo_CHudDeathNotice_ModTime_set = false;

	for(std::list<DeathMsgBlockEntry>::iterator it = deathMessageBlock.begin(); it != deathMessageBlock.end(); it++)
	{
		DeathMsgBlockEntry e = *it;

		bool attackerBlocked;
		switch(e.attackerMode)
		{
		case DMBM_ANY:
			attackerBlocked = true;
			break;
		case DMBM_EXCEPT:
			attackerBlocked = !e.attackerId.EqualsUserId(uidAttacker);
			break;
		case DMBM_EQUAL:
		default:
			attackerBlocked = e.attackerId.EqualsUserId(uidAttacker);
			break;
		}

		bool victimBlocked;
		switch(e.victimMode)
		{
		case DMBM_ANY:
			victimBlocked = true;
			break;
		case DMBM_EXCEPT:
			victimBlocked = !e.victimId.EqualsUserId(uidVictim);
			break;
		case DMBM_EQUAL:
		default:
			victimBlocked = e.victimId.EqualsUserId(uidVictim);
			break;
		}

		bool assisterBlocked;
		switch(e.assisterMode)
		{
		case DMBM_ANY:
			assisterBlocked = true;
			break;
		case DMBM_EXCEPT:
			assisterBlocked = !e.assisterId.EqualsUserId(uidAssister);
			break;
		case DMBM_EQUAL:
		default:
			assisterBlocked = e.assisterId.EqualsUserId(uidAssister);
			break;
		}

		bool matched = attackerBlocked && victimBlocked && assisterBlocked;
		if(matched)
		{
			csgo_CHudDeathNotice_ModTime_set = 0 <= e.modTime;
			csgo_CHudDeathNotice_ModTime = e.modTime;
			blocked = !csgo_CHudDeathNotice_ModTime_set;
			if(blocked) break;
		}
	}

	if(!blocked) detoured_csgo_CHudDeathNotice_FireGameEvent(this_ptr, event);
}

typedef void (__stdcall *csgo_CHudDeathNotice_UnkAddDeathNotice_t)(DWORD *this_ptr, void * arg0, bool bIsVictim, bool bIsKiller);

csgo_CHudDeathNotice_UnkAddDeathNotice_t detoured_csgo_CHudDeathNotice_UnkAddDeathNotice;

DWORD * csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr = 0;

void __stdcall touring_csgo_CHudDeathNotice_UnkAddDeathNotice(DWORD *this_ptr, void * arg0, bool bIsVictim, bool bIsKiller)
{
	csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr = this_ptr;

	if(csgo_CHudDeathNotice_HighLightId.Mode == DeathMsgId::Id_UserId && 0 < csgo_CHudDeathNotice_HighLightId.Id.userId
		|| csgo_CHudDeathNotice_HighLightId.Mode != DeathMsgId::Id_UserId)
	{
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0,
			csgo_CHudDeathNotice_HighLightId_matchedVictim,
			!csgo_CHudDeathNotice_HighLightId_matchedVictim && (csgo_CHudDeathNotice_HighLightId_matchedAttacker || csgo_CHudDeathNotice_HighLightAssists && csgo_CHudDeathNotice_HighLightId_matchedAssister));
		return;
	}
	else
	if(csgo_CHudDeathNotice_HighLightId.Mode == DeathMsgId::Id_UserId && 0 == csgo_CHudDeathNotice_HighLightId.Id.userId)
	{
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0, false, false);
		return;
	}

	detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0, bIsVictim, bIsKiller);
}

void * detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime;

void __declspec(naked) touring_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime(void)
{
	__asm cmp csgo_CHudDeathNotice_ModTime_set, 0
	__asm jnz __modTimeSet
	__asm jmp __continue
	
	__asm __modTimeSet:
	__asm movss xmm0, [csgo_CHudDeathNotice_ModTime]

	__asm __continue:
	__asm jmp detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime
}

bool csgo_CHudDeathNotice_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent) && AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice) && AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime))
	{
		detoured_csgo_CHudDeathNotice_FireGameEvent = (csgo_CHudDeathNotice_FireGameEvent_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent), (BYTE *)touring_csgo_CHudDeathNotice_FireGameEvent, (int)AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent_DSZ));
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice = (csgo_CHudDeathNotice_UnkAddDeathNotice_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice), (BYTE *)touring_csgo_CHudDeathNotice_UnkAddDeathNotice, (int)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_DSZ));
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime), (BYTE *)touring_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime, 0x11);

		firstResult = true;
	}

	return firstResult;
}

void csgo_CHudDeathNotice_Block(char const * uidAttacker, char const * uidVictim, char const * uidAssister, float modTime)
{
	char const * acmd;
	DeathMsgId attackerId = (int)-1;
	DeathMsgId victimId = (int)-1;
	DeathMsgId assisterId = (int)-1;

	acmd = uidAttacker;
	bool anyAttacker = !strcmp("*", acmd);
	bool notAttacker = StringBeginsWith(acmd, "!");
	if(!anyAttacker) attackerId = notAttacker ? (acmd +1) : acmd;

	acmd = uidVictim;
	bool anyVictim = !strcmp("*", acmd);
	bool notVictim = StringBeginsWith(acmd, "!");
	if(!anyVictim) victimId = notVictim ? (acmd +1) : acmd;

	acmd = uidAssister;
	bool anyAssister = !strcmp("*", acmd);
	bool notAssister = StringBeginsWith(acmd, "!");
	if(!anyAssister) assisterId = notAssister ? (acmd +1) : acmd;

	DeathMsgBlockEntry entry = {
		attackerId,
		anyAttacker ? DMBM_ANY : (notAttacker ? DMBM_EXCEPT : DMBM_EQUAL),
		victimId,
		anyVictim ? DMBM_ANY : (notVictim ? DMBM_EXCEPT : DMBM_EQUAL),
		assisterId,
		anyAssister ? DMBM_ANY : (notAssister ? DMBM_EXCEPT : DMBM_EQUAL),
		modTime
	};

	deathMessageBlock.push_back(entry);
}

void csgo_CHudDeathNotice_Block_List(void)
{
	Tier0_Msg("idAttacker,idVictim,idAssister,modTime(<0 means block):\n");
	for(std::list<DeathMsgBlockEntry>::iterator it = deathMessageBlock.begin(); it != deathMessageBlock.end(); it++)
	{
		DeathMsgBlockEntry e = *it;
						
		if(DMBM_ANY == e.attackerMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.attackerMode)
			{
				Tier0_Msg("!");
			}
			e.attackerId.Console_Print();
		}
		Tier0_Msg(",");
		if(DMBM_ANY == e.victimMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.victimMode)
			{
				Tier0_Msg("!");
			}
			e.victimId.Console_Print();
		}
		Tier0_Msg(",");
		if(DMBM_ANY == e.assisterMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.assisterMode)
			{
				Tier0_Msg("!");
			}
			e.assisterId.Console_Print();
		}
		Tier0_Msg(",%f\n", e.modTime);
	}
}

void csgo_CHudDeathNotice_Block_Clear(void)
{
	deathMessageBlock.clear();
}

void Console_csgo_CHudDeathNotice_Fake(char const * htmlString, bool bIsVictim, bool bIsKiller)
{
	if (!csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr)
	{
		Tier0_Warning("Error: There must have been at least one death notice for this to work after the command has been entered the first time (wait for a frag or load another demo)!");
		return;
	}

	std::wstring wideString;

	if (!AnsiStringToWideString(htmlString, wideString))
		Tier0_Warning("Error upon converting \"%s\" to a wide string.\n", htmlString);
	else
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr, (void *)wideString.c_str(), bIsVictim, bIsKiller);
}

void Console_csgo_CHudDeathNotice_HighLightId(IWrpCommandArgs * args)
{
	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		csgo_CHudDeathNotice_HighLightId = args->ArgV(1);
		return;
	}

	Tier0_Msg(
		"Usage:\n"
		"%s -1|0|<id> - -1 is default behaviour, 0 is never highlight, otherwise <id> is the ID (you can get it from mirv_deathmsg debug) of the player you want to highlight.\n"
		"Current setting: ",
		arg0
	);
	csgo_CHudDeathNotice_HighLightId.Console_Print();
	Tier0_Msg(
		"\n"
	);
	return;
}
