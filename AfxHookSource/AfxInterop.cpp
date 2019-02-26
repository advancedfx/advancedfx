#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "MirvTime.h"

#include <Windows.h>

#include <set>
#include <queue>

#include <mutex>
#include <atomic>

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;

namespace AfxInterop {

	const INT32 m_Version = 2;

	enum DrawingMessage
	{
		DrawingMessage_DrawingThreadBeforeHud = 1,
		DrawingMessage_NewTexture = 2,
		DrawingMessage_ReleaseTexture = 3,
		DrawingMessage_NewSurface = 4,
		DrawingMessage_ReleaseSurface = 5,
		DrawingMessage_PreapareDraw = 6
	};


	bool Connect(char const * pipeName);
	void Disconnect();
	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool ReadBoolean(HANDLE hFile, bool & outValue);
	bool ReadByte(HANDLE hFile, BYTE & outValue);
	bool ReadSByte(HANDLE hFile, signed char & outValue);
	bool ReadUInt32(HANDLE hFile, UINT32 & outValue);
	bool ReadCompressedUInt32(HANDLE hFile, UINT32 & outValue);
	bool ReadInt32(HANDLE hFile, INT32 & outValue);
	bool ReadCompressedInt32(HANDLE hFile, INT32 & outValue);
	bool ReadHandle(HANDLE hFile, HANDLE & outValue);
	bool ReadStringUTF8(HANDLE hFile, std::string & outValue);
	bool WriteBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes);
	bool WriteBoolean(HANDLE hFile, bool value);
	bool WriteByte(HANDLE hFile, BYTE value);
	bool WriteSByte(HANDLE hFile, signed char value);
	bool WriteUInt32(HANDLE hFile, UINT32 value);
	bool WriteCompressedUInt32(HANDLE hFile, UINT32 value);
	bool WriteInt32(HANDLE hFile, INT32 value);
	bool WriteCompressedInt32(HANDLE hFile, INT32 value);
	bool WriteSingle(HANDLE hFile, float value);
	bool WriteStringUTF8(HANDLE hFile, const std::string);
	bool WriteHandle(HANDLE hFille, HANDLE value);
	bool Flush(HANDLE hFile);

	bool m_Enabled = false;
	bool m_Connected = false;
	HANDLE m_hPipe = INVALID_HANDLE_VALUE;
	bool m_Server64Bit = false;

	HANDLE m_FbSurfaceHandle = NULL;

	namespace EngineThread {

		enum EngineMessage {
			EngineMessage_LevelInitPreEntity = 1,
			EngineMessage_LevelShutDown = 2,
			EngineMessage_BeforeFrameStart = 3,
			EngineMessage_BeforeHud = 4,
			EngineMessage_AfterFrameRenderEnd = 5,
			EngineMessage_EntityCreated = 6,
			EngineMessage_EntityDeleted = 7
		};

		class CConsole
		{
		public:
			CConsole(IWrpCommandArgs *args)
			{
				for (int i = 0; i < args->ArgC(); ++i)
				{
					m_Args.push(args->ArgV(i));
				}
			}

			bool Send(HANDLE hPipe)
			{
				if (!WriteCompressedUInt32(hPipe, (UINT32)m_Args.size())) return false;

				bool okay = true;

				while (!m_Args.empty())
				{
					okay = okay && WriteStringUTF8(hPipe, m_Args.front().c_str());
					m_Args.pop();
				}

				return okay;
			}

		private:
			std::queue<std::string> m_Args;
		};

		std::mutex m_ActiveMutex;
		bool m_Active = false;
		HANDLE m_hPipe = INVALID_HANDLE_VALUE;

		std::mutex m_ConnectMutex;
		bool m_WantsConnect = false;
		std::string m_PipeName("advancedfxInterop");

		int m_Frame = -1;
		bool m_FrameInfoSent = false;
		bool m_Suspended = false;

		std::queue<CConsole> m_Commands;

		void AddCommand(IWrpCommandArgs *args)
		{
			m_Commands.emplace(args);
		}

		bool SendCommands(HANDLE hPipe)
		{
			if (!WriteCompressedUInt32(hPipe, (UINT32)m_Commands.size())) return false;

			bool okay = true;

			while (!m_Commands.empty())
			{
				okay = okay && m_Commands.front().Send(hPipe);
				m_Commands.pop();
			}

			return okay;
		}
	}
	
	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxInterop");

		if (!m_Enabled) return;
	}

	int GetFrameCount() {
		return EngineThread::m_Frame;
	}

	bool GetFrameInfoSent() {
		return EngineThread::m_FrameInfoSent;
	}

	void BeforeFrameStart()
	{
		if (!m_Enabled) return;

		int errorLine = 0;

		EngineThread::m_ActiveMutex.lock();
		{
			if (EngineThread::m_Active)
			{
				if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto locked_error; }

				if (!EngineThread::SendCommands(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

				if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

				UINT32 commandCount;

				if(!ReadCompressedUInt32(EngineThread::m_hPipe, commandCount)) { errorLine = __LINE__; goto locked_error; }

				for (UINT32 i = 0; i < commandCount; ++i)
				{
					std::string command;

					if (!ReadStringUTF8(EngineThread::m_hPipe, command)) { errorLine = __LINE__; goto locked_error; }

					g_VEngineClient->ExecuteClientCmd(command.c_str());
				}
			}

			EngineThread::m_ActiveMutex.unlock();
		}

		return;

	locked_error:
		EngineThread::m_ActiveMutex.unlock();

		Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return;
	}

	void BeforeHud()
	{
		if (!m_Enabled) return;

		//if (WrpGlobals * pWrpGlobals = g_Hook_VClient_RenderView.GetGlobals()) {
		//	EngineThread::m_Frame = pWrpGlobals->framecount_get();
		//}
		//else {
		//	++EngineThread::m_Frame;
		//}

		++EngineThread::m_Frame;

		int errorLine = 0;

		EngineThread::m_ActiveMutex.lock();
		{
			if (EngineThread::m_Active)
			{
				if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_BeforeHud)) { errorLine = __LINE__; goto locked_error; }

				if (!WriteInt32(EngineThread::m_hPipe, EngineThread::m_Frame)) { errorLine = __LINE__; goto locked_error; }

				if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetAbsoluteFrameTime())) { errorLine = __LINE__; goto locked_error; }
				if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetTime())) { errorLine = __LINE__; goto locked_error; }
				if (!WriteSingle(EngineThread::m_hPipe, g_MirvTime.GetFrameTime())) { errorLine = __LINE__; goto locked_error; }

				if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; } // TODO: move this flush as required, currently it's good here, since no more data after this.

				EngineThread::m_FrameInfoSent = true;
			}

			EngineThread::m_ActiveMutex.unlock();
		}

		return;

	locked_error:
		EngineThread::m_ActiveMutex.unlock();

		Tier0_Warning("AfxInterop::BeforeFrameStart: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return;
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

	void DrawingThreadBeforeHud(int frameCount, bool frameInfoSent, const SOURCESDK::CViewSetup_csgo & view)
	{
		if (!m_Enabled) return;

		int errorLine = 0;

		static HANDLE o1 = 0;
		
		if (o1 != m_FbSurfaceHandle)
		{
			o1 = m_FbSurfaceHandle;
			Tier0_Warning("Surface switch: %i\n", m_FbSurfaceHandle);
		}

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
		
		//
		// This is the final message, before we allow to spin:

		AfxD3D_WaitForGPU(); // TODO: This is only required if Unity wants to render shit.

		SOURCESDK::VMatrix worldToView;
		SOURCESDK::VMatrix viewToProjection;
		SOURCESDK::VMatrix worldToProjection;
		SOURCESDK::VMatrix worldToPixels;

		g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

		if (!WriteInt32(m_hPipe, DrawingMessage_DrawingThreadBeforeHud)) { errorLine = __LINE__; goto error; }

		if (!WriteInt32(m_hPipe, view.width)) { errorLine = __LINE__; goto error; }
		if (!WriteInt32(m_hPipe, view.height)) { errorLine = __LINE__; goto error; }

		if (!WriteSingle(m_hPipe, worldToView.m[0][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[0][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[0][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[0][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[1][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[1][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[1][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[1][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[2][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[2][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[2][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[2][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[3][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[3][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[3][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, worldToView.m[3][3])) { errorLine = __LINE__; goto error; }

		if (!WriteSingle(m_hPipe, viewToProjection.m[0][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[0][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[0][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[0][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[1][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[1][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[1][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[1][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[2][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[2][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[2][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[2][3])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[3][0])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[3][1])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[3][2])) { errorLine = __LINE__; goto error; }
		if (!WriteSingle(m_hPipe, viewToProjection.m[3][3])) { errorLine = __LINE__; goto error; }

		if (!WriteHandle(m_hPipe, m_FbSurfaceHandle)) { errorLine = __LINE__; goto error; }

		if (!WriteInt32(m_hPipe, frameCount)) { errorLine = __LINE__; goto error; }

		if (!WriteBoolean(m_hPipe, frameInfoSent)) { errorLine = __LINE__; goto error; }
		
		if(!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

		bool done;
		do {
			if (!ReadBoolean(m_hPipe, done)) { errorLine = __LINE__; goto error; }
		} while (!done);

		return;

	error:
		Tier0_Warning("AfxInterop::DrawingThreadBeforeHud: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return;
	}

	std::set<IAfxInteropSurface *> m_Surfaces;

	bool DispatchSurfaceCreated(IAfxInteropSurface * surface)
	{
		int errorLine = 0;

		D3DSURFACE_DESC desc;

		if (SUCCEEDED(surface->AfxGetSurface()->GetDesc(&desc)))
		{

			if (!WriteInt32(m_hPipe, DrawingMessage_NewSurface)) { errorLine = __LINE__; goto error; }

			if (!WriteHandle(m_hPipe, (HANDLE)surface)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.Width)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.Height)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.Usage)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.Format)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.Pool)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.MultiSampleType)) { errorLine = __LINE__; goto error; }

			if (!WriteInt32(m_hPipe, desc.MultiSampleQuality)) { errorLine = __LINE__; goto error; }

			if (!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

			HANDLE remoteColor = NULL;
			HANDLE remoteDepth = NULL;

			if (!ReadHandle(m_hPipe, remoteColor)) { errorLine = __LINE__; goto error; }

			if (!ReadHandle(m_hPipe, remoteDepth)) { errorLine = __LINE__; goto error; }

			surface->AfxReplacementEnabled_set(false);
			surface->AfxSetReplacement(NULL);
			surface->AfxSetDepthSurface(NULL);

			IDirect3DDevice9 * device = NULL;
			if (SUCCEEDED(surface->AfxGetSurface()->GetDevice(&device)))
			{
				{
					IDirect3DTexture9 * replacementTexture = NULL;
					if (remoteColor)
					{
						if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &remoteColor)))
						{
							IDirect3DSurface9 * replacementSurface = NULL;

							if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
							{
								surface->AfxSetReplacement(replacementSurface);
								surface->AfxReplacementEnabled_set(true);
								replacementSurface->Release();
							}
							replacementTexture->Release();
						}
					}
				}
				{
					IDirect3DTexture9 * replacementTexture = NULL;
					if (remoteDepth)
					{
						if (SUCCEEDED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &replacementTexture, &remoteDepth)))
						{
							IDirect3DSurface9 * replacementSurface = NULL;

							if (SUCCEEDED(replacementTexture->GetSurfaceLevel(0, &replacementSurface)))
							{
								surface->AfxSetDepthSurface(replacementSurface);
								replacementSurface->Release();
							}
							replacementTexture->Release();
						}
					}
				}
				device->Release();
			}

			return true;
		}
		else { errorLine = __LINE__; goto error; }

	error:
		Tier0_Warning("AfxInterop::DispatchSurfaceCreated: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return false;
	}

	bool DispatchReleaseSurface(IAfxInteropSurface * surface)
	{
		int errorLine = 0;

		if (!WriteInt32(m_hPipe, DrawingMessage_ReleaseSurface)) { errorLine = __LINE__; goto error; }
		if (!WriteHandle(m_hPipe, (HANDLE)surface)) { errorLine = __LINE__; goto error; }
		if (!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

		// Wait for confirmation:
		bool done;
		do
		{
			if (!ReadBoolean(m_hPipe, done)) { errorLine = __LINE__; goto error; }
		} while (!done);

		surface->AfxSetReplacement(NULL);
		surface->AfxSetDepthSurface(NULL);

		return true;

	error:
		Tier0_Warning("AfxInterop::DispatchRelease: Error in line %i.\n", errorLine);
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

			Disconnect();
		}
		return false;
	}

	bool DispatchSharedSurfaces()
	{
		if (m_Connected) {
			for (std::set<IAfxInteropSurface *>::iterator it = m_Surfaces.begin(); it != m_Surfaces.end(); ++it)
			{
				if (!DispatchSurfaceCreated(*it)) return false;
			}
		}

		return true;
	}

	void OnCreatedSurface(IAfxInteropSurface * surface)
	{
		m_Surfaces.insert(surface);

		if(m_Connected) DispatchSurfaceCreated(surface);
	}

	void OnReleaseSurface(IAfxInteropSurface * surface)
	{
		if (m_Connected) DispatchReleaseSurface(surface);

		m_Surfaces.erase(surface);
	}

	/// <param name="info">can be nullptr</param>
	void OnSetRenderTarget(DWORD RenderTargetIndex, IAfxInteropSurface * surface)
	{
		if (0 == RenderTargetIndex)
		{
			m_FbSurfaceHandle = (HANDLE)surface;
		}
	}

	bool Connect(char const * pipeName) {

		if (!m_Enabled) return false;

		int errorLine = 0;

		Disconnect();

		{
			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(pipeName);

			while (true)
			{
				m_hPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (m_hPipe != INVALID_HANDLE_VALUE)
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
		}

		{
			std::string strPipeName("\\\\.\\pipe\\");
			strPipeName.append(pipeName);
			strPipeName.append("_engine");

			while (true)
			{
				EngineThread::m_hPipe = CreateFile(strPipeName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (EngineThread::m_hPipe != INVALID_HANDLE_VALUE)
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
		}

		INT32 version;
		if (!ReadInt32(m_hPipe, version)) { errorLine = __LINE__; goto error; }
		
		if (m_Version != version)
		{
			Tier0_Warning("Version %d is not a supported (%d) version.\n", version, m_Version);
			if (!WriteBoolean(m_hPipe, false)) { errorLine = __LINE__; goto error; }
			if (!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }
			{ errorLine = __LINE__; goto error; }
		}

		if (!WriteBoolean(m_hPipe, true)) { errorLine = __LINE__; goto error; }

		if(!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

		if(!ReadBoolean(m_hPipe, m_Server64Bit)) { errorLine = __LINE__; goto error; }

		m_Connected = true;

		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);
			EngineThread::m_Active = true;
		}

		if (!DispatchSharedSurfaces()) { errorLine = __LINE__; goto error; }

		return true;

	error:
		Tier0_Warning("AfxInterop::Connect: Error in line %i.\n", errorLine);
		Disconnect();
		return false;
	}

	void Disconnect() {
		{
			std::unique_lock<std::mutex> lock(EngineThread::m_ActiveMutex);

			if (INVALID_HANDLE_VALUE != EngineThread::m_hPipe)
			{
				if (!CloseHandle(EngineThread::m_hPipe))
				{
					Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
				}
				EngineThread::m_hPipe = INVALID_HANDLE_VALUE;
			}

			if (EngineThread::m_Active)
			{
				EngineThread::m_FrameInfoSent = false;
				EngineThread::m_Active = false;
			}
		}

		if (INVALID_HANDLE_VALUE != m_hPipe)
		{
			if (!CloseHandle(m_hPipe))
			{
				Tier0_Warning("AfxInterop::Disconnect: Error in line %i.\n", __LINE__);
			}
			m_hPipe = INVALID_HANDLE_VALUE;
		}

		if (m_Connected)
		{
			m_Connected = false;

			for (std::set<IAfxInteropSurface *>::iterator it = m_Surfaces.begin(); it != m_Surfaces.end(); ++it)
			{
				(*it)->AfxSetDepthSurface(NULL);
				(*it)->AfxSetReplacement(NULL);
			}
		}
	}


	bool ReadBytes(HANDLE hFile, LPVOID lpBuffer, int offset, DWORD numBytes)
	{
		lpBuffer = &(((char *)lpBuffer)[offset]);
		BOOL success = false;

		do
		{
			DWORD bytesRead;

			success = ReadFile(hFile, lpBuffer, numBytes, &bytesRead, NULL);

			if (!success)
			{
				Tier0_Warning("!ReadBytes: GetLastError=%d\n", GetLastError());
				return false;
			}

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

	bool ReadSByte(HANDLE hFile, signed char & value)
	{
		return ReadByte(hFile, (BYTE &)value);
	}

	bool ReadUInt32(HANDLE hFile, UINT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadCompressedUInt32(HANDLE hFile, UINT32 & outValue)
	{
		BYTE value;

		if (!ReadByte(hFile, value))
			return false;

		if (value < 255)
		{
			outValue = value;
			return true;
		}

		return ReadUInt32(hFile, outValue);
	}

	bool ReadInt32(HANDLE hFile, INT32 & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadCompressedInt32(HANDLE hFile, INT32 & outValue)
	{
		signed char value;

		if (!ReadSByte(hFile, value))
			return false;

		if (value < 127)
		{
			outValue = value;
			return true;
		}

		return ReadInt32(hFile, outValue);
	}

	bool ReadHandle(HANDLE hFile, HANDLE & outValue)
	{
		return ReadBytes(hFile, &outValue, 0, sizeof(outValue));
	}

	bool ReadStringUTF8(HANDLE hFile, std::string & outValue)
	{
		UINT32 length;

		if (!ReadCompressedUInt32(hFile, length)) return false;

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

	bool WriteByte(HANDLE hFile, BYTE value) {

		return WriteBytes(hFile, &value, 0, sizeof(value));
	}

	bool WriteSByte(HANDLE hFile, signed char value)
	{
		return WriteByte(hFile, (BYTE)value);
	}

	bool WriteUInt32(HANDLE hFile, UINT32 value) {
		return WriteBytes(hFile, &value, 0, sizeof(value));
	}

	bool WriteCompressedUInt32(HANDLE hFile, UINT32 value)
	{
		if (0 <= value && value <= 255 - 1)
			return WriteByte(hFile, (BYTE)value);

		return WriteByte(hFile, 255) && WriteUInt32(hFile, value);
	}

	bool WriteInt32(HANDLE hFile, INT32 value) {
		return WriteBytes(hFile, &value, 0, sizeof(value));
	}

	bool WriteCompressedInt32(HANDLE hFile, INT32 value)
	{
		if (-128 <= value && value <= 127 - 1)
			return WriteSByte(hFile, (signed char)value);

		return WriteByte(hFile, 255)
			&& WriteUInt32(hFile, value);
	}

	bool WriteSingle(HANDLE hFile, float value)
	{
		return WriteBytes(hFile, &value, 0, sizeof(value));
	}

	bool WriteStringUTF8(HANDLE hFile, const std::string value)
	{
		UINT32 length = (UINT32)value.length();

		return WriteCompressedUInt32(hFile, length)
			&& WriteBytes(hFile, (LPVOID)value.c_str(), 0, length);
	}

	bool WriteHandle(HANDLE hFile, HANDLE value)
	{
		return WriteBytes(hFile, &value, 0, sizeof(value));
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
		else if (0 == _stricmp("send", arg1))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			AfxInterop::EngineThread::AddCommand(&subArgs);

			return;
		}
	}

	Tier0_Msg(
		"afx_interop pipeName [...] - Name of the pipe to connect to.\n"
		"afx_interop connect [...] - Controls if interop connection is enabled.\n"
		"afx_interop send [<arg1>[ <arg2> [ ...]] - Queues a command to be sent to the server (lossy if connection is unstable).\n"
	);
}

#endif
