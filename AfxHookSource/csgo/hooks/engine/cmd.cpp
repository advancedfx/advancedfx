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

#include <fstream>

static std::fstream g_Command_Logfile;

void Log_Command(int eTarget, int eSource, const char * commandString, bool before) {
	if(!g_Command_Logfile.is_open())
		return;

	try {
		std::string strSrc("\"");
		std::string strDst("\"\"");
		std::string cmdString(commandString);
		size_t pos = 0;
		while((pos = cmdString.find(strSrc, pos)) != std::string::npos) {
			cmdString.replace(pos,strSrc.length(), strDst);
			pos += strDst.length();
		}
		g_Command_Logfile << std::to_string(GetTickCount()) << ";"  << std::to_string(eTarget) << ";"  << (before ? "BEGIN" : "END") << ";\"" << cmdString << "\"" << std::endl;
		g_Command_Logfile.flush();
	} catch(std::system_error) {

	}
}

std::map<int, std::list<std::string>> g_CmdExecuteCommand_Blocks;
int g_DebugExecuteCommand = 0;

void * g_Org_csgo_Cmd_ExecuteCommand;

bool My_csgo_Cmd_ExecuteCommand_Do(int eTarget, int eSource, const SOURCESDK::CSGO::CCommand& command) {
	if (command.ArgC()) {
		if (g_DebugExecuteCommand) {
			Tier0_Msg("Cmd_ExecuteCommand: %i -> \"%s\"\n", eSource, command.GetCommandString());
		}

		auto it = g_CmdExecuteCommand_Blocks.find(eSource);
		if (it != g_CmdExecuteCommand_Blocks.end()) {
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				if (StringWildCard1Matched(it2->c_str(), command.GetCommandString())) {
					if (g_DebugExecuteCommand) {
						Tier0_Msg("BLOCKED: Cmd_ExecuteCommand: %i -> \"%s\"\n", eSource, command.GetCommandString());
					}
					return true;
				}

			}
		}

		if (eSource == SOURCESDK::CSGO::kCommandSrcDemoFile && WrpConCommands::IsRegisteredSlow(command.ArgV()[0])) {
			Tier0_Warning("AFXWARNING: BLOCKED HLAE COMMAND FROM DEMO STREAM: \"%s\"\n", command.GetCommandString());
			return true;
		}
	}

	return false;
}

__declspec(naked) SOURCESDK::CSGO::ConCommandBase* __fastcall  My_csgo_Cmd_ExecuteCommand(int eTarget, const SOURCESDK::CSGO::CCommand& command) {

	__asm push ebp
	__asm mov ebp, esp
	__asm sub esp, __LOCAL_SIZE
	__asm mov eTarget, ecx
	__asm mov command, edx

	SOURCESDK::CSGO::ConCommandBase* result; 
	
	if (My_csgo_Cmd_ExecuteCommand_Do(eTarget, command.Source(), command)) {
		__asm mov edx, command
		__asm mov ecx, eTarget
		__asm mov esp, ebp
		__asm pop ebp
		__asm mov eax, 0
		__asm ret
	}
	
	Log_Command(eTarget, command.Source(), command.GetCommandString(), true);

	__asm mov edx, command
	__asm mov ecx, eTarget

	__asm call [g_Org_csgo_Cmd_ExecuteCommand] 

	__asm mov result, eax

	Log_Command(eTarget, command.Source(), command.GetCommandString(), false);

	__asm mov eax, result
	
	__asm mov edx, command
	__asm mov ecx, eTarget
	__asm mov esp, ebp
	__asm pop ebp
	__asm ret
}

typedef SOURCESDK::CSGO::ConCommandBase* (*tf2_engine_Cmd_ExecuteCommand_t)(const SOURCESDK::CSGO::CCommand& command, int eSource, int eTarget);

tf2_engine_Cmd_ExecuteCommand_t g_tf2_engine_Cmd_ExecuteCommand = nullptr;

SOURCESDK::CSGO::ConCommandBase* My_tf2_engine_Cmd_ExecuteCommand(const SOURCESDK::CSGO::CCommand& command, int eSource, int eTarget) {

	if (My_csgo_Cmd_ExecuteCommand_Do(eTarget, eSource, command)) {
		return nullptr;
	}

	Log_Command(eTarget, eSource, command.GetCommandString(), true);

	SOURCESDK::CSGO::ConCommandBase* result = g_tf2_engine_Cmd_ExecuteCommand(command,eSource,eTarget);

	Log_Command(eTarget, eSource, command.GetCommandString(), false);

	return result;
}

bool Install_csgo_tf2_Cmd_ExecuteCommand(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (SourceSdkVer_CSGO == g_SourceSdkVer && AFXADDR_GET(csgo_engine_Cmd_ExecuteCommand)){
		LONG error = NO_ERROR;

		g_Org_csgo_Cmd_ExecuteCommand = (void *)AFXADDR_GET(csgo_engine_Cmd_ExecuteCommand);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Org_csgo_Cmd_ExecuteCommand, My_csgo_Cmd_ExecuteCommand);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	} else if(SourceSdkVer_TF2 == g_SourceSdkVer && AFXADDR_GET(tf2_engine_Cmd_ExecuteCommand)){
		LONG error = NO_ERROR;

		g_tf2_engine_Cmd_ExecuteCommand = (tf2_engine_Cmd_ExecuteCommand_t)AFXADDR_GET(tf2_engine_Cmd_ExecuteCommand);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_tf2_engine_Cmd_ExecuteCommand, My_tf2_engine_Cmd_ExecuteCommand);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


CON_COMMAND(mirv_block_commands, "")
{
	if (!Install_csgo_tf2_Cmd_ExecuteCommand()) {
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
		else if (0 == _stricmp(arg1, "logFile")) {
			if (3 <= argC) {
				std::string filePath(args->ArgV(2));
				if(filePath.empty()) {
					if(g_Command_Logfile.is_open()) {
						try {
							g_Command_Logfile << "---- END session ----" << std::endl;
							g_Command_Logfile.close();
						} catch(std::system_error) {
							Tier0_Warning("AFXERROR: Could not close commmand log file.", filePath.c_str());
						}
					}
				} else {
					try {
						g_Command_Logfile.open(filePath, std::fstream::out | std::fstream::binary | std::fstream::app);
						g_Command_Logfile << "---- BEGIN session ----" << std::endl;
						g_Command_Logfile << "tick count;eTarget;begin/end;command" << std::endl;
					} catch(std::system_error) {
						Tier0_Warning("AFXERROR: Could not open file \"%s\" for appending.", filePath.c_str());
					}
				}
				return;
			}
			Tier0_Msg(
				"%s \"\"|<sFilePath> - Empty means no logfile.\n"
				"Current Status: %s\n"
				, arg0
				, g_Command_Logfile.is_open() ? "ACTIVE" : "inactive"
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
		"%s debug [...] - Debug commands being executed.\n"
		"%s logFile [...] - Log to file (slow, use for debug only).\n"
		"%s print [<iStack>] - Prints command blocks.\n"
		"%s clear [<iStack>] - Clears command blocks.\n"
		"%s remove <iStack> <iNr> - Removes a block.\n"
		"%s move <iStack> <iSourceNr> <iTargetNr> - Moves a block.\n"
		"%s add <iStack> <sWildCard> - Adds a block.\n"
		"\t<iStack> - 0 = Code, 1 = Client Cmd, 2 = User Input, 3 = Net Client, 4 = Net Server, 5 = Demo File.\n"
		"\t<sWildCard> - Wild card string (\\* = wildcard and \\\\ = \\).\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}
