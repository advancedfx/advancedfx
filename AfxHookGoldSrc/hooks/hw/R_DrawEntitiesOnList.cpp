#include "stdafx.h"

#include "R_DrawEntitiesOnList.h"

#include "../../hl_addresses.h"

#include "../../sv_hitboxes.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

bool g_In_R_DrawEntitiesOnList = false;

typedef void (*R_DrawEntitiesOnList_t) (void);
R_DrawEntitiesOnList_t g_Old_R_DrawEntitiesOnList = 0;

void New_R_DrawEntitiesOnList (void)
{
	g_In_R_DrawEntitiesOnList = true;
	g_Old_R_DrawEntitiesOnList();
	g_In_R_DrawEntitiesOnList = false;

	Draw_SV_Hitboxes();
}


bool Hook_R_DrawEntitiesOnList()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(R_DrawEntitiesOnList))
	{
		LONG error = NO_ERROR;

		g_Old_R_DrawEntitiesOnList = (R_DrawEntitiesOnList_t)AFXADDR_GET(R_DrawEntitiesOnList);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_R_DrawEntitiesOnList, New_R_DrawEntitiesOnList);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_DrawEntitiesOnList");
		}
	}
	else
		firstResult = false;

	return firstResult;
}
