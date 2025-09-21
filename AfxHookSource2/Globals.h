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

extern size_t getVTableFn(HMODULE dll, int index, const char* mangledClass);

namespace afxUtils {
	struct RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};
	std::string createTable(std::vector<std::vector<std::string>>& rows, char* delimiter, char* emptyRowDelimiter);
	std::string rgbaToHex(std::string str, std::string delim);
	uint32_t hexStrToInt(std::string str);
	uint32_t rgbaToHex(RGBA& color);
	std::string stringToLowerCase(const char* s);
}
