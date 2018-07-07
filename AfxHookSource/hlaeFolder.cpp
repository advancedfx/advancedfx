#include "stdafx.h"

#include "hlaeFolder.h"

#include <shared/StringTools.h>

#include <Windows.h>
#include <string>

#define DLL_NAME	L"AfxHookSource.dll"

std::wstring g_HlaeFolderW(L"");
std::string g_HlaeFolder("");

void CalculateHlaeFolderOnce()
{
	static bool firstRun = true;
	if (firstRun)
	{
		firstRun = false;
	}
	else
		return;

	LPWSTR fileName = 0;
	HMODULE hm;
	DWORD length;

	bool bOk =
		0 != (hm = GetModuleHandleW(DLL_NAME))
	;

	if(hm)
	{
		length = 100;
		fileName = (LPWSTR)malloc(length*sizeof(WCHAR));

		while(fileName && length == GetModuleFileNameW(hm, fileName, length))
			fileName = (LPWSTR)realloc(fileName, (length += 100)*sizeof(WCHAR));

		if(!fileName)
			return;

		bOk = 0 < length;
	}

	if(bOk)
	{
		g_HlaeFolderW.assign(fileName);

		size_t fp = g_HlaeFolderW.find_last_of(L'\\');
		if(std::string::npos != fp)
		{
			g_HlaeFolderW.resize(fp+1, L'\\');
		}
		
		WideStringToUTF8String(g_HlaeFolderW.c_str(), g_HlaeFolder);
	}

	free(fileName);

	return;
}

const char * GetHlaeFolder()
{
	CalculateHlaeFolderOnce();

	return g_HlaeFolder.c_str();
}

const wchar_t * GetHlaeFolderW()
{
	CalculateHlaeFolderOnce();

	return g_HlaeFolderW.c_str();
}
