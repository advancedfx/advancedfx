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
: m_Recording(false)
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
	if (!m_Recording)
		return;

	if (m_File)
	{
		WriteDictionary("afxFrame");
		Write((float)g_Hook_VClient_RenderView.GetGlobals()->absoluteframetime_get());
		m_HiddenFileOffset = ftell(m_File);
		Write((int)0);
	}
	
}

void CClientTools::OnAfterFrameRenderEnd(void)
{
	if (!m_Recording)
		return;

	if(m_RecordCamera)
	{
		WriteDictionary("afxCam");
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[0]);
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[1]);
		Write((float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[0]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[1]);
		Write((float)g_Hook_VClient_RenderView.LastCameraAngles[2]);
		Write((float)ScaleFov(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, (float)g_Hook_VClient_RenderView.LastCameraFov));
	}

	if (m_File && m_HiddenFileOffset && 0 < m_Hidden.size())
	{
		WriteDictionary("afxHidden");

		size_t curOffset = ftell(m_File);

		int offset = (int)(curOffset - m_HiddenFileOffset);

		fseek(m_File, m_HiddenFileOffset, SEEK_SET);
		Write((int)offset);
		fseek(m_File, curOffset, SEEK_SET);

		Write((int)m_Hidden.size());

		for (std::set<int>::iterator it = m_Hidden.begin(); it != m_Hidden.end(); ++it)
		{
			Write((int)(*it));
		}

		m_Hidden.clear();
		m_HiddenFileOffset = 0;
	}

	WriteDictionary("afxFrameEnd");
}

bool CClientTools::GetRecording(void)
{
	return m_Recording;
}

void CClientTools::StartRecording(wchar_t const * fileName)
{
	EndRecording();

	m_Recording = true;

	Dictionary_Clear();
	m_File = 0;

	m_HiddenFileOffset = 0;
	m_Hidden.clear();

	_wfopen_s(&m_File, fileName, L"wb");

	if (m_File)
	{
		fputs("afxGameRecord", m_File);
		fputc('\0', m_File);
		int version = 4;
		fwrite(&version, sizeof(version), 1, m_File);
	}
	else
		Tier0_Warning("ERROR opening file \"%s\" for writing.\n", fileName);
}

void CClientTools::EndRecording()
{
	if (!m_Recording)
		return;

	if (m_File)
	{
		fclose(m_File);
	}

	Dictionary_Clear();

	m_Recording = false;
}

void CClientTools::WriteDictionary(char const * value)
{
	int idx = Dictionary_Get(value);

	Write(idx);

	if (-1 == idx)
	{
		Write(value);
	}
}

void CClientTools::Write(bool value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CClientTools::Write(int value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CClientTools::Write(float value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CClientTools::Write(double value)
{
	if (!m_File) return;

	fwrite(&value, sizeof(value), 1, m_File);
}

void CClientTools::Write(char const * value)
{
	if (!m_File) return;

	fputs(value, m_File);
	fputc('\0', m_File);
}

void CClientTools::Write(SOURCESDK::Vector const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
}

void CClientTools::Write(SOURCESDK::QAngle const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
}

void CClientTools::Write(SOURCESDK::Quaternion const & value)
{
	Write((float)value.x);
	Write((float)value.y);
	Write((float)value.z);
	Write((float)value.w);
}

void CClientTools::MarkHidden(int value)
{
	m_Hidden.insert(value);
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
				char const * cmd2 = args->ArgV(2);

				clientTools->RecordViewModel_set(0 != atoi(cmd2));
				return true;
			}

			Tier0_Msg(
				"%s recordViewModel 0|1 - Enable (1) / Disable (0) recording of view models.\n"
				"Current value: %i.\n"
				, prefix
				, clientTools->RecordViewModel_get() ? 1 : 0
			);
			Tier0_Warning(
				"This feature is not fully supported, will only work in CSSV34 and CS:GO at the moment.\n"
				"It has the following general problems:\n"
				"- Most import plugins won't know how to handle the viewmodel FOV properly, meaning it will look different from in-game.\n"
				"In CS:GO it will have the following problems:\n"
				"- You'll need to set cl_custom_material_override 0.\n"
				"- There will be several trash viewmodels, not much we can do about."
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
		"%s recordWeapons [...]\n"
		"%s recordProjectiles [...]\n"
		"%s recordViewmodel [...] - (not recommended)\n"
		"%s recordInvisible [...] - (not recommended)\n"
		"%s debug [...]\n"
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

		if (!_stricmp(cmd1, "start"))
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

	Tier0_Msg(
		"%s start <sFilePath> - Start recording to file <sFilePath>, you should set a low host_framerate before (i.e. 30) and give the \".agr\" file extension.\n"
		"%s stop - Stop recording.\n"
		, prefix
		, prefix
	);
}
