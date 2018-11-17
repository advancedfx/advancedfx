#include "stdafx.h"

#include "AfxInterop.h"

#ifdef AFX_INTEROP

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"

#include <Windows.h>

#include <set>

#include <mutex>
#include <atomic>

extern WrpVEngineClient * g_VEngineClient;

extern Hook_VClient_RenderView g_Hook_VClient_RenderView;

namespace AfxInterop {

	const INT32 m_Version = 0;

	enum DrawingMessage
	{
		DrawingMessage_DrawingThreadBeforeHud = 1,
		DrawingMessage_NewTexture = 2,
		DrawingMessage_ReleaseTexture = 3,
	};


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
	bool WriteByte(HANDLE hFile, BYTE value);
	bool WriteInt32(HANDLE hFile, INT32 value);
	bool WriteStringUTF8(HANDLE hFile, const std::string);
	bool WriteHandle(HANDLE hFille, HANDLE value);
	bool Flush(HANDLE hFile);

	bool m_Enabled = false;
	bool m_Connected = false;
	HANDLE m_hPipe = INVALID_HANDLE_VALUE;
	bool m_Server64Bit = false;

	IDirect3DTexture9 * m_FbTexture = nullptr;
	IDirect3DTexture9 * m_FbDepthTexture = nullptr;

	namespace EngineThread {

		enum EngineMessage {
			EngineMessage_LevelInitPreEntity = 1,
			EngineMessage_LevelShutDown = 2,
			EngineMessage_BeforeFrameStart = 3,
			EngineMessage_BeforeFrameRenderStart = 4,
			EngineMessage_AfterFrameRenderEnd = 5,
			EngineMessage_EntityCreated = 6,
			EngineMessage_EntityDeleted = 7
		};

		std::mutex m_ActiveMutex;
		bool m_Active = false;
		HANDLE m_hPipe = INVALID_HANDLE_VALUE;

		std::mutex m_ConnectMutex;
		bool m_WantsConnect = false;
		std::string m_PipeName("advancedfxInterop");

		int m_Frame = -1;
	}

	class ISharedTextureHook abstract {
	public:
		virtual void Unmanage() = 0;
		virtual bool ShareWithServer(HANDLE hPipe) = 0;
		virtual INT32 GetTextureId() = 0;
	};

	class ITextureManager_SharedTextureHook abstract
	{
	public:
		virtual void SharedTextureHookReleased(ISharedTextureHook * sharedTextureHook) = 0;
	};

	class CSharedTextureHook : public IDirect3DTexture9, public ISharedTextureHook
	{
	public:
		CSharedTextureHook(
			ITextureManager_SharedTextureHook * textureManager,
			const char * textureName, const char * textureGroup,
			UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
			IDirect3DTexture9 * texture, HANDLE sharedHandle)
			: m_Parent(texture)
			, m_TextureName(textureName)
			, m_TextureGroup(textureGroup)
			, m_TextureManager(textureManager)
			, m_Width(Width)
			, m_Height(Height)
			, m_Levels(Levels)
			, m_Usage(Usage)
			, m_Format(Format)
			, m_SharedHandle(sharedHandle)
		{
		}

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
			return m_Parent->QueryInterface(riid, ppvObj);
		}

		STDMETHOD_(ULONG, AddRef)(THIS)
		{
			ULONG result = m_Parent->AddRef();

			++m_RefCount;

			return result;
		}

		STDMETHOD_(ULONG, Release)(THIS)
		{
			--m_RefCount;

			HRESULT result = m_Parent->Release();

			if (0 == m_RefCount)
				delete this;

			return result;
		}

		/*** IDirect3DBaseTexture9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) {
			return m_Parent->GetDevice(ppDevice);
		}

		STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) {
			return m_Parent->SetPrivateData(refguid, pData, SizeOfData, Flags);
		}

		STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) {
			return m_Parent->GetPrivateData(refguid, pData, pSizeOfData);
		}

		STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) {
			return m_Parent->FreePrivateData(refguid);
		}

		STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) {
			return m_Parent->SetPriority(PriorityNew);
		}

		STDMETHOD_(DWORD, GetPriority)(THIS) {
			return m_Parent->GetPriority();
		}

		STDMETHOD_(void, PreLoad)(THIS) {
			m_Parent->PreLoad();
		}

		STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) {
			return m_Parent->GetType();
		}

		STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) {
			return m_Parent->SetLOD(LODNew);
		}

		STDMETHOD_(DWORD, GetLOD)(THIS) {
			return m_Parent->GetLOD();
		}

		STDMETHOD_(DWORD, GetLevelCount)(THIS) {
			return m_Parent->GetLevelCount();
		}

		STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) {
			return m_Parent->SetAutoGenFilterType(FilterType);
		}

		STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) {
			return m_Parent->GetAutoGenFilterType();
		}

		STDMETHOD_(void, GenerateMipSubLevels)(THIS) {
			m_Parent->GenerateMipSubLevels();
		}

		STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc) {
			return m_Parent->GetLevelDesc(Level, pDesc);
		}

		STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel)
		{
			return m_Parent->GetSurfaceLevel(Level, ppSurfaceLevel);
		}

		STDMETHOD(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) {
			return m_Parent->LockRect(Level, pLockedRect, pRect, Flags);
		}

		STDMETHOD(UnlockRect)(THIS_ UINT Level) {
			return m_Parent->UnlockRect(Level);
		}

		STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect) {
			return m_Parent->AddDirtyRect(pDirtyRect);
		}

		//#ifdef D3D_DEBUG_INFO
		LPCWSTR Name = L"n/a (AfxInterop::CShareTextureHook)";
		UINT Width = 0;
		UINT Height = 0;
		UINT Levels = 0;
		DWORD Usage = 0;
		D3DFORMAT Format = D3DFMT_FORCE_DWORD;
		D3DPOOL Pool = D3DPOOL_FORCE_DWORD;
		DWORD Priority = 0;
		DWORD LOD = 0;
		D3DTEXTUREFILTERTYPE FilterType = D3DTEXF_FORCE_DWORD;
		UINT LockCount = 0;
		LPCWSTR CreationCallStack = L"n/a (AfxInterop::CShareTextureHook)";
		//#endif

		virtual void ISharedTextureHook::Unmanage() {
			m_TextureManager = nullptr;
		}

		virtual bool ISharedTextureHook::ShareWithServer(HANDLE hPipe)
		{
			// TextureId:
			if (!WriteInt32(hPipe, this->GetTextureId()))
				return false;

			if (!WriteStringUTF8(hPipe, m_TextureGroup))
				return false;

			if (!WriteStringUTF8(hPipe, m_TextureName))
				return false;

			if (!WriteInt32(hPipe, m_Width))
				return false;

			if (!WriteInt32(hPipe, m_Height))
				return false;

			if (!WriteInt32(hPipe, m_Levels))
				return false;

			if (!WriteInt32(hPipe, m_Usage))
				return false;

			if (!WriteInt32(hPipe, m_Format))
				return false;

			if (!WriteInt32(hPipe, m_Pool))
				return false;

			if (!WriteHandle(hPipe, m_SharedHandle))
				return false;

			return true;
		}

		virtual INT32 ISharedTextureHook::GetTextureId() {
			return (INT32)m_Parent;
		}

	private:
		ULONG m_RefCount;
		IDirect3DTexture9 * m_Parent;
		std::string m_TextureName;
		std::string m_TextureGroup;
		ITextureManager_SharedTextureHook * m_TextureManager;
		UINT m_Width;
		UINT m_Height;
		UINT m_Levels;
		DWORD m_Usage;
		D3DFORMAT m_Format;
		D3DPOOL m_Pool;
		HANDLE m_SharedHandle;

		CSharedTextureHook() {
			if (m_TextureManager) m_TextureManager->SharedTextureHookReleased(this);
		}
	};

	class CTextureManager : public ITextureManager_SharedTextureHook
	{
	public:
		void Manage(
			const char * textureName, const char * textureGroup,
			UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
			IDirect3DTexture9 * texture, HANDLE sharedHandle)
		{
			ISharedTextureHook * sharedTextureHook = new CSharedTextureHook(
				this,
				textureName,
				textureGroup,
				Width,
				Height,
				Levels,
				Usage,
				Format,
				Pool,
				texture,
				sharedHandle
			);

			m_SharedTextureHooks.insert(sharedTextureHook);

			if (m_Connected)
			{
				DispatchCreateTexture(sharedTextureHook);
			}
		}

		bool ConnectDispatchTextures()
		{
			if (m_Connected) {
				for (std::set<ISharedTextureHook *>::iterator it = m_SharedTextureHooks.begin(); it != m_SharedTextureHooks.end(); ++it)
				{
					if (!DispatchCreateTexture(*it)) return false;
				}
			}

			return true;
		}

		virtual void SharedTextureHookReleased(ISharedTextureHook * sharedTextureHook)
		{
			if (m_Connected)
			{
				DispatchReleaseTexture(sharedTextureHook);
			}

			m_SharedTextureHooks.erase(sharedTextureHook);
		}

		~CTextureManager() {

			for (std::set<ISharedTextureHook *>::iterator it = m_SharedTextureHooks.begin(); it != m_SharedTextureHooks.end(); ++it)
			{
				(*it)->Unmanage();
			}
		}

	private:
		std::set<ISharedTextureHook *> m_SharedTextureHooks;


		bool DispatchCreateTexture(ISharedTextureHook * texture)
		{
			int errorLine = 0;

			if(!WriteInt32(m_hPipe, DrawingMessage_NewTexture)) { errorLine = __LINE__; goto error; }
			if (!texture->ShareWithServer(m_hPipe)) { errorLine = __LINE__; goto error; }

			return true;

		error:
			Tier0_Warning("AfxInterop::CTextureManager::DispatchCreateTexture: Error in line %i.\n", errorLine);
			{
				std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

				Disconnect();
			}
			return false;
		}

		bool DispatchReleaseTexture(ISharedTextureHook * texture)
		{
			int errorLine = 0;

			if (!WriteInt32(m_hPipe, DrawingMessage_ReleaseTexture)) { errorLine = __LINE__; goto error; }
			if (!WriteInt32(m_hPipe, (INT32)texture->GetTextureId())) { errorLine = __LINE__; goto error; }
			if (!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

			// Wait for confirmation:
			bool done;
			do
			{
				if(!ReadBoolean(m_hPipe,done)) { errorLine = __LINE__; goto error; }
			} while (!done);

			return true;

		error:
			Tier0_Warning("AfxInterop::CTextureManager::DispatchReleaseTexture: Error in line %i.\n", errorLine);
			{
				std::unique_lock<std::mutex> lock(EngineThread::m_ConnectMutex);

				Disconnect();
			}
			return false;
		}

	} g_TextureManager;

	void DllProcessAttach() {
		m_Enabled = wcsstr(GetCommandLineW(), L"-afxInterop");

		if (!m_Enabled) return;
	}

	void BeforeFrameStart() {
		if (!m_Enabled) return;

		int errorLine = 0;

		//if (WrpGlobals * pWrpGlobals = g_Hook_VClient_RenderView.GetGlobals()) {
		//	EngineThread::m_Frame = pWrpGlobals->framecount_get();
		//}
		//else {
		//	++EngineThread::m_Frame;
		//}

		if(EngineThread::m_ActiveMutex.try_lock())
		{
			if (EngineThread::m_Active)
			{
				if (!WriteInt32(EngineThread::m_hPipe, EngineThread::EngineMessage_BeforeFrameStart)) { errorLine = __LINE__; goto locked_error; }
				if (!Flush(EngineThread::m_hPipe)) { errorLine = __LINE__; goto locked_error; }

				int commandCount = 0;
				{
					BYTE byteCommandCount;
					if (!ReadByte(EngineThread::m_hPipe, byteCommandCount)) { errorLine = __LINE__; goto locked_error; }
					commandCount = byteCommandCount;
				}
				if (255 == commandCount)
				{
					if (!ReadInt32(EngineThread::m_hPipe, commandCount)) { errorLine = __LINE__; goto locked_error; }
				}

				std::string command;

				while (0 < commandCount)
				{
					if (!ReadStringUTF8(EngineThread::m_hPipe, command)) { errorLine = __LINE__; goto locked_error; }

					g_VEngineClient->ExecuteClientCmd(command.c_str());

					--commandCount;
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
		
		//
		// This is the final message, before we allow to spin:

		if (!WriteInt32(m_hPipe, DrawingMessage_DrawingThreadBeforeHud)) { errorLine = __LINE__; goto error; }

		if (!WriteInt32(m_hPipe, (INT32)m_FbTexture)) { errorLine = __LINE__; goto error; }

		if (!WriteInt32(m_hPipe, (INT32)m_FbDepthTexture)) { errorLine = __LINE__; goto error; }

		// No frame info available yet:
		if (!WriteBoolean(m_hPipe, false)) { errorLine = __LINE__; goto error; }

		if (!Flush(m_hPipe)) { errorLine = __LINE__; goto error; }

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

	bool CreateTexture(const char * textureName, const char * textureGroup, IDirect3DDevice9 * device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, HRESULT & resultp)
	{
		if (!m_Enabled) return false;

		if (ppTexture && textureName && textureGroup && 0 == strcmp("RenderTargets", textureGroup) && (0 == strcmp("_rt_fullframefb", textureName) || 0 == strcmp("_rt_fullframedepth", textureName)))
		{
			HANDLE pHandle = NULL;

			if (!pSharedHandle) pSharedHandle = &pHandle;

			HRESULT hr = device->CreateTexture(Width, Height, 1,
				D3DUSAGE_RENDERTARGET,
				Format,
				D3DPOOL_DEFAULT,
				(IDirect3DTexture9**)ppTexture,
				pSharedHandle);

			if(!SUCCEEDED(hr)) MessageBox(NULL, SUCCEEDED(hr) ? "OKAY" : "ERROR", "AfxInterop::CreateTexture", MB_OK);

			if(SUCCEEDED(hr))
			{
				//g_TextureManager.Manage(textureName, textureGroup, Width, Height, Levels, Usage, Format, Pool, *ppTexture, *pSharedHandle);
				resultp = hr;
				return true;
			}

		}

		return false;
	}

	void OnSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
	{
		if (0 == RenderTargetIndex)
		{
			if (nullptr != pRenderTarget)
			{
				if (D3D_OK == pRenderTarget->GetContainer(__uuidof(IDirect3DTexture9), (void **)&m_FbTexture))
					return;
			}
		}

		m_FbTexture = nullptr;
	}

	void OnSetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
	{
		if (nullptr != pNewZStencil)
		{
			if (D3D_OK == pNewZStencil->GetContainer(__uuidof(IDirect3DTexture9), (void **)&m_FbDepthTexture))
				return;
		}

		m_FbDepthTexture = nullptr;
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

		if(!g_TextureManager.ConnectDispatchTextures()) { errorLine = __LINE__; goto error; }

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

	bool WriteByte(HANDLE hFile, BYTE value) {

		return WriteBytes(hFile, &value, 0, sizeof(value));
	}

	bool WriteInt32(HANDLE hFile, INT32 value) {
		return WriteBytes(hFile, &value, 0, sizeof(INT32));
	}

	bool WriteStringUTF8(HANDLE hFile, const std::string value)
	{
		int length = (int)value.length();

		if (length < 255) {
			return WriteByte(hFile, (BYTE)length);
		}

		if (!WriteByte(hFile, 255))
			return false;

		if (!WriteInt32(hFile, length))
			return false;

		return WriteBytes(hFile, (LPVOID)value.c_str(), 0, length);
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
	}

	Tier0_Msg(
		"afx_interop pipeName [...] - Name of the pipe to connect to.\n"
		"afx_interop connect [...] - Controls if interop connection is enabled.\n"
	);
}

#endif
