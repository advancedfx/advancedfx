#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "RenderView.h"
#include "csgo/ClientToolsCSgo.h"

#include "AfxStreams.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <list>
#include <queue>
#include <sstream>

/*

sub_1011CD40

void CHudBaseDeathNotice::FireGameEvent_UnkDoNotice( IGameEvent *event )

event->GetInt("attacker",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetInt("userid",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo
event->GetInt("assister",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetInt("assistedflash",0) 
event->GetString("weapon",0)
event->GetInt("headshot",0)
event->GetInt("penetrated",0)
event->GetInt("dominated",0)
event->GetInt("revenge",0)
event->GetInt("wipe",0)
event->GetInt("noscope",0) // new
event->GetInt("thrusmoke",0) // new
event->GetInt("attackerblind",0) // new

- SpawnTime
- Lifetime
- [LifetimeMod]
".?AVCUIPanel@panorama@@" (2nd ref) [thiscall 282] UnkSetFloatProp(word propId, float value)


*/


enum class DeathnoticeShowNumbers_e : int
{
	DeathnoticeShowNumbers_Default = 0,
	DeathnoticeShowNumbers_Numbers = 1,
	DeathnoticeShowNumbers_PrependNumbers = 2
};

DeathnoticeShowNumbers_e g_DeathnoticeShowNumbers = DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_Default;

int GetSpecKeyNumber(int entindex)
{
	// Left screen side keys: 1, 2, 3, 4, 5
	// Right screen side keys: 6, 7, 8, 9, 0

	static bool (*fnPlayerSidesWappedOnScreen)(void) = (bool (*)(void))AFXADDR_GET(csgo_Unknown_GetTeamsSwappedOnScreen);

	bool swapPlayerSide = fnPlayerSidesWappedOnScreen && fnPlayerSidesWappedOnScreen();

	int slotCT = 0;
	int slotT = 0;

	WrpGlobals* gl = g_Hook_VClient_RenderView.GetGlobals();
	int imax = gl ? gl->maxclients_get() : 0;

	for (int i = 1; i <= imax; ++i)
	{
		SOURCESDK::IClientEntity_csgo* ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
		SOURCESDK::C_BaseEntity_csgo* be = ce ? ce->GetBaseEntity() : 0;
		if (be && be->IsPlayer())
		{
			int be_team = be->GetTeamNumber();

			if (3 == be_team) // CT
			{
				if (be->entindex() == entindex)
				{
					int slot = 1 + slotCT;
					if (swapPlayerSide) slot += 5;
					slot = slot % 10;
					return slot;
				}

				++slotCT;
			}
			else if (2 == be_team) // T
			{
				if (be->entindex() == entindex)
				{
					int slot = 1 + slotT;
					if (!swapPlayerSide) slot += 5;
					slot = slot % 10;
					return slot;
				}

				++slotT;
			}
		}
	}

	return -1;
}


extern WrpVEngineClient * g_VEngineClient;


typedef void csgo_C_CSPlayer_IUnk_t;
typedef int(__fastcall * csgo_C_CSPlayer_IClientNetworkable_entindex_t)(csgo_C_CSPlayer_IUnk_t * This, void * edx);
csgo_C_CSPlayer_IClientNetworkable_entindex_t Truecsgo_C_CSPlayer_IClientNetworkable_entindex = 0;


typedef void CCSGO_HudDeathNotice_t;
typedef void (__fastcall *CCSGO_HudDeathNotice_FireGameEvent_t)(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event);
CCSGO_HudDeathNotice_FireGameEvent_t TrueCCSGO_HudDeathNotice_FireGameEvent = 0;
CCSGO_HudDeathNotice_t * CCSGO_HudDeathNotice_FireGameEvent_This = 0;

typedef void(__fastcall *CCSGO_HudDeathNotice_UnkRemoveNotices_t)(CCSGO_HudDeathNotice_t * This, void * edx);
CCSGO_HudDeathNotice_UnkRemoveNotices_t TrueCCSGO_HudDeathNotice_UnkRemoveNotices = 0;

typedef void csgo_panorama_CUIPanel;
typedef void(__fastcall * csgo_panorama_CUIPanel_UnkSetFloatProp_t)(csgo_panorama_CUIPanel * This, void * edx, WORD propId, float value);
csgo_panorama_CUIPanel_UnkSetFloatProp_t Truecsgo_panorama_CUIPanel_UnkSetFloatProp = 0;


enum DeathMsgIdMatchMode
{
	DMBM_EQUAL,
	DMBM_EXCEPT,
	DMBM_ANY
};

extern SOURCESDK::CSGO::IEngineTrace * g_pClientEngineTrace;

class CMyDeatMsgTraceFilter : public SOURCESDK::CSGO::CTraceFilter
{
public:
	CMyDeatMsgTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passentity);
	virtual bool ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask);

private:
	const SOURCESDK::CSGO::IHandleEntity *m_pPassEnt;
};

CMyDeatMsgTraceFilter::CMyDeatMsgTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passedict)
{
	m_pPassEnt = passedict;
}

bool CMyDeatMsgTraceFilter::ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask)
{
	return pHandleEntity && pHandleEntity != m_pPassEnt;
}

int EntIndexFromTrace() {
	if (!(CClientToolsCsgo::Instance() && g_pClientEngineTrace))
	{
		Tier0_Warning("Not supported for your engine / missing hooks,!\n");
		return 0;
	}

	double forward[3], right[3], up[3];

	double Rx = (g_Hook_VClient_RenderView.LastCameraAngles[0]);
	double Ry = (g_Hook_VClient_RenderView.LastCameraAngles[1]);
	double Rz = (g_Hook_VClient_RenderView.LastCameraAngles[2]);

	MakeVectors(Rz, Rx, Ry, forward, right, up);

	double Tx = (g_Hook_VClient_RenderView.LastCameraOrigin[0] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[0] - 0 * right[0] + 0 * up[0]);
	double Ty = (g_Hook_VClient_RenderView.LastCameraOrigin[1] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[1] - 0 * right[1] + 0 * up[1]);
	double Tz = (g_Hook_VClient_RenderView.LastCameraOrigin[2] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[2] - 0 * right[2] + 0 * up[2]);

	SOURCESDK::Vector vecAbsStart;
	vecAbsStart.Init((float)g_Hook_VClient_RenderView.LastCameraOrigin[0], (float)g_Hook_VClient_RenderView.LastCameraOrigin[1], (float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);

	SOURCESDK::Vector vecAbsEnd;
	vecAbsEnd.Init((float)Tx, (float)Ty, (float)Tz);

	SOURCESDK::CSGO::Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	SOURCESDK::C_BaseEntity_csgo * localPlayer = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(CClientToolsCsgo::Instance()->GetClientToolsInterface()->GetLocalPlayer());
	CMyDeatMsgTraceFilter traceFilter(localPlayer);

	SOURCESDK::CSGO::trace_t tr;

	g_pClientEngineTrace->TraceRay(ray, SOURCESDK_CSGO_MASK_ALL, &traceFilter, &tr);

	if (tr.DidHit() && tr.m_pEnt)
	{
		return tr.GetEntityIndex();
	}

	return 0;
}

struct DeathMsgId
{
	union {
		int userId;
		unsigned long long xuid;
		int specKey;
	} Id;
	enum {
		Id_UserId,
		Id_Xuid,
		Id_Key
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

		if (StringBeginsWith(consoleValue, "k"))
		{
			this->Mode = Id_Key;
			this->Id.specKey = atoi(consoleValue +1);
		}
		else if (StringBeginsWith(consoleValue, "x"))
		{
			unsigned long long val;
			
			if (0 == _stricmp("xTrace", consoleValue))
			{
				SOURCESDK::player_info_t_csgo pinfo;

				if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(EntIndexFromTrace(), &pinfo))
				{
					val = pinfo.xuid;
				}
				else
					val = 0;
			}
			else
				val = strtoull(consoleValue + 1, 0, 10);

			this->operator=(val);
		}
		else
		{
			int val;

			if (0 == _stricmp("trace", consoleValue))
			{
				SOURCESDK::player_info_t_csgo pinfo;

				if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(EntIndexFromTrace(), &pinfo))
				{
					val = pinfo.userID;
				}
				else
					val = 0;
			}
			else
				val = atoi(consoleValue);

			this->operator=(val);
		}
	}

	int ResolveToUserId()
	{
		switch(Mode)
		{
		case Id_Key:
			{
				SOURCESDK::player_info_t_csgo pinfo;
				SOURCESDK::IVEngineClient_014_csgo * vengineClient = g_VEngineClient->GetVEngineClient_csgo();
				int maxClients = g_VEngineClient->GetMaxClients();
				for (int i = 0; i < maxClients; ++i)
				{
					if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(i, &pinfo))
					{
						if(GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID)) == Id.specKey)
							return pinfo.userID;
					}
				}
			}
			return 0;
		case Id_Xuid:
			{
				SOURCESDK::player_info_t_csgo pinfo;
				SOURCESDK::IVEngineClient_014_csgo * vengineClient = g_VEngineClient->GetVEngineClient_csgo();
				int maxClients = g_VEngineClient->GetMaxClients();
				for (int i = 0; i < maxClients; ++i)
				{
					if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(i, &pinfo) && pinfo.xuid == Id.xuid)
					{
						return pinfo.userID;
					}
				}
			}			
			return 0;
		}
		
		return Id.userId;
	}

	void Console_Print()
	{
		switch(Mode)
		{
		case Id_Key:
			Tier0_Msg("k%i", Id.specKey);
			break;
		case Id_Xuid:
			{
				std::ostringstream oss;

				oss << Id.xuid;

				Tier0_Msg("x%s", oss.str().c_str());
			}
			break;
		case Id_UserId:
		default:
			Tier0_Msg("%i", Id.userId);
			break;
		}
	}

	bool EqualsUserId(int userId)
	{
		if (Mode == Id_UserId) return userId == Id.userId;

		if (userId < 1)
			return false;

		switch(Mode)
		{
		case Id_Key:
			if (g_VEngineClient)
			{
				if (SOURCESDK::IVEngineClient_014_csgo * pEngine = g_VEngineClient->GetVEngineClient_csgo())
				{
					int entIndex = g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(userId);
					return GetSpecKeyNumber(entIndex) == Id.specKey;
				}
			}
			break;

		case Id_Xuid:
			if (g_VEngineClient)
			{
				if (SOURCESDK::IVEngineClient_014_csgo * pEngine = g_VEngineClient->GetVEngineClient_csgo())
				{
					SOURCESDK::player_info_t_csgo pInfo;
					return (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(userId), &pInfo) && pInfo.xuid == Id.xuid);
				}
			}
			break;
					
		}

		return false;
	}
};

struct MyDeathMsgBoolEntry
{
	bool use = false;
	bool value;

	bool Console_Set(char const * value) {
		if (0 == _stricmp("default", value))
		{
			this->use = false;
			return true;
		}

		this->use = true;
		this->value = 0 != atoi(value);

		return true;
	}

	void Console_Edit(IWrpCommandArgs * args) {

		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);
			if (!Console_Set(arg1))
				Tier0_Warning("Error: Could not set %s!\n", arg1);
			return;
		}

		Tier0_Msg(
			"%s 0|1|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		Tier0_Msg("\n");
	}

	void Console_Print() {
		if (!use) Tier0_Msg("default");
		else Tier0_Msg("%i", value ? 1 : 0);
	}
};

struct MyDeathMsgIntEntry
{
	bool use = false;
	int value;

	bool Console_Set(char const * value) {
		if (0 == _stricmp("default", value))
		{
			this->use = false;
			return true;
		}

		this->use = true;
		this->value = atoi(value);

		return true;
	}

	void Console_Edit(IWrpCommandArgs * args) {

		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);
			if (!Console_Set(arg1))
				Tier0_Warning("Error: Could not set %s!\n", arg1);
			return;
		}

		Tier0_Msg(
			"%s <iValue>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		Tier0_Msg("\n");
	}

	void Console_Print() {
		if (!use) Tier0_Msg("default");
		else Tier0_Msg("%i", value);
	}
};

struct MyDeathMsgFloatEntry
{
	bool use = false;
	float value;

	bool Console_Set(char const * value) {
		if (0 == _stricmp("default", value))
		{
			this->use = false;
			return true;
		}

		this->use = true;
		this->value = (float)(atof(value));

		return true;
	}

	void Console_Edit(IWrpCommandArgs * args) {

		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);
			if (!Console_Set(arg1))
				Tier0_Warning("Error: Could not set %s!\n", arg1);
			return;
		}

		Tier0_Msg(
			"%s <fValue>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		Tier0_Msg("\n");
	}

	void Console_Print() {
		if (!use) Tier0_Msg("default");
		else Tier0_Msg("%f", value);
	}
};

struct MyDeathMsgCStringEntry
{
	bool use = false;
	const char * value;
};

struct MyDeathMsgIdEntry
{
	bool use = false;
	DeathMsgId value;

	bool Console_Set(char const * value) {
		if (0 == _stricmp("default", value))
		{
			this->use = false;
			return true;
		}

		this->use = true;
		this->value = value;

		return true;
	}

	void Console_Edit(IWrpCommandArgs * args) {

		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);
			if (!Console_Set(arg1))
				Tier0_Warning("Error: Could not set %s!\n", arg1);
			return;
		}

		Tier0_Msg(
			"%s <id>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		Tier0_Msg("\n");
	}

	void Console_Print() {
		if (!use) Tier0_Msg("default");
		else value.Console_Print();
	}
};

struct MyDeathMsgPlayerEntry
{
	MyDeathMsgCStringEntry name;

	MyDeathMsgIdEntry newId;

	MyDeathMsgBoolEntry isLocal;
};

struct DeathMsgFilterEntry
{
	DeathMsgFilterEntry(IWrpCommandArgs * args)
	{
		this->Console_SetFromArgs(args);
	}

	struct StringEntry
	{
		bool use = false;
		std::string value;

		bool Console_Set(char const * value) {
			this->use = true;
			this->value = value;

			return true;
		}

		void Console_Edit(IWrpCommandArgs * args) {

			int argc = args->ArgC();
			char const * arg0 = args->ArgV(0);

			if (2 <= argc)
			{
				char const * arg1 = args->ArgV(1);

				if (3 <= argc && 0 == _stricmp("set", arg1))
				{
					char const * arg2 = args->ArgV(2);

					if (0 == _stricmp("default", arg2))
					{
						this->use = false;
						return;
					}
				}
				else if (!Console_Set(arg1))
				{
					Tier0_Warning("Error: Could not set %s!\n", arg1);
					return;
				}
			}

			Tier0_Msg(
				"%s <strValue>|(set default)\n"
				"Current value: "
				, arg0
			);
			Console_Print();
			Tier0_Msg("\n");
		}

		void Console_Print() {
			if (!use) Tier0_Msg("default");
			else Tier0_Msg("\"%s\"", value.c_str());
		}
	};

	struct PlayerEntry
	{
		DeathMsgIdMatchMode mode = DMBM_ANY;
		DeathMsgId id = (int)0;

		StringEntry name;

		MyDeathMsgIdEntry newId;

		MyDeathMsgBoolEntry isLocal;

		bool Console_MatchSet(char const * value) {
			bool any = !strcmp("*", value);
			bool not = StringBeginsWith(value, "!");

			if (!any) id = not ? (value + 1) : value;

			mode = any ? DMBM_ANY : (not ? DMBM_EXCEPT : DMBM_EQUAL);

			return true;
		}

		void Console_MatchEdit(IWrpCommandArgs * args) {
			int argc = args->ArgC();
			char const * arg0 = args->ArgV(0);

			if (2 <= argc)
			{
				Console_MatchSet(args->ArgV(1));
				return;
			}

			Tier0_Msg(
				"%s *|<id>|!<id>\n"
				"Current value: "
				, arg0
			);
			Console_MatchPrint();
			Tier0_Msg("\n");
		}

		void Console_MatchPrint() {
			if (DMBM_ANY == mode)
				Tier0_Msg("*");
			else
			{
				if (DMBM_EXCEPT == mode)
				{
					Tier0_Msg("!");
				}
				id.Console_Print();
			}
		}
	};

	PlayerEntry attacker;
	PlayerEntry victim;
	PlayerEntry assister;

	MyDeathMsgIntEntry assistedflash;

	StringEntry weapon;

	MyDeathMsgIntEntry headshot;

	MyDeathMsgIntEntry penetrated;

	MyDeathMsgIntEntry dominated;

	MyDeathMsgIntEntry revenge;

	MyDeathMsgIntEntry wipe;

	MyDeathMsgIntEntry noscope;

	MyDeathMsgIntEntry thrusmoke;

	MyDeathMsgIntEntry attackerblind;

	MyDeathMsgFloatEntry lifetime;

	MyDeathMsgFloatEntry lifetimeMod;

	MyDeathMsgBoolEntry block;

	bool lastRule = false;

	void Console_Print()
	{
		Tier0_Msg("attackerMatch=");
		attacker.Console_MatchPrint();
		Tier0_Msg(" assisterMatch=");
		assister.Console_MatchPrint();
		Tier0_Msg(" victimMatch=");
		victim.Console_MatchPrint();
	}

	void Console_SetFromArgs(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		const char * arg0 = args->ArgV(0);

		for (int i = 1; i < argc; ++i)
		{
			const char * argI = args->ArgV(i);

			if (StringIBeginsWith(argI, "attackerMatch="))
			{
				attacker.Console_MatchSet(argI + strlen("attackerMatch="));
			}
			else if (StringIBeginsWith(argI, "attackerName="))
			{
				attacker.name.Console_Set(argI + strlen("attackerName="));
			}
			else if (StringIBeginsWith(argI, "attackerId="))
			{
				attacker.newId.Console_Set(argI + strlen("attackerId="));
			}
			else if (StringIBeginsWith(argI, "attackerIsLocal="))
			{
				attacker.isLocal.Console_Set(argI + strlen("attackerIsLocal="));
			}
			else if (StringIBeginsWith(argI, "assisterMatch="))
			{
				assister.Console_MatchSet(argI + strlen("assisterMatch="));
			}
			else if (StringIBeginsWith(argI, "assisterName="))
			{
				assister.name.Console_Set(argI + strlen("assisterName="));
			}
			else if (StringIBeginsWith(argI, "assisterId="))
			{
				assister.newId.Console_Set(argI + strlen("assisterId="));
			}
			else if (StringIBeginsWith(argI, "assisterIsLocal="))
			{
				assister.isLocal.Console_Set(argI + strlen("assisterIsLocal="));
			}
			else if (StringIBeginsWith(argI, "victimMatch="))
			{
				victim.Console_MatchSet(argI + strlen("victimMatch="));
			}
			else if (StringIBeginsWith(argI, "victimName="))
			{
				victim.name.Console_Set(argI + strlen("victimName="));
			}
			else if (StringIBeginsWith(argI, "victimId="))
			{
				victim.newId.Console_Set(argI + strlen("victimId="));
			}
			else if (StringIBeginsWith(argI, "victimIsLocal="))
			{
				victim.isLocal.Console_Set(argI + strlen("victimIsLocal="));
			}
			else if (StringIBeginsWith(argI, "assistedflash="))
			{
				assistedflash.Console_Set(argI + strlen("assistedflash="));
			}
			else if (StringIBeginsWith(argI, "weapon="))
			{
				weapon.Console_Set(argI + strlen("weapon="));
			}
			else if (StringIBeginsWith(argI, "headshot="))
			{
				headshot.Console_Set(argI + strlen("headshot="));
			}
			else if (StringIBeginsWith(argI, "penetrated="))
			{
				penetrated.Console_Set(argI + strlen("penetrated="));
			}
			else if (StringIBeginsWith(argI, "wipe="))
			{
				wipe.Console_Set(argI + strlen("wipe="));
			}
			else if (StringIBeginsWith(argI, "noscope="))
			{
				noscope.Console_Set(argI + strlen("noscope="));
			}
			else if (StringIBeginsWith(argI, "thrusmoke="))
			{
				thrusmoke.Console_Set(argI + strlen("thrusmoke="));
			}
			else if (StringIBeginsWith(argI, "attackerblind="))
			{
				attackerblind.Console_Set(argI + strlen("attackerblind="));
			}
			else if (StringIBeginsWith(argI, "dominated="))
			{
				dominated.Console_Set(argI + strlen("dominated="));
			}
			else if (StringIBeginsWith(argI, "revenge="))
			{
				revenge.Console_Set(argI + strlen("revenge="));
			}
			else if (StringIBeginsWith(argI, "lifetime="))
			{
				lifetime.Console_Set(argI + strlen("lifetime="));
			}
			else if (StringIBeginsWith(argI, "lifetimeMod="))
			{
				lifetimeMod.Console_Set(argI + strlen("lifetimeMod="));
			}
			else if (StringIBeginsWith(argI, "block="))
			{
				block.Console_Set(argI + strlen("block="));
			}
			else if (StringIBeginsWith(argI, "lastRule="))
			{
				lastRule = 0 != atoi(argI + strlen("lastRule="));
			}
			else {
				Tier0_Warning("Error: invalid option \"%s\".\n", argI);
			}
		}
	}

	void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		const char * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			const char * arg1 = args->ArgV(1);

			if (0 == _stricmp("attackerMatch", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attacker.Console_MatchEdit(args);
				return;
			}
			else if (0 == _stricmp("attackerName", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attacker.name.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("attackerId", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attacker.newId.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("attackerIsLocal", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attacker.isLocal.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("assisterMatch", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				assister.Console_MatchEdit(args);
				return;
			}
			else if (0 == _stricmp("assisterName", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				assister.name.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("assisterId", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				assister.newId.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("assisterIsLocal", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				assister.isLocal.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("victimMatch", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				victim.Console_MatchEdit(args);
				return;
			}
			else if (0 == _stricmp("victimName", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				victim.name.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("victimId", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				victim.newId.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("victimIsLocal", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				victim.isLocal.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("assistedflash", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				assistedflash.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("weapon", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				weapon.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("headshot", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				headshot.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("penetrated", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				penetrated.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("dominated", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				dominated.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("revenge", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				revenge.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("wipe", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				wipe.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("noscope", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				noscope.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("thrusmoke", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				thrusmoke.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("attackerblind", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attackerblind.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("lifetime", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				lifetime.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("lifetimeMod", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				lifetimeMod.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("block", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				block.Console_Edit(args);
				return;
			}
			else if (0 == _stricmp("lastRule", arg1))
			{
				if (3 <= argc)
				{
					const char * arg2 = args->ArgV(2);

					lastRule = 0 != atoi(arg2);
					return;
				}

				Tier0_Msg(
					"%s lastRule 0|1\n"
					"Current value: %i"
					, arg0
					, lastRule ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg("%s attackerMatch [...] = ", arg0);
		attacker.Console_MatchPrint();
		Tier0_Msg("\n");

		Tier0_Msg("%s attackerName [...] = ", arg0);
		attacker.name.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s attackerId [...] = ", arg0);
		attacker.newId.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s attackerIsLocal [...] = ", arg0);
		attacker.isLocal.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s assisterMatch [...] = ", arg0);
		assister.Console_MatchPrint();
		Tier0_Msg("\n");

		Tier0_Msg("%s assisterName [...] = ", arg0);
		assister.name.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s assisterId [...] = ", arg0);
		assister.newId.Console_Print();
		Tier0_Msg("\n");

		//Tier0_Msg("%s assisterIsLocal [...] = ", arg0);
		//assister.isLocal.Console_Print();
		//Tier0_Msg("\n");

		Tier0_Msg("%s victimMatch [...] = ", arg0);
		victim.Console_MatchPrint();
		Tier0_Msg("\n");

		Tier0_Msg("%s victimName [...] = ", arg0);
		victim.name.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s victimId [...] = ", arg0);
		victim.newId.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s victimIsLocal [...] = ", arg0);
		victim.isLocal.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s assistedflash [...] = ", arg0);
		assistedflash.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s weapon [...] = ", arg0);
		weapon.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s headshot [...] = ", arg0);
		headshot.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s penetrated [...] = ", arg0);
		penetrated.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s dominated [...] = ", arg0);
		dominated.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s revenge [...] = ", arg0);
		revenge.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s wipe [...] = ", arg0);
		wipe.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s noscope [...] = ", arg0);
		noscope.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s thrusmoke [...] = ", arg0);
		thrusmoke.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s attackerblind [...] = ", arg0);
		attackerblind.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s lifetime [...] = ", arg0);
		lifetime.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s lifetimeMod [...] = ", arg0);
		lifetimeMod.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s block [...] = ", arg0);
		block.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s lastRule [...] = %i\n", arg0, lastRule ? 1 : 0);
	}
};

static std::list<DeathMsgFilterEntry> deathMessageFilter;

class MyDeathMsgGameEventWrapper : public SOURCESDK::CSGO::IGameEvent
{
public:
	MyDeathMsgPlayerEntry attacker;
	MyDeathMsgPlayerEntry victim;
	MyDeathMsgPlayerEntry assister;

	MyDeathMsgIntEntry assistedflash;

	MyDeathMsgCStringEntry weapon;

	MyDeathMsgIntEntry headshot;

	MyDeathMsgIntEntry penetrated;

	MyDeathMsgIntEntry dominated;

	MyDeathMsgIntEntry revenge;

	MyDeathMsgIntEntry wipe;

	MyDeathMsgIntEntry noscope;

	MyDeathMsgIntEntry thrusmoke;

	MyDeathMsgIntEntry attackerblind;

	MyDeathMsgFloatEntry lifetime;

	MyDeathMsgFloatEntry lifetimeMod;

	MyDeathMsgBoolEntry block;

	MyDeathMsgGameEventWrapper(SOURCESDK::CSGO::IGameEvent * event)
		: m_Event(event)
	{

	}

	void ApplyDeathMsgFilterEntry(const DeathMsgFilterEntry & dme) {
		ApplyPlayerEntry(dme.attacker, attacker);
		ApplyPlayerEntry(dme.victim, victim);
		ApplyPlayerEntry(dme.assister, assister);
		ApplyIntEntry(dme.assistedflash, assistedflash);
		ApplyStringEntry(dme.weapon, weapon);
		ApplyIntEntry(dme.headshot, headshot);
		ApplyIntEntry(dme.penetrated, penetrated);
		ApplyIntEntry(dme.dominated, dominated);
		ApplyIntEntry(dme.revenge, revenge);
		ApplyIntEntry(dme.wipe, wipe);
		ApplyIntEntry(dme.noscope, noscope);
		ApplyIntEntry(dme.thrusmoke, thrusmoke);
		ApplyIntEntry(dme.attackerblind, attackerblind);
		ApplyFloatEntry(dme.lifetime, lifetime);
		ApplyFloatEntry(dme.lifetimeMod, lifetimeMod);
		ApplyBoolEntry(dme.block, block);
	}

public:
	virtual ~MyDeathMsgGameEventWrapper() {
	}
	
	virtual const char * GetName() const {
		return m_Event->GetName();
	}

	virtual bool IsReliable() const {
		return m_Event->IsReliable();
	}

	virtual bool IsLocal() const {
		return m_Event->IsLocal();
	}

	virtual bool IsEmpty(const char *keyName = NULL) {
		return m_Event->IsEmpty(keyName);
	}

	virtual bool GetBool(const char *keyName = NULL, bool defaultValue = false) {

		return m_Event->GetBool(keyName, defaultValue);
	}

	virtual int GetInt(const char *keyName = NULL, int defaultValue = 0) {

		if (attacker.newId.use && 0 == strcmp("attacker", keyName)) return attacker.newId.value.ResolveToUserId();
		if (victim.newId.use && 0 == strcmp("userid", keyName)) return victim.newId.value.ResolveToUserId();
		if (assister.newId.use && 0 == strcmp("assister", keyName)) return assister.newId.value.ResolveToUserId();
		if (assistedflash.use && 0 == strcmp("assistedflash", keyName)) return assistedflash.value;
		if (headshot.use && 0 == strcmp("headshot", keyName)) return headshot.value;
		if (penetrated.use && 0 == strcmp("penetrated", keyName)) return penetrated.value;
		if (dominated.use && 0 == strcmp("dominated", keyName)) return dominated.value;
		if (revenge.use && 0 == strcmp("revenge", keyName)) return revenge.value;
		if (wipe.use && 0 == strcmp("wipe", keyName)) return wipe.value;
		if (noscope.use && 0 == strcmp("noscope", keyName)) return noscope.value;
		if (thrusmoke.use && 0 == strcmp("thrusmoke", keyName)) return thrusmoke.value;
		if (attackerblind.use && 0 == strcmp("attackerblind", keyName)) return attackerblind.value;

		return m_Event->GetInt(keyName, defaultValue);
	}

	virtual SOURCESDK::uint64 GetUint64(const char *keyName = NULL, SOURCESDK::uint64 defaultValue = 0) {
		return m_Event->GetUint64(keyName, defaultValue);
	}
	
	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) {
		return m_Event->GetFloat(keyName, defaultValue);
	}

	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") {

		if (weapon.use && 0 == strcmp("weapon", keyName)) return weapon.value;

		return m_Event->GetString(keyName, defaultValue);
	}

	virtual void SetBool(const char *keyName, bool value) {
		m_Event->SetBool(keyName, value);
	}

	virtual void SetInt(const char *keyName, int value) {
		m_Event->SetInt(keyName, value);
	}

	virtual void SetUint64(const char *keyName, SOURCESDK::uint64 value) {
		m_Event->SetUint64(keyName, value);
	}

	virtual void SetFloat(const char *keyName, float value) {
		m_Event->SetFloat(keyName, value);
	}

	virtual void SetString(const char *keyName, const char *value) {
		m_Event->SetString(keyName, value);
	}

private:
	SOURCESDK::CSGO::IGameEvent * m_Event;

	void ApplyBoolEntry(const MyDeathMsgBoolEntry & source, MyDeathMsgBoolEntry & target) {
		if (source.use)
		{
			target.use = true;
			target.value = source.value;
		}
	}

	void ApplyIntEntry(const MyDeathMsgIntEntry & source, MyDeathMsgIntEntry & target) {
		if (source.use)
		{
			target.use = true;
			target.value = source.value;
		}
	}

	void ApplyDeathMsgIdEntry(const MyDeathMsgIdEntry & source, MyDeathMsgIdEntry & target) {
		if (source.use)
		{
			target.use = true;
			target.value = source.value;
		}
	}

	void ApplyFloatEntry(const MyDeathMsgFloatEntry & source, MyDeathMsgFloatEntry & target) {
		if (source.use)
		{
			target.use = true;
			target.value = source.value;
		}
	}

	void ApplyStringEntry(const DeathMsgFilterEntry::StringEntry & source, MyDeathMsgCStringEntry & target) {
		if (source.use)
		{
			target.use = true;
			target.value = source.value.c_str();
		}
	}

	void ApplyPlayerEntry(const DeathMsgFilterEntry::PlayerEntry & source, MyDeathMsgPlayerEntry & target) {
		ApplyStringEntry(source.name, target.name);
		ApplyDeathMsgIdEntry(source.newId, target.newId);
		ApplyBoolEntry(source.isLocal, target.isLocal);
	}
};

struct CHudDeathNoticeHookGlobals {

	struct CSettings {
		int Debug = 0;
	} Settings;

	MyDeathMsgGameEventWrapper * activeWrapper = 0;

	std::list<DeathMsgFilterEntry> Filter;

	MyDeathMsgFloatEntry Lifetime;

	MyDeathMsgFloatEntry LifetimeMod;

	bool useHighlightId;
	DeathMsgId highlightId;

	int entindex_callnum;
	int panorama_CUIPanel_UnkSetFloatProp_callnum;

	std::queue<int> nextRealCUnknownGetPlayerNameState;
	std::queue<int> nextRealEntindexState;

} g_HudDeathNoticeHookGlobals;

void __fastcall MyCCSGO_HudDeathNotice_FireGameEvent(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event)
{
	CCSGO_HudDeathNotice_FireGameEvent_This = This;

	MyDeathMsgGameEventWrapper myWrapper(event); // TODO: Consider to re-use this instead of allocating every time on stack.

	g_HudDeathNoticeHookGlobals.entindex_callnum = 0;
	g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum = 0;
	g_HudDeathNoticeHookGlobals.activeWrapper = &myWrapper;

	if (g_HudDeathNoticeHookGlobals.Lifetime.use)
	{
		myWrapper.lifetime.use = true;
		myWrapper.lifetime.value = g_HudDeathNoticeHookGlobals.Lifetime.value;
	}

	if (g_HudDeathNoticeHookGlobals.LifetimeMod.use)
	{
		myWrapper.lifetimeMod.use = true;
		myWrapper.lifetimeMod.value = g_HudDeathNoticeHookGlobals.LifetimeMod.value;
	}

	int uidAttacker = event->GetInt("attacker");
	int uidVictim = event->GetInt("userid");
	int uidAssister = event->GetInt("assister");

	if(0 < g_HudDeathNoticeHookGlobals.Settings.Debug)
	{
		Tier0_Msg("CHudDeathNotice::FireGameEvent: uidAttaker=%i, uidVictim=%i, uidAssister=%i weapon=\"%s\"\n", uidAttacker, uidVictim, uidAssister, event->GetString("weapon"));
	}

	for(std::list<DeathMsgFilterEntry>::iterator it = g_HudDeathNoticeHookGlobals.Filter.begin(); it != g_HudDeathNoticeHookGlobals.Filter.end(); it++)
	{
		DeathMsgFilterEntry & e = *it;

		bool attackerBlocked;
		switch(e.attacker.mode)
		{
		case DMBM_ANY:
			attackerBlocked = true;
			break;
		case DMBM_EXCEPT:
			attackerBlocked = !e.attacker.id.EqualsUserId(uidAttacker);
			break;
		case DMBM_EQUAL:
		default:
			attackerBlocked = e.attacker.id.EqualsUserId(uidAttacker);
			break;
		}

		bool victimBlocked;
		switch(e.victim.mode)
		{
		case DMBM_ANY:
			victimBlocked = true;
			break;
		case DMBM_EXCEPT:
			victimBlocked = !e.victim.id.EqualsUserId(uidVictim);
			break;
		case DMBM_EQUAL:
		default:
			victimBlocked = e.victim.id.EqualsUserId(uidVictim);
			break;
		}

		bool assisterBlocked;
		switch(e.assister.mode)
		{
		case DMBM_ANY:
			assisterBlocked = true;
			break;
		case DMBM_EXCEPT:
			assisterBlocked = !e.assister.id.EqualsUserId(uidAssister);
			break;
		case DMBM_EQUAL:
		default:
			assisterBlocked = e.assister.id.EqualsUserId(uidAssister);
			break;
		}

		bool matched = attackerBlocked && victimBlocked && assisterBlocked;

		if(matched)
		{
			myWrapper.ApplyDeathMsgFilterEntry(e);

			uidAttacker = myWrapper.GetInt("attacker", 0);
			uidVictim = myWrapper.GetInt("userid", 0);
			uidAssister = myWrapper.GetInt("assister", 0);

			if (e.lastRule) break;
		}
	}

	if (g_HudDeathNoticeHookGlobals.useHighlightId)
	{

		myWrapper.attacker.isLocal.use = true;
		myWrapper.attacker.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidAttacker);

		myWrapper.victim.isLocal.use = true;
		myWrapper.victim.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidVictim);

		myWrapper.assister.isLocal.use = true;
		myWrapper.assister.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidAssister);
	}

	if (!(myWrapper.block.use && myWrapper.block.value) ) {

		while (!g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.pop();
		}

		if (uidAttacker) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(1);
		if (uidVictim) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(2);
		if (uidAssister) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(3);

		while(!g_HudDeathNoticeHookGlobals.nextRealEntindexState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealEntindexState.pop();
		}

		if (uidAttacker) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(2);
		if (uidVictim) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(1);

		TrueCCSGO_HudDeathNotice_FireGameEvent(This, edx, &myWrapper);
	}

	g_HudDeathNoticeHookGlobals.activeWrapper = 0;
}

class MyDeathMsgGameEventFallback : public SOURCESDK::CSGO::IGameEvent
{
public:
	MyDeathMsgGameEventFallback() {
	}

public:
	virtual ~MyDeathMsgGameEventFallback() {
	}

	virtual const char * GetName() const {
		return "player_death";
	}

	virtual bool IsReliable() const {
		return true;
	}

	virtual bool IsLocal() const {
		return true;
	}

	virtual bool IsEmpty(const char *keyName = NULL) {
		return true;
	}

	virtual bool GetBool(const char *keyName = NULL, bool defaultValue = false) {
		if (0 == strcmp("realtime_passthrough", keyName)) return true;

		return defaultValue;
	}

	virtual int GetInt(const char *keyName = NULL, int defaultValue = 0) {
		return defaultValue;
	}

	virtual SOURCESDK::uint64 GetUint64(const char *keyName = NULL, SOURCESDK::uint64 defaultValue = 0) {
		return defaultValue;
	}

	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) {
		return defaultValue;
	}

	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") {
		return defaultValue;
	}

	virtual void SetBool(const char *keyName, bool value) {
		return;
	}

	virtual void SetInt(const char *keyName, int value) {
		return;
	}

	virtual void SetUint64(const char *keyName, SOURCESDK::uint64 value) {
		return;
	}

	virtual void SetFloat(const char *keyName, float value) {
		return;
	}

	virtual void SetString(const char *keyName, const char *value) {
		return;
	}

private:

};

struct CCsgoReplaceNameEntry
{
	DeathMsgId id;
	std::string name;
	int mode;

	CCsgoReplaceNameEntry(const char * id, const char * name, int mode)
	{
		this->id = id;
		this->name = name;
		this->mode = mode;
	}
};

struct CCsgoREplaceTeamNameEntry {
	bool Use_ClanName = false;
	std::string ClanName;
	bool Use_FlagImageString = false;
	std::string FlagImageString;
	bool Use_LogoImageString = false;
	std::string LogoImageString;
};

bool g_csgo_ReplaceTeamNameDebug = false;
std::map<int,CCsgoREplaceTeamNameEntry> g_csgo_ReplaceTeamNameList;

bool g_csgo_ReplacePlayerNameDebug = false;
std::list<CCsgoReplaceNameEntry> g_csgo_ReplacePlayerNameList;

// This function has a bug: entindex != entnum (returned by GetPlayerForUserID), but it's fine for now here.

int __fastcall Mycsgo_C_CSPlayer_IClientNetworkable_entindex(csgo_C_CSPlayer_IUnk_t * This, void * edx) {

	if (g_HudDeathNoticeHookGlobals.activeWrapper)
	{
		if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealEntindexState.empty())
		{
			int state = g_HudDeathNoticeHookGlobals.nextRealEntindexState.front();
			g_HudDeathNoticeHookGlobals.nextRealEntindexState.pop();

			switch (state)
			{
			case 1:
				if (g_HudDeathNoticeHookGlobals.activeWrapper->attacker.isLocal.use)
					return g_HudDeathNoticeHookGlobals.activeWrapper->attacker.isLocal.value ? g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("attacker", 0)) : 0;
				break;
			case 2:
				if (g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.use)
					return g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.value ? g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("userid", 0)) : 0;
				break;
			}
		}
	}

	return Truecsgo_C_CSPlayer_IClientNetworkable_entindex(This, edx);
}

typedef char *(__fastcall *csgo_C_Team_Get_ClanName_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );
typedef char *(__fastcall *csgo_C_Team_Get_FlagImageString_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );
typedef char *(__fastcall *csgo_C_Team_Get_LogoImageString_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );

csgo_C_Team_Get_ClanName_t g_Old_csgo_C_Team_Get_ClanName = nullptr;
csgo_C_Team_Get_FlagImageString_t g_Old_csgo_C_Team_Get_FlagImageString = nullptr;
csgo_C_Team_Get_LogoImageString_t g_Old_csgo_C_Team_Get_LogoImageString = nullptr;

char * __fastcall My_csgo_C_Team_Get_ClanName( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_ClanName( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_ClanName: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_ClanName)
	{
		result = const_cast<char *>(it->second.ClanName.c_str());
	}

	return result;
}

char * __fastcall My_csgo_C_Team_Get_FlagImageString( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_FlagImageString( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_FlagImageString: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_FlagImageString)
	{
		result = const_cast<char *>(it->second.FlagImageString.c_str());
	}

	return result;
}

char * __fastcall My_csgo_C_Team_Get_LogoImageString( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_LogoImageString( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_LogoImageString: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_LogoImageString)
	{
		result = const_cast<char *>(it->second.LogoImageString.c_str());
	}

	return result;
}


bool Hookcsgo_Team_GetFunctions(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		AFXADDR_GET(csgo_C_Team_vtable)
	)
	{
		LONG error = NO_ERROR;


		void **vtable = (void **)AFXADDR_GET(csgo_C_Team_vtable);
		
		g_Old_csgo_C_Team_Get_ClanName = (csgo_C_Team_Get_ClanName_t)vtable[187];
		g_Old_csgo_C_Team_Get_FlagImageString = (csgo_C_Team_Get_FlagImageString_t)vtable[188];
		g_Old_csgo_C_Team_Get_LogoImageString = (csgo_C_Team_Get_LogoImageString_t)vtable[189];
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_ClanName, My_csgo_C_Team_Get_ClanName);
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_FlagImageString, My_csgo_C_Team_Get_FlagImageString);
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_LogoImageString, My_csgo_C_Team_Get_LogoImageString);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

typedef const char *(__fastcall *csgo_CPlayerResource_GetPlayerName_t)( void* This, void* Edx, int index );

csgo_CPlayerResource_GetPlayerName_t g_Touring_csgo_CPlayerResource_GetPlayerName = nullptr;

const char * __fastcall My_csgo_CPlayerResource_GetPlayerName( void* This, void* Edx, int index ) {
	const char * result = g_Touring_csgo_CPlayerResource_GetPlayerName(This, Edx, index);

	SOURCESDK::player_info_t_csgo pinfo;

	if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(index, &pinfo))
	{
		if (g_csgo_ReplacePlayerNameDebug) {

			Tier0_Msg("CPlayerResource::GetPlayerName: %i -> %s\n", pinfo.userID, result);
		}

		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if(!(e.mode & (1 << 0))) continue;

			if (
				(e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo.userID)
				|| (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo.xuid)
				|| e.id.Mode == DeathMsgId::Id_Key && e.id.Id.specKey == GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID))
			) {
				result = it->name.c_str();
				break;
			}
		}
	}

	return result;
}

bool Hookcsgo_CPlayerResource_GetPlayerName(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		g_VEngineClient->GetVEngineClient_csgo()
		&& AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable)
	)
	{
		LONG error = NO_ERROR;


		void **vtable = (void **)AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable);

		g_Touring_csgo_CPlayerResource_GetPlayerName = (csgo_CPlayerResource_GetPlayerName_t)vtable[8];
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Touring_csgo_CPlayerResource_GetPlayerName, My_csgo_CPlayerResource_GetPlayerName);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

typedef wchar_t * (__fastcall *csgo_CUnknown_GetPlayerName_t)(void * This, void * Edx, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1);

csgo_CUnknown_GetPlayerName_t g_Old_csgo_CUnknown_GetPlayerName;

wchar_t * __fastcall My_csgo_CUnknown_GetPlayerName(void * This, void * Edx, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1)
{
	wchar_t * result = g_Old_csgo_CUnknown_GetPlayerName(This, Edx, entIndex, targetBuffer, targetByteCount, unknownArg1);

	SOURCESDK::player_info_t_csgo pinfo;
	MyDeathMsgPlayerEntry * playerEntry = nullptr;

	if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.empty())
	{
		int state = g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.front();
		g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.pop();

		switch (state)
		{
		case 1:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->attacker);
			break;
		case 2:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->victim);
			break;
		case 3:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->assister);
			break;
		}
	}

	if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(entIndex, &pinfo))
	{
		if (g_csgo_ReplacePlayerNameDebug) {

			std::string utf8PlayerName;

			if (WideStringToUTF8String(result, utf8PlayerName))
				Tier0_Msg("CUnknown::GetPlayerName: %i -> %s\n", pinfo.userID, utf8PlayerName.c_str());
			else
				Tier0_Msg("CUnknown::GetPlayerName: %i ERROR: could not get UTF8-String for player name.\n", pinfo.userID);
		}

		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if(!(e.mode & (1 << 1))) continue;

			if (
				(e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo.userID)
				|| (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo.xuid)
				|| e.id.Mode == DeathMsgId::Id_Key && e.id.Id.specKey == GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID))
			) {
				std::wstring widePlayerName;
				if (UTF8StringToWideString(it->name.c_str(), widePlayerName))
				{
					wcscpy_s(targetBuffer, targetByteCount / sizeof(wchar_t), widePlayerName.c_str());
				}
				break;
			}
		}

		if (playerEntry && playerEntry->name.use) {
			std::wstring widePlayerName;
			if (UTF8StringToWideString(playerEntry->name.value, widePlayerName))
			{
				wcscpy_s(targetBuffer, targetByteCount / sizeof(wchar_t), widePlayerName.c_str());
			}
		}

		if (g_HudDeathNoticeHookGlobals.activeWrapper)
		{
			switch (g_DeathnoticeShowNumbers)
			{
				case DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_Numbers:
				case DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_PrependNumbers:
				{
					wchar_t number[33];
					if (0 == _itow_s(GetSpecKeyNumber(entIndex), number, 10))
					{
						size_t len = wcslen(number);
						size_t oldLen = wcslen(targetBuffer);
						if (g_DeathnoticeShowNumbers == DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_PrependNumbers)
						{
							size_t pos = len + 1 + oldLen;
							if (pos < targetByteCount)
							{
								for(;len <= pos; --pos)
								{
									targetBuffer[pos] = targetBuffer[pos - len - 1];
								}
								targetBuffer[len] = L' ';
								for (pos = 0; pos < len; ++pos)
								{
									targetBuffer[pos] = number[pos];
								}
							}
						}
						else
						{
							len = len + 1;
							if (len <= targetByteCount / sizeof(wchar_t))
							{
								wcscpy_s(targetBuffer, len, number);
							}
						}
					}
				}
				break;
			}
		}
	}

	return result;
}


bool Hook_csgo_CUnknown_GetPlayerName(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		g_VEngineClient->GetVEngineClient_csgo()
		&& AFXADDR_GET(csgo_CUnknown_GetPlayerName)
	)
	{
		LONG error = NO_ERROR;

		g_Old_csgo_CUnknown_GetPlayerName = (csgo_CUnknown_GetPlayerName_t)AFXADDR_GET(csgo_CUnknown_GetPlayerName);
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_csgo_CUnknown_GetPlayerName, My_csgo_CUnknown_GetPlayerName);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

void __fastcall Mycsgo_panorama_CUIPanel_UnkSetFloatProp(csgo_panorama_CUIPanel * This, void * edx, WORD propId, float value) {

	if (g_HudDeathNoticeHookGlobals.activeWrapper)
	{
		switch (g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum)
		{
		case 0:
			// SpawnTime.
			break;
		case 1:
			// Lifetime.
			if (g_HudDeathNoticeHookGlobals.activeWrapper->lifetime.use) value = g_HudDeathNoticeHookGlobals.activeWrapper->lifetime.value;
			break;
		case 2:
			// LifetimeMod.
			if (g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.use
				&& value != 1.0f // multiplier is used
			) value = g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.value;
			break;
		}

		++g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum;
	}

	Truecsgo_panorama_CUIPanel_UnkSetFloatProp(This, edx, propId, value);
}


bool csgo_CHudDeathNotice_Install_Panorama(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		Hook_csgo_CUnknown_GetPlayerName()
		&& AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent)
		&& AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp)
		&& AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex)
	)
	{
		LONG error = NO_ERROR;

		TrueCCSGO_HudDeathNotice_FireGameEvent = (CCSGO_HudDeathNotice_FireGameEvent_t)AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent);
		Truecsgo_panorama_CUIPanel_UnkSetFloatProp = (csgo_panorama_CUIPanel_UnkSetFloatProp_t)AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp);
		Truecsgo_C_CSPlayer_IClientNetworkable_entindex = (csgo_C_CSPlayer_IClientNetworkable_entindex_t)AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueCCSGO_HudDeathNotice_FireGameEvent, MyCCSGO_HudDeathNotice_FireGameEvent);
		DetourAttach(&(PVOID&)Truecsgo_panorama_CUIPanel_UnkSetFloatProp, Mycsgo_panorama_CUIPanel_UnkSetFloatProp);
		DetourAttach(&(PVOID&)Truecsgo_C_CSPlayer_IClientNetworkable_entindex, Mycsgo_C_CSPlayer_IClientNetworkable_entindex);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

bool csgo_CHudDeathNotice_Install(void)
{
	return csgo_CHudDeathNotice_Install_Panorama();
}

void Console_DeathMsgId_PrintHelp(const char * cmd)
{
	Tier0_Msg(
		"%s accepts the following as <id...>:\n"
		"<iNumber> - UserID, Example: 2\n"
		"trace - UserID from a screen trace (i.e. current POV).\n"
		"x<iNumber> - XUID, Example: x76561197961927915\n"
		"xTrace - XUID from a screen trace (i.e. current POV).\n"
		"k<iNumber> - Spectator key number.\n"
		"We recommend getting the numbers from the output of \"mirv_listentities isPlayer=1\".\n"
		, cmd
	);
}

void Console_DeathMsgArgs_PrintHelp(const char * cmd, bool showMatch)
{
	Tier0_Msg(
		"%s <option>+\n"
		"Where <option> is (you don't have to use all):\n"
		"%s"
		"\t\"attackerName=<sName>\" - New attacker name.\n"
		"\t\"attackerId=<id>\" - New attacker id.\n"
		"\t\"attackerIsLocal=(0|1)\" - If to be considered local player.\n"
		"%s"
		"\t\"assisterName=<sName>\" - New assister name.\n"
		"\t\"assisterId=<id>\" - New assister id.\n"
		//"\t\"assisterIsLocal=(0|1)\" - If to be considered local player.\n"
		"%s"
		"\t\"victimName=<sName>\" - New victim name.\n"
		"\t\"victimId=<id>\" - New victim id.\n"
		"\t\"victimIsLocal=(0|1)\" - If to be considered local player.\n"
		"\t\"assistedflash=<iVal>\" - If flash assist.\n"
		"\t\"weapon=<sWeaponName>\" - Weapon name (i.e. ak47).\n"
		"\t\"headshot=<iVal>\" - If headshot.\n"
		"\t\"penetrated=<iVal>\" - If penetrated.\n"
		"\t\"dominated=<iVal>\" - If dominated.\n"
		"\t\"revenge=<iVal>\" - If revenge.\n"
		"\t\"wipe=<iVal>\" - Squad wipeout in Danger Zone(?).\n"
		"\t\"noscope=<iVal>\" - If noscope.\n"
		"\t\"thrusmoke=<iVal>\" - If thrusmoke.\n"
		"\t\"attackerblind=<iVal>\" - If attackerblind.\n"
		"\t\"lifetime=<fVal>\" - Life time in seconds.\n"
		"\t\"lifetimeMod=<fVal>\" - Life time modifier (for player considered to be local player).\n"
		"\t\"block=<iVal>\" - If to block this message (0 = No, 1 = Yes).\n"
		"\t\"lastRule=<iVal>\" - If this is the last rule to be applied (abort after this one), 0 = No, 1 = Yes.\n"
		, cmd
		, showMatch ? "\t\"attackerMatch=<matchExpr>\" - The attacker to match.\n" : ""
		, showMatch ? "\t\"assisterMatch=<matchExpr>\" - The assister to match.\n" : ""
		, showMatch ? "\t\"victimMatch=<matchExpr>\" - The victim to match.\n" : ""
	);
}

bool csgo_CHudDeathNotice_Console(IWrpCommandArgs * args)
{
	if (!csgo_CHudDeathNotice_Install())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("filter", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("add", arg2)) {

					if (4 <= argc)
					{
						CSubWrpCommandArgs subArgs(args, 3);

						g_HudDeathNoticeHookGlobals.Filter.emplace_back(&subArgs);
						return true;
					}
				
					std::string prefix(arg0);
					prefix += " filter add";
					Console_DeathMsgArgs_PrintHelp(prefix.c_str(), true);
					return true;
				}
				else if (0 == _stricmp("move", arg2)) {

					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						int listNr = atoi(arg3);
						int targetNr = atoi(arg4);

						if (listNr < 0 || listNr >= (int)g_HudDeathNoticeHookGlobals.Filter.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						if (targetNr < 0 || targetNr > (int)g_HudDeathNoticeHookGlobals.Filter.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <tragetNr>\n", targetNr);
							return true;
						}

						std::list<DeathMsgFilterEntry>::iterator sourceIt = g_HudDeathNoticeHookGlobals.Filter.begin();

						std::list<DeathMsgFilterEntry>::iterator targetIt = g_HudDeathNoticeHookGlobals.Filter.begin();

						if (listNr <= targetNr)
						{
							std::advance(sourceIt, listNr);

							targetIt = sourceIt;

							std::advance(targetIt, targetNr - listNr);
						}
						else
						{
							std::advance(targetIt, targetNr);

							sourceIt = targetIt;

							std::advance(sourceIt, listNr - targetNr);
						}

						g_HudDeathNoticeHookGlobals.Filter.splice(targetIt, g_HudDeathNoticeHookGlobals.Filter, sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter move <listNr> <targetListNr> - Move entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("edit", arg2)) {

					if (4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_HudDeathNoticeHookGlobals.Filter.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}
						std::list<DeathMsgFilterEntry>::iterator sourceIt = g_HudDeathNoticeHookGlobals.Filter.begin();

						std::advance(sourceIt, listNr);

						CSubWrpCommandArgs subArgs(args, 4);

						sourceIt->Console_Edit(&subArgs);

						return true;
					}

					Tier0_Msg(
						"%s filter edit <listNr> - Edit entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("remove", arg2)) {

					if (4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_HudDeathNoticeHookGlobals.Filter.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}
						std::list<DeathMsgFilterEntry>::iterator sourceIt = g_HudDeathNoticeHookGlobals.Filter.begin();

						std::advance(sourceIt, listNr);

						g_HudDeathNoticeHookGlobals.Filter.erase(sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter remove <listNr> - Remove entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("clear", arg2)) {

					g_HudDeathNoticeHookGlobals.Filter.clear();

					return true;
				}
				else if (0 == _stricmp("print", arg2)) {

					Tier0_Msg("nr: id, name\n");

					int nr = 0;

					for (std::list<DeathMsgFilterEntry>::iterator it = g_HudDeathNoticeHookGlobals.Filter.begin(); it != g_HudDeathNoticeHookGlobals.Filter.end(); ++it)
					{
						Tier0_Msg(
							"%i: "
							, nr
						);

						it->Console_Print();

						Tier0_Msg("\n");

						++nr;
					}
					Tier0_Msg("---- EOL ----\n");

					return true;
				}
			}

			Tier0_Msg(
				"%s filter add [...] - Add an entry.\n"
				"%s filter edit [...] - Edit an entry.\n"
				"%s filter move [...] - Move an entry.\n"
				"%s filter remove [...] - Remove an entry.\n"
				"%s filter clear - Clear filter list.\n"
				"%s filter print - Print current list entries.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("fake", arg1)) {
			if (!CCSGO_HudDeathNotice_FireGameEvent_This)
			{
				Tier0_Warning("One message must have been triggered first, before this will work.\n");
				return true;
			}

			if (3 <= argc)
			{
				CSubWrpCommandArgs subArgs(args, 2);
				MyDeathMsgGameEventFallback fallBack;

				g_HudDeathNoticeHookGlobals.Filter.emplace_front(&subArgs);

				MyCCSGO_HudDeathNotice_FireGameEvent(CCSGO_HudDeathNotice_FireGameEvent_This, 0, &fallBack);

				g_HudDeathNoticeHookGlobals.Filter.erase(g_HudDeathNoticeHookGlobals.Filter.begin());

				return true;
			}

			std::string prefix(arg0);
			prefix += " fake";
			Console_DeathMsgArgs_PrintHelp(prefix.c_str(), false);
			return true;
		}
		else if (0 == _stricmp("help", arg1)) {
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("id", arg2)) {
					Console_DeathMsgId_PrintHelp(arg0);
					return true;
				}
			}
		}
		else if (0 == _stricmp("lifetime", arg1)) {
		
			CSubWrpCommandArgs subArgs(args, 2);

			g_HudDeathNoticeHookGlobals.Lifetime.Console_Edit(&subArgs);
			
			return true;
		}
		else if (0 == _stricmp("lifetimeMod", arg1)) {

			CSubWrpCommandArgs subArgs(args, 2);

			g_HudDeathNoticeHookGlobals.LifetimeMod.Console_Edit(&subArgs);

			return true;
		}
		else if (0 == _stricmp("localPlayer", arg1)) {

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("default", arg2))
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = false;
				}
				else
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = true;
					g_HudDeathNoticeHookGlobals.highlightId = arg2;
				}
				
				return true;
			}

			Tier0_Msg(
				"%s localPlayer default|<id>\n"
				"Current value: "
				, arg0
			);
			if (g_HudDeathNoticeHookGlobals.useHighlightId)
			{
				g_HudDeathNoticeHookGlobals.highlightId.Console_Print();
			}
			else
			{
				Tier0_Msg("default");
			}
			Tier0_Msg("\n");
			return true;
		}
		else if (0 == _stricmp("showNumbers", arg1)) {

			if (3 <= argc)
			{
				int value = atoi(args->ArgV(2));

				if (value <= 0)
				{
					g_DeathnoticeShowNumbers = DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_Default;
				}
				else if(1 == value)
				{
					g_DeathnoticeShowNumbers = DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_Numbers;
				}
				else
				{
					g_DeathnoticeShowNumbers = DeathnoticeShowNumbers_e::DeathnoticeShowNumbers_PrependNumbers;
				}
				
				return true;
			}

			Tier0_Msg(
				"%s showNumbers 0|1|2 - Default (0), only numbers (1), prepend numbers (2)\n"
				"Current value: %i\n"
				, arg0
				, g_DeathnoticeShowNumbers
			);			
			return true;
		}
		else if (0 == _stricmp("debug", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				g_HudDeathNoticeHookGlobals.Settings.Debug = atoi(arg2);

				return true;
			}

			Tier0_Msg(
				"%s debug 0|1\n"
				"Current value: %i\n"
				, arg0
				, g_HudDeathNoticeHookGlobals.Settings.Debug
			);
			return true;
		}
		else if (0 == _stricmp("deprecated", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			Tier0_Msg(
					"Usage:\n"
					"mirv_deathmsg block [...] - block specific death messages.\n"
					"mirv_deathmsg cfg [...] - configure death message properties, i.e. noticeLifeTime.\n"
					"mirv_deathmsg highLightId [...] - control highlighting.\n"
					"mirv_deathmsg help id - Display help on the ids that can be used.\n"
					"mirv_deathmsg debug [...] - enable debug message in console.\n"
			);

			return true;
		}
		else if(0 == _stricmp("block", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			if(4 <= argc)
			{
				char const * uidAttacker = args->ArgV(2);
				char const * uidVictim = args->ArgV(3);
				char const * uidAssister = 5 <= argc ? args->ArgV(4) : "*";

				CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
				fakeArgs.AddArg("filter");
				fakeArgs.AddArg("add");

				std::string arg;				
				
				arg = "attackerMatch=";
				arg += uidAttacker;
				fakeArgs.AddArg(arg.c_str());

				arg = "victimMatch=";
				arg += uidVictim;
				fakeArgs.AddArg(arg.c_str());

				arg = "assisterMatch=";
				arg += uidAssister;
				fakeArgs.AddArg(arg.c_str());

				fakeArgs.AddArg("block=1");

				csgo_CHudDeathNotice_Console(&fakeArgs);

				return true;
			}
			else
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("list", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("filter");
					fakeArgs.AddArg("print");
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				else
				if(0 == _stricmp("clear", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("filter");
					fakeArgs.AddArg("clear");
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg block <idAttacker> <idVictim> - block these ids\n"
				"\tRemarks: * to match any id, use !x to match any id apart from x.\n"
				"mirv_deathmsg block <idAttacker> <idVictim> <idAssister> - block these ids\n"
				"\tRemarks: * to match any id, use !x to match any id apart from x.\n"
				"mirv_deathmsg block list - list current blocks\n"
				"mirv_deathmsg block clear - clear current blocks\n"
				"(Use mirv_deathmsg help id to get help on the ids.)\n"
			);
			return true;
		}
		else
		if(0 == _stricmp("modTime", arg1))
		{
			Tier0_Warning("This command has been removed.\n");

			return true;
		}
		else
		if(0 == _stricmp("highLightId", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			int argc = args->ArgC();
			const char * arg0 = args->ArgV(0);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (atoi(arg2) < 0)
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = false;
				}
				else
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = true;
					g_HudDeathNoticeHookGlobals.highlightId = arg2;
				}

				return true;
			}

			Tier0_Msg(
				"Usage:\n"
				"%s -1|0|<id> - -1 is default behaviour, 0 is never highlight, otherwise <id> is the ID (you can get it from mirv_deathmsg debug) of the player you want to highlight.\n"
				"Current setting: ",
				arg0
			);
			if (!g_HudDeathNoticeHookGlobals.useHighlightId) Tier0_Msg("-1");
			else g_HudDeathNoticeHookGlobals.highlightId.Console_Print();
			Tier0_Msg(
				"\n"
			);
			return true;
		}
		else
		if(0 == _stricmp("highLightAssists", arg1))
		{
			Tier0_Warning("This command has been removed.\n");

			return true;
		}
		else
		if(0 == _stricmp("cfg", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			if(4 <= argc)
			{
				char const * arg2 = args->ArgV(2);
				float arg3 = (float)atof(args->ArgV(3));

				if(0 == _stricmp("noticeLifeTime", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("lifetime");
					fakeArgs.AddArg(arg3 < 0 ? "default" : args->ArgV(3));
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				else
				if(0 == _stricmp("localPlayerLifeTimeMod", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("lifetimeMod");
					fakeArgs.AddArg(arg3 < 0 ? "default" : args->ArgV(3));
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg cfg noticeLifeTime f - This is what you want. Current: %f\n"
				"mirv_deathmsg cfg localPlayerLifeTimeMod f - Current: %f\n"
				"Where f is a floating point value in seconds. Use -1 (a negative value) to use the orginal value instead.\n",
				g_HudDeathNoticeHookGlobals.Lifetime.use ? g_HudDeathNoticeHookGlobals.Lifetime.value : -1,
				g_HudDeathNoticeHookGlobals.LifetimeMod.use ? g_HudDeathNoticeHookGlobals.LifetimeMod.value : -1
			);
			return true;
		}
	}


	Tier0_Msg(
		"%s filter [...] - Filter death messages.\n"
		"%s fake [...] - Fake a death message (needs one message being triggered earlier).\n"
		"%s help id [...] - Print help on <id...> usage.\n"
		"%s lifetime [...] - Controls lifetime of deathmessages.\n"
		"%s lifetimeMod [...] - Controls lifetime modifier of deathmessages for the \"local\" player.\n"
		"%s localPlayer [...] - Controls what is considered \"local\" player (and thus highlighted in death notices).\n"
		"%s showNumbers [...] - Controls if and in what style to show numbers instead of player names.\n"
		"%s debug [...] - Enable / Disable debug spew upon death messages.\n"
		"%s deprecated - Print deprecated commands.\n"
		"Hint: Jump back in demo (i.e. to round start) to clear death messages.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;

}

bool csgo_ReplacePlayerName_Console(IWrpCommandArgs * args)
{
	if (!Hook_csgo_CUnknown_GetPlayerName() || !Hookcsgo_CPlayerResource_GetPlayerName())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("filter", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("add", arg2)) {

					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						g_csgo_ReplacePlayerNameList.emplace_back(arg3, arg4, 6 <= argc ? atoi(args->ArgV(5)) : 3);
						return true;
					}

					Tier0_Msg(
						"%s filter add <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> <sNewName> [<iMode>] - <iMode>: 1 name only, 2 decorated, 3 both (default)\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("edit", arg2))
				{
					if (4 <= argc)
					{
						const char* arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::advance(sourceIt, listNr);

						if (5 <= argc)
						{
							const char* arg4 = args->ArgV(4);

							if (0 == _stricmp("id", arg4))
							{
								if (6 <= argc)
								{
									sourceIt->id = args->ArgV(5);
									return true;
								}

								Tier0_Msg(
									"%s filter edit %i id <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> - Set new ID.\n"
									"Current value: "
									, arg0, listNr
								);
								sourceIt->id.Console_Print();
								Tier0_Msg("\n");

								return true;
							}
							else if (0 == _stricmp("name", arg4))
							{
								if (6 <= argc)
								{
									sourceIt->name = args->ArgV(5);
									return true;
								}

								Tier0_Msg(
									"%s filter edit %i name <sName> - Set new name.\n"
									"Current value: %s\n"
									, arg0, listNr
									, sourceIt->name.c_str()
								);

								return true;
							}
						}

						Tier0_Msg(
							"%s filter edit %i id [...]\n"
							"%s filter edit %i name [...]\n"
							, arg0, listNr
							, arg0, listNr
						);
						return true;
					}

					Tier0_Msg(
						"%s filter edit <listNr> [...] - Edit entry.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("move", arg2))
				{
					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						int listNr = atoi(arg3);
						int targetNr = atoi(arg4);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						if (targetNr < 0 || targetNr > (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <tragetNr>\n", targetNr);
							return true;
						}

						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::list<CCsgoReplaceNameEntry>::iterator targetIt = g_csgo_ReplacePlayerNameList.begin();

						if (listNr <= targetNr)
						{
							std::advance(sourceIt, listNr);

							targetIt = sourceIt;

							std::advance(targetIt, targetNr -listNr);
						}
						else
						{
							std::advance(targetIt, targetNr);

							sourceIt = targetIt;

							std::advance(sourceIt, listNr - targetNr);
						}

						g_csgo_ReplacePlayerNameList.splice(targetIt, g_csgo_ReplacePlayerNameList, sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter move <listNr> <targetListNr> - Move entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("remove", arg2)) {

					if (4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}
						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::advance(sourceIt, listNr);

						g_csgo_ReplacePlayerNameList.erase(sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter remove <listNr> - Remove entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("removeMatching", arg2))
				{
					if (4 <= argc)
					{
						CSubWrpCommandArgs subArgs(args, 3);
						DeathMsgId id;
						id = args->ArgV(3);

						int userId = id.ResolveToUserId();
						if(userId)
						{
							for(auto it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); )
							{
								if(it->id.EqualsUserId(userId))
								{
									auto itDelete = it;
									++it;
									g_csgo_ReplacePlayerNameList.erase(itDelete);
								}
								else ++it;
							}
						}

						return true;
					}

					Tier0_Msg(
						"%s filter removeMatching <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> - Remove entries that _currently_ match.\n"
						, arg0
					);
					return true;
				}				
				else if (0 == _stricmp("clear", arg2)) {

					g_csgo_ReplacePlayerNameList.clear();
				
					return true;
				}			
				else if (0 == _stricmp("print", arg2)) {

					Tier0_Msg("nr: id, name\n");

					int nr = 0;

					for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it)
					{
						Tier0_Msg(
							"%i: "
							, nr
						);

						it->id.Console_Print();

						Tier0_Msg(", \"%s\"\n", it->name.c_str());

						++nr;
					}
					Tier0_Msg("---- EOL ----\n");

					return true;
				}					
			}

			Tier0_Msg(
				"%s filter add [...] - Add an entry.\n"
				"%s filter edit [...] - Edit an entry.\n"
				"%s filter move [...] - Move an entry.\n"
				"%s filter remove [...] - Remove an entry.\n"
				"%s filter removeMatching [...] - Remove matching entries.\n"
				"%s filter clear - Clear filter list.\n"
				"%s filter print - Print current list entries.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("help", arg1)) {
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("id", arg2)) {
					Console_DeathMsgId_PrintHelp(arg0);
					return true;
				}
			}
		}
		else if (0 == _stricmp("deprecated", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			Tier0_Msg(
				"Usage:\n"
				"%s <playerId> <name> - replace <playerId> with given <name>.\n"
				"%s debug 0|1 - print <playerId> -> <name> pairs into console as they get queried by the game.\n"
				"%s delete <playerId> - delete replacement for <playerId>.\n"
				"%s list - list all <playerId> -> <name> replacements currently active.\n"
				"%s clear - clear all replacements.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("list", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			CFakeWrpCommandArgs fakeArgs(args->ArgV(0));
			fakeArgs.AddArg("filter");
			fakeArgs.AddArg("print");

			csgo_ReplacePlayerName_Console(&fakeArgs);

			return true;
		}
		else if (0 == _stricmp("clear", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			CFakeWrpCommandArgs fakeArgs(args->ArgV(0));
			fakeArgs.AddArg("filter");
			fakeArgs.AddArg("clear");

			csgo_ReplacePlayerName_Console(&fakeArgs);

			return true;
		}
		else if (3 <= argc)
		{
			const char * arg2 = args->ArgV(2);

			if (0 == _stricmp("debug", arg1))
			{
				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				g_csgo_ReplacePlayerNameDebug = 0 != atoi(arg2);
				return true;
			}
			else if (0 == _stricmp("delete", arg1)) {

				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				int id = atoi(arg2);

				for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it)
				{
					CCsgoReplaceNameEntry & entry = *it;

					if(id == entry.id.ResolveToUserId())
					{
						it = g_csgo_ReplacePlayerNameList.erase(it);
						if (it == g_csgo_ReplacePlayerNameList.end())
							break;
					}
				}
				return true;
			}
			else if (1 <= strlen(arg1) && (isdigit(arg1[0]) || 'x' == tolower(arg1[0]))) {

				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				g_csgo_ReplacePlayerNameList.emplace_back(arg1, arg2, 3);
				return true;

			}
		}
	}


	Tier0_Msg(
		"%s filter [...] - Filter names.\n"
		"%s help id [...]- Print help on <id...> usage.\n"
		"%s deprecated - Print deprecated commands.\n"
		"%s debug 0|1"
		"Please note: The engine might cache names, so set the filter up early enough!\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;
}


bool csgo_ReplaceTeamName_Console(IWrpCommandArgs * args)
{
	if (!Hookcsgo_Team_GetFunctions())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if(2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if(0 == _stricmp("add", arg1) && 3 <= argC)
		{
			int index = atoi(args->ArgV(2));
			CCsgoREplaceTeamNameEntry entry;

			for(int i = 3; i < argC; ++i)
			{
				const char * curArg = args->ArgV(i);
				if(StringBeginsWith(curArg, "clanName="))
				{
					entry.Use_ClanName = true;
					entry.ClanName = curArg + strlen("clanName=");
				}
				else if(StringBeginsWith(curArg, "flagImageString="))
				{
					entry.Use_FlagImageString = true;
					entry.FlagImageString = curArg + strlen("flagImageString=");
				}
				else if(StringBeginsWith(curArg, "logoImageString="))
				{
					entry.Use_LogoImageString = true;
					entry.LogoImageString = curArg + strlen("logoImageString=");
				}
				else {
					Tier0_Warning("Argument %i (\"%s\") is not valid.\n", i, curArg);
					return true;
				}
			}

			g_csgo_ReplaceTeamNameList[index] = entry;
			return true;
		}
		else if(0 == _stricmp("remove", arg1) && 3 <= argC)
		{
			int index = atoi(args->ArgV(2));
			auto it = g_csgo_ReplaceTeamNameList.find(index);
			if(it != g_csgo_ReplaceTeamNameList.end())
			{
				g_csgo_ReplaceTeamNameList.erase(it);
			}
			else Tier0_Warning("AFXERROR: Invalid index.\n");

			return true;
		}
		else if(0 == _stricmp("clear", arg1))
		{
			g_csgo_ReplaceTeamNameList.clear();
			return true;
		}
		else if(0 == _stricmp("print", arg1))
		{
			for(auto it=g_csgo_ReplaceTeamNameList.begin(); it !=g_csgo_ReplaceTeamNameList.end(); ++it)
			{
				CCsgoREplaceTeamNameEntry &entry = it->second;

				Tier0_Msg("%i ->", it->first);
				if(entry.Use_ClanName) Tier0_Msg(" \"clanName=%s\"", entry.ClanName.c_str());
				if(entry.Use_FlagImageString) Tier0_Msg(" \"flagImageString=%s\"", entry.FlagImageString.c_str());
				if(entry.Use_LogoImageString) Tier0_Msg(" \"logoImageString=%s\"", entry.LogoImageString.c_str());
				Tier0_Msg("\n");
			}
			return true;
		}
		else if(0 == _stricmp("debug", arg1) && 3 <= argC)
		{
			g_csgo_ReplaceTeamNameDebug = 0 != atoi(args->ArgV(2));
			return true;
		}
	}

	Tier0_Msg(
		"%s add <iTeamNumber> <sTeamName> (\"clanName=<sText>\"|\"flagImageString=<sText>\"|\"logoImageString=<sText>\")* - Replace team text(s) (team numbers: 1 = Spectator, 2 = TERRORIST, 3 = CT, ...)\n"
		"%s remove <iTeamNumber>\n"
		"%s clear\n"
		"%s print\n"
		"%s debug 0|1\n"
		"Please note: The engine might cache names, so set the filter up early enough!\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;
}
