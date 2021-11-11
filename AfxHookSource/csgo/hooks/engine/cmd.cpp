#include "stdafx.h"

#include "cmd.h"

#include "SourceInterfaces.h"
#include <csgo/sdk_src/public/tier1/convar.h>

#include "WrpVEngineClient.h"
#include "WrpConsole.h"

#include "addresses.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <map>
#include <list>
#include <regex>

std::map<int, std::list<std::string>> g_CmdExecuteCommand_Blocks;
int g_DebugExecuteCommand = 0;

void * g_Org_csgo_Cmd_ExecuteCommand;

__declspec(naked) SOURCESDK::CSGO::ConCommandBase* __fastcall  My_csgo_Cmd_ExecuteCommand(int eTarget, const SOURCESDK::CSGO::CCommand& command) {

	__asm push ebp
	__asm mov ebp, esp
	__asm sub esp, __LOCAL_SIZE
	__asm mov eTarget, ecx
	__asm mov command, edx

	if (command.ArgC()) {
		if (g_DebugExecuteCommand) {
			Tier0_Msg("Cmd_ExecuteCommand: %i -> \"%s\"\n", (int)(command.Source()), command.GetCommandString());
		}

		int cmd_source = (int)(command.Source());

		auto it = g_CmdExecuteCommand_Blocks.find(cmd_source);
		if (it != g_CmdExecuteCommand_Blocks.end()) {
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				if (StringWildCard1Matched(it2->c_str(), command.GetCommandString())) {
					if (g_DebugExecuteCommand) {
						Tier0_Msg("BLOCKED: Cmd_ExecuteCommand: %i -> \"%s\"\n", (int)(command.Source()), command.GetCommandString());
					}
					__asm mov edx, command
					__asm mov ecx, eTarget
					__asm mov esp, ebp
					__asm pop ebp
					__asm mov eax, 0
					__asm ret
				}
					
			}
		}

		if (command.Source() == SOURCESDK::CSGO::kCommandSrcDemoFile && WrpConCommands::IsRegisteredSlow(command.ArgV()[0])) {
			Tier0_Warning("AFXWARNING: BLOCKED HLAE COMMAND FROM DEMO STREAM: \"%s\"\n", command.GetCommandString());
			__asm mov edx, command
			__asm mov ecx, eTarget
			__asm mov esp, ebp
			__asm pop ebp
			__asm mov eax, 0
			__asm ret
		}
	}

	__asm mov edx, command
	__asm mov ecx, eTarget
	__asm mov esp, ebp
	__asm pop ebp
	__asm jmp [g_Org_csgo_Cmd_ExecuteCommand]
}


bool Install_csgo_Cmd_ExecuteCommand(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_engine_Cmd_ExecuteCommand))
	{
		LONG error = NO_ERROR;

		g_Org_csgo_Cmd_ExecuteCommand = (void *)AFXADDR_GET(csgo_engine_Cmd_ExecuteCommand);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Org_csgo_Cmd_ExecuteCommand, My_csgo_Cmd_ExecuteCommand);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CON_COMMAND(mirv_block_commands, "")
{
	if (!Install_csgo_Cmd_ExecuteCommand()) {
		Tier0_Warning("AFXERROR: Missing hooks.\n");
		return;
	}

	int argC = args->ArgC();
	const char* arg0 = args->ArgV(0);

	if (2 <= argC) {
		const char* arg1 = args->ArgV(1);
		if (0 == _stricmp(arg1, "debug")) {
			if (3 <= argC) {
				g_DebugExecuteCommand = 0 != atoi(args->ArgV(2));
				return;
			}
			Tier0_Msg(
				"%s debug 0|1\n"
				"Current Value: %i\n"
				, arg0
				, g_DebugExecuteCommand ? 1 : 0
			);
			return;
		}
		else if (0 == _stricmp(arg1, "print")) {
			auto it = g_CmdExecuteCommand_Blocks.begin();
			if (3 <= argC)
				auto it = g_CmdExecuteCommand_Blocks.find(atoi(args->ArgV(2)));

			while (it != g_CmdExecuteCommand_Blocks.end()) {
				int idx = 0;
				for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
					Tier0_Msg("%i: %i: \"%s\"\n", it->first, idx, it2->c_str());
					++idx;
				}

				if (3 <= argC) break;
				++it;
			}

			return;
		}
		else if (0 == _stricmp(arg1, "clear")) {
			if (3 <= argC) {
				g_CmdExecuteCommand_Blocks.erase(atoi(args->ArgV(2)));
				return;
			}
			g_CmdExecuteCommand_Blocks.clear();
			return;
		}
		else if (0 == _stricmp(arg1, "remove")) {
			if (4 <= argC) {
				auto it = g_CmdExecuteCommand_Blocks.find(atoi(args->ArgV(2)));
				if (it != g_CmdExecuteCommand_Blocks.end()) {
					int listNr = atoi(args->ArgV(3));
					if (0 <= listNr && listNr < (int)it->second.size()) {
						auto it2 = it->second.begin();
						std::advance(it2, listNr);
						it->second.erase(it2);
						if (it->second.empty())
							g_CmdExecuteCommand_Blocks.erase(it);
					}
				}
				return;
			}
		}
		else if (0 == _stricmp(arg1, "move")) {
			if (5 <= argC) {
				auto it = g_CmdExecuteCommand_Blocks.find(atoi(args->ArgV(2)));
				if (it != g_CmdExecuteCommand_Blocks.end()) {
					int listNr = atoi(args->ArgV(3));
					int targetNr = atoi(args->ArgV(4));

					if (0 <= listNr && listNr <= (int)it->second.size()
						&& 0 <= targetNr && targetNr <= (int)it->second.size()
						) {

						auto sourceIt = it->second.begin();

						auto targetIt = it->second.begin();

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

						it->second.splice(targetIt, it->second, sourceIt);
					}
				}
				return;
			}
		}
		else if (0 == _stricmp(arg1, "add")) {
			if (4 <= argC) {
				int stackNr = atoi(args->ArgV(2));
				std::string str;
				for (int i = 3; i < argC; ++i) {
					if (3 < i) str += " ";
					str += args->ArgV(i);
				}
				g_CmdExecuteCommand_Blocks[stackNr].emplace_front(str);
				return;
			}
		}
	}

	Tier0_Msg(
		"%s debug [...] - Debug commands beging executed.\n"
		"%s print [<iStack>] - Prints command blocks.\n"
		"%s clear [<iStack>] - Clears command blocks.\n"
		"%s remove <iStack> <iNr> - Removes a block.\n"
		"%s move <iStack> <iSourceNr> <iTargetNr> - Moves a block.\n"
		"%s add <iStack> <sWildCard> - Adds a block.\n"
		"\t<iStack> - 0 Code, 1 Client Cmd, 2 User Input, 3 Net Client, 4 Net Server, 5 Demo File.\n"
		"\t<sWildCard> - Wild card string (\\* = wildcard and \\\\ = \\).\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}