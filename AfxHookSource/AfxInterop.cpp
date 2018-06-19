#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"

#include <Windows.h>

#include <mutex>
#include <atomic>

namespace AfxInterop {

	const INT32 m_Version = 0;

	bool Connect(char const * pipeName);
	void Disconnect();
	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool ReadBoolean(HANDLE hFile, bool & outValue);
	bool ReadInt32(HANDLE hFile, INT32 & outValue);
	bool ReadHandle(HANDLE hFile, HANDLE & outValue);
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool WriteBoolean(HANDLE hFile, bool value);
	bool Flush(HANDLE hFile);

	bool m_Enabled = false;
	bool m_Connected = false;
	int m_Frame = 0;
	HANDLE m_HPipe = INVALID_HANDLE_VALUE;
	bool m_Server64Bit = false;

	namespace EngineThread {

		std::mutex m_ActiveMutex;
		bool m_Active = false;
		HANDLE m_HWritePipe = INVALID_HANDLE_VALUE;
		HANDLE m_HReadPipe = INVALID_HANDLE_VALUE;

		std::mutex m_ConnectMutex;
		bool m_WantsConnect = false;
		std::string m_PipeName("advancedfxInterop");
	}

	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxInterop");
	}

	void BeforeFrameStart() {

	}

	void BeforeFrameRenderStart() {

	}

	void Shutdown() {
		Disconnect();
	}

	void LevelInitPreEntity(char const* pMapName) {

	}

	void LevelShutdown() {

	}

	bool Enabled() {
		return m_Enabled;
	}

	void DrawingThreadBeforeHud(void)
	{
		{
			// Connetion handling:

			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			if (EngineThread::m_WantsConnect && !m_Connected)
			{
				Connect(EngineThread::m_PipeName.c_str());
			}
			else if (!EngineThread::m_WantsConnect && m_Connected)
			{
				Disconnect();
			}
		}

		if (!m_Connected) return;


	}

	bool Connect(char const * pipeName) {
		if (!m_Enabled) return false;

		int errorLine = 0;

		Disconnect();

		while (true)
		{
			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(pipeName);

			m_HPipe = CreateFile(strPipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (m_HPipe != INVALID_HANDLE_VALUE)
				break;

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				Tier0_Warning("Could not open pipe. GLE=%d\n", GetLastError());
				{ errorLine = __LINE__; goto error; }
			}

			if (!WaitNamedPipe(pipeName, 20000))
			{
				Tier0_Warning("Could not open pipe: 20 second wait timed out.\n");
				{ errorLine = __LINE__; goto error; }
			}
		}

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

		HANDLE engineWritePipe = INVALID_HANDLE_VALUE;
		HANDLE engineReadPipe = INVALID_HANDLE_VALUE;

		if(!ReadHandle(m_HPipe, engineWritePipe)) { errorLine = __LINE__; goto error; }

		if (!ReadHandle(m_HPipe, engineReadPipe)) { errorLine = __LINE__; goto error; }

		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			EngineThread::m_HWritePipe = engineWritePipe;
			EngineThread::m_HReadPipe = engineWritePipe;
			EngineThread::m_Active = true;
		}

		m_Connected = true;

	error:
		Tier0_Warning("AfxInterop::Connect: Error in line %i.\n", errorLine);
		Disconnect();
		return false;
	}

	void Disconnect() {
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			if (INVALID_HANDLE_VALUE != EngineThread::m_HReadPipe)
			{
				if (!CloseHandle(EngineThread::m_HReadPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				EngineThread::m_HReadPipe = INVALID_HANDLE_VALUE;
			}

			if (INVALID_HANDLE_VALUE != EngineThread::m_HWritePipe)
			{
				if (!CloseHandle(EngineThread::m_HWritePipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				EngineThread::m_HWritePipe = INVALID_HANDLE_VALUE;
			}

			if (EngineThread::m_Active)
			{
				EngineThread::m_Active = false;
			}
		}

		if (m_Connected)
		{
			if (INVALID_HANDLE_VALUE != m_HPipe)
			{
				if (!CloseHandle(m_HPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				m_HPipe = INVALID_HANDLE_VALUE;
			}
		}

		m_Connected = false;
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
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	
	}

	bool ReadInt32(HANDLE hFile, INT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(INT32));
	}

	bool ReadHandle(HANDLE hFile, HANDLE & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(HANDLE));
	}
	
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		DWORD bytesWritten;

		if (!WriteFile(hFile, &(((char *)lpBuffer)[offset]), numBytes, &bytesWritten, NULL) || numBytes != bytesWritten)
			return false;

		return true;
	}

	bool WriteBoolean(HANDLE hFile, bool value) {
		return WriteBytes(hFile, &value, 0, sizeof(bool));
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
