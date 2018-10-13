#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"

#include <Windows.h>

#include <mutex>
#include <atomic>

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;

namespace AfxInterop {

	const INT32 m_Version = 0;

	bool Connect(char const * pipeName);
	void Disconnect();
	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool ReadBoolean(HANDLE hFile, bool & outValue);
	bool ReadByte(HANDLE hFile, BYTE & outValue);
	bool ReadInt32(HANDLE hFile, INT32 & outValue);
	bool ReadHandle(HANDLE hFile, HANDLE & outValue);
	bool ReadStringUTF8(HANDLE hFile, std::string & outValue);
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool WriteBoolean(HANDLE hFile, bool value);
	bool WriteInt32(HANDLE hFile, INT32 value);
	bool Flush(HANDLE hFile);

	bool m_Enabled = false;
	bool m_Connected = false;
	HANDLE m_HPipe = INVALID_HANDLE_VALUE;
	bool m_Server64Bit = false;

	namespace EngineThread {

		enum ClientMessage {
			ClientMessage_LevelInitPreEntity = 1,
			ClientMessage_LevelShutDown = 2,
			ClientMessage_BeforeFrameStart = 3,
			ClientMessage_BeforeFrameRenderStart = 4,
			ClientMessage_AfterFrameRenderEnd = 5,
			ClientMessage_EntityCreated = 6,
			ClientMessage_EntityDeleted = 7
		};

		std::mutex m_ActiveMutex;
		bool m_Active = false;
		HANDLE m_HPipe = INVALID_HANDLE_VALUE;

		std::mutex m_ConnectMutex;
		bool m_WantsConnect = false;
		std::string m_PipeName("advancedfxInterop");

		int m_Frame = -1;
	}

	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxInterop");

		if (!m_Enabled) return;
	}

	void BeforeFrameStart() {
		if (!m_Enabled) return;

		return;

		int errorLine = 0;

		if (WrpGlobals * pWrpGlobals = g_Hook_VClient_RenderView.GetGlobals()) {
			EngineThread::m_Frame = pWrpGlobals->framecount_get();
		}
		else {
			++EngineThread::m_Frame;
		}

		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			if (EngineThread::m_Active)
			{
				if (!WriteInt32(EngineThread::m_HPipe, EngineThread::ClientMessage_BeforeFrameStart)) { errorLine = __LINE__; goto error; }
				if (!Flush(EngineThread::m_HPipe)) { errorLine = __LINE__; goto error; }

				int commandCount = 0;
				{
					BYTE byteCommandCount;
					if (!ReadByte(EngineThread::m_HPipe, byteCommandCount)) { errorLine = __LINE__; goto error; }
					commandCount = byteCommandCount;
				}
				if (255 == commandCount)
				{
					if (!ReadInt32(EngineThread::m_HPipe, commandCount)) { errorLine = __LINE__; goto error; }
				}

				std::string command;

				while (0 < commandCount)
				{
					if (!ReadStringUTF8(EngineThread::m_HPipe, command)) { errorLine = __LINE__; goto error; }

					g_VEngineClient->ExecuteClientCmd(command.c_str());

					--commandCount;
				}
			}
		}

		return;

	error:
		Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return;
	}

	void BeforeFrameRenderStart() {
		if (!m_Enabled) return;
	}

	void Shutdown() {
		if (!m_Enabled) return;

		Disconnect();
	}

	void LevelInitPreEntity(char const* pMapName) {
		if (!m_Enabled) return;

	}

	void LevelShutdown() {
		if (!m_Enabled) return;

	}

	bool Enabled() {
		return m_Enabled;
	}

	void DrawingThreadBeforeHud(void)
	{
		if (!m_Enabled) return;

		int errorLine = 0;

		{
			// Connection handling:

			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			if (EngineThread::m_WantsConnect && !m_Connected)
			{
				Connect(EngineThread::m_PipeName.c_str());
			}
			else if (!EngineThread::m_WantsConnect && m_Connected)
			{
				Disconnect();
			}

			if (!m_Connected)
				return;
		}

		// No frame info available yet:
		if (!WriteBoolean(m_HPipe, false)) { errorLine = __LINE__; goto error; }

		return;

	error:
		Tier0_Warning("AfxInterop::DrawingThreadBeforeHud: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return;
	}

	bool Connect(char const * pipeName) {
		if (!m_Enabled) return false;

		int errorLine = 0;

		Disconnect();

		std::string strPipeName("\\\\.\\pipe\\");
		strPipeName.append(pipeName);

		while (true)
		{
			m_HPipe = CreateFile(strPipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (m_HPipe != INVALID_HANDLE_VALUE)
				break;

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				Tier0_Warning("Could not open pipe. GLE=%d\n", GetLastError());
				{ errorLine = __LINE__; goto error; }
			}

			if (!WaitNamedPipe(strPipeName.c_str(), 5000))
			{
				Tier0_Warning("WaitNamedPipe: timed out.\n");
				{ errorLine = __LINE__; goto error; }
			}
		}

		Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());

		INT32 version;
		if (!ReadInt32(m_HPipe, version)) { errorLine = __LINE__; goto error; }
		
		if (m_Version != version)
		{
			Tier0_Warning("Version %d is not a supported (%d) version.\n", version, m_Version);
			if (!WriteBoolean(m_HPipe, false)) { errorLine = __LINE__; goto error; }
			if (!Flush(m_HPipe)) { errorLine = __LINE__; goto error; }
			{ errorLine = __LINE__; goto error; }
		}

		if (!WriteBoolean(m_HPipe, true)) { errorLine = __LINE__; goto error; }

		if(!Flush(m_HPipe)) { errorLine = __LINE__; goto error; }

		if(!ReadBoolean(m_HPipe, m_Server64Bit)) { errorLine = __LINE__; goto error; }

		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(pipeName);
			strPipeName.append("_engine");

			while (true)
			{
				EngineThread::m_HPipe = CreateFile(strPipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (EngineThread::m_HPipe != INVALID_HANDLE_VALUE)
					break;

				if (GetLastError() != ERROR_PIPE_BUSY)
				{
					Tier0_Warning("Could not open pipe. GLE=%d\n", GetLastError());
					{ errorLine = __LINE__; goto error; }
				}

				if (!WaitNamedPipe(strPipeName.c_str(), 5000))
				{
					Tier0_Warning("WaitNamedPipe: timed out.\n");
					{ errorLine = __LINE__; goto error; }
				}
			}

			Tier0_Msg("Connected to \"%s\".\n", strPipeName.c_str());

			EngineThread::m_Active = true;
		}

		m_Connected = true;

		return true;

	error:
		Tier0_Warning("AfxInterop::Connect: Error in line %i.\n", errorLine);
		Disconnect();
		return false;
	}

	void Disconnect() {
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			if (INVALID_HANDLE_VALUE != EngineThread::m_HPipe)
			{
				if (!CloseHandle(EngineThread::m_HPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				EngineThread::m_HPipe = INVALID_HANDLE_VALUE;
			}

			if (EngineThread::m_Active)
			{
				EngineThread::m_Active = false;
			}
		}

		if (INVALID_HANDLE_VALUE != m_HPipe)
		{
			if (!CloseHandle(m_HPipe))
			{
				Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
			}
			m_HPipe = INVALID_HANDLE_VALUE;
		}

		if (m_Connected)
		{
			m_Connected = false;
		}
	}


	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		lpBuffer = &(((char *)lpBuffer)[offset]);
		BOOL success = false;

		do
		{
			DWORD bytesRead;

			success = ReadFile( hFile, lpBuffer, numBytes, &bytesRead, NULL);

			if (!success)
				return false;

			numBytes -= bytesRead;
			lpBuffer = &(((char *)lpBuffer)[bytesRead]);

		} while (0 < numBytes);

		return true;
	}

	bool ReadBoolean(HANDLE hFile, bool & outValue)
	{
		BYTE useVal;

		bool result =  ReadBytes(hFile, &useVal, 0, sizeof(useVal));
		
		if (result) outValue = 0 != useVal ? true : false;

		return result;
	}

	bool ReadByte(HANDLE hFile, BYTE & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadInt32(HANDLE hFile, INT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadHandle(HANDLE hFile, HANDLE & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadStringUTF8(HANDLE hFile, std::string & outValue)
	{
		int length = 0;
		BYTE byteVal = 0;

		if (!ReadByte(hFile, byteVal)) return false;

		length = byteVal;

		if (255 == length)
		{
			if (!ReadInt32(hFile, length)) return false;
		}

		outValue.resize(length);

		if (!ReadBytes(hFile,&outValue[0],0,length)) return false;

		return true;
	}
	
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		DWORD bytesWritten;

		if (!WriteFile(hFile, &(((char *)lpBuffer)[offset]), numBytes, &bytesWritten, NULL) || numBytes != bytesWritten)
			return false;

		return true;
	}

	bool WriteBoolean(HANDLE hFile, bool value) {

		BYTE useVal = value ? 1 : 0;

		return WriteBytes(hFile, &useVal, 0, sizeof(useVal));
	}

	bool WriteInt32(HANDLE hFile, INT32 value) {
		return WriteBytes(hFile, &value, 0, sizeof(INT32));
	}

	bool Flush(HANDLE hFile)
	{
		if (!FlushFileBuffers(hFile))
			return false;

		return true;
	}

}

CON_COMMAND(afx_interop, "Controls advancedfxInterop (i.e. with Unity engine).")
{
	if (!AfxInterop::m_Enabled)
	{
		Tier0_Warning("Error: afx_interop command requires -afxInterop launch option!\n");
		return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("pipeName", arg1))
		{
			std::unique_lock<std::mutex> lock(AfxInterop::EngineThread::m_ConnectMutex);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);
				
				AfxInterop::EngineThread::m_PipeName = arg2;

				return;
			}

			Tier0_Msg(
				"afx_interop pipeName <sName> - Set new name.\n"
				"Current value: %s\n"
				, AfxInterop::EngineThread::m_PipeName
			);
			return;
		}
		else if (0 == _stricmp("connect", arg1))
		{
			std::unique_lock<std::mutex> lock(AfxInterop::EngineThread::m_ConnectMutex);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				AfxInterop::EngineThread::m_WantsConnect = 0 != atoi(arg2);

				return;
			}

			Tier0_Msg(
				"afx_interop connect 0|1 - 0 = disabled (default), 1 = enabled.\n"
				"Current value: %i\n"
				, AfxInterop::EngineThread::m_WantsConnect ? 1 : 0
			);
			return;
		}
	}

	Tier0_Msg(
		"afx_interop pipeName [...] - Name of the pipe to connect to.\n"
		"afx_interop connect [...] - Controls if interop connection is enabled.\n"
	);
}

#endif
