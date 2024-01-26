#include "stdafx.h"

#include "R_DrawEntitiesOnList.h"

#include "../../hl_addresses.h"

#include "../../sv_hitboxes.h"

#include <Windows.h>
#include "../../../shared/AfxDetours.h"

bool g_In_R_DrawEntitiesOnList = false;

void * g_Old_R_DrawEntitiesOnList_In = nullptr;
void * g_Old_R_DrawEntitiesOnList_Out = nullptr;

void __cdecl On_R_DrawEtitiesOnList_In() {
	g_In_R_DrawEntitiesOnList = true;
}

void __declspec(naked) Touring_R_DrawEntitiesOnList_In() {
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call On_R_DrawEtitiesOnList_In
	__asm pop edx
	__asm pop ecx
	__asm pop eax
	__asm jmp g_Old_R_DrawEntitiesOnList_In
}

void __cdecl On_R_DrawEtitiesOnList_Out() {
	g_In_R_DrawEntitiesOnList = false;

	Draw_SV_Hitboxes();
}


void __declspec(naked) Touring_R_DrawEntitiesOnList_Out() {
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call On_R_DrawEtitiesOnList_Out
	__asm pop edx
	__asm pop ecx
	__asm pop eax
	__asm jmp g_Old_R_DrawEntitiesOnList_Out
}


bool Hook_R_DrawEntitiesOnList()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(R_DrawEntitiesOnList_In)
		&& AFXADDR_GET(R_DrawEntitiesOnList_Out)
	) {
		LONG error = NO_ERROR;

		void * R_DrawEntitiesOnList_In_Continue = (void *)(AFXADDR_GET(R_DrawEntitiesOnList_In)+9);

		g_Old_R_DrawEntitiesOnList_In = MdtAllocExecuteableMemory(
			9 // for the original code
			+ 5 // for or JMP back into original code
		);

		Asm32ReplaceWithJmp(&((unsigned char *)g_Old_R_DrawEntitiesOnList_In)[9],5,R_DrawEntitiesOnList_In_Continue);

		MdtMemBlockInfos mbis;
		MdtMemAccessBegin((LPVOID)AFXADDR_GET(R_DrawEntitiesOnList_In),9,&mbis);
		memcpy(g_Old_R_DrawEntitiesOnList_In,(LPCVOID)AFXADDR_GET(R_DrawEntitiesOnList_In),9);
		MdtMemAccessEnd(&mbis);

		void * R_DrawEntitiesOnList_Out_Continue = (void *)(AFXADDR_GET(R_DrawEntitiesOnList_Out)+12);

		g_Old_R_DrawEntitiesOnList_Out = MdtAllocExecuteableMemory(
			12 // for the original XOR and MOV
			+ 5 // for or JMP back into original code
		);

		Asm32ReplaceWithJmp(&((unsigned char *)g_Old_R_DrawEntitiesOnList_Out)[12],5,R_DrawEntitiesOnList_Out_Continue);

		MdtMemAccessBegin((LPVOID)AFXADDR_GET(R_DrawEntitiesOnList_Out),12,&mbis);
		memcpy(g_Old_R_DrawEntitiesOnList_Out,(LPCVOID)AFXADDR_GET(R_DrawEntitiesOnList_Out),12);
		MdtMemAccessEnd(&mbis);

		Asm32ReplaceWithJmp((void*)AFXADDR_GET(R_DrawEntitiesOnList_In),9,Touring_R_DrawEntitiesOnList_In);
		Asm32ReplaceWithJmp((void*)AFXADDR_GET(R_DrawEntitiesOnList_Out),12,Touring_R_DrawEntitiesOnList_Out);

		firstResult = true;
	}
	else
		firstResult = false;

	return firstResult;
}
