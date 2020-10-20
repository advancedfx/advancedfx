#include "stdafx.h"
#include "csgo_net_chan.h"

#include "WrpConsole.h"

#include "addresses.h"
#include <SourceInterfaces.h>
#include <csgo/bitbuf/demofilebitbuf.h>

#include <build/protobuf/csgo/netmessages.pb.h>

#include <shared/AfxDetours.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

int g_i_MirvPov = 0;

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
						if (msg.has_is_hltv())
						{
							msg.set_is_hltv(false);
						}
						if (msg.has_player_slot())
						{
							msg.set_player_slot(g_i_MirvPov - 1);
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

	if (AFXADDR_GET(csgo_C_CSPlayer_vtable))
	{
		AfxDetourPtr((PVOID*)&(((DWORD*)AFXADDR_GET(csgo_C_CSPlayer_vtable))[169]), Mycsgo_C_CSPlayer_EyeAngles, (PVOID*)&Truecsgo_C_CSPlayer_EyeAngles);

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
		if (SOURCESDK::IClientEntity_csgo* ce1 = SOURCESDK::g_Entitylist_csgo->GetClientEntity(1))
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

	if (AFXADDR_GET(csgo_C_CSPlayer_vtable))
	{
		AfxDetourPtr((PVOID*)&(((DWORD*)AFXADDR_GET(csgo_C_CSPlayer_vtable))[126]), Mycsgo_C_CSPlayer_UpdateOnRemove, (PVOID*)&Truecsgo_C_CSPlayer_UpdateOnRemove);

		firstResult = true;
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
		&& AFXADDR_GET(csgo_C_BasePlayer_SetAsLocalPlayer)
		))
	{
		Tier0_Warning("Not supported for your engine / missing hooks,!\n");
		return;
	}

	int argC = args->ArgC();

	if (2 <= argC)
	{
		g_i_MirvPov = atoi(args->ArgV(1));

		unsigned char* pData = (unsigned char *)AFXADDR_GET(csgo_crosshair_localplayer_check) + 15;

		MdtMemBlockInfos mbis;
		MdtMemAccessBegin(pData, 2, &mbis);

		if (g_i_MirvPov)
		{
			pData[0] = 0x90;
			pData[1] = 0x90;
		}
		else
		{
			pData[0] = 0x74;
			pData[1] = 0xcc;
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