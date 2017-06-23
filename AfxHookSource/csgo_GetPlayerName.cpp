#include "stdafx.h"

#include "csgo_GetPlayerName.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <map>

std::map<int, wchar_t *> getPlayerNameReplace;

bool csgo_debug_GetPlayerName = false;

typedef wchar_t * (__stdcall *csgo_CUnknown_GetPlayerName_t)(DWORD *this_ptr, int playerId, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1);

csgo_CUnknown_GetPlayerName_t detoured_csgo_CUnknown_GetPlayerName;

wchar_t * __stdcall touring_csgo_CUnknown_GetPlayerName(DWORD *this_ptr, int playerId, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1)
{
	wchar_t * result =  detoured_csgo_CUnknown_GetPlayerName(this_ptr, playerId, targetBuffer, targetByteCount, unknownArg1);

	if(csgo_debug_GetPlayerName)
	{
		std::string ansiPlayerName;
		
		if(WideStringToAnsiString(result, ansiPlayerName))
			Tier0_Msg("CUnknown::GetPlayerName: %i -> %s\n", playerId, ansiPlayerName.c_str());
		else
			Tier0_Msg("CUnknown::GetPlayerName: %i ERROR: could not get AnsiString for player name.ß", playerId);
	}

	std::map<int, wchar_t *>::iterator it = getPlayerNameReplace.find(playerId);
	if(it != getPlayerNameReplace.end())
	{
		wcscpy_s(targetBuffer, targetByteCount, it->second);
	}

	return result;
}

bool csgo_GetPlayerName_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_CUnknown_GetPlayerName))
	{
		detoured_csgo_CUnknown_GetPlayerName = (csgo_CUnknown_GetPlayerName_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CUnknown_GetPlayerName), (BYTE *)touring_csgo_CUnknown_GetPlayerName, (int)AFXADDR_GET(csgo_CUnknown_GetPlayerName_DSZ));

		firstResult = true;
	}

	return firstResult;
}

void csgo_GetPlayerName_Replace(char const * uidPlayer, char const * newName)
{
	std::wstring widePlayerName;
	int iUidPlayer =  atoi(uidPlayer);

	if(!AnsiStringToWideString(newName, widePlayerName))
	{
		Tier0_Msg("Error converting player name to wide string.\n");
		return;
	}

	size_t strLen = 1+wcslen(widePlayerName.c_str());

	wchar_t * newString = new wchar_t[strLen];

	wcscpy_s( newString, strLen, widePlayerName.c_str() );

	// remove old element first:
	csgo_GetPlayerName_Replace_Delete(uidPlayer);

	getPlayerNameReplace[iUidPlayer] = newString;
}

void csgo_GetPlayerName_Replace_List(void)
{
	for(std::map<int, wchar_t *>::iterator it = getPlayerNameReplace.begin(); it != getPlayerNameReplace.end(); ++it)
	{
		std::string ansiPlayerName;
		
		WideStringToAnsiString(it->second, ansiPlayerName);
	
		Tier0_Msg("%i -> %s\n", it->first, ansiPlayerName.c_str());
	}
}

void csgo_GetPlayerName_Replace_Delete(char const * uidPlayer)
{
	int iUidPlayer = atoi(uidPlayer);
	std::map<int, wchar_t *>::iterator it = getPlayerNameReplace.find(iUidPlayer);
	if(it != getPlayerNameReplace.end())
	{
		delete it->second;
		getPlayerNameReplace.erase(it);
	}
}

void csgo_GetPlayerName_Replace_Clear(void)
{
	for(std::map<int, wchar_t *>::iterator it = getPlayerNameReplace.begin(); it != getPlayerNameReplace.end(); ++it)
	{
		delete it->second;
	}

	getPlayerNameReplace.clear();
}
