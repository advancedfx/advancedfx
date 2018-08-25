#include "stdafx.h"

#include "hlaeFolder.h"

#include <Windows.h>
#include <string>

#define DLL_NAME	"AfxHookGoldSrc.dll"

std::string g_HlaeFolder("");

void CalculateHlaeFolder()
{
	LPSTR fileName = 0;
	HMODULE hm;
	DWORD length;

	bool bOk =
		0 != (hm = GetModuleHandle(DLL_NAME))
	;

	if(hm)
	{
		length = 100;
		fileName = (LPSTR)malloc(length);

		while(fileName && length == GetModuleFileNameA(hm, fileName, length))
			fileName = (LPSTR)realloc(fileName, (length += 100));

		if(!fileName)
			return;

		bOk = 0 < length;
	}

	if(bOk)
	{
		g_HlaeFolder.assign(fileName);

		size_t fp = g_HlaeFolder.find_last_of('\\');
		if(std::string::npos != fp)
		{
			g_HlaeFolder.resize(fp+1);
		}
	}

	free(fileName);

	return;
}

char const * GetHlaeFolder()
{
	static bool firstRun = true;
	if(firstRun)
	{
		firstRun = false;
		CalculateHlaeFolder();
	}

	return g_HlaeFolder.c_str();
}
