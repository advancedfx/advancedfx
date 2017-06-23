#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-03-03 dominik.matrixstorm.com
//
// First changes:
// 2017-03-03 dominik.matrixstorm.com

#include "csgo_dt_recv.h"

#include "addresses.h"
#include "SourceInterfaces.h"

#include <shared/detours.h>

typedef void csgo_CRecvProxyData_t;

typedef void(*csgo_RecvProxy_Int32ToInt8_t)(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut);

csgo_RecvProxy_Int32ToInt8_t detoured_csgo_RecvProxy_Int32ToInt8;

bool g_csgo_dt_recv_force_players_m_bClientSideAnimation = false;

void touring_csgo_RecvProxy_Int32ToInt8(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut)
{
	bool patch = g_csgo_dt_recv_force_players_m_bClientSideAnimation && (((DWORD)pOut - (DWORD)pStruct) == 0x288C); // 0x288c is offset of m_bClientSideAnimation

	if (patch)
	{
		//Tier0_Msg("CONSIDERING\n");

		patch = false;

		int imax = SOURCESDK::g_Entitylist_csgo->GetHighestEntityIndex();

		for (int i = 0; i <= imax; ++i)
		{
			SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
			SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

			if (((void *)be == pStruct))
			{
				//Tier0_Msg("CONSIDERING2\n");

				if (!strcmp("class C_CSPlayer", be->GetClassname()))
				{
					patch = true;
					break;
				}
				//else Tier0_Msg("%s\n", be->GetClassname());
			}
		}
	}

	if (patch)
	{

		bool * pValue = (bool *)((unsigned char *)pData + 0x8);
		bool oldValue = *pValue;
		Tier0_Msg("HLAE is m_bClientSideAnimation on C_CSPlayer entity with offset 0x%08x is %i, forcing 0.\n", pStruct, oldValue ? 1 : 0);
		*pValue = false;
		detoured_csgo_RecvProxy_Int32ToInt8(pData, pStruct, pOut);
		*pValue = oldValue;
		if(*(bool *)pOut == true)
			Tier0_Msg("OKAY.\n");
	}
	else
	{	
		detoured_csgo_RecvProxy_Int32ToInt8(pData, pStruct, pOut);
	}
}

bool Hook_csgo_RecvProxy_Int32ToInt8(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_RecvProxy_Int32ToInt8))
	{
		detoured_csgo_RecvProxy_Int32ToInt8 = (csgo_RecvProxy_Int32ToInt8_t)DetourApply((BYTE *)AFXADDR_GET(csgo_RecvProxy_Int32ToInt8), (BYTE *)touring_csgo_RecvProxy_Int32ToInt8, (int)0x06);

		firstResult = true;
	}

	return firstResult;
}

