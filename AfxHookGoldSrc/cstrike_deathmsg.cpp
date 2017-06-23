#include "stdafx.h"

#include <hlsdk.h>

#include "cmdregister.h"
#include "hl_addresses.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include <list>


// HLSDK:
#define MAX_PLAYER_NAME_LENGTH		32
#define MAX_DEATHNOTICES 4

// the structs size is 0xa0, however the fields might be in wrong oder,
// since I don't access them atm and thus did not validate em.
struct cstrike_DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	int iId2; // the index number of the second sprite or -1
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

struct tfc_DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

extern cl_enginefuncs_s *pEngfuncs;

cstrike_DeathNoticeItem * cstrike_rgDeathNoticeList;
tfc_DeathNoticeItem * tfc_rgDeathNoticeList;

#define cstrike_OFS_Draw_YRes 0x114
#define cstrike_OFS_Draw_AfterYRes (cstrike_OFS_Draw_YRes +0x0A)

#define tfc_OFS_Draw_YRes 0x09
#define tfc_OFS_Draw_AfterYRes (tfc_OFS_Draw_YRes +0x05)

DWORD DeathMsg_Draw_AfterYRes;

std::list<cstrike_DeathNoticeItem> cstrike_DeathNotices;
std::list<tfc_DeathNoticeItem> tfc_DeathNotices;

DWORD DeathMsg_Draw_ItemIndex;

typedef int (* MsgFunc_DeathMsg_t)(const char *pszName, int iSize, void *pbuf);
typedef int (__stdcall *DeathMsg_Draw_t)(DWORD *this_ptr, float flTime );
typedef int (__stdcall *DeathMsg_Msg_t)(DWORD *this_ptr, const char *pszName, int iSize, void *pbuf );

MsgFunc_DeathMsg_t detoured_MsgFunc_DeathMsg;
DeathMsg_Draw_t detoured_DeathMsg_Draw;
DeathMsg_Msg_t detoured_DeathMsg_Msg;

enum DeathMsgBlockMode
{
	DMBM_EQUAL,
	DMBM_EXCEPT,
	DMBM_ANY
};

struct DeathMsgBlockEntry
{
	BYTE attackerId;
	DeathMsgBlockMode attackerMode;
	BYTE victimId;
	DeathMsgBlockMode victimMode;
};

std::list<DeathMsgBlockEntry> deathMessageBlock;


size_t deathMessagesMax = MAX_DEATHNOTICES;

int touring_MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	if(2 <= iSize && 0 < deathMessageBlock.size())
	{
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
				attackerBlocked = e.attackerId != ((BYTE *)pbuf)[0];
				break;
			case DMBM_EQUAL:
			default:
				attackerBlocked = e.attackerId == ((BYTE *)pbuf)[0];
				break;
			}

			bool victimBlocked;
			switch(e.victimMode)
			{
			case DMBM_ANY:
				victimBlocked = true;
				break;
			case DMBM_EXCEPT:
				victimBlocked = e.victimId != ((BYTE *)pbuf)[1];
				break;
			case DMBM_EQUAL:
			default:
				victimBlocked = e.victimId == ((BYTE *)pbuf)[1];
				break;
			}

			if(attackerBlocked && victimBlocked)
				return 1;
		}
	}

	return detoured_MsgFunc_DeathMsg(pszName, iSize, pbuf);
}

#define DEF_DeathMsg_Draw(modification) \
int __stdcall modification ## _touring_DeathMsg_Draw(DWORD *this_ptr, float flTime ) \
{ \
	int iRet = 1; \
	DeathMsg_Draw_ItemIndex = -1; \
	\
	for( \
		std::list<modification ## _DeathNoticeItem>::iterator it =  modification ## _DeathNotices.begin(); \
		1 == iRet && it !=  modification ## _DeathNotices.end(); \
		it++ \
	) { \
		DeathMsg_Draw_ItemIndex++; \
		modification ## _rgDeathNoticeList[0] = *it; \
		\
		iRet = detoured_DeathMsg_Draw(this_ptr, flTime); \
		\
		/* check if the message has been deleted: */ \
		if( \
			0 == modification ## _rgDeathNoticeList[0].iId \
		) { \
			DeathMsg_Draw_ItemIndex--; \
			it =  modification ## _DeathNotices.erase(it); \
		} \
		\
		modification ## _rgDeathNoticeList[0].iId = 0; \
	} \
	\
	return iRet; \
}

DEF_DeathMsg_Draw(cstrike)

DEF_DeathMsg_Draw(tfc)

#define DEF_DeathMsg_Msg(modification) \
int __stdcall modification ## _touring_DeathMsg_Msg(DWORD *this_ptr, const char *pszName, int iSize, void *pbuf ) \
{ \
	for(int i=0; i<MAX_DEATHNOTICES; i++) \
	{ \
		memset(&modification ## _rgDeathNoticeList[i], 0, sizeof(modification ## _DeathNoticeItem)); \
	} \
	\
	int i = detoured_DeathMsg_Msg(this_ptr, pszName, iSize, pbuf); \
	\
	if(i) \
	{ \
		if(0 < deathMessagesMax) \
		{ \
			/* make space for new element: */ \
			while(deathMessagesMax <= modification ## _DeathNotices.size()) \
				modification ## _DeathNotices.pop_front(); \
			\
			/* Pick up the message: */ \
			modification ## _DeathNoticeItem di = modification ## _rgDeathNoticeList[0]; \
			\
			modification ## _DeathNotices.push_back(di); \
		} \
		else \
			modification ## _DeathNotices.clear(); \
	} \
	\
	memset(&modification ## _rgDeathNoticeList[0], 0, sizeof(modification ## _DeathNoticeItem)); \
	\
	return i; \
}

DEF_DeathMsg_Msg(cstrike)

DEF_DeathMsg_Msg(tfc)

bool g_DeathMsg_ForceOffset = false;
int g_DeathMsg_Offset;

__declspec(naked) void cstrike_DeathMsg_DrawHelperY() {
	__asm {
		mov     ebp,dword ptr [esi+18h] ; original code

		mov dl, g_DeathMsg_ForceOffset
		test dl, dl
		JZ __OrgValue

		mov edx, g_DeathMsg_Offset
		JMP __Continue

		__OrgValue:
		mov     edx,dword ptr [esp+38h] ; original code
		
		__Continue:
		; imul    ebp,ebx ; original code
		imul    ebp, DeathMsg_Draw_ItemIndex
		
		JMP [DeathMsg_Draw_AfterYRes]
	}
}

__declspec(naked) void tfc_DeathMsg_DrawHelperY() {
	__asm {
		mov ebp, DeathMsg_Draw_ItemIndex
		imul ebp, 0x14

		push eax
		mov al, g_DeathMsg_ForceOffset
		test al, al
		JZ __OrgValue

		add ebp, g_DeathMsg_Offset
		JMP __Continue

		__OrgValue:
		add ebp, 0x22
		
		__Continue:
		pop eax

		JMP [DeathMsg_Draw_AfterYRes]
	}
}

#define DEF_Hook_DeathMsg(modification) \
bool Hook_DeathMsg_ ## modification() \
{ \
	/* 1. Fill addresses: */ \
	\
	modification ## _rgDeathNoticeList = (modification ## _DeathNoticeItem *)HL_ADDR_GET(modification ## _rgDeathNoticeList); \
	\
	DWORD dwDraw = (DWORD)HL_ADDR_GET(modification ## _CHudDeathNotice_Draw); \
	\
	DeathMsg_Draw_AfterYRes = dwDraw +modification ## _OFS_Draw_AfterYRes; \
	\
	/* Detour Functions: */ \
	detoured_MsgFunc_DeathMsg = (MsgFunc_DeathMsg_t)DetourApply((BYTE *)HL_ADDR_GET(modification ## _MsgFunc_DeathMsg), (BYTE *)touring_MsgFunc_DeathMsg, (int)HL_ADDR_GET(modification ## _MsgFunc_DeathMsg_DSZ)); \
	\
	detoured_DeathMsg_Draw = (DeathMsg_Draw_t)DetourClassFunc((BYTE *)dwDraw, (BYTE *)modification ## _touring_DeathMsg_Draw, (int)HL_ADDR_GET(modification ## _CHudDeathNotice_Draw_DSZ)); \
	detoured_DeathMsg_Msg = (DeathMsg_Msg_t)DetourClassFunc((BYTE *)HL_ADDR_GET(modification ## _CHudDeathNotice_MsgFunc_DeathMsg), (BYTE *)modification ## _touring_DeathMsg_Msg, (int)HL_ADDR_GET(modification ## _CHudDeathNotice_MsgFunc_DeathMsg_DSZ)); \
	\
	/* Patch Draw fn: */ \
	Asm32ReplaceWithJmp((void *)(dwDraw + modification ## _OFS_Draw_YRes), modification ## _OFS_Draw_AfterYRes - modification ## _OFS_Draw_YRes, (void *)modification ## _DeathMsg_DrawHelperY); \
	\
	return true; \
}

DEF_Hook_DeathMsg(cstrike)

DEF_Hook_DeathMsg(tfc)

bool Hook_DeathMsg()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	const char *gameDir = pEngfuncs->pfnGetGameDirectory();
	
	if(gameDir)
	{	
		if(!strcmp("cstrike",gameDir))
		{
			firstResult = Hook_DeathMsg_cstrike();
		}
		else if(!strcmp("tfc",gameDir))
		{
			firstResult = Hook_DeathMsg_tfc();
		}
	}

	return firstResult;
}


REGISTER_CMD_FUNC(deathmsg)
{
	if(!Hook_DeathMsg())
	{
		pEngfuncs->Con_Printf(
			"Error: Hook not installed.\n"
			"Maybe your modification \"%s\" is not supported?\n",
			pEngfuncs->pfnGetGameDirectory()
		);
		return;
	}

	int argc = pEngfuncs->Cmd_Argc();

	if(2 <= argc) {
		char * acmd = pEngfuncs->Cmd_Argv(1);

		if(!_stricmp(acmd, "block")) {
			if(4 == argc)
			{
				int attackerId = -1;
				int victimId = -1;

				acmd = pEngfuncs->Cmd_Argv(2);
				bool anyAttacker = !strcmp("*", acmd);
				bool notAttacker = StringBeginsWith(acmd, "!");
				if(!anyAttacker) attackerId = atoi(notAttacker ? (acmd +1) : acmd);

				acmd = pEngfuncs->Cmd_Argv(3);
				bool anyVictim = !strcmp("*", acmd);
				bool notVictim = StringBeginsWith(acmd, "!");
				if(!anyVictim) victimId = atoi(notVictim ? (acmd +1) : acmd);

				DeathMsgBlockEntry entry = {
					(BYTE)attackerId,
					anyAttacker ? DMBM_ANY : (notAttacker ? DMBM_EXCEPT : DMBM_EQUAL),
					(BYTE)victimId,
					anyVictim ? DMBM_ANY : (notVictim ? DMBM_EXCEPT : DMBM_EQUAL)
				};

				deathMessageBlock.push_back(entry);

				return;
			}
			if(3 == argc)
			{
				acmd = pEngfuncs->Cmd_Argv(2);
				if(!_stricmp(acmd, "list"))
				{
					pEngfuncs->Con_Printf("Attacker -> Victim\n");
					for(std::list<DeathMsgBlockEntry>::iterator it = deathMessageBlock.begin(); it != deathMessageBlock.end(); it++)
					{
						DeathMsgBlockEntry e = *it;
						
						if(DMBM_ANY == e.attackerMode)
							pEngfuncs->Con_Printf("*");
						else
						{
							if(DMBM_EXCEPT == e.attackerMode)
							{
								pEngfuncs->Con_Printf("!");
							}
							pEngfuncs->Con_Printf("%i", e.attackerId);
						}
						pEngfuncs->Con_Printf(" -> ");
						if(DMBM_ANY == e.victimMode)
							pEngfuncs->Con_Printf("*");
						else
						{
							if(DMBM_EXCEPT == e.victimMode)
							{
								pEngfuncs->Con_Printf("!");
							}
							pEngfuncs->Con_Printf("%i", e.victimId);
						}
						pEngfuncs->Con_Printf("\n");
					}
					return;
				}
				if(!_stricmp(acmd, "clear")) {
					deathMessageBlock.clear();
					return;
				}
			}
			pEngfuncs->Con_Printf(
				PREFIX "deathmsg block\n"
				"\t<attackerId> <victimId> - adds a block, use * to match any id, use !x to match any id apart from x\n"
				"\tlist - lists current blocks\n"
				"\tclear - clears all blocks\n"
			);
			return;
		}
		if(!_stricmp(acmd, "fake")) {
			if(6 == argc) {
				int iA = atoi(pEngfuncs->Cmd_Argv(2));
				int iV = atoi(pEngfuncs->Cmd_Argv(3));
				int iT = atoi(pEngfuncs->Cmd_Argv(4));
				acmd = pEngfuncs->Cmd_Argv(5);
				size_t sz = 3*sizeof(BYTE)+sizeof(char)*(1+strlen(acmd));

				BYTE * pMem = (BYTE *)malloc(sz);
				pMem[0] = iA;
				pMem[1] = iV;
				pMem[2] = iT;
				strcpy_s((char *)(&pMem[3]), sz-3, acmd);
				detoured_MsgFunc_DeathMsg("DeathMsg", (int)sz, pMem);
				free(pMem);
				return;
			}
			pEngfuncs->Con_Printf(
				PREFIX "deathmsg fake <attackerId> <victimId> <0|1> <weaponString> - Use at your own risk.\n"
			);
			return;
		}
		if(!_stricmp(acmd, "max")) {
			if(3==argc) {
				deathMessagesMax = atoi(pEngfuncs->Cmd_Argv(2));
		
				if(deathMessagesMax < 0) deathMessagesMax = 0;
				return;
			}
			
			pEngfuncs->Con_Printf(
				PREFIX "deathmsg max <value>\n"
				"Current: %i\n",
				deathMessagesMax
			);
			return;
		}
		if(!_stricmp(acmd, "offset"))
		{
			if(3==argc)
			{
				acmd = pEngfuncs->Cmd_Argv(2);
				g_DeathMsg_ForceOffset = 0 != _stricmp(acmd, "default");
				if(g_DeathMsg_ForceOffset) g_DeathMsg_Offset = atoi(acmd);
				return;
			}

			pEngfuncs->Con_Printf(
				PREFIX "deathmsg offset default|<value>\n"
				"If you want the same height like in in-eye demos for HLTV demos use 40 as value.\n"
			);
			if(!g_DeathMsg_ForceOffset)
				pEngfuncs->Con_Printf("Current: default\n");
			else
				pEngfuncs->Con_Printf("Current: %i\n", g_DeathMsg_Offset);

			return;
		}
	}

	pEngfuncs->Con_Printf(
		PREFIX "deathmsg\n"
		"\tblock - block messages\n"
		"\tfake - fake a message\n"
		"\tmax - maximum hud history row count\n"
		"\toffset - set death message screen offset\n"
	);
}

REGISTER_CMD_FUNC(cstrike_deathmsg)
{
	CALL_CMD(deathmsg)
}
