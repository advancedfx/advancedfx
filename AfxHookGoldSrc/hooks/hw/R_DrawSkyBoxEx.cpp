#include "stdafx.h"

#include "R_DrawSkyBoxEx.h"

#include <hl_addresses.h>
#include <shared/AfxDetours.h>

#include <Windows.h>
#include "../../../shared/AfxDetours.h"

typedef void (*R_DrawSkyBoxEx_t) (void);
R_DrawSkyBoxEx_t g_Old_R_DrawSkyBoxEx = 0;

GLuint g_oldSkyTextures[6];
GLuint * g_R_DrawSkyBoxEx_NewTextures = 0;

bool g_bReplacedSkyTextures = false;

void * g_Old_R_DrawSkyBox_Begin = nullptr;
void * g_Old_R_DrawSkyBox_End = nullptr;


void __cdecl On_R_DrawSkyBox_Begin() {
	if(g_R_DrawSkyBoxEx_NewTextures)
	{
		g_bReplacedSkyTextures = true;

		MdtMemBlockInfos mbis;

		MdtMemAccessBegin((LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint), &mbis);

		memcpy(g_oldSkyTextures, (LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint));
		memcpy((LPVOID)HL_ADDR_GET(skytextures), g_R_DrawSkyBoxEx_NewTextures, 6*sizeof(GLuint));

		MdtMemAccessEnd(&mbis);
	}
	else
		g_bReplacedSkyTextures = false;
}

void __declspec(naked) Touring_R_DrawSkyBox_Begin() {
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call On_R_DrawSkyBox_Begin
	__asm pop edx
	__asm pop ecx
	__asm pop eax
	__asm jmp g_Old_R_DrawSkyBox_Begin
}

void __cdecl On_R_DrawSkyBox_End() {
	if(g_bReplacedSkyTextures)
	{
		MdtMemBlockInfos mbis;

		MdtMemAccessBegin((LPVOID)HL_ADDR_GET(skytextures), 6*sizeof(GLuint), &mbis);

		memcpy((LPVOID)HL_ADDR_GET(skytextures), g_oldSkyTextures, 6*sizeof(GLuint));

		MdtMemAccessEnd(&mbis);
	}
}


void __declspec(naked) Touring_R_DrawSkyBox_End() {
	__asm push eax
	__asm push ecx
	__asm push edx
	__asm call On_R_DrawSkyBox_End
	__asm pop edx
	__asm pop ecx
	__asm pop eax
	__asm jmp g_Old_R_DrawSkyBox_End
}


bool Hook_R_DrawSkyBoxEx()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(R_DrawSkyBox_Begin)
		&& AFXADDR_GET(R_DrawSkyBox_End)
		&& AFXADDR_GET(skytextures)
	) {
		LONG error = NO_ERROR;

		void * R_DrawSkyBox_Begin_Continue = (void *)(AFXADDR_GET(R_DrawSkyBox_Begin)+8);

		g_Old_R_DrawSkyBox_Begin = MdtAllocExecuteableMemory(
			8 // for the original code
			+ 5 // for or JMP back into original code
		);

		Asm32ReplaceWithJmp(&((unsigned char *)g_Old_R_DrawSkyBox_Begin)[8],5,R_DrawSkyBox_Begin_Continue);

		MdtMemBlockInfos mbis;
		MdtMemAccessBegin((LPVOID)AFXADDR_GET(R_DrawSkyBox_Begin),7,&mbis);
		memcpy(g_Old_R_DrawSkyBox_Begin,(LPCVOID)AFXADDR_GET(R_DrawSkyBox_Begin),8);
		MdtMemAccessEnd(&mbis);

		void * R_DrawSkyBox_End_Continue = (void *)(AFXADDR_GET(R_DrawSkyBox_End)+6);
		void * R_DrawSkyBox_End_JZ_Continue = (void *)(AFXADDR_GET(R_DrawSkyBox_End)+16);

		g_Old_R_DrawSkyBox_End = MdtAllocExecuteableMemory(
			4 // for the original code (TEST, POP, POP)
			+ 2 // for the new JZ to other JMP
			+ 5 // for or JMP back into original code
			+ 5 // for or JMP back into original code after JZ
		);

		((unsigned char *)g_Old_R_DrawSkyBox_End)[4] = 0x74 ; // JZ
		((unsigned char *)g_Old_R_DrawSkyBox_End)[5] = 10 ; // JUMP to 10 bytes after JZ.

		Asm32ReplaceWithJmp(&((unsigned char *)g_Old_R_DrawSkyBox_End)[6],5,R_DrawSkyBox_End_Continue);
		Asm32ReplaceWithJmp(&((unsigned char *)g_Old_R_DrawSkyBox_End)[11],5,R_DrawSkyBox_End_JZ_Continue);

		MdtMemAccessBegin((LPVOID)AFXADDR_GET(R_DrawSkyBox_End),4,&mbis);
		memcpy(g_Old_R_DrawSkyBox_End,(LPCVOID)AFXADDR_GET(R_DrawSkyBox_End),4);
		MdtMemAccessEnd(&mbis);

		Asm32ReplaceWithJmp((void*)AFXADDR_GET(R_DrawSkyBox_Begin),8,Touring_R_DrawSkyBox_Begin);
		Asm32ReplaceWithJmp((void*)AFXADDR_GET(R_DrawSkyBox_End),4+2,Touring_R_DrawSkyBox_End);

		firstResult = true;
	}
	else
		firstResult = false;

	return firstResult;
}
