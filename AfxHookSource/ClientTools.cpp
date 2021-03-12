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
	if(m_AfxGameRecord.StartRecording(fileName, 5))
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
		"%s recordPlayerCameras [...]\n"
		"%s recordWeapons [...]\n"
		"%s recordProjectiles [...]\n"
		"%s recordViewmodels [...]\n"
		"%s recordInvisible [...] - (not recommended)\n"
		"%s debug [...]\n"
		, prefix
		, prefix
		, prefix
		, prefix
		, prefix
		, prefix
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
