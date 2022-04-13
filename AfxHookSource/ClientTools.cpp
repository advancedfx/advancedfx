#include "stdafx.h"

#include "ClientTools.h"

#include "RenderView.h"
#include "CamIO.h"
#include "WrpConsole.h"

#include <shared/StringTools.h>

#include <iostream>
#include <fstream>
#include <string>

CClientTools * CClientTools::m_Instance = 0;


CClientTools::CClientTools()
{
	if (0 != m_Instance)
		throw "CClientTools::CClientTools: singelton pattern void.";

	m_Instance = this;
}

CClientTools::~CClientTools()
{
	m_Instance = 0;
}

void CClientTools::OnPostToolMessage(void * hEntity, void * msg)
{
}

void CClientTools::OnBeforeFrameRenderStart(void)
{
	if (!m_AfxGameRecord.GetRecording())
		return;

	m_AfxGameRecord.BeginFrame((float)g_Hook_VClient_RenderView.GetGlobals()->absoluteframetime_get());
}

void CClientTools::OnAfterFrameRenderEnd(void)
{
	if (!m_AfxGameRecord.GetRecording())
		return;

	if(m_RecordCamera)
	{
		m_AfxGameRecord.WriteDictionary("afxCam");
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[0]);
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[1]);
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraAngles[0]);
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraAngles[1]);
		m_AfxGameRecord.Write((float)g_Hook_VClient_RenderView.LastCameraAngles[2]);
		m_AfxGameRecord.Write((float)ScaleFov(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, (float)g_Hook_VClient_RenderView.LastCameraFov));
	}

	m_AfxGameRecord.EndFrame();
}

bool CClientTools::GetRecording(void)
{
	return m_AfxGameRecord.GetRecording();
}

void CClientTools::StartRecording(wchar_t const * fileName)
{
	if(m_AfxGameRecord.StartRecording(fileName, GetAgrVersion()))
	{
		if (!EnableRecordingMode_get() && !SuppotsAutoEnableRecordingMode()) {
			Tier0_Warning(
				"WARNING: The recording needs to be enabled with [...] enabled 1 before loading the demo!\n"
				"(This is required, because either this game leaks memory when recording mode is enabled or because some features won't work otherwise.)\n"
				"Enabling the recording (but it might be too late already).\n"
			);
			EnableRecordingMode_set(true);
		}
	}
	else
		Tier0_Warning("ERROR opening file \"%s\" for writing.\n", fileName);
}

void CClientTools::EndRecording()
{
	m_AfxGameRecord.EndRecording();
}

void CClientTools::Write(SOURCESDK::Vector const & value)
{
	m_AfxGameRecord.Write((float)value.x);
	m_AfxGameRecord.Write((float)value.y);
	m_AfxGameRecord.Write((float)value.z);
}

void CClientTools::Write(SOURCESDK::QAngle const & value)
{
	m_AfxGameRecord.Write((float)value.x);
	m_AfxGameRecord.Write((float)value.y);
	m_AfxGameRecord.Write((float)value.z);
}

void CClientTools::Write(SOURCESDK::Quaternion const & value)
{
	m_AfxGameRecord.Write((float)value.x);
	m_AfxGameRecord.Write((float)value.y);
	m_AfxGameRecord.Write((float)value.z);
	m_AfxGameRecord.Write((float)value.w);
}

void CClientTools::WriteMatrix3x4(SOURCESDK::matrix3x4_t const &value)
{
	m_AfxGameRecord.Write(value[0][0]);
	m_AfxGameRecord.Write(value[0][1]);
	m_AfxGameRecord.Write(value[0][2]);
	m_AfxGameRecord.Write(value[0][3]);

	m_AfxGameRecord.Write(value[1][0]);
	m_AfxGameRecord.Write(value[1][1]);
	m_AfxGameRecord.Write(value[1][2]);
	m_AfxGameRecord.Write(value[1][3]);

	m_AfxGameRecord.Write(value[2][0]);
	m_AfxGameRecord.Write(value[2][1]);
	m_AfxGameRecord.Write(value[2][2]);
	m_AfxGameRecord.Write(value[2][3]);
}

void CClientTools::WriteBones(SOURCESDK::CStudioHdr * hdr, SOURCESDK::matrix3x4_t const *boneState, const SOURCESDK::matrix3x4_t & parentTransform) {
	if(nullptr != hdr && nullptr != boneState) {
		SOURCESDK::matrix3x4_t tmp;
		SOURCESDK::matrix3x4_t bones;

		m_AfxGameRecord.Write((bool)true);

		m_AfxGameRecord.Write((int)hdr->numbones());

		for (int i = 0; i < hdr->numbones(); ++i)
		{
			const SOURCESDK::mstudiobone_t *pBone = hdr->pBone(i);

			if ( !(pBone->flags & SOURCESDK_BONE_USED_BY_ANYTHING ) )
			{
				bones[0][0] = 1; bones[0][1] = 0; bones[0][2] = 0;  bones[0][3] = 0;
				bones[1][0] = 0; bones[1][1] = 1; bones[1][2] = 0;  bones[1][3] = 0;
				bones[2][0] = 0; bones[2][1] = 0; bones[2][2] = 1;  bones[2][3] = 0;
			} else {
				bool bRoot = pBone->parent == -1;

				if (!InvertMatrix(bRoot ? parentTransform : boneState[pBone->parent], tmp))
					Tier0_Warning("AFXERROR: parent matrix inversion failed for bone %i (parent bone %i).\n", i,pBone->parent);

				R_ConcatTransforms(tmp, boneState[i], bones);
			}

			WriteMatrix3x4(bones);
		}

	} else {
		m_AfxGameRecord.Write((bool)false);
	}
}

bool CClientTools::InvertMatrix(const SOURCESDK::matrix3x4_t &matrix, SOURCESDK::matrix3x4_t &out_matrix)
{

	double b0[4] = { 1, 0, 0, 0 };
	double b1[4] = { 0, 1, 0, 0 };
	double b2[4] = { 0, 0, 1, 0 };
	double b3[4] = { 0, 0, 0, 1 };

	double M[4][4] = {
		matrix[0][0], matrix[0][1], matrix[0][2],  matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2],  matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2],  matrix[2][3],
		0, 0, 0, 1
	};

	unsigned char P[4];
	unsigned char Q[4];

	double L[4][4];
	double U[4][4];

	if (!Afx::Math::LUdecomposition(M, P, Q, L, U))
		return false;

	double inv0[4] = { 1,0,0,0 };
	double inv1[4] = { 0,1,0,0 };
	double inv2[4] = { 0,0,1,0 };
	double inv3[4] = { 0,0,0,1 };

	Afx::Math::SolveWithLU(L, U, P, Q, b0, inv0);
	Afx::Math::SolveWithLU(L, U, P, Q, b1, inv1);
	Afx::Math::SolveWithLU(L, U, P, Q, b2, inv2);
	Afx::Math::SolveWithLU(L, U, P, Q, b3, inv3);

	out_matrix[0][0] = (float)inv0[0];
	out_matrix[0][1] = (float)inv1[0];
	out_matrix[0][2] = (float)inv2[0];
	out_matrix[0][3] = (float)inv3[0];

	out_matrix[1][0] = (float)inv0[1];
	out_matrix[1][1] = (float)inv1[1];
	out_matrix[1][2] = (float)inv2[1];
	out_matrix[1][3] = (float)inv3[1];

	out_matrix[2][0] = (float)inv0[2];
	out_matrix[2][1] = (float)inv1[2];
	out_matrix[2][2] = (float)inv2[2];
	out_matrix[2][3] = (float)inv3[2];

	//pEngfuncs->Con_Printf("(%f, %f, %f, %f)\n", inv0[3], inv1[3], inv2[3], inv3[3]);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ClientTools_Console_Cfg(IWrpCommandArgs * args)
{
	CClientTools * clientTools = CClientTools::Instance();

	if (!clientTools)
	{
		//Tier0_Warning("Error: Feature not available!\n");
		return false;
	}

	int argc = args->ArgC();

	char const * prefix = args->ArgV(0);

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("recordCamera", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordCamera_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordCamera 0|1 - Enable (1) / Disable (0) recording of main camera (includes FOV).\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordCamera_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("recordPlayers", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordPlayers_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordPlayers 0|1 - Enable (1) / Disable (0) recording of players.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordPlayers_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("recordPlayerCameras", cmd1))
		{
			if (3 <= argc)
			{
				char const* cmd2 = args->ArgV(2);

				clientTools->RecordPlayerCameras_set(atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordPlayerCameras 0|<iEntIndex>|-1 - Disable (0), all (-1, default) or entity index of player camera to record. Needs recordPlayers enabled to work.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordPlayerCameras_get()
			);
			return true;
		}
		else if (0 == _stricmp("recordWeapons", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordWeapons_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordWeapons 0|1 - Enable (1) / Disable (0) recording of weapons.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordWeapons_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("recordProjectiles", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordProjectiles_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordProjectiles 0|1 - Enable (1) / Disable (0) recording of Projectiles.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordProjectiles_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("recordViewModel", cmd1))
		{
			if (3 <= argc)
			{
				char const* cmd2 = args->ArgV(2);

				clientTools->RecordViewModels_set(0 != atoi(cmd2) ? -1 : 0);
				return true;
			}

			Tier0_Msg(
				"%s recordViewModel 0|1 - Enable (1) / Disable (0) recording of viewmodel(s).\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordViewModels_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("recordViewModels", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordViewModels_set(atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordViewModels 0|<iPlayerEntIndex>|-1 - Disable (0), all (-1, default) or entity index (CS:GO only) of player of whom record view models.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordViewModels_get()
			);
			Tier0_Warning(
				"This feature is not fully supported, will only work in CSSV34 and CS:GO at the moment.\n"
				"It has the following general problems:\n"
				"- Most import plugins won't know how to handle the viewmodel FOV properly, meaning it will look different from in-game.\n"
				"In CS:GO it will have the following additional problems:\n"
				"- You'll need to set cl_custom_material_override 0.\n"
			);
			return true;
		}
		else if (0 == _stricmp("recordInvisible", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordInvisible_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordInvisible 0|1 - Enable (1) / Disable (0) recording of invisible entities. (Enabling it can cause AGRs with trash data.)\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordInvisible_get() ? 1 : 0
			);
			return true;
		}
		else if (0 == _stricmp("debug", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				clientTools->Debug_set(atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s debug 0|1|2 - Debug level.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->Debug_get()
			);
			return true;
		}
	}

	Tier0_Msg(
		"%s recordCamera [...]\n"
		"%s recordPlayers [...]\n"
		, prefix
		, prefix
	);
	if(clientTools->SupportsRecordPlayerCameras()) {
		Tier0_Msg(
			"%s recordPlayerCameras [...]\n"
			, prefix
		);
	}
	Tier0_Msg(
		"%s recordWeapons [...]\n"
		"%s recordProjectiles [...]\n"
		"%s recordViewmodel [...]\n"
		, prefix
		, prefix
		, prefix
	);
	if(clientTools->SupportsRecordViewModelMultiple()) {
		Tier0_Msg(
			"%s recordViewmodels [...]\n"
			, prefix
		);
	}
	Tier0_Msg(
		"%s recordInvisible [...] - (not recommended)\n"
		"%s debug [...]\n"
		, prefix
		, prefix
	);

	return false;
}

CON_COMMAND(mirv_agr, "AFX GameRecord")
{
	CClientTools * clientTools = CClientTools::Instance();

	if (!clientTools)
	{
		Tier0_Warning("Error: Feature not available!\n");
		return;
	}

	int argc = args->ArgC();

	char const * prefix = args->ArgV(0);

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("enabled", cmd1)) {
			if (3 <= argc) {
				const char * cmd2 = args->ArgV(2);

				clientTools->EnableRecordingMode_set(0 != atoi(cmd2));
				return;
			}

			Tier0_Msg(
				"%s enabled 0|1\n"
				"Current value: %i\n"
				, prefix
				, clientTools->EnableRecordingMode_get() ? 1 : 0
			);
			return;
		}
		else if (!_stricmp(cmd1, "start"))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				std::wstring wideFilePath;

				if (UTF8StringToWideString(cmd2, wideFilePath))
				{
					clientTools->StartRecording(wideFilePath.c_str());
				}

				if (clientTools->GetRecording())
				{
					Tier0_Msg("Started AGR recording.\n");
					return;
				}

				Tier0_Warning(
					"Error.\n"
				);
				return;
			}
		}
		else if (!_stricmp(cmd1, "stop"))
		{
			clientTools->EndRecording();

			Tier0_Msg(
				"Stopped AGR recording.\n"
			);
			return;
		}
	}

	if (ClientTools_Console_Cfg(args))
		return;

	if (!clientTools->SuppotsAutoEnableRecordingMode()) {
		Tier0_Msg(
			"%s enabled [...] - Enable / disable recording (Has to be enabled before loading the demo if you want to use AGR!).\n"
			, prefix
		);
	}

	Tier0_Msg(
		"%s start <sFilePath> - Start recording to file <sFilePath>, you should set a low host_framerate before (i.e. 30) and give the \".agr\" file extension.\n"
		"%s stop - Stop recording.\n"
		, prefix
		, prefix
	);
}
