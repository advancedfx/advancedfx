#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2016-11-06 dominik.matrixstorm.com
//
// First changes:
// 2016-11-06 dominik.matrixstorm.com

#include "csgo_IPrediction.h"

#include "addresses.h"
#include "asmClassTools.h"

CAfxCsgoPrediction * g_AfxCsgoPrediction = 0;

bool g_csgo_DuckFix = false;

bool * GetPtr_csgo_C_BasePlayer_m_bDucked(void)
{
	if (AFXADDR_GET(csgo_pLocalPlayer)
		&& AFXADDR_GET(csgo_C_BasePlayer_OFS_m_bDucked) != (AfxAddr)-1
		&& 0 != *(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer))
	{
		return (bool *)(*(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer) + AFXADDR_GET(csgo_C_BasePlayer_OFS_m_bDucked));
	}

	return 0;
}

bool * GetPtr_csgo_C_BasePlayer_m_bDucking(void)
{
	if (AFXADDR_GET(csgo_pLocalPlayer)
		&& AFXADDR_GET(csgo_C_BasePlayer_OFS_m_bDucking) != (AfxAddr)-1
		&& 0 != *(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer))
	{
		return (bool *)(*(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer) + AFXADDR_GET(csgo_C_BasePlayer_OFS_m_bDucking));
	}

	return 0;
}

float * GetPtr_csgo_C_BasePlayer_m_flDuckAmount(void)
{
	if (AFXADDR_GET(csgo_pLocalPlayer)
		&& AFXADDR_GET(csgo_C_BasePlayer_OFS_m_flDuckAmount) != (AfxAddr)-1
		&& 0 != *(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer))
	{
		return (float *)(*(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer) + AFXADDR_GET(csgo_C_BasePlayer_OFS_m_flDuckAmount));
	}

	return 0;
}

bool csgo_CanDuckFix(void)
{
	return
		0 != g_AfxCsgoPrediction
		&& 0 != GetPtr_csgo_C_BasePlayer_m_bDucked()
		&& 0 != GetPtr_csgo_C_BasePlayer_m_bDucking()
		&& 0 != GetPtr_csgo_C_BasePlayer_m_flDuckAmount();
}


#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'ebp' modified by inline assembly code

CAfxCsgoPrediction::CAfxCsgoPrediction(SOURCESDK::CSGO::IPrediction * parent)
{
	m_Parent = parent;
}

CAfxCsgoPrediction::~CAfxCsgoPrediction()
{
	delete m_Parent; // TODO: is this correct?
}

void CAfxCsgoPrediction::Init(void)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 1)
}

void CAfxCsgoPrediction::Shutdown(void)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 2)
}

void CAfxCsgoPrediction::Update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command)
{
	if (g_csgo_DuckFix)
	{
		return;

		bool * p_m_bDucked = GetPtr_csgo_C_BasePlayer_m_bDucked();
		bool * p_m_bDucking = GetPtr_csgo_C_BasePlayer_m_bDucking();
		float * p_m_flDuckAmount = GetPtr_csgo_C_BasePlayer_m_flDuckAmount();

		if (p_m_bDucked && p_m_bDucking && p_m_flDuckAmount)
		{
			m_Parent->Update(startframe, validframe, incoming_acknowledged, outgoing_command);

			bool newDucked = *p_m_bDucked;
			*p_m_bDucking = newDucked;
			*p_m_flDuckAmount = newDucked ? 1.0f : 0.0f;

			return;
		}
	}

	m_Parent->Update(startframe, validframe, incoming_acknowledged, outgoing_command);
}

void CAfxCsgoPrediction::PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 4)
}

void CAfxCsgoPrediction::PostEntityPacketReceived(void)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 5)
}

void CAfxCsgoPrediction::PostNetworkDataReceived(int commands_acknowledged)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 6)
}

void CAfxCsgoPrediction::OnReceivedUncompressedPacket(void)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 7)
}

void CAfxCsgoPrediction::GetViewOrigin(SOURCESDK::Vector& org)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 8)
}

void CAfxCsgoPrediction::SetViewOrigin(SOURCESDK::Vector& org)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 9)
}

void CAfxCsgoPrediction::GetViewAngles(SOURCESDK::QAngle& ang)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 10)
}

void CAfxCsgoPrediction::SetViewAngles(SOURCESDK::QAngle& ang)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 11)
}

void CAfxCsgoPrediction::GetLocalViewAngles(SOURCESDK::QAngle& ang)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 12)
}

void CAfxCsgoPrediction::SetLocalViewAngles(SOURCESDK::QAngle& ang)
{
	JMP_CLASSMEMBERIFACE_FN(CAfxCsgoPrediction, m_Parent, 13)
}

#pragma warning(pop)
