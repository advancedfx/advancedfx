#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "RenderView.h"
#include "csgo/ClientToolsCSgo.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>

#include <list>
#include <queue>
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
- [LifetimeMod]
".?AVCUIPanel@panorama@@" (2nd ref) [thiscall 282] UnkSetFloatProp(word propId, float value)


*/

extern WrpVEngineClient * g_VEngineClient;


typedef bool(__fastcall * csgo_VEngineClient_GetPlayerInfo_t)(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int ent_num, SOURCESDK::player_info_t_csgo *pinfo);
csgo_VEngineClient_GetPlayerInfo_t Truecsgo_VEngineClient_GetPlayerInfo = 0;

typedef int(__fastcall * csgo_VEngineClient_GetPlayerForUserID_t)(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int userID);
csgo_VEngineClient_GetPlayerForUserID_t Truecsgo_VEngineClient_GetPlayerForUserID = 0;


typedef void csgo_C_CSPlayer_IUnk_t;
typedef int(__fastcall * csgo_C_CSPlayer_IClientNetworkable_entindex_t)(csgo_C_CSPlayer_IUnk_t * This, void * edx);
csgo_C_CSPlayer_IClientNetworkable_entindex_t Truecsgo_C_CSPlayer_IClientNetworkable_entindex = 0;


typedef void CCSGO_HudDeathNotice_t;
typedef void (__fastcall *CCSGO_HudDeathNotice_FireGameEvent_t)(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event);
CCSGO_HudDeathNotice_FireGameEvent_t TrueCCSGO_HudDeathNotice_FireGameEvent = 0;
CCSGO_HudDeathNotice_t * CCSGO_HudDeathNotice_FireGameEvent_This = 0;

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
			unsigned long long val;
			
			if (0 == _stricmp("xTrace", consoleValue))
			{
				SOURCESDK::player_info_t_csgo pinfo;

				if (Truecsgo_VEngineClient_GetPlayerInfo(g_VEngineClient->GetVEngineClient_csgo(), 0, EntIndexFromTrace(), &pinfo))
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

				if (Truecsgo_VEngineClient_GetPlayerInfo(g_VEngineClient->GetVEngineClient_csgo(), 0, EntIndexFromTrace(), &pinfo))
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
		if (Mode == Id_Xuid)
		{
			SOURCESDK::player_info_t_csgo pinfo;
			SOURCESDK::IVEngineClient_014_csgo * vengineClient = g_VEngineClient->GetVEngineClient_csgo();
			int maxClients = g_VEngineClient->GetMaxClients();
			for (int i = 0; i < maxClients; ++i)
			{
				if (Truecsgo_VEngineClient_GetPlayerInfo(vengineClient, 0, i, &pinfo) && pinfo.xuid == Id.xuid)
				{
					return pinfo.userID;
				}
			}

			return 0;
		}

		return Id.userId;
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
				return (Truecsgo_VEngineClient_GetPlayerInfo(g_VEngineClient->GetVEngineClient_csgo(), 0, Truecsgo_VEngineClient_GetPlayerForUserID(g_VEngineClient->GetVEngineClient_csgo(), 0, userId), &pInfo) && pInfo.xuid == Id.xuid);
			}
		}

		return false;
	}
};

struct MyDeathMsgBoolEntry
{
	bool use = false;
	bool value;

	bool Console_Set(char const * value) {
		if (0 == strcmp("default", value))
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
		if (0 == strcmp("default", value))
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
		if (0 == strcmp("default", value))
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
		if (0 == strcmp("default", value))
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

				if (3 <= argc && 0 == strcmp("set", arg1))
				{
					char const * arg2 = args->ArgV(2);

					if (0 == strcmp("default", arg2))
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
			else Tier0_Msg("\"%s\"", value);
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

	StringEntry weapon;

	MyDeathMsgIntEntry headshot;

	MyDeathMsgIntEntry penetrated;

	MyDeathMsgIntEntry dominated;

	MyDeathMsgIntEntry revenge;

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

		Tier0_Msg("%s assisterIsLocal [...] = ", arg0);
		assister.isLocal.Console_Print();
		Tier0_Msg("\n");

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

	MyDeathMsgCStringEntry weapon;

	MyDeathMsgIntEntry headshot;

	MyDeathMsgIntEntry penetrated;

	MyDeathMsgIntEntry dominated;

	MyDeathMsgIntEntry revenge;

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
		ApplyStringEntry(dme.weapon, weapon);
		ApplyIntEntry(dme.headshot, headshot);
		ApplyIntEntry(dme.penetrated, penetrated);
		ApplyIntEntry(dme.dominated, dominated);
		ApplyIntEntry(dme.revenge, revenge);
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
		if (headshot.use && 0 == strcmp("headshot", keyName)) return headshot.value;
		if (penetrated.use && 0 == strcmp("penetrated", keyName)) return penetrated.value;
		if (dominated.use && 0 == strcmp("dominated", keyName)) return dominated.value;
		if (revenge.use && 0 == strcmp("revenge", keyName)) return revenge.value;

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

	bool inPlayerInfoCall = false;

	std::queue<int> nextRealPlayerInfoState;
	std::queue<int> nextRealEntindexState;

} g_HudDeathNoticeHookGlobals;

void SetLifetimeClassValues(DWORD this_ptr)
{
	static float org_CHudDeathNotice_nNoticeLifeTime = *(float *)((BYTE *)this_ptr + 0x58);
	static float org_CHudDeathNotice_nLocalPlayerLifeTimeMod = *(float *)((BYTE *)this_ptr + 0x68);

	if (g_HudDeathNoticeHookGlobals.activeWrapper)
	{
		*(float *)((BYTE *)this_ptr + 0x58) = g_HudDeathNoticeHookGlobals.activeWrapper->lifetime.value;
		*(float *)((BYTE *)this_ptr + 0x68) = g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.value;
	}
	else
	{
		*(float *)((BYTE *)this_ptr + 0x58) = org_CHudDeathNotice_nNoticeLifeTime;
		*(float *)((BYTE *)this_ptr + 0x68) = org_CHudDeathNotice_nLocalPlayerLifeTimeMod;
	}
}

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

			if (e.lastRule) break;
		}
	}

	int curAttackerId = myWrapper.GetInt("attacker", 0);
	int curUserId = myWrapper.GetInt("userid", 0);
	int curAssisterId = myWrapper.GetInt("assister", 0);

	if (g_HudDeathNoticeHookGlobals.useHighlightId)
	{

		myWrapper.attacker.isLocal.use = true;
		myWrapper.attacker.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(curAttackerId);

		myWrapper.victim.isLocal.use = true;
		myWrapper.victim.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(curUserId);

		myWrapper.assister.isLocal.use = true;
		myWrapper.assister.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(curAssisterId);
	}

	if (!(myWrapper.block.use && myWrapper.block.value) ) {

		while (!g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.pop();
		}

		if (curAttackerId) g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.push(1);
		if (curUserId) g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.push(2);
		if (curAssisterId) g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.push(3);

		while(!g_HudDeathNoticeHookGlobals.nextRealEntindexState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealEntindexState.pop();
		}

		if (curAttackerId) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(2);
		if (curUserId) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(1);

		if (!g_Adresses_ClientIsPanorama)
		{
			SetLifetimeClassValues((DWORD)This);
		}

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

	CCsgoReplaceNameEntry(const char * id, const char * name)
	{
		this->id = id;
		this->name = name;
	}
};

std::list<CCsgoReplaceNameEntry> g_csgo_ReplaceNameList;

int __fastcall Mycsgo_VEngineClient_GetPlayerForUserID(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int userID)
{
	if (!g_HudDeathNoticeHookGlobals.activeWrapper)
		return Truecsgo_VEngineClient_GetPlayerForUserID(This, edx, userID);

	g_HudDeathNoticeHookGlobals.inPlayerInfoCall = true;

	int result = Truecsgo_VEngineClient_GetPlayerForUserID(This, edx, userID);

	g_HudDeathNoticeHookGlobals.inPlayerInfoCall = false;

	return result;
}



bool __fastcall Mycsgo_VEngineClient_GetPlayerInfo(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int ent_num, SOURCESDK::player_info_t_csgo *pinfo) {

	if(g_HudDeathNoticeHookGlobals.inPlayerInfoCall)
		return Truecsgo_VEngineClient_GetPlayerInfo(This, edx, ent_num, pinfo);

	MyDeathMsgPlayerEntry * playerEntry = nullptr;

	if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.empty())
	{
		int state = g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.front();
		g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.pop();

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

	bool result = Truecsgo_VEngineClient_GetPlayerInfo(This, edx, ent_num, pinfo);

	if (pinfo)
	{
		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplaceNameList.begin(); it != g_csgo_ReplaceNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if ((e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo->userID) || (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo->xuid))
			{
				strcpy_s(pinfo->name, e.name.c_str());
				break;
			}
		}

		if (playerEntry && playerEntry->name.use) {
			strcpy_s(pinfo->name, playerEntry->name.value);
		}
	}

	return result;
}

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
					return g_HudDeathNoticeHookGlobals.activeWrapper->attacker.isLocal.value ? Truecsgo_VEngineClient_GetPlayerForUserID(g_VEngineClient->GetVEngineClient_csgo(), 0, g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("attacker", 0)) : 0;
				break;
			case 2:
				if (g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.use)
					return g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.value ? Truecsgo_VEngineClient_GetPlayerForUserID(g_VEngineClient->GetVEngineClient_csgo(), 0, g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("userid", 0)) : 0;
				break;
			}
		}
	}

	return Truecsgo_C_CSPlayer_IClientNetworkable_entindex(This, edx);
}


// >>> ScalfromUI only >>> /////////////////////////////////////////////////////
//

typedef wchar_t * (__stdcall *csgo_CUnknown_GetPlayerName_t)(DWORD *this_ptr, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1);

csgo_CUnknown_GetPlayerName_t detoured_csgo_CUnknown_GetPlayerName;

wchar_t * __stdcall touring_csgo_CUnknown_GetPlayerName(DWORD *this_ptr, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1)
{
	wchar_t * result = detoured_csgo_CUnknown_GetPlayerName(this_ptr, entIndex, targetBuffer, targetByteCount, unknownArg1);

	SOURCESDK::player_info_t_csgo pinfo;
	MyDeathMsgPlayerEntry * playerEntry = nullptr;

	if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.empty())
	{
		int state = g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.front();
		g_HudDeathNoticeHookGlobals.nextRealPlayerInfoState.pop();

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
		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplaceNameList.begin(); it != g_csgo_ReplaceNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if ((e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo.userID) || (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo.xuid))
			{
				std::wstring widePlayerName;
				if (UTF8StringToWideString(it->name.c_str(), widePlayerName))
				{
					wcscpy_s(targetBuffer, targetByteCount, widePlayerName.c_str());
				}
				break;
			}
		}

		if (playerEntry && playerEntry->name.use) {
			std::wstring widePlayerName;
			if (UTF8StringToWideString(playerEntry->name.value, widePlayerName))
			{
				wcscpy_s(targetBuffer, targetByteCount, widePlayerName.c_str());
			}
		}
	}

	return result;
}

bool csgo_GetPlayerName_Install_Scaleform(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CUnknown_GetPlayerName))
	{
		detoured_csgo_CUnknown_GetPlayerName = (csgo_CUnknown_GetPlayerName_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CUnknown_GetPlayerName), (BYTE *)touring_csgo_CUnknown_GetPlayerName, (int)AFXADDR_GET(csgo_CUnknown_GetPlayerName_DSZ));

		firstResult = true;
	}

	return firstResult;
}

//
// <<< ScaleformUI only <<< ////////////////////////////////////////////////////

bool csgo_ReplaceName_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	SOURCESDK::IVEngineClient_014_csgo * nativeEngineClient;

	if (
		g_VEngineClient
		&& (nativeEngineClient = g_VEngineClient->GetVEngineClient_csgo())
		&& (g_Adresses_ClientIsPanorama || csgo_GetPlayerName_Install_Scaleform())
	) {
		LONG error = NO_ERROR;

		Truecsgo_VEngineClient_GetPlayerInfo = (csgo_VEngineClient_GetPlayerInfo_t)(*(void **)((*(char **)(nativeEngineClient)) + sizeof(void *) * 8));

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_VEngineClient_GetPlayerInfo, Mycsgo_VEngineClient_GetPlayerInfo);
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
			if (g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.use) value = g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.value;
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
	
	SOURCESDK::IVEngineClient_014_csgo * nativeEngineClient;

	if(
		csgo_ReplaceName_Install()
		&& g_VEngineClient
		&& (nativeEngineClient = g_VEngineClient->GetVEngineClient_csgo())
		&& AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent)
		&& AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp)
		&& AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex)
	)
	{
		LONG error = NO_ERROR;

		Truecsgo_VEngineClient_GetPlayerForUserID = (csgo_VEngineClient_GetPlayerForUserID_t)(*(void **)((*(char **)(nativeEngineClient)) + sizeof(void *) * 9));
		TrueCCSGO_HudDeathNotice_FireGameEvent = (CCSGO_HudDeathNotice_FireGameEvent_t)AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent);
		Truecsgo_panorama_CUIPanel_UnkSetFloatProp = (csgo_panorama_CUIPanel_UnkSetFloatProp_t)AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp);
		Truecsgo_C_CSPlayer_IClientNetworkable_entindex = (csgo_C_CSPlayer_IClientNetworkable_entindex_t)AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_VEngineClient_GetPlayerForUserID, Mycsgo_VEngineClient_GetPlayerForUserID);
		DetourAttach(&(PVOID&)TrueCCSGO_HudDeathNotice_FireGameEvent, MyCCSGO_HudDeathNotice_FireGameEvent);
		DetourAttach(&(PVOID&)Truecsgo_panorama_CUIPanel_UnkSetFloatProp, Mycsgo_panorama_CUIPanel_UnkSetFloatProp);
		DetourAttach(&(PVOID&)Truecsgo_C_CSPlayer_IClientNetworkable_entindex, Mycsgo_C_CSPlayer_IClientNetworkable_entindex);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

bool csgo_CHudDeathNotice_Install_Scaleform(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	SOURCESDK::IVEngineClient_014_csgo * nativeEngineClient;

	if (
		csgo_ReplaceName_Install()
		&& g_VEngineClient
		&& (nativeEngineClient = g_VEngineClient->GetVEngineClient_csgo())
		&& AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent)
		&& AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex)
		)
	{
		LONG error = NO_ERROR;

		Truecsgo_VEngineClient_GetPlayerForUserID = (csgo_VEngineClient_GetPlayerForUserID_t)(*(void **)((*(char **)(nativeEngineClient)) + sizeof(void *) * 9));
		TrueCCSGO_HudDeathNotice_FireGameEvent = (CCSGO_HudDeathNotice_FireGameEvent_t)AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent);
		Truecsgo_C_CSPlayer_IClientNetworkable_entindex = (csgo_C_CSPlayer_IClientNetworkable_entindex_t)AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_VEngineClient_GetPlayerForUserID, Mycsgo_VEngineClient_GetPlayerForUserID);
		DetourAttach(&(PVOID&)TrueCCSGO_HudDeathNotice_FireGameEvent, MyCCSGO_HudDeathNotice_FireGameEvent);
		DetourAttach(&(PVOID&)Truecsgo_C_CSPlayer_IClientNetworkable_entindex, Mycsgo_C_CSPlayer_IClientNetworkable_entindex);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

bool csgo_CHudDeathNotice_Install(void)
{
	return g_Adresses_ClientIsPanorama ? csgo_CHudDeathNotice_Install_Panorama() : csgo_CHudDeathNotice_Install_Scaleform();
}

void Console_DeathMsgId_PrintHelp(const char * cmd)
{
	Tier0_Msg(
		"%s accepts the following as <id...>:\n"
		"<iNumber> - UserID, Example: 2\n"
		"trace - UserID from a screen trace (i.e. current POV).\n"
		"x<iNumber> - XUID, Example: x76561197961927915\n"
		"xTrace - XUID from a screen trace (i.e. current POV).\n"
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
		"\t\"assisterIsLocal=(0|1)\" - If to be considered local player.\n"
		"%s"
		"\t\"victimName=<sName>\" - New victim name.\n"
		"\t\"victimId=<id>\" - New victim id.\n"
		"\t\"victimIsLocal=(0|1)\" - If to be considered local player.\n"
		"\t\"weapon=<sWeaponName>\" - Weapon name (i.e. ak47).\n"
		"\t\"headshot=<iVal>\" - If headshot.\n"
		"\t\"penetrated=<iVal>\" - If penetrated.\n"
		"\t\"dominated=<iVal>\" - If dominated.\n"
		"\t\"revenge=<iVal>\" - If revenge.\n"
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
							Tier0_Msg("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						if (targetNr < 0 || targetNr > (int)g_HudDeathNoticeHookGlobals.Filter.size())
						{
							Tier0_Msg("Error: %i is not in valid range for <tragetNr>\n", targetNr);
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
							Tier0_Msg("Error: %i is not in valid range for <listNr>\n", listNr);
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
							Tier0_Msg("Error: %i is not in valid range for <listNr>\n", listNr);
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
				Tier0_Warning("One message must have been triggerred first, before this will work.\n");
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
		else if (0 == _stricmp("debug", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				g_HudDeathNoticeHookGlobals.Settings.Debug = atoi(arg2);

				return true;
			}

			Tier0_Msg(
				"%s localPlayer default|<id>\n"
				"Current value: %i\n"
				, arg0
				, g_HudDeathNoticeHookGlobals.Settings.Debug
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
		"%s debug [...] - Enable / Disable debug spew upon death messages.\n"
		"Hint: Jump back in demo (i.e. to round start) to clear death messages.\n"
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

bool csgo_ReplaceName_Console(IWrpCommandArgs * args) {

	if (!csgo_ReplaceName_Install())
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


						g_csgo_ReplaceNameList.emplace_back(arg3, arg4);
						return true;
					}

					Tier0_Msg(
						"%s filter add <idUser> <sNewName>\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("add", arg2)) {

					if (6 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						int nr = 0;
						int targetNr = atoi(arg3);
						CCsgoReplaceNameEntry * entry = nullptr;

						for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplaceNameList.begin(); it != g_csgo_ReplaceNameList.end(); ++it)
						{
							if (nr == targetNr)
							{
								entry = &(*it);
								break;
							}

							++nr;
						}

						if (entry)
						{

							const char * arg4 = args->ArgV(4);
							const char * arg5 = args->ArgV(5);

							if (0 == _stricmp("id", arg4))
							{
								entry->id = atoi(arg5);
								return true;
							}
							else if (0 == _stricmp("name", arg4))
							{
								entry->name = arg5;
								return true;
							}
						}
						else {
							Tier0_Warning(
								"Error: %i is not a valid %s filter list entry."
								, targetNr
								, arg0
							);
							return true;
						}
					}

					Tier0_Msg(
						"%s filter edit <listNr> id <idUser> - Set new ID.\n"
						"%s filter edit <listNr> name <sNewName> - Set new name.\n"
						, arg0
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("move", arg2)) {

					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						int listNr = atoi(arg3);
						int targetNr = atoi(arg4);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplaceNameList.size())
						{
							Tier0_Msg("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						if (targetNr < 0 || targetNr > (int)g_csgo_ReplaceNameList.size())
						{
							Tier0_Msg("Error: %i is not in valid range for <tragetNr>\n", targetNr);
							return true;
						}

						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplaceNameList.begin();

						std::list<CCsgoReplaceNameEntry>::iterator targetIt = g_csgo_ReplaceNameList.begin();

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

						g_csgo_ReplaceNameList.splice(targetIt, g_csgo_ReplaceNameList, sourceIt);

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

						if (listNr < 0 || listNr >= (int)g_csgo_ReplaceNameList.size())
						{
							Tier0_Msg("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}
						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplaceNameList.begin();

						std::advance(sourceIt, listNr);

						g_csgo_ReplaceNameList.erase(sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter remove <listNr> - Remove entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("clear", arg2)) {

					g_csgo_ReplaceNameList.clear();
				
					return true;
				}			
				else if (0 == _stricmp("print", arg2)) {

					Tier0_Msg("nr: id, name\n");

					int nr = 0;

					for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplaceNameList.begin(); it != g_csgo_ReplaceNameList.end(); ++it)
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
	}


	Tier0_Msg(
		"%s filter [...] - Filter names.\n"
		"%s help id [...]- Print help on <id...> usage.\n"
		"Please note: The engine might cache names, so set th filter up early enough!\n"
		, arg0
		, arg0
	);
	return true;
}