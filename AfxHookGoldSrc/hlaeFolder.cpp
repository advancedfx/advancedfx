#include "stdafx.h"

#include "hlaeFolder.h"

#include <shared/StringTools.h>
#include <shared/FileTools.h>

#include <Windows.h>
#include <Shlobj.h>
#include <string>

#ifdef _DEBUG
#define DLL_NAME	L"AfxHookGoldSrc_d.dll"
#else
#define DLL_NAME	L"AfxHookGoldSrc.dll"
#endif

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

		while (fileName && length == GetModuleFileNameW(hm, fileName, length))
		{
			LPWSTR newFileName;
			if (nullptr == (newFileName = (LPWSTR)realloc(fileName, (length += 100) * sizeof(WCHAR))))
			{
				free(fileName);
			}
			fileName = newFileName;
		}

		if(!fileName)
			return;

		bOk = 0 < length;
	}

	if(bOk)
	{
		g_HlaeFolderW.assign(fileName);

		// strip DLL name to get path:
		size_t fp = g_HlaeFolderW.find_last_of(L'\\');
		if(std::string::npos != fp)
		{
			g_HlaeFolderW.resize(fp +1, L'\\');
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


std::wstring g_HlaeRoamingAppDataFolderW(L"");
std::string g_HlaeRoamingAppDataFolder("");

void CalculateHlaeRoamingAppDataFolderOnce()
{
	static bool firstRun = true;
	if (firstRun)
	{
		firstRun = false;
	}
	else
		return;

	PWSTR path = nullptr;

	if (S_OK == SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT,0,&path))
	{
		g_HlaeRoamingAppDataFolderW = path;

		size_t fp = g_HlaeRoamingAppDataFolderW.find_last_of(L'\\');
		if (std::string::npos == fp || g_HlaeRoamingAppDataFolderW.length() != fp)
		{
			g_HlaeRoamingAppDataFolderW.resize(g_HlaeRoamingAppDataFolderW.length() + 1, L'\\');
		}

		g_HlaeRoamingAppDataFolderW += L"HLAE\\";

		WideStringToUTF8String(g_HlaeRoamingAppDataFolderW.c_str(), g_HlaeRoamingAppDataFolder);

		CreatePath(g_HlaeRoamingAppDataFolderW.c_str(), std::wstring());
	}

	CoTaskMemFree(path);

	return;
}

const char* GetHlaeRoamingAppDataFolder()
{
	CalculateHlaeRoamingAppDataFolderOnce();

	return g_HlaeRoamingAppDataFolder.c_str();
}

const wchar_t* GetHlaeRoamingAppDataFolderW()
{
	CalculateHlaeRoamingAppDataFolderOnce();

	return g_HlaeRoamingAppDataFolderW.c_str();
}
