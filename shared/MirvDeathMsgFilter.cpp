#include "MirvDeathMsgFilter.h"

bool MirvDeathMsg_Console(IWrpCommandArgs * args)
{
	return true;
};

void Console_DeathMsgArgs_PrintHelp(const char * cmd, bool showMatch)
{
	if (nullptr == conMessage) {
		return;
	}

	conMessage(
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
		"\t\"weapon=<sWeaponName>\" - Weapon name (e.g. ak47).\n"
		"\t\"headshot=<iVal>\" - If headshot.\n"
		"\t\"penetrated=<iVal>\" - If penetrated.\n"
		"\t\"dominated=<iVal>\" - If dominated.\n"
		"\t\"revenge=<iVal>\" - If revenge.\n"
		"\t\"wipe=<iVal>\" - Squad wipeout in Danger Zone(?).\n"
		"\t\"noscope=<iVal>\" - If noscope.\n"
		"\t\"thrusmoke=<iVal>\" - If thrusmoke.\n"
		"\t\"attackerblind=<iVal>\" - If attackerblind.\n"
		#ifdef GAME_CS2
		"\t\"attackerinair=<iVal>\" - If attackerinair.\n"
		#endif
		"\t\"lifetime=<fVal>\" - Life time in seconds.\n"
		"\t\"lifetimeMod=<fVal>\" - Life time modifier (for player considered to be local player).\n"
		"\t\"block=<iVal>\" - If to block this message (0 = No, 1 = Yes).\n"
		"\t\"lastRule=<iVal>\" - If this is the last rule to be applied (abort after this one), 0 = No, 1 = Yes.\n"
		, cmd
		, showMatch ? "\t\"attackerMatch=<matchExpr>\" - The attacker to match.\n" : ""
		, showMatch ? "\t\"assisterMatch=<matchExpr>\" - The assister to match.\n" : ""
		, showMatch ? "\t\"victimMatch=<matchExpr>\" - The victim to match.\n" : ""
	);
};

bool MirvDeathMsg::filter(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	if (nullptr == conMessage || nullptr == conWarning) {
		return false;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);
	if (3 <= argc)
	{
		const char * arg2 = args->ArgV(2);

		if (0 == _stricmp("add", arg2)) {

			if (4 <= argc)
			{
				CSubWrpCommandArgs subArgs(args, 3);

				filterGlobals.Filter.emplace_back(&subArgs);
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

				if (listNr < 0 || listNr >= (int)filterGlobals.Filter.size())
				{
					conWarning("Error: %i is not in valid range for <listNr>\n", listNr);
					return true;
				}

				if (targetNr < 0 || targetNr > (int)filterGlobals.Filter.size())
				{
					conWarning("Error: %i is not in valid range for <tragetNr>\n", targetNr);
					return true;
				}

				std::list<DeathMsgFilterEntry>::iterator sourceIt = filterGlobals.Filter.begin();

				std::list<DeathMsgFilterEntry>::iterator targetIt = filterGlobals.Filter.begin();

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

				filterGlobals.Filter.splice(targetIt, filterGlobals.Filter, sourceIt);

				return true;
			}

			conMessage(
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

				if (listNr < 0 || listNr >= (int)filterGlobals.Filter.size())
				{
					conWarning("Error: %i is not in valid range for <listNr>\n", listNr);
					return true;
				}
				std::list<DeathMsgFilterEntry>::iterator sourceIt = filterGlobals.Filter.begin();

				std::advance(sourceIt, listNr);

				CSubWrpCommandArgs subArgs(args, 4);

				sourceIt->Console_Edit(&subArgs);

				return true;
			}

			conMessage(
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

				if (listNr < 0 || listNr >= (int)filterGlobals.Filter.size())
				{
					conWarning("Error: %i is not in valid range for <listNr>\n", listNr);
					return true;
				}
				std::list<DeathMsgFilterEntry>::iterator sourceIt = filterGlobals.Filter.begin();

				std::advance(sourceIt, listNr);

				filterGlobals.Filter.erase(sourceIt);

				return true;
			}

			conMessage(
				"%s filter remove <listNr> - Remove entry in list.\n"
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("clear", arg2)) {

			filterGlobals.Filter.clear();

			return true;
		}
		else if (0 == _stricmp("print", arg2)) {

			conMessage("nr: id, name\n");

			int nr = 0;

			for (std::list<DeathMsgFilterEntry>::iterator it = filterGlobals.Filter.begin(); it != filterGlobals.Filter.end(); ++it)
			{
				conMessage(
					"%i: "
					, nr
				);

				it->Console_Print();

				conMessage("\n");

				++nr;
			}
			conMessage("---- EOL ----\n");

			return true;
		}
	}

	conMessage(
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

};

bool MirvDeathMsg::lifetime(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	CSubWrpCommandArgs subArgs(args, 2);
	filterGlobals.Lifetime.Console_Edit(&subArgs);
	return true;
};

bool MirvDeathMsg::lifetimeMod(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	CSubWrpCommandArgs subArgs(args, 2);
	filterGlobals.LifetimeMod.Console_Edit(&subArgs);
	return true;
};

bool MirvDeathMsg::localPlayer(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	if (nullptr == conMessage || nullptr == conWarning) {
		return false;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (3 <= argc)
	{
		const char * arg2 = args->ArgV(2);

		if (0 == _stricmp("default", arg2))
		{
			filterGlobals.useHighlightId = false;
		}
		else
		{
			filterGlobals.useHighlightId = true;
			filterGlobals.highlightId = arg2;
		}
		
		return true;
	}

	conMessage(
		"%s localPlayer default|<userid>|<xuid>|<specKey>|<trace>|<xTrace>\n"
		"Current value: "
		, arg0
	);
	if (filterGlobals.useHighlightId)
	{
		filterGlobals.highlightId.Console_Print();
	}
	else
	{
		conMessage("default");
	}
	conMessage("\n");
	return true;
};

bool MirvDeathMsg::debug(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	if (nullptr == conMessage || nullptr == conWarning) {
		return false;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (3 <= argc)
	{
		const char * arg2 = args->ArgV(2);

		filterGlobals.Settings.Debug = atoi(arg2);

		return true;
	}

	conMessage(
		"%s debug 0|1\n"
		"Current value: %i\n"
		, arg0
		, filterGlobals.Settings.Debug
	);
	return true;

};

bool MirvDeathMsg::showNumbers(IWrpCommandArgs * args, MirvDeathMsgGlobals &filterGlobals)
{
	if (nullptr == conMessage || nullptr == conWarning) {
		return false;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (3 <= argc)
	{
		int value = atoi(args->ArgV(2));

		if (value <= 0)
		{
			filterGlobals.showNumbers = MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::Default;
		}
		else if(1 == value)
		{
			filterGlobals.showNumbers = MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::Numbers;
		}
		else
		{
			filterGlobals.showNumbers = MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::PrependNumbers;
		}
		
		return true;
	}

	conMessage(
		"%s showNumbers 0|1|2 - Default (0), only numbers (1), prepend numbers (2)\n"
		"Current value: %i\n"
		, arg0
		, filterGlobals.showNumbers
	);			
	return true;

};