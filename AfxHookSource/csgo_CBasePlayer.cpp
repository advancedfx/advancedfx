#include "stdafx.h"

#include "csgo_CBasePlayer.h"

#include "addresses.h"

#include <shared/AfxDetours.h>


typedef void csgo_CRecvProxyData_t;

typedef void(*csgo_C_BasePlayer_RecvProxy_ObserverTarget_t)(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut);

csgo_C_BasePlayer_RecvProxy_ObserverTarget_t detoured_csgo_C_BasePlayer_RecvProxy_ObserverTarget;

bool g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget = false;

void touring_csgo_C_BasePlayer_RecvProxy_ObserverTarget(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut)
{
	if (!g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget)
		detoured_csgo_C_BasePlayer_RecvProxy_ObserverTarget(pData, pStruct, pOut);
}

bool Hook_csgo_C_BasePlayer_RecvProxy_ObserverTarget(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_BasePlayer_RecvProxy_ObserverTarget))
	{
		detoured_csgo_C_BasePlayer_RecvProxy_ObserverTarget = (csgo_C_BasePlayer_RecvProxy_ObserverTarget_t)DetourApply((BYTE *)AFXADDR_GET(csgo_C_BasePlayer_RecvProxy_ObserverTarget), (BYTE *)touring_csgo_C_BasePlayer_RecvProxy_ObserverTarget, (int)0x06);

		firstResult = true;
	}

	return firstResult;
}
/*
typedef void csgo_datamap_t;

typedef int(__stdcall * csgo_CPredictionCopy_TransferData_t)(DWORD * this_ptr, const char *operation, int entindex, csgo_datamap_t *dmap);

csgo_CPredictionCopy_TransferData_t detoured_csgo_CPredictionCopy_TransferData;

int g_csgo_NetOnly_CPredictionCopy_TransferData_EntIndex = -1;

enum
{
	CS_GO_PC_NON_NETWORKED_ONLY = 0,
	CS_GO_PC_NETWORKED_ONLY,

	CS_GO_PC_COPYTYPE_COUNT,
	CS_GO_PC_EVERYTHING = CS_GO_PC_COPYTYPE_COUNT,
};

int __stdcall touring_csgo_CPredictionCopy_TransferData(DWORD * this_ptr, const char *operation, int entindex, csgo_datamap_t *dmap)
{
	if (0 <= g_csgo_NetOnly_CPredictionCopy_TransferData_EntIndex)
	{
		int * p_m_nType = (int *)((DWORD)this_ptr + 0x4);

		*p_m_nType = CS_GO_PC_NETWORKED_ONLY;
	}
	
	return detoured_csgo_CPredictionCopy_TransferData(this_ptr, operation, entindex, dmap);
}


bool Hook_csgo_CPredictionCopy_TransferData(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CPredictionCopy_TransferData))
	{
		detoured_csgo_CPredictionCopy_TransferData = (csgo_CPredictionCopy_TransferData_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CPredictionCopy_TransferData), (BYTE *)touring_csgo_CPredictionCopy_TransferData, (int)AFXADDR_GET(csgo_CPredictionCopy_TransferData_DSZ));

		firstResult = true;
	}

	return firstResult;
}
*/
