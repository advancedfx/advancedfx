#include "stdafx.h"

#include "UnkDrawHud.h"

#include <shared/AfxDetours.h>
#include <hl_addresses.h>

#include "../../filming.h"

#ifdef AFX_SCRIPT
#include "../../scripting.h"
#endif // AFX_SCRIPT

void * g_UnkDrawHudInContinue = 0;
void * g_UnkDrawHudOutContinue;
void * g_UnkDrawHudInCall;
void * g_UnkDrawHudOutCall;

bool g_UnkDrawHudCallFromEngine = true;
bool g_UnkDrawHudCalledFromEngine = false;


void Wrapper_UnkDrawHudIn(void)
{
#ifdef AFX_SCRIPT
	ScriptEvent_OnHudBegin();
#endif // AFX_SCRIPT
	g_Filming.OnHudBeginEvent();
}


bool Wrapper_UnkDrawHudOut(void)
{
	bool b1, b2;
	b1 = g_Filming.OnHudEndEvent();
#ifdef AFX_SCRIPT
	b2 = ScriptEvent_OnHudEnd();
#else
	b2 = false;
#endif // AFX_SCRIPT

	if(g_UnkDrawHudCallFromEngine)
		g_UnkDrawHudCalledFromEngine = true;

	return b1 || b2;
}

DWORD g_New_UnkDrawHudIn_TempMem;

__declspec(naked) void New_UnkDrawHudIn(void)
{
	__asm call Wrapper_UnkDrawHudIn

	__asm mov al, g_UnkDrawHudCallFromEngine
	__asm cmp al, 0
	__asm jnz __ContinueJmp

	__asm ; emulate prologue (since we are called as function):
	__asm push ebp
	__asm push ebx
	__asm xor ebx, ebx
	__asm mov ebp, offset g_New_UnkDrawHudIn_TempMem
	__asm add ebp, 4

	__asm __ContinueJmp:
	__asm call [g_UnkDrawHudInCall]
	__asm jmp [g_UnkDrawHudInContinue]
}

__declspec(naked) void New_UnkDrawHudOut(void)
{
	__asm call [g_UnkDrawHudOutCall]
	
	__asm call Wrapper_UnkDrawHudOut
	__asm cmp al, 0
	__asm jnz New_UnkDrawHudIn

	__asm mov al, g_UnkDrawHudCallFromEngine
	__asm cmp al, 0
	__asm jnz __ContinueJmp

	__asm ; emulate epilogue (since we are called as function):
	__asm pop ebx
	__asm pop ebp
	__asm ret
	
	__asm __ContinueJmp:
	__asm jmp [g_UnkDrawHudOutContinue]
}

void Additional_UnkDrawHud()
{
	g_UnkDrawHudCallFromEngine = false;

	if(g_UnkDrawHudCalledFromEngine)
	{
		New_UnkDrawHudIn();
	}

	g_UnkDrawHudCallFromEngine = true;
}

void Reset_UnkDrawHudCalledFromEngine()
{
	g_UnkDrawHudCalledFromEngine = false;
}

void Hook_UnkDrawHud()
{
	if (!g_UnkDrawHudInContinue && NULL != HL_ADDR_GET(UnkDrawHudIn))
	{
		Asm32ReplaceWithJmp((void *)HL_ADDR_GET(UnkDrawHudIn), HL_ADDR_GET(UnkDrawHudInContinue)-HL_ADDR_GET(UnkDrawHudIn), (void *)New_UnkDrawHudIn);
		Asm32ReplaceWithJmp((void *)HL_ADDR_GET(UnkDrawHudOut), HL_ADDR_GET(UnkDrawHudOutContinue)-HL_ADDR_GET(UnkDrawHudOut), (void *)New_UnkDrawHudOut);
		
		g_UnkDrawHudInContinue = (void *)HL_ADDR_GET(UnkDrawHudInContinue);
		g_UnkDrawHudOutContinue = (void *)HL_ADDR_GET(UnkDrawHudOutContinue);
		g_UnkDrawHudInCall = (void *)HL_ADDR_GET(UnkDrawHudInCall);
		g_UnkDrawHudOutCall = (void *)HL_ADDR_GET(UnkDrawHudOutCall);
	}
}

