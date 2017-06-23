#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <list>

typedef void (__stdcall *csgo_CHudDeathNotice_FireGameEvent_t)(DWORD *this_ptr, SOURCESDK::CSGO::IGameEvent * event);

csgo_CHudDeathNotice_FireGameEvent_t detoured_csgo_CHudDeathNotice_FireGameEvent;

int csgo_debug_CHudDeathNotice_FireGameEvent = 0;

enum DeathMsgBlockMode
{
	DMBM_EQUAL,
	DMBM_EXCEPT,
	DMBM_ANY
};

enum DeathMsgBlockAction
{
	DMBA_BLOCK,
	DMBA_MODTIME
};

struct DeathMsgBlockEntry
{
	int attackerId;
	DeathMsgBlockMode attackerMode;
	int victimId;
	DeathMsgBlockMode victimMode;
	int assisterId;
	DeathMsgBlockMode assisterMode;
	float modTime;
};

std::list<DeathMsgBlockEntry> deathMessageBlock;

float csgo_CHudDeathNotice_nScrollInTime = -1;
float csgo_CHudDeathNotice_nFadeOutTime = -1;
float csgo_CHudDeathNotice_nNoticeLifeTime = -1;
float csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod = -1;

float org_CHudDeathNotice_nScrollInTime;
float org_CHudDeathNotice_nFadeOutTime;
float org_CHudDeathNotice_nNoticeLifeTime;
float org_CHudDeathNotice_nLocalPlayerLifeTimeMod;

int csgo_CHudDeathNotice_HighLightId = -1;
bool csgo_CHudDeathNotice_HighLightAssists = true;

bool csgo_CHudDeathNotice_HighLightId_matchedVictim;
bool csgo_CHudDeathNotice_HighLightId_matchedAssister;
bool csgo_CHudDeathNotice_HighLightId_matchedAttacker;

bool csgo_CHudDeathNotice_ModTime_set;
float csgo_CHudDeathNotice_ModTime;

void __stdcall touring_csgo_CHudDeathNotice_FireGameEvent(DWORD *this_ptr, SOURCESDK::CSGO::IGameEvent * event)
{
	static bool firstRun = true;
	if(firstRun)
	{
		firstRun = false;
		org_CHudDeathNotice_nScrollInTime = *(float *)((BYTE *)this_ptr+0x5c);
		org_CHudDeathNotice_nFadeOutTime = *(float *)((BYTE *)this_ptr+0x58);
		org_CHudDeathNotice_nNoticeLifeTime = *(float *)((BYTE *)this_ptr+0x54);
		org_CHudDeathNotice_nLocalPlayerLifeTimeMod = *(float *)((BYTE *)this_ptr+0x64);
	}

	*(float *)((BYTE *)this_ptr+0x5c) = 0 <= csgo_CHudDeathNotice_nScrollInTime ? csgo_CHudDeathNotice_nScrollInTime : org_CHudDeathNotice_nScrollInTime;
	*(float *)((BYTE *)this_ptr+0x58) = 0 <= csgo_CHudDeathNotice_nFadeOutTime ? csgo_CHudDeathNotice_nFadeOutTime : org_CHudDeathNotice_nFadeOutTime;
	*(float *)((BYTE *)this_ptr+0x54) = 0 <= csgo_CHudDeathNotice_nNoticeLifeTime ? csgo_CHudDeathNotice_nNoticeLifeTime : org_CHudDeathNotice_nNoticeLifeTime;
	*(float *)((BYTE *)this_ptr+0x64) = 0 <= csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod ? csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod : org_CHudDeathNotice_nLocalPlayerLifeTimeMod;
	
	int uidAttacker = event->GetInt("attacker");
	int uidVictim = event->GetInt("userid");
	int uidAssister = event->GetInt("assister");
	bool blocked = false;

	csgo_CHudDeathNotice_HighLightId_matchedAttacker = csgo_CHudDeathNotice_HighLightId == uidAttacker;
	csgo_CHudDeathNotice_HighLightId_matchedVictim = csgo_CHudDeathNotice_HighLightId == uidVictim;
	csgo_CHudDeathNotice_HighLightId_matchedAssister = csgo_CHudDeathNotice_HighLightId == uidAssister;

	if(0 < csgo_debug_CHudDeathNotice_FireGameEvent)
	{
		Tier0_Msg("CHudDeathNotice::FireGameEvent: uidAttaker=%i, uidVictim=%i, uidAssister=%i\n", uidAttacker, uidVictim, uidAssister);
		if(2 <= csgo_debug_CHudDeathNotice_FireGameEvent)
			Tier0_Msg(
				"org_scrollInTime=%f,org_fadeOutTime=%f,org_noticeLifeTime=%f,org_localPlayerLifeTimeMod=%f\n",
				org_CHudDeathNotice_nScrollInTime,
				org_CHudDeathNotice_nFadeOutTime,
				org_CHudDeathNotice_nNoticeLifeTime,
				org_CHudDeathNotice_nLocalPlayerLifeTimeMod
			);
	}

	csgo_CHudDeathNotice_ModTime_set = false;

	for(std::list<DeathMsgBlockEntry>::iterator it = deathMessageBlock.begin(); it != deathMessageBlock.end(); it++)
	{
		DeathMsgBlockEntry e = *it;

		bool attackerBlocked;
		switch(e.attackerMode)
		{
		case DMBM_ANY:
			attackerBlocked = true;
			break;
		case DMBM_EXCEPT:
			attackerBlocked = e.attackerId != uidAttacker;
			break;
		case DMBM_EQUAL:
		default:
			attackerBlocked = e.attackerId == uidAttacker;
			break;
		}

		bool victimBlocked;
		switch(e.victimMode)
		{
		case DMBM_ANY:
			victimBlocked = true;
			break;
		case DMBM_EXCEPT:
			victimBlocked = e.victimId != uidVictim;
			break;
		case DMBM_EQUAL:
		default:
			victimBlocked = e.victimId == uidVictim;
			break;
		}

		bool assisterBlocked;
		switch(e.assisterMode)
		{
		case DMBM_ANY:
			assisterBlocked = true;
			break;
		case DMBM_EXCEPT:
			assisterBlocked = e.assisterId != uidAssister;
			break;
		case DMBM_EQUAL:
		default:
			assisterBlocked = e.assisterId == uidAssister;
			break;
		}

		bool matched = attackerBlocked && victimBlocked && assisterBlocked;
		if(matched)
		{
			csgo_CHudDeathNotice_ModTime_set = 0 <= e.modTime;
			csgo_CHudDeathNotice_ModTime = e.modTime;
			blocked = !csgo_CHudDeathNotice_ModTime_set;
			if(blocked) break;
		}
	}

	if(!blocked) detoured_csgo_CHudDeathNotice_FireGameEvent(this_ptr, event);
}

typedef void (__stdcall *csgo_CHudDeathNotice_UnkAddDeathNotice_t)(DWORD *this_ptr, void * arg0, bool bIsVictim, bool bIsKiller);

csgo_CHudDeathNotice_UnkAddDeathNotice_t detoured_csgo_CHudDeathNotice_UnkAddDeathNotice;

DWORD * csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr = 0;

void __stdcall touring_csgo_CHudDeathNotice_UnkAddDeathNotice(DWORD *this_ptr, void * arg0, bool bIsVictim, bool bIsKiller)
{
	csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr = this_ptr;

	if(0 < csgo_CHudDeathNotice_HighLightId)
	{
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0,
			csgo_CHudDeathNotice_HighLightId_matchedVictim,
			!csgo_CHudDeathNotice_HighLightId_matchedVictim && (csgo_CHudDeathNotice_HighLightId_matchedAttacker || csgo_CHudDeathNotice_HighLightAssists && csgo_CHudDeathNotice_HighLightId_matchedAssister));
		return;
	}
	else
	if(0 == csgo_CHudDeathNotice_HighLightId)
	{
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0, false, false);
		return;
	}

	detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(this_ptr, arg0, bIsVictim, bIsKiller);
}

void * detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime;

void __declspec(naked) touring_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime(void)
{
	__asm cmp csgo_CHudDeathNotice_ModTime_set, 0
	__asm jnz __modTimeSet
	__asm jmp __continue
	
	__asm __modTimeSet:
	__asm movss xmm0, [csgo_CHudDeathNotice_ModTime]

	__asm __continue:
	__asm jmp detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime
}

bool csgo_CHudDeathNotice_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent) && AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice) && AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime))
	{
		detoured_csgo_CHudDeathNotice_FireGameEvent = (csgo_CHudDeathNotice_FireGameEvent_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent), (BYTE *)touring_csgo_CHudDeathNotice_FireGameEvent, (int)AFXADDR_GET(csgo_CHudDeathNotice_FireGameEvent_DSZ));
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice = (csgo_CHudDeathNotice_UnkAddDeathNotice_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice), (BYTE *)touring_csgo_CHudDeathNotice_UnkAddDeathNotice, (int)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_DSZ));
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime = (void *)DetourApply((BYTE *)AFXADDR_GET(csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime), (BYTE *)touring_csgo_CHudDeathNotice_UnkAddDeathNotice_AddMovie_AfterModTime, 0x11);

		firstResult = true;
	}

	return firstResult;
}

void csgo_CHudDeathNotice_Block(char const * uidAttacker, char const * uidVictim, char const * uidAssister, float modTime)
{
	char const * acmd;
	int attackerId = -1;
	int victimId = -1;
	int assisterId = -1;

	acmd = uidAttacker;
	bool anyAttacker = !strcmp("*", acmd);
	bool notAttacker = StringBeginsWith(acmd, "!");
	if(!anyAttacker) attackerId = atoi(notAttacker ? (acmd +1) : acmd);

	acmd = uidVictim;
	bool anyVictim = !strcmp("*", acmd);
	bool notVictim = StringBeginsWith(acmd, "!");
	if(!anyVictim) victimId = atoi(notVictim ? (acmd +1) : acmd);

	acmd = uidAssister;
	bool anyAssister = !strcmp("*", acmd);
	bool notAssister = StringBeginsWith(acmd, "!");
	if(!anyAssister) assisterId = atoi(notAssister ? (acmd +1) : acmd);

	DeathMsgBlockEntry entry = {
		attackerId,
		anyAttacker ? DMBM_ANY : (notAttacker ? DMBM_EXCEPT : DMBM_EQUAL),
		victimId,
		anyVictim ? DMBM_ANY : (notVictim ? DMBM_EXCEPT : DMBM_EQUAL),
		assisterId,
		anyAssister ? DMBM_ANY : (notAssister ? DMBM_EXCEPT : DMBM_EQUAL),
		modTime
	};

	deathMessageBlock.push_back(entry);
}

void csgo_CHudDeathNotice_Block_List(void)
{
	Tier0_Msg("uidAttacker,uidVictim,uidAssister,modTime(<0 means block):\n");
	for(std::list<DeathMsgBlockEntry>::iterator it = deathMessageBlock.begin(); it != deathMessageBlock.end(); it++)
	{
		DeathMsgBlockEntry e = *it;
						
		if(DMBM_ANY == e.attackerMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.attackerMode)
			{
				Tier0_Msg("!");
			}
			Tier0_Msg("%i", e.attackerId);
		}
		Tier0_Msg(",");
		if(DMBM_ANY == e.victimMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.victimMode)
			{
				Tier0_Msg("!");
			}
			Tier0_Msg("%i", e.victimId);
		}
		Tier0_Msg(",");
		if(DMBM_ANY == e.assisterMode)
			Tier0_Msg("*");
		else
		{
			if(DMBM_EXCEPT == e.assisterMode)
			{
				Tier0_Msg("!");
			}
			Tier0_Msg("%i", e.assisterId);
		}
		Tier0_Msg(",%f\n", e.modTime);
	}
}

void csgo_CHudDeathNotice_Block_Clear(void)
{
	deathMessageBlock.clear();
}

void Console_csgo_CHudDeathNotice_Fake(char const * htmlString, bool bIsVictim, bool bIsKiller)
{
	if (!csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr)
	{
		Tier0_Warning("Error: There must have been at least one death notice for this to work after the command has been entered the first time (wait for a frag or load another demo)!");
		return;
	}

	std::wstring wideString;

	if (!AnsiStringToWideString(htmlString, wideString))
		Tier0_Warning("Error upon converting \"%s\" to a wide string.\n", htmlString);
	else
		detoured_csgo_CHudDeathNotice_UnkAddDeathNotice(csgo_CHudDeathNotice_UnkAddDeathNotice_last_this_ptr, (void *)wideString.c_str(), bIsVictim, bIsKiller);
}