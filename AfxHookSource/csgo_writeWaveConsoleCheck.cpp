#include "stdafx.h"

#include "csgo_writeWaveConsoleCheck.h"

#include "addresses.h"

#include <shared/detours.h>


bool csgo_writeWaveConsoleCheckOverride = false;

DWORD csgo_writeWaveConsoleOpenJNZ_JMP = 0;
DWORD csgo_writeWaveConsoleOpenJNZ_CONTINUE = 0;

DWORD csgo_DS_CanRecord_ConsoleOpenCall_JMP = 0;
DWORD csgo_DS_CanRecord_ConsoleOpenCall_CONTINUE = 0;


__declspec(naked) void touring_csgo_writeWaveConsoleCheck(void)
{
	__asm
	{
		jnz __console_open
		jmp [csgo_writeWaveConsoleOpenJNZ_CONTINUE]

		__console_open:
		mov al, [csgo_writeWaveConsoleCheckOverride]
		test al, al
		setz al
		test al, al
		jnz __console_open_jnz
		jmp [csgo_writeWaveConsoleOpenJNZ_CONTINUE]

		__console_open_jnz:
		jmp [csgo_writeWaveConsoleOpenJNZ_JMP]
	}
}

__declspec(naked) void touring_DS_CanRecord_ConsoleOpenCall(void)
{
	__asm
	{
		call eax
		test al, al

		jnz __console_open
		jmp[csgo_DS_CanRecord_ConsoleOpenCall_CONTINUE]

		__console_open:
		mov al, [csgo_writeWaveConsoleCheckOverride]
		test al, al
		setz al
		test al, al
		jnz __console_open_jnz
		jmp[csgo_DS_CanRecord_ConsoleOpenCall_CONTINUE]

		__console_open_jnz:
		jmp[csgo_DS_CanRecord_ConsoleOpenCall_JMP]
	}
}

bool Hook_csgo_writeWaveConsoleCheck(void)
{
	static bool firstRun = true;
	static bool firstResult = false;

	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_writeWaveConsoleOpenJNZ) && AFXADDR_GET(csgo_DS_CanRecord_ConsoleOpenCall))
	{
		DWORD addr_csgo_writeWaveConsoleOpenJNZ = AFXADDR_GET(csgo_writeWaveConsoleOpenJNZ);
		csgo_writeWaveConsoleOpenJNZ_CONTINUE = addr_csgo_writeWaveConsoleOpenJNZ + 6;
		csgo_writeWaveConsoleOpenJNZ_JMP = addr_csgo_writeWaveConsoleOpenJNZ + 6 + *(DWORD *)(addr_csgo_writeWaveConsoleOpenJNZ + 2);

		Asm32ReplaceWithJmp((void *)addr_csgo_writeWaveConsoleOpenJNZ, 6, touring_csgo_writeWaveConsoleCheck);

		DWORD addr_csgo_DS_CanRecord_ConsoleOpenCall = AFXADDR_GET(csgo_DS_CanRecord_ConsoleOpenCall);
		csgo_DS_CanRecord_ConsoleOpenCall_CONTINUE = addr_csgo_DS_CanRecord_ConsoleOpenCall + 6;
		csgo_DS_CanRecord_ConsoleOpenCall_JMP = addr_csgo_DS_CanRecord_ConsoleOpenCall + 6 + (DWORD)(*(signed char *)(addr_csgo_DS_CanRecord_ConsoleOpenCall +5));

		Asm32ReplaceWithJmp((void *)addr_csgo_DS_CanRecord_ConsoleOpenCall, 6, touring_DS_CanRecord_ConsoleOpenCall);

		firstResult = true;
	}

	return firstResult;
}
