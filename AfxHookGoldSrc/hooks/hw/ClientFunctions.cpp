#include "stdafx.h"

#include "ClientFunctions.h"

#include "../../hl_addresses.h"
#include <shared/AfxDetours.h>

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

	g_ClientFunctionTable = (void * *)AFXADDR_GET(engine_ClientFunctionTable);
}

void ReplaceClientFunction(ClientFunctionTableEntry entry, void * newFunction)
{
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin(g_ClientFunctionTable +entry, sizeof(void *), &mbis);

	*(g_ClientFunctionTable +entry) = newFunction;

	MdtMemAccessEnd(&mbis);
}
