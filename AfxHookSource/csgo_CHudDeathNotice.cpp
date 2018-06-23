#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "RenderView.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <Windows.h>
#include <shared/Detours/src/detours.h>

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
- [LifetimeMod]
".?AVCUIPanel@panorama@@" (2nd ref) [thiscall 282] UnkSetFloatProp(word propId, float value)


*/

extern WrpVEngineClient * g_VEngineClient;



typedef bool(__fastcall * csgo_VEngineClient_GetPlayerInfo_t)(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int ent_num, SOURCESDK::player_info_t_csgo *pinfo);
csgo_VEngineClient_GetPlayerInfo_t Truecsgo_VEngineClient_GetPlayerInfo = 0;

typedef int(__fastcall * csgo_VEngineClient_GetPlayerForUserID_t)(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int userID);
csgo_VEngineClient_GetPlayerForUserID_t Truecsgo_VEngineClient_GetPlayerForUserID = 0;

typedef int(__fastcall * csgo_IClientNetworkable_entindex_t)(SOURCESDK::IClientNetworkable_csgo * This, void * edx);
csgo_IClientNetworkable_entindex_t Truecsgo_IClientNetworkable_entindex = 0;

typedef void CCSGO_HudDeathNotice_t;
typedef void (__fastcall *CCSGO_HudDeathNotice_FireGameEvent_t)(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event);
CCSGO_HudDeathNotice_FireGameEvent_t TrueCCSGO_HudDeathNotice_FireGameEvent = 0;
CCSGO_HudDeathNotice_t * CCSGO_HudDeathNotice_FireGameEvent_This = 0;


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

struct MyDeathMsgPlayerEntry
{
	MyDeathMsgCStringEntry name;

	MyDeathMsgIntEntry newId;

	MyDeathMsgBoolEntry isLocal;
};

struct DeathMsgFilterEntry
{
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
		DeathMsgIdMatchMode mode = DMBM_EQUAL;
		DeathMsgId id = (int)0;

		StringEntry name;

		MyDeathMsgIntEntry newId;

		MyDeathMsgBoolEntry isLocal;

		void Console_MatchEdit(IWrpCommandArgs * args) {
			int argc = args->ArgC();
			char const * arg0 = args->ArgV(0);

			if (2 <= argc)
			{
				char const * arg1 = args->ArgV(1);
				bool any = !strcmp("*", arg1);
				bool not = StringBeginsWith(arg1, "!");
				
				if (!any) id = not ? (arg1 + 1) : arg1;

				mode = any ? DMBM_ANY : (not ? DMBM_EXCEPT : DMBM_EQUAL);

				return;
			}

			Tier0_Msg(
				"%s *|(!)?(<id>|x<id>)\n"
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

	bool lastRule;

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

		Tier0_Msg("%s assisterMatch [...] = ", arg0);
		assister.Console_MatchPrint();
		Tier0_Msg("\n");

		Tier0_Msg("%s assisterName [...] = ", arg0);
		assister.name.Console_Print();
		Tier0_Msg("\n");

		Tier0_Msg("%s assisterId [...] = ", arg0);
		assister.newId.Console_Print();
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

		if (attacker.newId.use && 0 == strcmp("attacker", keyName)) return attacker.newId.value;
		if (victim.newId.use && 0 == strcmp("userid", keyName)) return victim.newId.value;
		if (assister.newId.use && 0 == strcmp("assister", keyName)) return assister.newId.value;
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
		ApplyIntEntry(source.newId, target.newId);
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

} g_HudDeathNoticeHookGlobals;

void __fastcall MyCCSGO_HudDeathNotice_FireGameEvent(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event)
{
	CCSGO_HudDeathNotice_FireGameEvent_This = This;

	MyDeathMsgGameEventWrapper myWrapper(event);

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
		Tier0_Msg("CHudDeathNotice::FireGameEvent: uidAttaker=%i, uidVictim=%i, uidAssister=%i\n", uidAttacker, uidVictim, uidAssister);
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

	if (g_HudDeathNoticeHookGlobals.useHighlightId)
	{
		myWrapper.attacker.isLocal.use = true;
		myWrapper.attacker.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(myWrapper.GetInt("attacker", 0));

		myWrapper.assister.isLocal.use = true;
		myWrapper.assister.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(myWrapper.GetInt("assister", 0));

		myWrapper.victim.isLocal.use = true;
		myWrapper.victim.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(myWrapper.GetInt("userid", 0));
	}

	if(!myWrapper.block.value) TrueCCSGO_HudDeathNotice_FireGameEvent(This, edx, &myWrapper);

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

bool __fastcall Mycsgo_VEngineClient_GetPlayerInfo(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int ent_num, SOURCESDK::player_info_t_csgo *pinfo) {

	bool result =  Truecsgo_VEngineClient_GetPlayerInfo(This, edx, ent_num, pinfo);

	//if (pinfo) strcpy_s(pinfo->name, "Test");
	
	return result;
}

int __fastcall Mycsgo_VEngineClient_GetPlayerForUserID(SOURCESDK::IVEngineClient_014_csgo * This, void * edx, int userID) {

	int result = Truecsgo_VEngineClient_GetPlayerForUserID(This, edx, userID);

	return result;
}

int __fastcall Mycsgo_IClientNetworkable_entindex_t(SOURCESDK::IClientNetworkable_csgo * This, void * edx) {

	int result = Truecsgo_IClientNetworkable_entindex(This, edx);

	return result;
}

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

bool csgo_CHudDeathNotice_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	SOURCESDK::IVEngineClient_014_csgo * nativeEngineClient;

	if(
		g_VEngineClient
		&& (nativeEngineClient = g_VEngineClient->GetVEngineClient_csgo())
		&& csgo_ReplaceName_Install()
		&& AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent)
	)
	{
		LONG error = NO_ERROR;

		TrueCCSGO_HudDeathNotice_FireGameEvent = (CCSGO_HudDeathNotice_FireGameEvent_t)AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent);
		Truecsgo_VEngineClient_GetPlayerForUserID = (csgo_VEngineClient_GetPlayerForUserID_t)(*(void **)((*(char **)(nativeEngineClient)) + sizeof(void *) * 9));

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueCCSGO_HudDeathNotice_FireGameEvent, MyCCSGO_HudDeathNotice_FireGameEvent);
		DetourAttach(&(PVOID&)Truecsgo_VEngineClient_GetPlayerInfo, Mycsgo_VEngineClient_GetPlayerInfo);
		DetourAttach(&(PVOID&)Truecsgo_VEngineClient_GetPlayerForUserID, Mycsgo_VEngineClient_GetPlayerForUserID);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

bool csgo_CHudDeathNotice_Console(IWrpCommandArgs * args)
{
	if (!csgo_CHudDeathNotice_Install())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	return true;

	Tier0_Msg(
		"mirv_deathmsg accepts the following as <id...>:\n"
		"<number> - UserID, Example: 2\n"
		"x<number> - XUID, Example: x76561197961927915\n"
		"We recommend getting the numbers from the output of \"mirv_listentities isPlayer=1\".\n"
	);
}

bool csgo_ReplaceName_Console(IWrpCommandArgs * args) {
	csgo_ReplaceName_Install();

	Tier0_Warning("Being coded atm, sorry.");

	return true;
}