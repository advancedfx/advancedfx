#include "stdafx.h"

#include "csgo_c_baseanimatingoverlay.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "RenderView.h"
#include "csgo_MemAlloc.h"
#include "WrpConsole.h"

#include <shared/AfxDetours.h>

#include <map>

typedef void csgo_CRecvProxyData_t;
typedef void(*csgo_C_BasePlayer_RecvProxy_t)(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut);

csgo_C_BasePlayer_RecvProxy_t detoured_csgo_RecvProxy_m_flCycle;
csgo_C_BasePlayer_RecvProxy_t detoured_csgo_RecvProxy_m_flPrevCycle;


int g_csgo_PlayerAnimStateFix = 0;

float mirv_cycle_mod(float value)
{
	if (value < 0)
		while (value < 0) value += 1.0f;
	else
		while (1 < value) value -= 1.0f;

	return value;
}

class CCsgoPlayerAnimStateFix : public ITier0MemAllocFreeNotifyee
{
public:

	/// <returns> If pValue was changed. </returns>
	bool Fix(float * pValue, float * pOut, float & outOrgValue)
	{
		outOrgValue = *pValue;

		float newNetValue = outOrgValue;
		float newEngineValue = *pOut;

		const std::pair<const std::map<float *, Entry>::iterator, bool> & res = m_Map.insert(std::make_pair(pOut, Entry(newNetValue, newEngineValue, 0)));

		if (!res.second)
		{
			// Known.

			if (0.0f != newNetValue)
			{
				float oldNetValue = res.first->second.oldNet;
				res.first->second.oldNet = newNetValue;

				float oldEngineValue = res.first->second.oldEngine;
				res.first->second.oldEngine = newEngineValue;

				float deltaNet = newNetValue >= oldNetValue ? newNetValue - oldNetValue : newNetValue + 1.0f - oldNetValue;

				float net = oldNetValue;
				float engine = newEngineValue;
				float totalError;

				// I wish I was better at math, the correction values could need some numerical optimization!

				if (net >= engine)
				{
					if (0.5f >= net - engine)
						totalError = net - engine;
					else
						totalError = net -1.0f -engine;
				}
				else
				{
					if (0.5f >= net +1.0f -engine)
						totalError = net + 1.0f - engine;
					else
						totalError = net +1.0f - 1.0f - engine;
				}

				if (totalError < -0.5f)
				{
					// actually should never happen, just for readability.
					if (1 < g_csgo_PlayerAnimStateFix) Tier0_Warning("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Engine: %f  | Net: %f -> %f (%f) | total error: %f, ERROR in HLAE code, limiting to -0.5!.\n", pOut, newEngineValue, oldNetValue, newNetValue, deltaNet, totalError);
					totalError = -0.5f;
				}
				else
				if (0.5f < totalError)
				{
					// actually should never happen, just for readability.
					if (1 < g_csgo_PlayerAnimStateFix) Tier0_Warning("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Engine: %f  | Net: %f -> %f (%f) | total error: %f, ERROR in HLAE code, limiting to 0.5!.\n", pOut, newEngineValue, oldNetValue, newNetValue, deltaNet, totalError);
					totalError = 0.5f;
				}
				res.first->second.oldError = totalError;

				float targetVal;

				if (0.3f <= abs(totalError))
				{
					targetVal = newNetValue; // give up

					if (1 < g_csgo_PlayerAnimStateFix) Tier0_Warning("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Engine: %f  | Net: %f -> %f (%f) | total error: %f, GIVING UP, new target: %f.\n", pOut, newEngineValue, oldNetValue, newNetValue, deltaNet, totalError, targetVal);
				}
				else
				{
					float targetDelta = deltaNet +totalError;
					if (targetDelta < 0) targetDelta = 0;
					targetVal = mirv_cycle_mod(newEngineValue + targetDelta);

					//float targetDelta = 0;
					//targetVal = newEngineValue;

					if (1 < g_csgo_PlayerAnimStateFix) Tier0_Msg("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Engine: %f  | Net: %f -> %f (%f) | total error: %f, new target: %f (%f).\n", pOut, newEngineValue, oldNetValue, newNetValue, deltaNet, totalError, targetVal, targetDelta);
				}

				*pValue = targetVal;

				float * p_m_flPrevCycle = (float *)((char *)pValue - 0x10);
				*p_m_flPrevCycle = newEngineValue; // fix up this shit as well.

				return true;
			}

			if (1 < g_csgo_PlayerAnimStateFix)
				Tier0_Msg("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Got %f, RE-STARTING.\n", pOut, newNetValue);

			return false;
		}

		if (1 < g_csgo_PlayerAnimStateFix)
			Tier0_Msg("CCsgoPlayerAnimStateFix::Fix on m_flCycle at 0x%08x: Got %f, STARTING.\n", pOut, newNetValue);

		return false;
	}

	virtual void OnTier0MemAllocFree(void * pMem)
	{
		m_Map.erase((float *)pMem);
	}

private:
	struct Entry
	{
		float oldNet;
		float oldEngine;
		float oldError;

		Entry(float oldNet, float oldEngine, float oldError)
		{
			this->oldNet = oldNet;
			this->oldEngine = oldEngine;
			this->oldError = oldError;
		}
	};

	std::map<float *, Entry> m_Map;

} g_CsgoPlayerAnimStateFix;

void touring_csgo__RecvProxy_m_flCycle(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut)
{
	volatile float * pValue = (float *)((char const *)pData + 0x8);

	float oldCycle = *(float *)pOut;
	float newCycle = *pValue;

	bool broken = 0.0f != newCycle && (
		oldCycle > newCycle && 0.5f > oldCycle -newCycle
		|| oldCycle < newCycle && 0.5f < newCycle -oldCycle
	);

	if (g_csgo_PlayerAnimStateFix)
	{
		float orgValue;
		bool fixed = g_CsgoPlayerAnimStateFix.Fix((float *)pValue, (float *)pOut, orgValue);

		detoured_csgo_RecvProxy_m_flCycle(pData, pStruct, pOut);

		//Tier0_Msg("touring_csgo__RecvProxy_m_flCycle: %f\n", *pValue);

		if(fixed) *pValue = orgValue;

		//g_csgo_PlayerAnimStateFix_origValues.emplace_back(p_m_flPlaybackRate); // we want to get at m_flPlayBackRate later on :-)
		
		return;
	}
	
	static DWORD lastTickCount = 0;

	DWORD tickCount = GetTickCount();

	if(0 < g_csgo_PlayerAnimStateFix && broken && (lastTickCount +10000 <= tickCount || tickCount <= lastTickCount -10000))
	{
	
		Tier0_Warning("touring_csgo__RecvProxy_m_flCycle: HLAE detected cycle inconsistency at 0x%08x: %f -> %f! (Supressing warnings for 10 seconds.)\n", pOut, oldCycle, newCycle);
		lastTickCount = tickCount;
	}

	detoured_csgo_RecvProxy_m_flCycle(pData, pStruct, pOut);
}

void touring_csgo__RecvProxy_m_flPrevCycle(const csgo_CRecvProxyData_t *pData, void *pStruct, void *pOut)
{
	Tier0_Msg("touring_csgo__RecvProxy_m_flPrevCycle\n");

	detoured_csgo_RecvProxy_m_flPrevCycle(pData, pStruct, pOut);
}


void Enable_csgo_PlayerAnimStateFix_set(int value)
{
	if (value < 0) value = 0;
	else if (3 < value) value = 3;

	g_csgo_PlayerAnimStateFix = value;
}

int Enable_csgo_PlayerAnimStateFix_get(void)
{
	return g_csgo_PlayerAnimStateFix;
}

typedef void * csgo_mystique_animation_t;

csgo_mystique_animation_t detoured_csgo_mystique_animation;

extern bool g_csgo_FirstFrameAfterNetUpdateEnd;

float g_csgo_mystique_annimation_factor = 0.9f;

void __stdcall touring_csgo_mystique_animation(DWORD * this_ptr, DWORD arg0, float argXmm1, float argXmm2)
{
	//volatile float * mystique = (float *)((char *)this_ptr + 0x98); // new cycle value
	//volatile float * p_m_flCycle = (float *)((char*)this_ptr + 0x4FC);
	volatile float * p_m_unk_flLastCurTime = (float *)((char*)this_ptr + 0x6c);
	//volatile bool * p_m_oldSequence = (bool *)((char*)this_ptr + 0x50);


	float frameTime = g_Hook_VClient_RenderView.GetGlobals()->frametime_get();
	float curTime = g_Hook_VClient_RenderView.GetGlobals()->curtime_get();

	if (g_csgo_PlayerAnimStateFix)
	{
		*p_m_unk_flLastCurTime = curTime -frameTime;
	}

	//if(!g_csgo_PlayerAnimStateFix)
	{
		__asm mov ecx, this_ptr
		__asm push arg0
		__asm movss xmm2, argXmm2
		__asm movss xmm1, argXmm1
		__asm call detoured_csgo_mystique_animation
	}

	if (g_csgo_PlayerAnimStateFix)
	{
		*p_m_unk_flLastCurTime = curTime;
	}
}

void __declspec(naked) naked_touring_csgo_mystique_animation(void)
{
	__asm push ebp
	__asm mov ebp, esp
	__asm push ecx

	__asm sub esp, 4
	__asm movss dword ptr[esp], xmm2
	__asm sub esp, 4
	__asm movss dword ptr[esp], xmm1
	__asm mov eax, [ebp + 8]
	__asm push eax
	__asm push ecx
	__asm call touring_csgo_mystique_animation

	__asm pop ecx
	__asm mov esp, ebp
	__asm pop ebp

	__asm ret 4
}

bool Hook_csgo_PlayerAnimStateFix(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_DT_Animationlayer_m_flCycle_fn)
		//&& AFXADDR_GET(csgo_DT_Animationlayer_m_flPrevCycle_fn)
		//&& AFXADDR_GET(csgo_mystique_animation)
	)
	{
		MdtMemBlockInfos mbis;

		csgo_C_BasePlayer_RecvProxy_t * pMovArgFn;

		pMovArgFn = *(csgo_C_BasePlayer_RecvProxy_t **)AFXADDR_GET(csgo_DT_Animationlayer_m_flCycle_fn);
		//MdtMemAccessBegin(pMovArgFn, sizeof(void *), &mbis);
		detoured_csgo_RecvProxy_m_flCycle = *pMovArgFn;
		*pMovArgFn = touring_csgo__RecvProxy_m_flCycle;
		//MdtMemAccessEnd(&mbis);

		/*
		pMovArgFn = *(csgo_C_BasePlayer_RecvProxy_t **)AFXADDR_GET(csgo_DT_Animationlayer_m_flPrevCycle_fn);
		//MdtMemAccessBegin(pMovArgFn, sizeof(void *), &mbis);
		detoured_csgo_RecvProxy_m_flPrevCycle = *pMovArgFn;
		*pMovArgFn = touring_csgo__RecvProxy_m_flPrevCycle;
		//MdtMemAccessEnd(&mbis);
		*/

		//detoured_csgo_mystique_animation = DetourApply((BYTE *)AFXADDR_GET(csgo_mystique_animation), (BYTE *)naked_touring_csgo_mystique_animation, 0x0a);

		firstResult = true;
	}

	return firstResult;
}


