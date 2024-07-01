#include "stdafx.h"
#include "csgo_net_chan.h"

#include "csgo/ClientToolsCsgo.h"

#include "WrpConsole.h"
#include "WrpVEngineClient.h"

#include "addresses.h"
#include <SourceInterfaces.h>
#include <csgo/bitbuf/demofilebitbuf.h>

#include <build/protobuf/csgo/netmessages.pb.h>

#include <shared/AfxDetours.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


int g_i_MirvPov = 0;
int g_Org_svc_ServerInfo_PlayerSlot = 0;
bool g_WasHltv = 0;

struct csgo_bf_read {
	char const* m_pDebugName;
	bool m_bOverflow;
	int m_nDataBits;
	size_t m_nDataBytes;
	unsigned int m_nInBufWord;
	int m_nBitsAvail;
	unsigned int const* m_pDataIn;
	unsigned int const* m_pBufferEnd;
	unsigned int const* m_pData;
};


typedef const SOURCESDK::QAngle& (__fastcall *csgo_C_CSPlayer_EyeAngles_t)(SOURCESDK::C_BaseEntity_csgo* This, void* Edx);
csgo_C_CSPlayer_EyeAngles_t Truecsgo_C_CSPlayer_EyeAngles;

const SOURCESDK::QAngle& __fastcall Mycsgo_C_CSPlayer_EyeAngles(SOURCESDK::C_BaseEntity_csgo * This, void * Edx)
{
	if (g_i_MirvPov)
	{
		if (This->entindex() == g_i_MirvPov)
		{
			DWORD ofs = AFXADDR_GET(csgo_C_CSPlayer_ofs_m_angEyeAngles);
			return *((SOURCESDK::QAngle*)((char *)This + ofs));
		}
	}

	return Truecsgo_C_CSPlayer_EyeAngles(This, Edx);
}

typedef void csgo_CNetChan_t;
typedef int(__fastcall* csgo_CNetChan_ProcessMessages_t)(csgo_CNetChan_t* This, void* edx, csgo_bf_read * pReadBuf, bool bWasReliable);
csgo_CNetChan_ProcessMessages_t Truecsgo_CNetChan_ProcessMessages = 0;

int __fastcall Mycsgo_CNetChan_ProcessMessages(csgo_CNetChan_t* This, void* Edx, csgo_bf_read* pReadBuf, bool bWasReliable)
{
	if (g_i_MirvPov)
	{
		SOURCESDK::CSGO::CBitRead readBuf(pReadBuf->m_pData, pReadBuf->m_nDataBytes);

		while (0 < readBuf.GetNumBytesLeft())
		{
			int packet_cmd = readBuf.ReadVarInt32();
			int packet_size = readBuf.ReadVarInt32();
			if (packet_size < readBuf.GetNumBytesLeft())
			{
				switch (packet_cmd)
				{
				case svc_ServerInfo:
					{
						CSVCMsg_ServerInfo msg;
						msg.ParseFromArray(readBuf.GetBasePointer() + readBuf.GetNumBytesRead(), packet_size);
						g_WasHltv = false;
						if (msg.has_is_hltv())
						{
							g_WasHltv = msg.is_hltv();
							msg.set_is_hltv(false);
						}
						if (msg.has_player_slot())
						{
							g_Org_svc_ServerInfo_PlayerSlot = msg.player_slot();
							msg.set_player_slot(g_i_MirvPov - 1);
						} else {
							g_Org_svc_ServerInfo_PlayerSlot = 0;
						}
						msg.SerializePartialToArray(const_cast<unsigned char*>(readBuf.GetBasePointer()) + readBuf.GetNumBytesRead(), packet_size);
					}
					break;
				case svc_SetView:
					{
						CSVCMsg_SetView msg;
						msg.ParseFromArray(readBuf.GetBasePointer() + readBuf.GetNumBytesRead(), packet_size);
						if (msg.has_entity_index())
						{
							msg.set_entity_index(g_i_MirvPov);
						}
						msg.SerializePartialToArray(const_cast<unsigned char*>(readBuf.GetBasePointer()) + readBuf.GetNumBytesRead(), packet_size);
					}
					break;
				}

				readBuf.SeekRelative(packet_size * 8);
			}
			else
				break;
		}
	}

	bool result = Truecsgo_CNetChan_ProcessMessages(This, Edx, pReadBuf, bWasReliable);

	if (g_i_MirvPov)
	{
		static WrpConVarRef cvarClPredict;
		cvarClPredict.RetryIfNull("cl_predict"); // GOTV would have this on 0, so force it too.
		cvarClPredict.SetDirectHack(0);
	}

	return result;
}

bool csgo_CNetChan_ProcessMessages_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CNetChan_ProcessMessages))
	{
		LONG error = NO_ERROR;

		Truecsgo_CNetChan_ProcessMessages = (csgo_CNetChan_ProcessMessages_t)AFXADDR_GET(csgo_CNetChan_ProcessMessages);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_CNetChan_ProcessMessages, Mycsgo_CNetChan_ProcessMessages);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


bool csgo_C_CSPlayer_EyeAngles_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_CSPlayer_vtable) && AFXADDR_GET(csgo_client_C_CSPlayer_EyeAngles_vtable_index))
	{
		AfxDetourPtr((PVOID*)&(((DWORD*)AFXADDR_GET(csgo_C_CSPlayer_vtable))[AFXADDR_GET(csgo_client_C_CSPlayer_EyeAngles_vtable_index)]), Mycsgo_C_CSPlayer_EyeAngles, (PVOID*)&Truecsgo_C_CSPlayer_EyeAngles);

		firstResult = true;
	}

	return firstResult;
}


typedef float(__fastcall* csgo_C_CS_Player__GetFOV_t)(SOURCESDK::C_BasePlayer_csgo* This, void* Edx);

csgo_C_CS_Player__GetFOV_t True_csgo_C_CS_Player__GetFOV;

float __fastcall My_csgo_C_CS_Player__GetFOV(SOURCESDK::C_BasePlayer_csgo * This, void* Edx)
{
	if (g_i_MirvPov && This->entindex() == g_i_MirvPov)
	{
		bool* pIsLocalPlayer = (bool*)((char*)This + AFXADDR_GET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer));

		bool oldIsLocalPlayer = *pIsLocalPlayer;
		
		*pIsLocalPlayer = false;

		float result = True_csgo_C_CS_Player__GetFOV(This, Edx);

		*pIsLocalPlayer = oldIsLocalPlayer;

		return result;
	}

	return True_csgo_C_CS_Player__GetFOV(This, Edx);
}

bool Install_csgo_C_CS_Player__GetFOVs(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_CSPlayer_vtable) && AFXADDR_GET(csgo_client_C_CS_Player_GetFOV_vtable_index))
	{
		AfxDetourPtr((PVOID*)&(((DWORD*)AFXADDR_GET(csgo_C_CSPlayer_vtable))[AFXADDR_GET(csgo_client_C_CS_Player_GetFOV_vtable_index)]), My_csgo_C_CS_Player__GetFOV, (PVOID*)&True_csgo_C_CS_Player__GetFOV);

		firstResult = true;
	}

	return firstResult;
}



typedef int(__fastcall* csgo_DamageIndicator_MessageFunc_t)(void* This, void* Edx, const char * pMsg);
csgo_DamageIndicator_MessageFunc_t Truecsgo_DamageIndicator_MessageFunc = 0;

bool __fastcall MYcsgo_DamageIndicator_MessageFunc(void* This, void* Edx, const char* pMsg)
{
	if (g_i_MirvPov)
	{
		bool abort = false;

		__asm mov eax, pMsg
		__asm mov eax, [eax + 0x10]
		__asm cmp eax, g_i_MirvPov
		__asm jz __cont
		__asm mov abort, 1
		__asm __cont:

		if (abort) return true;
	}

	return Truecsgo_DamageIndicator_MessageFunc(This, Edx, pMsg);
}

bool csgo_DamageIndicator_MessageFunc_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_DamageIndicator_MessageFunc))
	{
		LONG error = NO_ERROR;

		Truecsgo_DamageIndicator_MessageFunc = (csgo_DamageIndicator_MessageFunc_t)AFXADDR_GET(csgo_DamageIndicator_MessageFunc);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)Truecsgo_DamageIndicator_MessageFunc, MYcsgo_DamageIndicator_MessageFunc);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


typedef void(__fastcall* csgo_C_CSPlayer_UpdateOnRemove_t)(SOURCESDK::C_BasePlayer_csgo* This, void* Edx);

csgo_C_CSPlayer_UpdateOnRemove_t Truecsgo_C_CSPlayer_UpdateOnRemove = nullptr;

typedef void(__fastcall* csgo_C_BasePlayer_SetAsLocalPlayer_t)(void* Ecx, void* Edx);

void __fastcall Mycsgo_C_CSPlayer_UpdateOnRemove(SOURCESDK::C_BasePlayer_csgo* This, void* Edx)
{
	if (g_i_MirvPov && This->entindex() == g_i_MirvPov)
	{
		if (SOURCESDK::IClientEntity_csgo* ce1 = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_Org_svc_ServerInfo_PlayerSlot+1))
		{
			if (SOURCESDK::C_BaseEntity_csgo* be1 = ce1->GetBaseEntity())
			{
				if (be1->IsPlayer())
				{
					// The target fake local player is being deleted, emergency case, switch back to real one.

					static csgo_C_BasePlayer_SetAsLocalPlayer_t setAsLocalPlayer = (csgo_C_BasePlayer_SetAsLocalPlayer_t)AFXADDR_GET(csgo_C_BasePlayer_SetAsLocalPlayer);
					setAsLocalPlayer(be1, 0);
				}
			}
		}
	}

	Truecsgo_C_CSPlayer_UpdateOnRemove(This, Edx);
}

bool csgo_C_CSPlayer_UpdateOnRemove_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_CSPlayer_vtable) && AFXADDR_GET(csgo_client_C_CSPlayer_UpdateOnRemove_vtable_index))
	{
		AfxDetourPtr((PVOID*)&(((DWORD*)AFXADDR_GET(csgo_C_CSPlayer_vtable))[AFXADDR_GET(csgo_client_C_CSPlayer_UpdateOnRemove_vtable_index)]), Mycsgo_C_CSPlayer_UpdateOnRemove, (PVOID*)&Truecsgo_C_CSPlayer_UpdateOnRemove);

		firstResult = true;
	}

	return firstResult;
}

typedef void (__fastcall * C_BaseViewModel_FireEvent_t)(SOURCESDK::C_BaseViewModel_csgo * This, void * Edx, const SOURCESDK::Vector& origin, const SOURCESDK::QAngle& angles, int event, const char *options );

C_BaseViewModel_FireEvent_t g_Org_C_BaseViewModel_FireEvent;

void __fastcall My_C_BaseViewModel_FireEvent(SOURCESDK::C_BaseViewModel_csgo * This, void * Edx,  const SOURCESDK::Vector& origin, const SOURCESDK::QAngle& angles, int event, const char *options ) {

	if(0 != g_i_MirvPov) {
		// Fix only play (sound) events for viewmodels of of local player in case when not observing.

		if (SOURCESDK::IClientEntity_csgo* ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_i_MirvPov))
		{
			if (SOURCESDK::C_BaseEntity_csgo* be = ce->GetBaseEntity())
			{
				if (be->IsPlayer()
					&& 0 == ((SOURCESDK::C_BasePlayer_csgo *)be)->GetObserverMode() // OBS_MODE_NONE
				){
					if(SOURCESDK::C_BaseEntity_csgo* pe = This->GetOwner()) {
						if (pe->entindex() != ce->entindex())
						 return;
					}

				}

			}
		}
	}

	g_Org_C_BaseViewModel_FireEvent(This, Edx, origin, angles, event, options);

}

bool Install_csgo_C_BaseViewModel_FireEvent(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_C_BaseViewModel_FireEvent))
	{
		LONG error = NO_ERROR;

		g_Org_C_BaseViewModel_FireEvent = (C_BaseViewModel_FireEvent_t)AFXADDR_GET(csgo_C_BaseViewModel_FireEvent);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Org_C_BaseViewModel_FireEvent, My_C_BaseViewModel_FireEvent);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}


void * g_pGameResource = nullptr;

typedef void (__fastcall * csgo_CPlayerResource_Dtor_t)( void* This, void* Edx, int flags);

csgo_CPlayerResource_Dtor_t g_Touring_csgo_CPlayerResource_Dtor;

void __fastcall My_csgo_CPlayerResource_Dtor( void* This, void* Edx, int flags) {
	if(This == g_pGameResource) g_pGameResource = nullptr;
}

typedef int (__fastcall *csgo_CPlayerResource_GetPing_t)( void* This, void* Edx, int index );

csgo_CPlayerResource_GetPing_t g_Touring_csgo_CPlayerResource_GetPing = nullptr;

int __fastcall My_csgo_CPlayerResource_GetPing( void* This, void* Edx, int index ) {
	g_pGameResource = This;

	int result = g_Touring_csgo_CPlayerResource_GetPing(This, Edx, index);

	return result;
}

int MirvGetPing(int playerIndex) {
	if(0 == g_pGameResource) {
		return 0;
	}

	return g_Touring_csgo_CPlayerResource_GetPing(g_pGameResource, 0, playerIndex);
}

bool Hook_csgo_CPlayerResource_GetPing(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable)
		&& AFXADDR_GET(csgo_client_CPlayerResource_GetPing_vtable_index))
	{
		LONG error = NO_ERROR;


		void **vtable = (void **)AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable);

		g_Touring_csgo_CPlayerResource_Dtor = (csgo_CPlayerResource_Dtor_t)vtable[0];
		g_Touring_csgo_CPlayerResource_GetPing = (csgo_CPlayerResource_GetPing_t)vtable[AFXADDR_GET(csgo_client_CPlayerResource_GetPing_vtable_index)];
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Touring_csgo_CPlayerResource_Dtor, My_csgo_CPlayerResource_Dtor);
		DetourAttach(&(PVOID&)g_Touring_csgo_CPlayerResource_GetPing, My_csgo_CPlayerResource_GetPing);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

typedef void * csgo_client_AdjustInterpolationAmount_t;

csgo_client_AdjustInterpolationAmount_t True_csgo_client_AdjustInterpolationAmount;

int g_Mirv_Pov_PingAdjustMent = 0;

// [0] - other, [1] - mirv_pov "local" player
// Default is currently normal engine behaviour.
float g_Mirv_Pov_Interp_OrgFac[2] = {2,1};
float g_Mirv_Pov_Interp_PingFac[2] = {1,0};
float g_Mirv_Pov_Interp_Offset[2] = {0,0};

void Mirv_Pov_Interp_CompensateLatencyOn() {
	g_Mirv_Pov_Interp_OrgFac[0] = 2;
	g_Mirv_Pov_Interp_PingFac[0] = 1;
	g_Mirv_Pov_Interp_Offset[0] = 0;
	g_Mirv_Pov_Interp_OrgFac[1] = 1;
	g_Mirv_Pov_Interp_PingFac[1] = 0;
	g_Mirv_Pov_Interp_Offset[1] = 0;
}

void Mirv_Pov_Interp_CompensateLatencyOff() {
	g_Mirv_Pov_Interp_OrgFac[0] = 1;
	g_Mirv_Pov_Interp_PingFac[0] = 0;
	g_Mirv_Pov_Interp_Offset[0] = 0;
	g_Mirv_Pov_Interp_OrgFac[1] = 1;
	g_Mirv_Pov_Interp_PingFac[1] = 0;
	g_Mirv_Pov_Interp_Offset[1] = 0;
}

void Mirv_Pov_Interp_Default() {
	Mirv_Pov_Interp_CompensateLatencyOn();
}

float __cdecl My_Adjust_csgo_client_AdjustInterpolationAmount(SOURCESDK::C_BasePlayer_csgo * This, float baseInterpolation) {

	if(0 == g_i_MirvPov) return baseInterpolation;

	bool wasProbablyPredicted = false;

	if(This->ShouldPredict()) {
		if(SOURCESDK::C_BasePlayer_csgo * pOwner = This->GetPredictionOwner())
		{
			wasProbablyPredicted = pOwner->entindex() == g_i_MirvPov;
		}
	}

	if (This->entindex() != g_i_MirvPov && !wasProbablyPredicted && !This->IsClientCreated())
	{
		return g_Mirv_Pov_Interp_OrgFac[0] * baseInterpolation + g_Mirv_Pov_Interp_PingFac[0] * (g_Mirv_Pov_PingAdjustMent / 1000.0f) + g_Mirv_Pov_Interp_Offset[0];
	}

	//Tier0_Msg("Local: %i\n", This->entindex());

	return g_Mirv_Pov_Interp_OrgFac[1] * baseInterpolation + g_Mirv_Pov_Interp_PingFac[1] * (g_Mirv_Pov_PingAdjustMent / 1000.0f) + g_Mirv_Pov_Interp_Offset[1];
}

void __declspec(naked) My_csgo_client_AdjustInterpolationAmount()
{
	__asm sub esp, 8
	__asm mov [esp], ecx
	__asm call True_csgo_client_AdjustInterpolationAmount
	__asm movss dword ptr[esp+4], xmm0
	__asm call My_Adjust_csgo_client_AdjustInterpolationAmount
	__asm fstp [esp]
	__asm movss xmm0, [esp]
	__asm add esp, 8
	__asm ret
}

bool Install_csgo_client_AdjustInterpolationAmount(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_client_AdjustInterpolationAmount))
	{
		LONG error = NO_ERROR;

		True_csgo_client_AdjustInterpolationAmount = (csgo_client_AdjustInterpolationAmount_t)AFXADDR_GET(csgo_client_AdjustInterpolationAmount);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_csgo_client_AdjustInterpolationAmount, My_csgo_client_AdjustInterpolationAmount);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}
	return firstResult;
}

typedef float(__fastcall* csgo_CBoundedCvar_GetFloat_t)(void * This, void* Edx);
csgo_CBoundedCvar_GetFloat_t True_csgo_CBoundedCvar_CmdRate_GetFloat;
csgo_CBoundedCvar_GetFloat_t True_csgo_CBoundedCvar_UpdateRate_GetFloat;
csgo_CBoundedCvar_GetFloat_t True_csgo_CBoundedCvar_Rate_GetFloat;

extern WrpVEngineClient * g_VEngineClient;

float Do_csgo_CBoundedCvar_GetFloat_WithOrgLocalPlayer(void * This, void * Edx, csgo_CBoundedCvar_GetFloat_t fn) {
	if (g_i_MirvPov && g_VEngineClient && g_VEngineClient->IsConnected()) {
		unsigned char * p_b_ishltv = *((unsigned char **)AFXADDR_GET(csgo_engine_m_SplitScreenPlayers)) + AFXADDR_GET(csgo_engine_m_SplitScreenPlayers_ishltv_ofs);
		unsigned char b_was_hltv = *p_b_ishltv;
		*p_b_ishltv = g_WasHltv ? 1 : 0;

		float result = fn(This, Edx);

		*p_b_ishltv = b_was_hltv;				
		return result;
	}

	return fn(This, Edx);
}

float __fastcall My_csgo_CBoundedCvar_CmdRate_GetFloat(SOURCESDK::C_BasePlayer_csgo * This, void* Edx) {
	return Do_csgo_CBoundedCvar_GetFloat_WithOrgLocalPlayer(This, Edx, True_csgo_CBoundedCvar_CmdRate_GetFloat);
}

float __fastcall My_csgo_CBoundedCvar_UpdateRate_GetFloat(SOURCESDK::C_BasePlayer_csgo * This, void* Edx) {
	return Do_csgo_CBoundedCvar_GetFloat_WithOrgLocalPlayer(This, Edx, True_csgo_CBoundedCvar_UpdateRate_GetFloat);
}

float __fastcall My_csgo_CBoundedCvar_Rate_GetFloat(SOURCESDK::C_BasePlayer_csgo * This, void* Edx) {
	return Do_csgo_CBoundedCvar_GetFloat_WithOrgLocalPlayer(This, Edx, True_csgo_CBoundedCvar_Rate_GetFloat);
}


bool Install_csgo_engine_RateCvarHooks(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_engine_CBoundedCvar_CmdRate_vtable)
		&& AFXADDR_GET(csgo_engine_CBoundedCvar_UpdateRate_vtable)
		&& AFXADDR_GET(csgo_engine_CBoundedCvar_Rate_vtable)
		&& AFXADDR_GET(csgo_engine_m_SplitScreenPlayers)
		&& AFXADDR_GET(csgo_engine_m_SplitScreenPlayers_ishltv_ofs))
	{
		LONG error = NO_ERROR;

		{
			void **vtable = (void **)AFXADDR_GET(csgo_engine_CBoundedCvar_CmdRate_vtable);
			True_csgo_CBoundedCvar_CmdRate_GetFloat = (csgo_CBoundedCvar_GetFloat_t)vtable[12];
		}
		{
			void **vtable = (void **)AFXADDR_GET(csgo_engine_CBoundedCvar_UpdateRate_vtable);
			True_csgo_CBoundedCvar_UpdateRate_GetFloat = (csgo_CBoundedCvar_GetFloat_t)vtable[12];
		}
		{
			void **vtable = (void **)AFXADDR_GET(csgo_engine_CBoundedCvar_Rate_vtable);
			True_csgo_CBoundedCvar_Rate_GetFloat = (csgo_CBoundedCvar_GetFloat_t)vtable[12];
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_csgo_CBoundedCvar_CmdRate_GetFloat, My_csgo_CBoundedCvar_CmdRate_GetFloat);
		DetourAttach(&(PVOID&)True_csgo_CBoundedCvar_UpdateRate_GetFloat, My_csgo_CBoundedCvar_UpdateRate_GetFloat);
		DetourAttach(&(PVOID&)True_csgo_CBoundedCvar_Rate_GetFloat, My_csgo_CBoundedCvar_Rate_GetFloat);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}
	return firstResult;
}


CON_COMMAND(mirv_pov, "Forces a POV on a GOTV demo.")
{
	if (!(AFXADDR_GET(csgo_C_CSPlayer_ofs_m_angEyeAngles)
		&& csgo_CNetChan_ProcessMessages_Install()
		&& csgo_C_CSPlayer_EyeAngles_Install()
		&& csgo_DamageIndicator_MessageFunc_Install()
		&& csgo_C_CSPlayer_UpdateOnRemove_Install()
		&& Install_csgo_C_CS_Player__GetFOVs()
		&& AFXADDR_GET(csgo_C_BasePlayer_SetAsLocalPlayer)
		&& AFXADDR_GET(csgo_crosshair_localplayer_check)
		&& Install_csgo_C_BaseViewModel_FireEvent()
		&& Install_csgo_client_AdjustInterpolationAmount()
		&& Hook_csgo_CPlayerResource_GetPing()
		//&& AFXADDR_GET(csgo_C_BaseEntity_ofs_m_bPredictable)
		&& Install_csgo_engine_RateCvarHooks()
		))
	{
		Tier0_Warning("Not supported for your engine / missing hooks!\n");
		return;
	}

	int argC = args->ArgC();

	if (2 <= argC)
	{
		static bool firstRun = true;
		static unsigned char mem_jz[6] = { 0 };

		g_i_MirvPov = atoi(args->ArgV(1));

		unsigned char* pData = (unsigned char *)AFXADDR_GET(csgo_crosshair_localplayer_check);

		MdtMemBlockInfos mbis;
		MdtMemAccessBegin(pData, AFXADDR_GET(csgo_crosshair_localplayer_check_NOPSZ), &mbis);

		if (firstRun) {
			firstRun = false;
			memcpy(mem_jz, pData, AFXADDR_GET(csgo_crosshair_localplayer_check_NOPSZ));
		}

		if (g_i_MirvPov)
		{
			memset(pData, 0x90, AFXADDR_GET(csgo_crosshair_localplayer_check_NOPSZ));
		}
		else
		{
			memcpy(pData, mem_jz, AFXADDR_GET(csgo_crosshair_localplayer_check_NOPSZ));
		}

		MdtMemAccessEnd(&mbis);

		return;
	}

	Tier0_Msg(
		"mirv_pov <iPlayerEntityIndex> - Needs to be set before loading demo / connecting! Forces POV on a GOTV to the given player entity index, set 0 to disable.\n"
		"Current value: %i\n"
		, g_i_MirvPov
	);
}