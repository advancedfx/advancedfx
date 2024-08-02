#pragma once

#include "AfxConsole.h"
#include "StringTools.h"

#include <list>

typedef advancedfx::ICommandArgs IWrpCommandArgs;
typedef advancedfx::CSubCommandArgs CSubWrpCommandArgs;

extern advancedfx::Con_Printf_t conMessage;
extern advancedfx::Con_Printf_t conWarning;



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

	void operator=(char const * consoleValue);

	int ResolveToUserId();

	void Console_Print()
	{
		switch(Mode)
		{
		case Id_Key:
			conMessage("k%i", Id.specKey);
			break;
		case Id_Xuid:
			{
				conMessage("x%lld", Id.xuid);
			}
			break;
		case Id_UserId:
		default:
			conMessage("%i", Id.userId);
			break;
		}
	};

	bool EqualsUserId(int userId);
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
				conMessage("Error: Could not set %s!\n", arg1);
			return;
		}

		conMessage(
			"%s 0|1|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		conMessage("\n");
	}

	void Console_Print() {
		if (!use) conMessage("default");
		else conMessage("%i", value ? 1 : 0);
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
				conWarning("Error: Could not set %s!\n", arg1);
			return;
		}

		conMessage(
			"%s <iValue>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		conMessage("\n");
	}

	void Console_Print() {
		if (!use) conMessage("default");
		else conMessage("%i", value);
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
				conWarning("Error: Could not set %s!\n", arg1);
			return;
		}

		conMessage(
			"%s <fValue>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		conMessage("\n");
	}

	void Console_Print() {
		if (!use) conMessage("default");
		else conMessage("%f", value);
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
				conWarning("Error: Could not set %s!\n", arg1);
			return;
		}

		conMessage(
			"%s <id>|default\n"
			"Current value: "
			, arg0
		);
		Console_Print();
		conMessage("\n");
	}

	void Console_Print() {
		if (!use) conMessage("default");
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
	};

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
					conWarning("Error: Could not set %s!\n", arg1);
					return;
				}
			}

			conMessage(
				"%s <strValue>|(set default)\n"
				"Current value: "
				, arg0
			);
			Console_Print();
			conMessage("\n");
		}

		void Console_Print() {
			if (!use) conMessage("default");
			else conMessage("\"%s\"", value.c_str());
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

			conMessage(
				"%s *|<id>|!<id>\n"
				"Current value: "
				, arg0
			);
			Console_MatchPrint();
			conMessage("\n");
		}

		void Console_MatchPrint() {
			if (DMBM_ANY == mode)
				conMessage("*");
			else
			{
				if (DMBM_EXCEPT == mode)
				{
					conMessage("!");
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

	#ifdef GAME_CS2
		MyDeathMsgIntEntry attackerinair;
	#endif

	MyDeathMsgFloatEntry lifetime;

	MyDeathMsgFloatEntry lifetimeMod;

	MyDeathMsgBoolEntry block;

	bool lastRule = false;

	void Console_Print()
	{
		conMessage("attackerMatch=");
		attacker.Console_MatchPrint();
		conMessage(" assisterMatch=");
		assister.Console_MatchPrint();
		conMessage(" victimMatch=");
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
			#ifdef GAME_CS2
			else if (StringIBeginsWith(argI, "attackerinair="))
			{
				attackerinair.Console_Set(argI + strlen("attackerinair="));
			}
			#endif
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
				conWarning("Error: invalid option \"%s\".\n", argI);
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
			#ifdef GAME_CS2
			else if (0 == _stricmp("attackerinair", arg1))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				attackerinair.Console_Edit(args);
				return;
			}
			#endif
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

				conMessage(
					"%s lastRule 0|1\n"
					"Current value: %i"
					, arg0
					, lastRule ? 1 : 0
				);
				return;
			}
		}

		conMessage("%s attackerMatch [...] = ", arg0);
		attacker.Console_MatchPrint();
		conMessage("\n");

		conMessage("%s attackerName [...] = ", arg0);
		attacker.name.Console_Print();
		conMessage("\n");

		conMessage("%s attackerId [...] = ", arg0);
		attacker.newId.Console_Print();
		conMessage("\n");

		conMessage("%s attackerIsLocal [...] = ", arg0);
		attacker.isLocal.Console_Print();
		conMessage("\n");

		conMessage("%s assisterMatch [...] = ", arg0);
		assister.Console_MatchPrint();
		conMessage("\n");

		conMessage("%s assisterName [...] = ", arg0);
		assister.name.Console_Print();
		conMessage("\n");

		conMessage("%s assisterId [...] = ", arg0);
		assister.newId.Console_Print();
		conMessage("\n");

		//Tier0_Msg("%s assisterIsLocal [...] = ", arg0);
		//assister.isLocal.Console_Print();
		//Tier0_Msg("\n");

		conMessage("%s victimMatch [...] = ", arg0);
		victim.Console_MatchPrint();
		conMessage("\n");

		conMessage("%s victimName [...] = ", arg0);
		victim.name.Console_Print();
		conMessage("\n");

		conMessage("%s victimId [...] = ", arg0);
		victim.newId.Console_Print();
		conMessage("\n");

		conMessage("%s victimIsLocal [...] = ", arg0);
		victim.isLocal.Console_Print();
		conMessage("\n");

		conMessage("%s assistedflash [...] = ", arg0);
		assistedflash.Console_Print();
		conMessage("\n");

		conMessage("%s weapon [...] = ", arg0);
		weapon.Console_Print();
		conMessage("\n");

		conMessage("%s headshot [...] = ", arg0);
		headshot.Console_Print();
		conMessage("\n");

		conMessage("%s penetrated [...] = ", arg0);
		penetrated.Console_Print();
		conMessage("\n");

		conMessage("%s dominated [...] = ", arg0);
		dominated.Console_Print();
		conMessage("\n");

		conMessage("%s revenge [...] = ", arg0);
		revenge.Console_Print();
		conMessage("\n");

		conMessage("%s wipe [...] = ", arg0);
		wipe.Console_Print();
		conMessage("\n");

		conMessage("%s noscope [...] = ", arg0);
		noscope.Console_Print();
		conMessage("\n");

		conMessage("%s thrusmoke [...] = ", arg0);
		thrusmoke.Console_Print();
		conMessage("\n");

		conMessage("%s attackerblind [...] = ", arg0);
		attackerblind.Console_Print();
		conMessage("\n");

		#ifdef GAME_CS2
		conMessage("%s attackerinair [...] = ", arg0);
		attackerinair.Console_Print();
		conMessage("\n"); 
		#endif

		conMessage("%s lifetime [...] = ", arg0);
		lifetime.Console_Print();
		conMessage("\n");

		conMessage("%s lifetimeMod [...] = ", arg0);
		lifetimeMod.Console_Print();
		conMessage("\n");

		conMessage("%s block [...] = ", arg0);
		block.Console_Print();
		conMessage("\n");

		conMessage("%s lastRule [...] = %i\n", arg0, lastRule ? 1 : 0);
	}
};

class MyDeathMsgGameEventWrapperBase
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

	#ifdef GAME_CS2
	MyDeathMsgIntEntry attackerinair;
	#endif

	MyDeathMsgFloatEntry lifetime;

	MyDeathMsgFloatEntry lifetimeMod;

	MyDeathMsgBoolEntry block;

	MyDeathMsgGameEventWrapperBase(){}

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
		#ifdef GAME_CS2
		ApplyIntEntry(dme.attackerinair, attackerinair);
		#endif
		ApplyFloatEntry(dme.lifetime, lifetime);
		ApplyFloatEntry(dme.lifetimeMod, lifetimeMod);
		ApplyBoolEntry(dme.block, block);
	}

private:

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

struct MirvDeathMsgGlobals {

	enum class DeathnoticeShowNumbers_e : int
	{
		Default = 0,
		Numbers = 1,
		PrependNumbers = 2
	};

	struct CSettings {
		int Debug = 0;
	} Settings;

	std::list<DeathMsgFilterEntry> Filter;

	MyDeathMsgFloatEntry Lifetime;

	MyDeathMsgFloatEntry LifetimeMod;

	bool useHighlightId;
	DeathMsgId highlightId;

	DeathnoticeShowNumbers_e showNumbers;

};

void Console_DeathMsgArgs_PrintHelp(const char * cmd, bool showMatch);

struct MirvDeathMsg
{
	bool filter(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
	bool lifetime(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
	bool lifetimeMod(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
	bool localPlayer(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
	bool debug(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
	bool showNumbers(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals);
};
