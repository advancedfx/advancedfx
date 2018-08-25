#include "stdafx.h"

#include "ClientFunctions.h"

#include "../../hl_addresses.h"
#include <shared/detours.h>

#include "../client/HookClient.h"

void * * g_ClientFunctionTable = 0;

void * GetClientFunction(ClientFunctionTableEntry entry)
{
	return g_ClientFunctionTable[entry];
}

void HookClientFunctions()
{
	static bool firstRun = true;
	if(!firstRun) return;
	firstRun = true;

	g_ClientFunctionTable = (void * *)HL_ADDR_GET(ClientFunctionTable);

	HookClient();
}

void ReplaceClientFunction(ClientFunctionTableEntry entry, void * newFunction)
{
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin(g_ClientFunctionTable +entry, sizeof(void *), &mbis);

	*(g_ClientFunctionTable +entry) = newFunction;

	MdtMemAccessEnd(&mbis);
}
