#pragma once
#include <Windows.h>
#include "../shared/binutils.h"
#include "../shared/MirvInput.h"
#include "WrpConsole.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

extern void ErrorBox(char const * messageText);

extern void ErrorBox();

extern size_t getAddress(HMODULE dll, char const* pattern);

namespace afxUtils {
	std::string createTable(std::vector<std::vector<std::string>>& rows, char* delimiter, char* emptyRowDelimiter);
	std::string rgbaToHex(std::string str, advancedfx::Con_Printf_t &conMessage);
	uint32_t hexStrToInt(std::string str);
}