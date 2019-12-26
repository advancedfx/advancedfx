#include "stdafx.h"

#include "csgo_S_StartSound.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <list>

std::list<char *> g_S_StartSound_BlockMasks;

bool g_csgo_S_StartSound_Debug = false;

DWORD csgo_S_StartSound_StringConversion_retAddr = 0;

bool csgo_S_StartSound_OnString(char * pStr, DWORD maxStrLen)
{
	if(g_csgo_S_StartSound_Debug)
	{
		Tier0_Msg("S_StartSound: \"%s\"", pStr);
	}

	for(std::list<char *>::iterator it = g_S_StartSound_BlockMasks.begin(); it != g_S_StartSound_BlockMasks.end();++it)
	{
		if(StringWildCard1Matched(*it,pStr))
		{
			if(g_csgo_S_StartSound_Debug)
			{
				Tier0_Msg(" -> BLOCKED!\n");
			}

			return true;
		}
	}

	if(g_csgo_S_StartSound_Debug)
	{
		Tier0_Msg("\n");
	}

	return false;
}

__declspec(naked) void touring_csgo_S_StartSound_StringConversion(void)
{
	__asm mov  eax, [ecx]
	__asm lea  edx, [esp+0x78]
	__asm push 0x104 ; for our own call
	__asm push edx ; for our own call
	__asm push 0x104
	__asm push edx
	__asm call dword ptr [eax]
	
	__asm call csgo_S_StartSound_OnString
	__asm add esp, 8 ; caller cleans stack

	__asm cmp al, 0
	__asm jnz __SoundBlockedRet
	
	__asm jmp [csgo_S_StartSound_StringConversion_retAddr]

	__asm __SoundBlockedRet:
	__asm xor eax, eax
	__asm pop edi
	__asm pop esi
	__asm pop ebx
	__asm mov esp, ebp
	__asm pop ebp
	__asm ret
}


bool csgo_S_StartSound_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_S_StartSound_StringConversion))
	{
		csgo_S_StartSound_StringConversion_retAddr = AFXADDR_GET(csgo_S_StartSound_StringConversion) +0xe;
		Asm32ReplaceWithJmp((void *)AFXADDR_GET(csgo_S_StartSound_StringConversion), 0xe, touring_csgo_S_StartSound_StringConversion);

		firstResult = true;
	}

	return firstResult;
}

void csgo_S_StartSound_Block_Add(char const * szMask)
{
	size_t bufLen = strlen(szMask)+1;
	char * szCpy = (char *)malloc(sizeof(char)*bufLen);
	strcpy_s(szCpy, bufLen, szMask);

	g_S_StartSound_BlockMasks.push_back(szCpy);
}

void csgo_S_StartSound_Block_Print(void)
{
	Tier0_Msg("index: \"mask\"\n");

	int i=0;
	for(std::list<char *>::iterator it = g_S_StartSound_BlockMasks.begin(); it != g_S_StartSound_BlockMasks.end();++it)
	{
		Tier0_Msg("%i: \"%s\"\n", i, *it);

		++i;
	}
}

void csgo_S_StartSound_Block_Remove(int index)
{
	int i=0;
	for(std::list<char *>::iterator it = g_S_StartSound_BlockMasks.begin(); it != g_S_StartSound_BlockMasks.end();++it)
	{
		if(i == index)
		{
			free(*it);
			g_S_StartSound_BlockMasks.erase(it);
			break;
		}

		++i;
	}
}

void csgo_S_StartSound_Block_Clear(void)
{
	while(!g_S_StartSound_BlockMasks.empty())
	{
		char * pSz = g_S_StartSound_BlockMasks.front();
		free(pSz);
		g_S_StartSound_BlockMasks.pop_front();
	}
}
