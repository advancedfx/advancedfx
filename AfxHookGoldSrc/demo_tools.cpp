#include "stdafx.h"

#include <windows.h>

#include <hlsdk.h>

#include "cmdregister.h"

#include "hl_addresses.h"
#include <shared/AfxDetours.h>

#include <list>

#include <shared/hldemo/hldemo.h>

std::list<int> g_BlockedVoiceEntsList;

//
//
//

extern cl_enginefuncs_s *pEngfuncs;

//
// For more information see:
// doc/notes_goldsrc/debug_CL_ParseServerMessage.txt

//
// Types and structs
//

typedef int qboolean;

// Quake 1 common.h:
typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;


//
// MSG Handler (by peeking ahead)
//

#define MAXCLIENTS 32

void Handle_CmdRead_Intercepted(void)
{
	int *pmsg_readcount = (int *)HL_ADDR_GET(msg_readcount);
	sizebuf_t * pnet_message = (sizebuf_t *)HL_ADDR_GET(net_message);
	int myreadcount=*pmsg_readcount;

	bool bScan = true;

	while(bScan)
	{

		// now similar to Quake's MSG_ReadByte():

		if (myreadcount+1 > pnet_message->cursize)
			return; // msg_badread or end of stream

		unsigned char uc = (unsigned char)(pnet_message->data[myreadcount]);
		myreadcount++;

		switch (uc)
		{
		case svc_voicedata:
			// 4 + 4 + 4 + 16
			unsigned char ucEntity;
			unsigned short uscbSize;
			int iEntity;

			if (myreadcount+(int)sizeof(ucEntity)+(int)sizeof(uscbSize) > pnet_message->cursize)
				return; // msg_badread

			ucEntity = (unsigned char)(pnet_message->data[myreadcount]);
			myreadcount += (int)sizeof(ucEntity);
			uscbSize = *(unsigned short *)((unsigned char *)(pnet_message->data) +myreadcount);
			myreadcount += (int)sizeof(uscbSize)+ uscbSize;

			if (myreadcount > pnet_message->cursize)
				return; // msg_badread

			iEntity = ucEntity+1;
			for (std::list<int>::iterator iter = g_BlockedVoiceEntsList.begin(); iter != g_BlockedVoiceEntsList.end(); iter++)
			{
				if (*iter==iEntity)
				{
					// blocked, skip the message:
					MdtMemBlockInfos mbis;
					MdtMemAccessBegin(pmsg_readcount, sizeof(int), &mbis);
					*pmsg_readcount = myreadcount;
					MdtMemAccessEnd(&mbis);
					break;
				}
			}
			break;
		default:
			bScan=false;
		}
	}

}

//
// Hooking related
//

DWORD dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue=NULL;

__declspec(naked) void tour_CL_ParseServerMessage_CmdRead()
{
	Handle_CmdRead_Intercepted();
	__asm
	{
		JMP [dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue]
	}
}

void install_tour_CL_ParseServerMessage_CmdRead()
{
	if(dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue) return;

	dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue = (DWORD)DetourApply((BYTE *)HL_ADDR_GET(CL_ParseServerMessage_CmdRead),(BYTE *)tour_CL_ParseServerMessage_CmdRead,HL_ADDR_GET(CL_ParseServerMessage_CmdRead_DSZ));

	// adjust call in detoured part:
	DWORD *pdwFixAdr = (DWORD *)((char *)dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue + 0x02 + 0x01);

	DWORD dwAdr = *pdwFixAdr;

	dwAdr -= dwHL_ADDR_CL_ParseServerMessage_CmdRead_continue - HL_ADDR_GET(CL_ParseServerMessage_CmdRead);

	*pdwFixAdr = dwAdr;
}

REGISTER_CMD_FUNC(voice_block)
{
	install_tour_CL_ParseServerMessage_CmdRead();
	
	bool bShowHelp=true;
	int icarg=pEngfuncs->Cmd_Argc();

	if (icarg ==2 )
	{
		char* pcmd = pEngfuncs->Cmd_Argv(1);

		if (strcmp(pcmd,"list")==0)
		{
			bShowHelp=false;
			pEngfuncs->Con_Printf("Blocked ids: ");
			for (std::list<int>::iterator iter = g_BlockedVoiceEntsList.begin(); iter != g_BlockedVoiceEntsList.end(); iter++)
				pEngfuncs->Con_Printf("%i, ",*iter);
			pEngfuncs->Con_Printf("END.\n");
		}
		else if (strcmp(pcmd,"clear")==0)
		{
			bShowHelp=false;
			g_BlockedVoiceEntsList.clear();
		}
	} else if (icarg==3) {
		char* pcmd = pEngfuncs->Cmd_Argv(1);
		int iid = atoi(pEngfuncs->Cmd_Argv(2));

		if (strcmp(pcmd,"add")==0)
		{
			bShowHelp=false;
			g_BlockedVoiceEntsList.push_back(iid);
		}
		else if (strcmp(pcmd,"del")==0)
		{
			bShowHelp=false;
			bool bfound=false;
			for (std::list<int>::iterator iter = g_BlockedVoiceEntsList.begin(); iter != g_BlockedVoiceEntsList.end(); iter++)
			{
				if(*iter==iid)
				{
					bfound=true;
					g_BlockedVoiceEntsList.erase(iter);
					break;
				}
			}
			if (bfound) pEngfuncs->Con_Printf("Deleted %i from list.\n",iid);
			else  pEngfuncs->Con_Printf("%i not found in list.\n",iid);
		}
	}

	if (bShowHelp)
	{
		pEngfuncs->Con_Printf("Usage:\n" PREFIX "voice_block add <id> - adds id to list of blocked entity ids\n" PREFIX "voice_block list\n" PREFIX "voice_block del <id>\n" PREFIX "voice_block clear - clears complete list\n");
	}
}
