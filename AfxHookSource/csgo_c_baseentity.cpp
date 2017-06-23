#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-03-04 dominik.matrixstorm.com
//
// First changes:
// 2017-03-04 dominik.matrixstorm.com

#include "csgo_c_baseentity.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "RenderView.h"

#include <shared/detours.h>

#include <map>

bool g_csgo_Force_GetInterpolationAmount = false;

typedef float(__stdcall *csgo_C_BaseEntity_GetInterpolationAmount_t)(DWORD *this_ptr, int flags);

csgo_C_BaseEntity_GetInterpolationAmount_t detoured_csgo__C_BaseEntity_GetInterpolationAmount;

struct MirvSimTimeData
{
	float lastTime;
	float lastTime2;
};

std::map<DWORD *, MirvSimTimeData> m_LastSim;

float csgo_mirv_interp_amount(float value, DWORD *this_ptr)
{

	Tier0_Msg("IN: %f, ", value);
	if (g_csgo_Force_GetInterpolationAmount)
	{
		float * pSimTime = (float *)((char *)this_ptr + 264);

		MirvSimTimeData & simData = m_LastSim[this_ptr];

		value = *pSimTime -simData.lastTime;
		simData.lastTime = value;
		if (0 == value)
		{
			value = *pSimTime - simData.lastTime2;
		}
		else
			simData.lastTime2 = *pSimTime;
	}
	Tier0_Msg("OUT: %f\n", value);


	return value;
}

void __declspec(naked) touring_csgo__C_BaseEntity_GetInterpolationAmount(DWORD *this_ptr, int flags)
{
	__asm push ebp
	__asm mov ebp, esp
	__asm push [ebp+8]
	__asm push [ebp+4]
	__asm call detoured_csgo__C_BaseEntity_GetInterpolationAmount
	__asm push [ebp+4]
	__asm push 0
	__asm movss [esp], xmm0
	__asm call csgo_mirv_interp_amount
	__asm fstp [esp]
	__asm movss xmm0, [esp]
	__asm add esp, 8
	__asm mov esp, ebp
	__asm pop ebp
	__asm ret 8
}

bool Hook_csgo_C_BaseEntity_GetInterpolationAmount(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CUnknown_GetPlayerName))
	{
		detoured_csgo__C_BaseEntity_GetInterpolationAmount = (csgo_C_BaseEntity_GetInterpolationAmount_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_C_BaseEntity_GetInterpolationAmount), (BYTE *)touring_csgo__C_BaseEntity_GetInterpolationAmount, (int)0xc);

		firstResult = true;
	}

	return firstResult;
}

