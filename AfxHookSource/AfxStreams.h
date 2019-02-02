#pragma once

#define AFXSTREAMS_REFTRACKER 0

#include "SourceInterfaces.h"
#include "AfxInterfaces.h"
#include "AfxClasses.h"
#include "WrpConsole.h"
#include "AfxShaders.h"
#include "csgo_Stdshader_dx9_Hooks.h"
#include "CamIO.h"
#include "MatRenderContextHook.h"

#define AFX_SHADERS_CSGO 0

#if AFX_SHADERS_CSGO
#include <shaders/build/afxHook_splinerope_ps20.h>
#include <shaders/build/afxHook_splinerope_ps20b.h>
#include <shaders/build/afxHook_splinecard_vs20.h>
#include <shaders/build/afxHook_spritecard_vs20.h>
#include <shaders/build/afxHook_spritecard_ps20.h>
#include <shaders/build/afxHook_spritecard_ps20b.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps20.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps20b.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps30.h>
#endif

#include <shared/bvhexport.h>

#include <string>
#include <set>
#include <list>
#include <queue>
#include <map>
#include <stack>
#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>

typedef void(__stdcall * CCSViewRender_Render_t)(void * this_ptr, const SOURCESDK::vrect_t_csgo * rect);
typedef void(__stdcall * CCSViewRender_RenderView_t)(void * this_ptr, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw);

class CAfxStreams;

extern CAfxStreams g_AfxStreams;


class IAfxStreamContext abstract
{
public:
	//
	// Stream settings info

	virtual bool ViewRenderShouldForceNoVis(bool orgValue) = 0;

	//
	// General state:

	virtual void DrawingHudBegin(void) = 0;

	virtual void DrawingHudEnd(void) = 0;

	virtual void DrawingSkyBoxViewBegin(void) = 0;

	virtual void DrawingSkyBoxViewEnd(void) = 0;

	//
	// Context specific functions:

	//virtual void Viewport(int x, int y, int width, int height) = 0;

	virtual SOURCESDK::IMaterial_csgo * MaterialHook(SOURCESDK::IMaterial_csgo * material, void * proxyData) = 0;

	virtual void DrawInstances(int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance) = 0;

	virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0) = 0;

	virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists) = 0;

	virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0) = 0;

	/// <remarks>
	/// This is a good chance for an implmentation
	//  to hook and follow the subcontext actually,
	/// also a good place to inject markers for the rendering state
	/// (so we get to know about the sub-queue / sub-context).
	/// </remarks>
	virtual void QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor) = 0;

#if AFX_SHADERS_CSGO
	virtual void SetVertexShader(CAfx_csgo_ShaderState & state) = 0;

	virtual void SetPixelShader(CAfx_csgo_ShaderState & state) = 0;
#endif
};

#if AFXSTREAMS_REFTRACKER
void AfxStreams_RefTracker_Inc(void);
void AfxStreams_RefTracker_Dec(void);

int AfxStreams_RefTracker_Get(void);

#define AFXSTREAMS_REFTRACKER_INC AfxStreams_RefTracker_Inc();
#define AFXSTREAMS_REFTRACKER_DEC AfxStreams_RefTracker_Dec();

#else

#define AFXSTREAMS_REFTRACKER_INC
#define AFXSTREAMS_REFTRACKER_DEC

#endif

class CAfxImageBuffer;

class CAfxImageBufferPool
{
public:
	CAfxImageBufferPool();

	/// <remarks>Must not be called until all buffers are done.</remarks>
	~CAfxImageBufferPool();

	CAfxImageBuffer * AquireBuffer(void);

	void ImageBuffer_Done(CAfxImageBuffer * buffer);

private:
	std::stack<CAfxImageBuffer *> m_Buffers;
	std::mutex m_BuffersMutex;
	std::condition_variable m_BufferAvailableCondition;
};

class CAfxImageBuffer
{
public:
	enum ImageBufferPixelFormat
	{
		IBPF_BGR,
		IBPF_BGRA,
		IBPF_A,
		IBPF_ZFloat
	};

	void * Buffer;

	ImageBufferPixelFormat PixelFormat;
	int Width;
	int Height;
	size_t ImagePitch;
	size_t ImageBytes;

	CAfxImageBuffer(CAfxImageBufferPool * pool);
	~CAfxImageBuffer();

	/// <summary>Releases the buffer back to the pool (it may not be used anymore until being aquired from the pool again).</summary>
	void Release(void);

	bool AutoRealloc(ImageBufferPixelFormat pixelFormat, int width, int height);
	bool WriteToFile(const std::wstring & path, bool ifZip, bool ifBmpNotTga) const;

	/// <summary>
	/// Resizes and merges this BGR format buffer with the blue component of
	/// an buffer of same Width, Height and ImagePitch.
	/// On success the resulting format is BGRA
	/// </summary>
	bool BgrMergeBlueToRgba(CAfxImageBuffer const * alphaBuffer);

private:
	size_t m_BufferBytesAllocated;
	CAfxImageBufferPool * m_Pool;
};


class CAfxRecordStream;

class CAfxStreamShared
{
public:
	virtual ~CAfxStreamShared()
	{
	}

	//
	// Reference counting:

	virtual int AddRef(void)
	{
		AFXSTREAMS_REFTRACKER_INC

		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(void)
	{
		m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (0 == result)
			delete this;

		AFXSTREAMS_REFTRACKER_DEC

		return result;
	}

	//
	// Lock functions:
	//
	// These should be used when accessing concurrent resources on the stream.

	void InterLock(void)
	{
		std::unique_lock<std::mutex> lock(m_LockMutex);

		m_LockCondition.wait(lock, [this]() { return m_LockCount == 0; });

		++m_LockCount;
	}

	void InterLockIncrement(void)
	{
		std::unique_lock<std::mutex> lock(m_LockMutex);

		++m_LockCount;
	}

	void InterLockDecrement(void)
	{
		bool notify = false;

		{
			std::unique_lock<std::mutex> lock(m_LockMutex);
			--m_LockCount;

			notify = m_LockCount == 0;
		}

		if (notify)
			m_LockCondition.notify_one();
	}

protected:
	int LockCount_get(void) {
		return m_LockCount;
	}

private:
	std::mutex m_RefMutex;
	std::mutex m_LockMutex;
	std::condition_variable m_LockCondition;
	int m_LockCount = 0;
	int m_RefCount = 0;
};

class CAfxStreamSharedInterLock
{
public:
	CAfxStreamSharedInterLock(CAfxStreamShared * stream)
		: m_Stream(stream)
	{
		if (m_Stream)
			m_Stream->InterLock();
	}

	~CAfxStreamSharedInterLock()
	{
		if (m_Stream)
			m_Stream->InterLockDecrement();
	}

private:
	CAfxStreamShared * m_Stream;
};

class CAfxStream
	: public CAfxStreamShared
{
public:
	CAfxStream()
	{
	}

	//
	// Polymorphism:

	virtual CAfxStream * AsAfxStream(void) { return this; }
	virtual CAfxRecordStream * AsAfxRecordStream(void) { return 0; }

	//
	// Hooks:

	virtual void LevelShutdown(void)
	{
	}

protected:
	virtual ~CAfxStream()
	{
	}

private:
};

class CAfxSafeRefCounted
	: public SOURCESDK::IRefCounted_csgo
{
public:
	CAfxSafeRefCounted()
		: m_RefCount(0)
	{
	}

	//
	// SOURCESDK::IRefCounted_csgo:

	virtual int AddRef(void)
	{
		AFXSTREAMS_REFTRACKER_INC

		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(void)
	{
		m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (0 == result)
			delete this;

		AFXSTREAMS_REFTRACKER_DEC

		return result;
	}

protected:
	std::mutex m_RefMutex;
	int m_RefCount;

	virtual ~CAfxSafeRefCounted()
	{
	}

private:

};

class CAfxFunctor abstract
	: public SOURCESDK::CSGO::CFunctor
{
public:
	CAfxFunctor()
		: m_RefCount(0)
	{
	}

	//
	// SOURCESDK::IRefCounted_csgo:

	virtual int AddRef(void)
	{
		AFXSTREAMS_REFTRACKER_INC

		return ++m_RefCount;
	}

	virtual int Release(void)
	{
		int result = --m_RefCount;
		
		if (0 == result)
			delete this;

		AFXSTREAMS_REFTRACKER_DEC
			
		return result;
	}

protected:
	virtual ~CAfxFunctor()
	{
	}

private:
	std::atomic_int m_RefCount;
};

IAfxMatRenderContext * GetCurrentContext();

void QueueOrExecute(IAfxMatRenderContextOrg * ctx, SOURCESDK::CSGO::CFunctor * functor);

class CAfxLeafExecute_Functor
	: public CAfxFunctor
{
public:
	CAfxLeafExecute_Functor(SOURCESDK::CSGO::CFunctor * functor)
		: m_Functor(functor)
	{
		m_Functor->AddRef();
	}

	virtual void operator()()
	{
		SOURCESDK::CSGO::ICallQueue * queue = GetCurrentContext()->GetOrg()->GetCallQueue();

		if (queue)
		{
			queue->QueueFunctor(this);
		}
		else
		{
			(*m_Functor)();
		}
	}

protected:
	virtual ~CAfxLeafExecute_Functor()
	{
		m_Functor->Release();
	}

private:
	SOURCESDK::CSGO::CFunctor * m_Functor;

};

class CAfxBlockFunctor
	: public CAfxFunctor
{
public:
	CAfxBlockFunctor(bool block)
		: m_Block(block)
	{

	}

	virtual void operator()();

private:
	bool m_Block;
};

class CAfxBaseFxStream;

struct AfxViewportData_t
{
	int x;
	int y;
	int width;
	int height;
	float zNear;
	float zFar;
};

class CAfxRenderViewStream
	: public CAfxStreamShared
{
public:
	enum StreamCaptureType
	{
		SCT_Normal,
		SCT_Depth24,
		SCT_Depth24ZIP,
		SCT_DepthF,
		SCT_DepthFZIP
	};

	enum DrawType
	{
		DT_NoChange,
		DT_Draw,
		DT_NoDraw
	};

	static CAfxRenderViewStream * EngineThreadStream_get()
	{
		return m_EngineThreadStream;
	}


	CAfxRenderViewStream();

	//
	//

	virtual CAfxRenderViewStream * AsAfxRenderViewStream(void) { return this; }

	virtual CAfxBaseFxStream * AsAfxBaseFxStream(void) { return 0; }

	char const * AttachCommands_get(void);
	void AttachCommands_set(char const * value);

	char const * DetachCommands_get(void);
	void DetachCommands_set(char const * value);

	DrawType DrawHud_get(void);
	void DrawHud_set(DrawType value);

	DrawType DrawViewModel_get(void);
	void DrawViewModel_set(DrawType value);

	virtual float SmokeOverlayAlphaFactor_get(void)
	{
		return 1.0f;
	}

	StreamCaptureType StreamCaptureType_get(void);
	void StreamCaptureType_set(StreamCaptureType value);

	bool ForceBuildingCubemaps_get(void)
	{
		return m_ForceBuildingCubemaps;
	}

	void ForceBuildingCubemaps_set(bool value)
	{
		m_ForceBuildingCubemaps = value;
	}

	virtual void LevelShutdown(void)
	{
	}

	void QueueCapture(IAfxMatRenderContextOrg * ctx, CAfxRecordStream * captureTarget, int x, int y, int width, int height);

	//
	// State information:

	virtual void OnRenderBegin(const AfxViewportData_t & viewport)
	{
		m_EngineThreadStream = this;
	}

	virtual void OnRenderEnd()
	{
		m_EngineThreadStream = nullptr;
	}

protected:
	bool m_ForceBuildingCubemaps = false;
	StreamCaptureType m_StreamCaptureType;
	bool m_DrawingSkyBox;

	virtual ~CAfxRenderViewStream();

private:
	class CCaptureFunctor :
		public CAfxFunctor
	{
	public:
		CCaptureFunctor(CAfxRenderViewStream & stream, CAfxRecordStream * captureTarget, int x, int y, int width, int height);

		void operator()();

	private:
		CAfxRenderViewStream & m_Stream;
		CAfxRecordStream * m_CaptureTarget;
		int m_X;
		int m_Y;
		int m_Width;
		int m_Height;
	};

	static CAfxRenderViewStream * m_EngineThreadStream;

	DrawType m_DrawViewModel;
	DrawType m_DrawHud;
	std::string m_AttachCommands;
	std::string m_DetachCommands;

	void Capture(CAfxRecordStream * captureTarget, int x, int y, int width, int height);
};

class CAfxSingleStream;
class CAfxTwinStream;

class CAfxRecordStream abstract
: public CAfxStream
{
public:
	CAfxRecordStream(char const * streamName);

	virtual CAfxRecordStream * AsAfxRecordStream(void) { return this; }

	virtual CAfxSingleStream * AsAfxSingleStream(void) { return 0; }
	virtual CAfxTwinStream * AsAfxTwinStream(void) { return 0; }

	char const * StreamName_get(void);

	bool Record_get(void);
	void Record_set(bool value);

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordStart();

	void QueueCaptureStart(IAfxMatRenderContextOrg * ctx);

	void QueueCaptureEnd(IAfxMatRenderContextOrg * ctx, const std::wstring & takeDir, int frameNumber, wchar_t const * fileExtension);

	/// <remarks>This is not guaranteed to be called, i.e. not called upon buffer re-allocation error.</remarks>
	virtual void OnImageBufferCaptured(CAfxRenderViewStream * stream, CAfxImageBuffer * buffer) = 0;

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordEnd();

protected:
	virtual void CaptureStart(void) = 0;

	/// <param name="outPath">Can be 0 in case the path could not be created successfully.</param>
	virtual void CaptureEnd(std::wstring const * outPath) = 0;

	static void WriteFile_EnterScope(void)
	{
		m_Shared.WriteFile_EnterScope();	
	}

	static void WriteFile_ExitScope(void)
	{
		m_Shared.WriteFile_ExitScope();
	}

private:
	class CCaptureStartFunctor
		: public CAfxFunctor
	{
	public:
		CCaptureStartFunctor(CAfxRecordStream & stream)
			: m_Stream(stream)
		{
			m_Stream.AddRef();
			m_Stream.InterLockIncrement();
		}

		void operator()()
		{
			m_Stream.CaptureStart();
		}

	private:
		CAfxRecordStream & m_Stream;
	};

	class CCaptureEndFunctor
		: public CAfxFunctor
	{
	public:
		CCaptureEndFunctor(CAfxRecordStream & stream, const std::wstring & takeDir, int frameNumber, wchar_t const * fileExtension)
			: m_Stream(stream)
		{
			m_OutPathOk = m_Stream.CreateCapturePath(takeDir, frameNumber, fileExtension, m_OutPath);
		}

		void operator()()
		{
			m_Stream.CaptureEnd(m_OutPathOk ? &m_OutPath : 0);
			m_Stream.InterLockDecrement();
			m_Stream.Release();
		}

	private:
		CAfxRecordStream & m_Stream;
		std::wstring m_OutPath;
		bool m_OutPathOk;
	};

	class CShared
	{
	public:
		void WriteFile_EnterScope(void)
		{
			std::unique_lock<std::mutex> lock(m_WriteFileMutex);

			m_WriteFileCondition.wait(lock, [this]() { return m_WriteFileCount < m_MaxInScope; });

			++m_WriteFileCount;
		}

		void WriteFile_ExitScope(void)
		{
			{
				std::unique_lock<std::mutex> lock(m_WriteFileMutex);
				--m_WriteFileCount;
			}

			m_WriteFileCondition.notify_one();
		}

	private:
		static const int m_MaxInScope = 2;
		std::mutex m_WriteFileMutex;
		std::condition_variable m_WriteFileCondition;
		int m_WriteFileCount = 0;
	};

	static CShared m_Shared;

	std::string m_StreamName;
	std::wstring m_CapturePath;
	bool m_Record;
	bool m_TriedCreatePath;
	bool m_SucceededCreatePath;

	/// <remarks>This is only called between RecordStart and RecordEnd and only if Record is true.</remarks>
	bool CreateCapturePath(const std::wstring & takeDir, int frameNumber, wchar_t const * fileExtension, std::wstring &outPath);
};

class CAfxSingleStream
:  public CAfxRecordStream
{
public:
	CAfxSingleStream(char const * streamName, CAfxRenderViewStream * stream);

	virtual CAfxSingleStream * AsAfxSingleStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAfxRenderViewStream * Stream_get(void);

	virtual void OnImageBufferCaptured(CAfxRenderViewStream * stream, CAfxImageBuffer * buffer);

protected:
	virtual ~CAfxSingleStream();

	virtual void CaptureStart(void);

	/// <param name="outPath">Can be 0 in case the path could not be created successfully.</param>
	virtual void CaptureEnd(std::wstring const * outPath);

private:
	CAfxRenderViewStream * m_Stream;
	CAfxImageBuffer * m_Buffer;
	std::mutex m_CaptureMutex;
	std::condition_variable m_CaptureCondition;
	bool m_Capturing = false;
};

class CAfxTwinStream
: public CAfxRecordStream
{
public:
	enum StreamCombineType
	{
		SCT_ARedAsAlphaBColor,
		SCT_AColorBRedAsAlpha,
		SCT_AHudWhiteBHudBlack
	};

	/// <remarks>Takes ownership of given streams.</remarks>
	CAfxTwinStream(char const * streamName, CAfxRenderViewStream * streamA, CAfxRenderViewStream * streamB, StreamCombineType streamCombineType);

	virtual CAfxTwinStream * AsAfxTwinStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAfxRenderViewStream * StreamA_get();
	CAfxRenderViewStream * StreamB_get();

	StreamCombineType StreamCombineType_get(void);
	void StreamCombineType_set(StreamCombineType value);

	virtual void OnImageBufferCaptured(CAfxRenderViewStream * stream, CAfxImageBuffer * buffer);

protected:
	virtual ~CAfxTwinStream();

	virtual void CaptureStart(void);

	/// <param name="outPath">Can be 0 in case the path could not be created successfully.</param>
	virtual void CaptureEnd(std::wstring const * outPath);

private:
	CAfxRenderViewStream * m_StreamA;
	CAfxRenderViewStream * m_StreamB;
	StreamCombineType m_StreamCombineType;

	CAfxImageBuffer * m_BufferA;
	CAfxImageBuffer * m_BufferB;
	std::mutex m_CaptureMutex;
	std::condition_variable m_CaptureCondition;
	bool m_Capturing = false;
};


extern bool g_DebugEnabled;


class CAfxBaseFxStream
: public CAfxRenderViewStream
{
public:
	class CActionKey
	{
	public:
		std::string m_Name;

		CActionKey(char const * name)
		: m_Name(name)
		{
		}

		CActionKey(const CActionKey & key, bool toLower)
		: m_Name(key.m_Name)
		{
			if(toLower) ToLower();
		}

		CActionKey(const CActionKey & x)
		: m_Name(x.m_Name)
		{
		}
		
		bool operator < (const CActionKey & y) const
		{
			return m_Name.compare(y.m_Name) < 0;
		}

		void ToLower(void);

	private:
	};

private:
	class CAfxBaseFxStreamContext;

public:

	class CAction
		: public CAfxSafeRefCounted
	{
	public:
		CAction()
		: m_IsStockAction(false)
		, m_Key("(unnamed)")
		{
		}

		int GetRefCount(void)
		{
			return m_RefCount;
		}

		bool IsStockAction_get(void)
		{
			return m_IsStockAction;
		}

		void IsStockAction_set(bool value)
		{
			m_IsStockAction = value;
		}
		
		CActionKey const & Key_get(void)
		{
			return m_Key;
		}

		void Key_set(CActionKey const & value)
		{
			m_Key = value;
		}

		virtual CAction * ResolveAction(CAfxTrackedMaterial * trackedMaterial)
		{
			return this;
		}

		virtual void MainThreadInitialize(void)
		{
		}

		virtual void LevelShutdown(void)
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch)
		{
		}

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, CAfxTrackedMaterial * trackedMaterial);

		virtual void DrawInstances(CAfxBaseFxStreamContext * ch, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance )
		{
			ch->GetCtx()->GetOrg()->DrawInstances(nInstanceCount, pInstance);
		}

		virtual void Draw(CAfxBaseFxStreamContext * ch, IAfxMesh * am, int firstIndex = -1, int numIndices = 0)
		{
			am->GetParent()->Draw(firstIndex, numIndices);
		}

		virtual void Draw_2(CAfxBaseFxStreamContext * ch, IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists)
		{
			am->GetParent()->Draw(pLists, nLists);
		}

		virtual void DrawModulated(CAfxBaseFxStreamContext * ch, IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0 )
		{
			am->GetParent()->DrawModulated(vecDiffuseModulation, firstIndex, numIndices);
		}

#if AFX_SHADERS_CSGO
		virtual void SetVertexShader(CAfxBaseFxStreamContext * ch, CAfx_csgo_ShaderState & state)
		{
		}

		virtual void SetPixelShader(CAfxBaseFxStreamContext * ch, CAfx_csgo_ShaderState & state)
		{
		}
#endif

	protected:
		bool m_IsStockAction;
		CActionKey m_Key;

		virtual ~CAction()
		{
		}

		virtual CAction * SafeSubResolveAction(CAction * action, CAfxTrackedMaterial * trackedMaterial)
		{
			if(action)
				return action->ResolveAction(trackedMaterial);
			
			return action;
		}

	private:

	};

	static void Console_ListActions(void)
	{
		m_Shared.Console_ListActions();
	}

	static void Console_AddReplaceAction(IWrpCommandArgs * args)
	{
		m_Shared.Console_AddReplaceAction(args);
	}

	static CAction * GetAction(CActionKey const & key)
	{
		return m_Shared.GetAction(key);
	}

	static bool RemoveAction(CActionKey const & key)
	{
		return m_Shared.RemoveAction(key);
	}

	CAfxBaseFxStream();

	static void AfxStreamsInit(void);
	static void AfxStreamsShutdown(void);

	static void MainThreadInitialize(void);

	virtual void OnRenderBegin(const AfxViewportData_t & viewport);

	virtual void OnRenderEnd(void);

	void Console_ActionFilter_Add(const char * expression, CAction * action);
	void Console_ActionFilter_AddEx(CAfxStreams * streams, IWrpCommandArgs * args);
	void Console_ActionFilter_Print(void);
	void Console_ActionFilter_Remove(int id);
	void Console_ActionFilter_Move(int id, int moveBeforeId);
	void Console_ActionFilter_Clear();

	virtual CAfxBaseFxStream * AsAfxBaseFxStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAction * RetrieveAction(CAfxTrackedMaterial * tackedMaterial, SOURCESDK::CSGO::CBaseHandle const & entityHandle);

	CAction * ClientEffectTexturesAction_get(void);
	void ClientEffectTexturesAction_set(CAction * value);

	CAction * WorldTexturesAction_get(void);
	void WorldTexturesAction_set(CAction * value);

	CAction * SkyBoxTexturesAction_get(void);
	void SkyBoxTexturesAction_set(CAction * value);

	CAction * StaticPropTexturesAction_get(void);
	void StaticPropTexturesAction_set(CAction * value);

	CAction * CableAction_get(void);
	void CableAction_set(CAction * value);

	CAction * PlayerModelsAction_get(void);
	void PlayerModelsAction_set(CAction * value);

	CAction * WeaponModelsAction_get(void);
	void WeaponModelsAction_set(CAction * value);

	CAction * StatTrakAction_get(void);
	void StatTrakAction_set(CAction * value);

	CAction * ShellModelsAction_get(void);
	void ShellModelsAction_set(CAction * value);

	CAction * OtherModelsAction_get(void);
	void OtherModelsAction_set(CAction * value);

	CAction * DecalTexturesAction_get(void);
	void DecalTexturesAction_set(CAction * value);

	CAction * EffectsAction_get(void);
	void EffectsAction_set(CAction * value);

	CAction * ShellParticleAction_get(void);
	void ShellParticleAction_set(CAction * value);

	CAction * OtherParticleAction_get(void);
	void OtherParticleAction_set(CAction * value);

	CAction * StickerAction_get(void);
	void StickerAction_set(CAction * value);

	CAction * ErrorMaterialAction_get(void);
	void ErrorMaterialAction_set(CAction * value);

	CAction * OtherAction_get(void);
	void OtherAction_set(CAction * value);

	CAction * WriteZAction_get(void);
	void WriteZAction_set(CAction * value);

	CAction * DevAction_get(void);

	CAction * OtherEngineAction_get(void);

	CAction * OtherSpecialAction_get(void);

	CAction * VguiAction_get(void);

	enum EClearBeforeHud
	{
		EClearBeforeHud_No,
		EClearBeforeHud_Black,
		EClearBeforeHud_White
	};

	EClearBeforeHud ClearBeforeHud_get(void);
	void ClearBeforeHud_set(EClearBeforeHud value);

	bool TestAction_get(void);
	void TestAction_set(bool value);

	float DepthVal_get(void);
	void DepthVal_set(float value);

	float DepthValMax_get(void);
	void DepthValMax_set(float value);

	enum EDrawDepth
	{
		EDrawDepth_None,
		EDrawDepth_Gray,
		EDrawDepth_Rgb
	};

	EDrawDepth DrawDepth_get(void)
	{
		return m_DrawDepth;
	}

	void DrawDepth_set(EDrawDepth value)
	{
		m_DrawDepth = value;
	}

	enum EDrawDepthMode
	{
		EDrawDepthMode_Inverse,
		EDrawDepthMode_Linear,
		EDrawDepthMode_LogE
	};

	EDrawDepthMode DrawDepthMode_get()
	{
		return m_DrawDepthMode;
	}

	void DrawDepthMode_set(EDrawDepthMode value)
	{
		m_DrawDepthMode = value;
	}

	virtual float SmokeOverlayAlphaFactor_get(void);
	void SmokeOverlayAlphaFactor_set(float value);

	bool ShouldForceNoVisOverride_get(void);
	void ShouldForceNoVisOverride_set(bool value);
	
	bool DebugPrint_get(void);
	void DebugPrint_set(bool value);

	void InvalidateMap(void);

	void Picker_Pick(bool pickEntityNotMaterial, bool wasVisible);
	void Picker_Stop(void);
	void Picker_Print(void);

	/*
	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	/// <pram name="depth24ZIP">if not to24, then set SCT_Normal otherwise: false: set capturetype SCT_Depth24; true: set capturetype SCT_Depth24ZIP</param>
	void ConvertStreamDepth(bool to24, bool depth24ZIP);

	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	void ConvertDepthActions(bool to24);
	*/

protected:
	class CShared
	{
	public:
		CShared();
		~CShared();

		void AfxStreamsInit(void);
		void AfxStreamsShutdown(void);

		void AddRef();
		void Release();

		void Console_ListActions(void);
		void Console_AddReplaceAction(IWrpCommandArgs * args);
		CAction * GetAction(CActionKey const & key);
		bool RemoveAction(CActionKey const & key);

		void MainThreadInitialize(void);

		void LevelShutdown(void);

		CAfxBaseFxStreamContext * RequestStreamContext(void);
		void ReturnStreamContext(CAfxBaseFxStreamContext * streamContext);


		CAction * DrawAction_get(void);
		CAction * NoDrawAction_get(void);
		CAction * DepthAction_get(void);
		//CAction * Depth24Action_get(void);
		CAction * MaskAction_get(void);
		CAction * WhiteAction_get(void);
		CAction * BlackAction_get(void);
	
	private:
		std::mutex m_StreamContextsMutex;
		std::queue<CAfxBaseFxStreamContext *> m_StreamContexts;

		int m_RefCount = 0;
		int m_MainThreadInitalizeLevel = 0;
		int m_ShutDownLevel = 0;
		std::map<CActionKey, CAction *> m_Actions;
		CAction * m_DrawAction = 0;
		CAction * m_NoDrawAction = 0;
		CAction * m_DebugDumpAction = 0;
		CAction * m_DepthAction = 0;
		//CAction * m_Depth24Action = 0;
		CAction * m_MaskAction = 0;
		CAction * m_WhiteAction = 0;
		CAction * m_BlackAction = 0;

		void CreateStdAction(CAction * & stdTarget, CActionKey const & key, CAction * action);
		void CreateAction(CActionKey const & key, CAction * action, bool isStockAction = false);
		bool Console_CheckActionKey(CActionKey & key);
	};

	static CShared m_Shared;

	CAction * m_ClientEffectTexturesAction;
	CAction * m_WorldTexturesAction;
	CAction * m_SkyBoxTexturesAction;
	CAction * m_StaticPropTexturesAction;
	CAction * m_CableAction;
	CAction * m_PlayerModelsAction;
	CAction * m_WeaponModelsAction;
	CAction * m_StatTrakAction;
	CAction * m_ShellModelsAction;
	CAction * m_OtherModelsAction;
	CAction * m_DecalTexturesAction;
	CAction * m_EffectsAction;
	CAction * m_ShellParticleAction;
	CAction * m_OtherParticleAction;
	CAction * m_StickerAction;
	CAction * m_ErrorMaterialAction;
	CAction * m_OtherAction;
	CAction * m_WriteZAction;
	CAction * m_DevAction;
	CAction * m_OtherEngineAction;
	CAction * m_OtherSpecialAction;
	CAction * m_VguiAction;
	bool m_TestAction;
	float m_DepthVal;
	float m_DepthValMax;
	float m_SmokeOverlayAlphaFactor;
	bool m_ShouldForceNoVisOverride;

	EClearBeforeHud m_ClearBeforeHud = EClearBeforeHud_No;
	EDrawDepth m_DrawDepth = EDrawDepth_None;
	EDrawDepthMode m_DrawDepthMode = EDrawDepthMode_Linear;

	virtual ~CAfxBaseFxStream();

	void SetActionAndInvalidateMap(CAction * & target, CAction * src);
	void SetAction(CAction * & target, CAction * src);

private:
	class CActionDebugDump
	: public CAction
	{
	public:
		CActionDebugDump()
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch);
	};

#if AFX_SHADERS_CSGO
	class CActionAfxVertexLitGenericHookKey
	{
	public:
		ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxVertexLitGenericHookKey()
		{
		}

		CActionAfxVertexLitGenericHookKey(
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxVertexLitGenericHookKey(const CActionAfxVertexLitGenericHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxVertexLitGenericHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxVertexLitGenericHook
	: public CAction
	{
	public:
		CActionAfxVertexLitGenericHook(CActionAfxVertexLitGenericHookKey & key);

		virtual bool CheckIsCompatible(IMaterial_csgo * material)
		{
			if(!material)
				return false;

			char const * shaderName = material->GetShaderName();

			return !strcmp(shaderName, "VertexLitGernic")
				|| !strcmp(shaderName, "UnlitGeneric");
		}

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20b m_Combos_ps20b;
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps30 m_Combos_ps30;
		CActionAfxVertexLitGenericHookKey m_Key;
	};

	class CActionUnlitGenericFallback
	: public CActionAfxVertexLitGenericHook
	{
	public:
		CActionUnlitGenericFallback(CActionAfxVertexLitGenericHookKey & key, char const * unlitGenericFallbackMaterialName);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

	protected:
		virtual ~CActionUnlitGenericFallback();

	private:
		CAfxMaterial * m_Material;
		std::string m_MaterialName;
	};
#endif

	class CActionNoDraw
	: public CAction
	{
	public:
		CActionNoDraw()
		: CAction()
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch);

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, CAfxTrackedMaterial * trackedMaterial);

/*		We could speed up a bit here, but I am not sure if it's safe to do this,
        so we just let it draw for now and block the drawing instead.

		virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0)
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void Draw_2(IAfxMesh * am, CPrimList_csgo *pLists, int nLists)
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void DrawModulated(IAfxMesh * am, const Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0 )
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void DrawInstances(IAfxMatRenderContext * ctx, int nInstanceCount, const MeshInstanceData_t_csgo *pInstance )
		{
			return;
		}
*/
	};

	class CActionDraw
	: public CAction
	{
	public:
		CActionDraw()
		: CAction()
		{
		}

		virtual bool CheckIsCompatible(CAfxTrackedMaterial * tackedMaterial)
		{
			return true;
		}
	};

#if AFX_SHADERS_CSGO
	class CActionAfxSpritecardHookKey
	{
	public:
		ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxSpritecardHookKey()
		{
		}

		CActionAfxSpritecardHookKey(
			ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxSpritecardHookKey(const CActionAfxSpritecardHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxSpritecardHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxSpritecardHook
	: public CAction
	{
	public:
		CActionAfxSpritecardHook(CActionAfxSpritecardHookKey & key);

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetVertexShader(CAfx_csgo_ShaderState & state);
		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_splinecard_vs20 m_Combos_splinecard_vs20;
		static csgo_Stdshader_dx9_Combos_spritecard_vs20 m_Combos_spritecard_vs20;
		static csgo_Stdshader_dx9_Combos_spritecard_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_spritecard_ps20b m_Combos_ps20b;
		CActionAfxSpritecardHookKey m_Key;
	};

	class CActionAfxSplineRopeHookKey
	{
	public:
		ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxSplineRopeHookKey()
		{
		}

		CActionAfxSplineRopeHookKey(
			ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxSplineRopeHookKey(const CActionAfxSplineRopeHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxSplineRopeHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxSplineRopeHook
	: public CAction
	{
	public:
		CActionAfxSplineRopeHook(CActionAfxSplineRopeHookKey & key);

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_splinerope_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_splinerope_ps20b m_Combos_ps20b;
		CActionAfxSplineRopeHookKey m_Key;
	};
#endif

	class CActionReplace
	: public CAction
	{
	public:
		CActionReplace(
			char const * materialName,
			CAction * fallBackAction);

		void OverrideColor(float const color[3])
		{
			m_OverrideColor = true;
			m_Color[0] = color[0];
			m_Color[1] = color[1];
			m_Color[2] = color[2];
		}

		void OverrideBlend(float blend)
		{
			m_OverrideBlend = true;
			m_Blend = blend;
		}

		void OverrideDepthWrite(bool depthWrite)
		{
			m_OverrideDepthWrite = true;
			m_DepthWrite = depthWrite;
		}

		virtual CAction * ResolveAction(CAfxTrackedMaterial * trackedMaterial);

		virtual void MainThreadInitialize(void);

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch);

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, CAfxTrackedMaterial * trackedMaterial);

 		virtual void DrawInstances(CAfxBaseFxStreamContext * ch, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance )
		{
			if(m_OverrideColor || m_OverrideBlend)
			{
				SOURCESDK::MeshInstanceData_t_csgo * first = const_cast<SOURCESDK::MeshInstanceData_t_csgo *>(pInstance);

				for(int i = 0; i < nInstanceCount; ++i)
				{
					if(m_OverrideColor)
					{
						first->m_DiffuseModulation.x = m_Color[0];
						first->m_DiffuseModulation.y = m_Color[1];
						first->m_DiffuseModulation.z = m_Color[2];
					}
					if(m_OverrideBlend)
					{
						first->m_DiffuseModulation.w = m_Blend;
					}
 
					++first;
				}
			}
 
			ch->GetCtx()->GetOrg()->DrawInstances(nInstanceCount, pInstance); 
		} 

	protected:
		virtual ~CActionReplace();

	private:
		CAfxOwnedMaterial * m_Material;
		std::string m_MaterialName;
		CAction * m_FallBackAction;
		bool m_OverrideColor;
		float m_Color[3];
		bool m_OverrideBlend;
		float m_Blend;
		bool m_OverrideDepthWrite;
		bool m_DepthWrite;
		CAfxTrackedMaterial * m_TrackedMaterial;

		void ExamineMaterial(SOURCESDK::IMaterial_csgo *, bool & outSplinetype, bool & outUseinstancing);
	};

	class CActionDebugDepth
	: public CAction
	{
	public:
		CActionDebugDepth(CAction * fallBackAction);

		virtual CAction * ResolveAction(CAfxTrackedMaterial * trackedMaterial);

		virtual void MainThreadInitialize(void);

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch);

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, CAfxTrackedMaterial * trackedMaterial);

	protected:
		~CActionDebugDepth();
	private:
		class CStatic
		{
		public:
			void SetDepthVal(float min, float max);

			~CStatic()
			{
				delete m_MatDebugDepthVal;
				delete m_MatDebugDepthValMax;
			}

		private:
			WrpConVarRef * m_MatDebugDepthVal = 0;
			WrpConVarRef * m_MatDebugDepthValMax = 0;
			std::mutex m_MatDebugDepthValsMutex;
		};

		static CStatic m_Static;

		CAction * m_FallBackAction;
		CAfxOwnedMaterial * m_DebugDepthMaterial;
		CAfxTrackedMaterial * m_TrackedMaterial;
	};

#if AFX_SHADERS_CSGO
	class CActionStandardResolve
	: public CAction
	{
	public:
		enum ResolveFor {
			RF_DrawDepth,
			RF_DrawDepth24,
			RF_GreenScreen,
			RF_Black,
			RF_White
		};

		CActionStandardResolve(ResolveFor resolveFor, CAction * fallBackAction);

		virtual CAction * ResolveAction(IMaterial_csgo * material);

	protected:
		~CActionStandardResolve();
	
	private:
		class CShared
		{
		public:
			CShared();
			~CShared();

			void AddRef();
			void Release();

			void LevelShutdown(IAfxStreams4Stream * streams);

			CAction * GetStdDepthAction();
			CAction * GetStdDepth24Action();
			CAction * GetStdMatteAction();
			CAction * GetStdBlackAction();
			CAction * GetStdWhiteAction();

			CAction * GetSplineRopeHookAction(CActionAfxSplineRopeHookKey & key);
			CAction * GetSpritecardHookAction(CActionAfxSpritecardHookKey & key);
			CAction * GetVertexLitGenericHookAction(CActionAfxVertexLitGenericHookKey & key);

		private:
			int m_RefCount;
			int m_ShutDownLevel;

			CAction * m_StdDepthAction;
			CAction * m_StdDepth24Action;
			CAction * m_StdMatteAction;
			CAction * m_StdBlackAction;
			CAction * m_StdWhiteAction;

			std::map<CActionAfxSplineRopeHookKey, CActionAfxSplineRopeHook *> m_SplineRopeHookActions;
			std::map<CActionAfxSpritecardHookKey, CActionAfxSpritecardHook *> m_SpritecardHookActions;
			std::map<CActionAfxVertexLitGenericHookKey, CActionAfxVertexLitGenericHook *> m_VertexLitGenericHookActions;

			void InvalidateSplineRopeHookActions();
			void InvalidateSpritecardHookActions();
			void InvalidateVertexLitGenericHookActions();
		};

		static CShared m_Shared;


		ResolveFor m_ResolveFor;
		CAction * m_FallBackAction;
	};
#endif

	class CActionFilterValue
	{
	public:
		enum TriState {
			TS_DontCare,
			TS_True,
			TS_False
		};

		CActionFilterValue()
		: m_UseHandle(false)
		, m_IsErrorMaterial(TS_DontCare)
		, m_MatchAction(0)
		{
		}

		CActionFilterValue(
			bool useHandle,
			SOURCESDK::CSGO::CBaseHandle const & handle,
			char const * name,
			char const * textureGroupName,
			char const * shaderName,
			TriState isErrorMaterial,
			CAction * matchAction)
			: m_UseHandle(useHandle)
			, m_Handle(handle)
			, m_Name(name)
			, m_TextureGroupName(textureGroupName)
			, m_ShaderName(shaderName)
			, m_IsErrorMaterial(isErrorMaterial)
			, m_MatchAction(matchAction)
		{
			if (matchAction) matchAction->AddRef();
		}

		CActionFilterValue(char const * matchString, CAction * matchAction)
		: m_UseHandle(false)
		, m_Handle()
		, m_Name(matchString)
		, m_TextureGroupName("\\*")
		, m_ShaderName("\\*")
		, m_IsErrorMaterial(TS_DontCare)
		, m_MatchAction(matchAction)
		{
			if(matchAction) matchAction->AddRef();
		}

		CActionFilterValue(const CActionFilterValue & x)
		: m_UseHandle(x.m_UseHandle)
		, m_Handle(x.m_Handle)
		, m_Name(x.m_Name)
		, m_TextureGroupName(x.m_TextureGroupName)
		, m_ShaderName(x.m_ShaderName)
		, m_IsErrorMaterial(x.m_IsErrorMaterial)
		, m_MatchAction(x.m_MatchAction)
		{
			if(m_MatchAction) m_MatchAction->AddRef();
		}

		~CActionFilterValue()
		{
			if(m_MatchAction) m_MatchAction->Release();
		}

		CActionFilterValue & operator= (const CActionFilterValue & x)
		{
			if (m_MatchAction) m_MatchAction->Release();

			this->m_UseHandle = x.m_UseHandle;
			this->m_Handle = x.m_Handle;
			this->m_Name = x.m_Name;
			this->m_TextureGroupName = x.m_TextureGroupName;
			this->m_ShaderName = x.m_ShaderName;
			this->m_IsErrorMaterial = x.m_IsErrorMaterial;
			this->m_MatchAction = x.m_MatchAction;
			
			if(m_MatchAction) m_MatchAction->AddRef();

			return *this;
		}

		static CActionFilterValue * Console_Parse(CAfxStreams * streams, IWrpCommandArgs * args);

		void Console_Print(int id)
		{
			std::string handleStr("(don't care)");

			if (m_UseHandle)
			{
				handleStr = std::to_string(m_Handle.ToInt());
			}

			Tier0_Msg("id=%i, \"handle=%s\", \"name=%s\", \"textureGroup=%s\", \"shader=%s\", \"isErrrorMaterial=%s\", \"action=%s\"\n",
				id,
				handleStr.c_str(),
				m_Name.c_str(),
				m_TextureGroupName.c_str(),
				m_ShaderName.c_str(),
				m_IsErrorMaterial == TS_True ? "1" : (m_IsErrorMaterial == TS_False ? "0" : "(don't care)"),
				m_MatchAction ? m_MatchAction->Key_get().m_Name.c_str() : "(null)"
			);
		}

		CAction * GetMatchAction(void)
		{
			return m_MatchAction;
		}

		bool GetUseHandle(void)
		{
			return m_UseHandle;
		}

		SOURCESDK::CSGO::CBaseHandle const & GetHandle(void)
		{
			return m_Handle;
		}

		bool CalcMatch_Material(CAfxTrackedMaterial * trackedMaterial);

	private:
		bool m_UseHandle;
		SOURCESDK::CSGO::CBaseHandle m_Handle;
		std::string m_Name;
		std::string m_TextureGroupName;
		std::string m_ShaderName;
		TriState m_IsErrorMaterial;
		CAction * m_MatchAction;
	};

	class CAfxBaseFxStreamContext
		: public IAfxStreamContext
	{
	public:
		CAfxBaseFxStreamContext()
			: m_CurrentAction(0)
		{
		}

		~CAfxBaseFxStreamContext()
		{
		}

		CAfxBaseFxStream * GetStream()
		{
			return m_Stream;
		}

		IAfxMatRenderContext * GetCtx()
		{
			return m_Ctx;
		}

		bool DrawingSkyBoxView_get(void)
		{
			return m_DrawingSkyBoxView;
		}

		void RenderBegin(CAfxBaseFxStream * stream, const AfxViewportData_t & viewport);

		void RenderEnd(void);

		//
		// IAfxStreamContext:

		virtual bool ViewRenderShouldForceNoVis(bool orgValue);

		virtual void DrawingHudBegin(void);

		virtual void DrawingHudEnd(void);

		virtual void DrawingSkyBoxViewBegin(void);

		virtual void DrawingSkyBoxViewEnd(void);

		//virtual void Viewport(int x, int y, int width, int height);

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(SOURCESDK::IMaterial_csgo * material, void * proxyData);

		virtual void DrawInstances(int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance);

		virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0);

		virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists);

		virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0);

		virtual void QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor);

#if AFX_SHADERS_CSGO
		virtual void SetVertexShader(CAfx_csgo_ShaderState & state);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);
#endif

	private:
		bool m_DrawingHud;
		bool m_DrawingSkyBoxView;
		SOURCESDK::CSGO::CBaseHandle m_CurrentEntityHandle;

		class CQueueBeginFunctor
			: public CAfxFunctor
		{
		public:
			CQueueBeginFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()();

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CQueueEndFunctor
			: public CAfxFunctor
		{
		public:
			CQueueEndFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()();

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CDrawingHudBeginFunctor
			: public CAfxFunctor
		{
		public:
			CDrawingHudBeginFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->DrawingHudBegin();
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CDrawingHudEndFunctor
			: public CAfxFunctor
		{
		public:
			CDrawingHudEndFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->DrawingHudEnd();
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CDrawingSkyBoxViewBeginFunctor
			: public CAfxFunctor
		{
		public:
			CDrawingSkyBoxViewBeginFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->DrawingSkyBoxViewBegin();
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CDrawingSkyBoxViewEndFunctor
			: public CAfxFunctor
		{
		public:
			CDrawingSkyBoxViewEndFunctor(CAfxBaseFxStreamContext * streamContext)
				: m_StreamContext(streamContext)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->DrawingSkyBoxViewEnd();
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
		};

		class CUpdateCurrentEnitityHandleFunctor
			: public CAfxFunctor
		{
		public:
			CUpdateCurrentEnitityHandleFunctor(CAfxBaseFxStreamContext * streamContext, SOURCESDK::CSGO::CBaseHandle handle)
				: m_StreamContext(streamContext)
				, m_Handle(handle)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->UpdateCurrentEntityHandle(m_Handle);
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
			SOURCESDK::CSGO::CBaseHandle m_Handle;
		};

		CAfxBaseFxStream * m_Stream;
		IAfxMatRenderContext * m_Ctx = 0;
		CAfxBaseFxStreamContext * m_ChildContext;
		bool m_IsRootCtx;
		CAfxBaseFxStream::CAction * m_CurrentAction;
		//std::map<void *, SOURCESDK::CSGO::CBaseHandle> m_ProxyDataToEntityHandle;
		AfxViewportData_t m_Viewport;
		bool m_IsNextDepth;

		void QueueBegin();
		void QueueEnd();

		void BindAction(CAction * action)
		{
			if (m_CurrentAction)
			{
				m_CurrentAction->AfxUnbind(this);
				m_CurrentAction->Release();
			}
			if (action)
			{
				action->AddRef();
			}
			m_CurrentAction = action;
		}

		bool IfRootThenUpdateCurrentEntityHandle();

		void UpdateCurrentEntityHandle(SOURCESDK::CSGO::CBaseHandle handle);
	};

	CAfxBaseFxStreamContext * m_ActiveStreamContext;

	bool m_DebugPrint;
	struct CCacheEntry
	{
		std::map<SOURCESDK::CSGO::CBaseHandle, CAction *> EntityActions;
		CAction * DefaultAction;

		CCacheEntry()
			: DefaultAction(0)
		{
		}
	};
	std::map<CAfxTrackedMaterial *, CCacheEntry> m_Map;
	std::shared_timed_mutex m_MapMutex;

	class CMapRleaseNotification : public IAfxMaterialFree
	{
	public:
		CMapRleaseNotification(CAfxBaseFxStream * stream)
			: m_Stream(stream)
		{
		}

		~CMapRleaseNotification()
		{
			std::unique_lock<std::shared_timed_mutex> unique_lock(m_Stream->m_MapMutex);

			std::map<CAfxTrackedMaterial *, CCacheEntry> & map = m_Stream->m_Map;

			for (std::map<CAfxTrackedMaterial *, CCacheEntry>::iterator it = map.begin(); it != map.end(); ++it)
			{
				it->first->RemoveNotifyee(this);
			}
		}

		virtual void AfxMaterialFree(CAfxTrackedMaterial * trackedMaterial)
		{
			std::unique_lock<std::shared_timed_mutex> unique_lock(m_Stream->m_MapMutex);

			std::map<CAfxTrackedMaterial *, CCacheEntry>::iterator it = m_Stream->m_Map.find(trackedMaterial);

			if (it != m_Stream->m_Map.end())
			{

				for (std::map<SOURCESDK::CSGO::CBaseHandle, CAction *>::iterator itIt = it->second.EntityActions.begin(); itIt != it->second.EntityActions.end(); ++itIt)
				{
					itIt->second->Release();
				}

				it->second.DefaultAction->Release();

				m_Stream->m_Map.erase(it);
			}
		}

	private:
		CAfxBaseFxStream * m_Stream;

	} * m_MapRleaseNotification;

	class CPickerEntValue : public IAfxMaterialFree
	{
	public:
		int Index;
		std::set<CAfxTrackedMaterial *> Materials;

		virtual void AfxMaterialFree(CAfxTrackedMaterial * trackedMaterial)
		{
			std::unique_lock<std::shared_timed_mutex> unique_lock(m_Stream->m_PickerMutex);

			Materials.erase(trackedMaterial);
		}

		CPickerEntValue(CAfxBaseFxStream * stream, int index, CAfxTrackedMaterial * trackedMaterial)
			: m_Stream(stream)
			, Index(index)
		{
			Materials.insert(trackedMaterial);

			trackedMaterial->AddNotifyee(this);
		}

		~CPickerEntValue()
		{
			for (std::set<CAfxTrackedMaterial *>::iterator it = Materials.begin(); it != Materials.end(); ++it)
			{
				(*it)->RemoveNotifyee(this);
			}
		}

	private:
		CAfxBaseFxStream * m_Stream;
	};

	std::list<CActionFilterValue> m_ActionFilter;

	struct CPickerMatValue
	{
		int Index;
		std::set<int> Entities;

		CPickerMatValue(int index, int entity)
		{
			Index = index;
			Entities.insert(entity);
		}
	};
	std::map<CAfxTrackedMaterial *, CPickerMatValue> m_PickerMaterials;
	bool m_PickingMaterials;
	bool m_PickerMaterialsAlerted;

	class CPickerMaterialsRleaseNotification : public IAfxMaterialFree
	{
	public: 
		CPickerMaterialsRleaseNotification(CAfxBaseFxStream * stream)
			: m_Stream(stream)
		{
		}

		~CPickerMaterialsRleaseNotification()
		{
			std::unique_lock<std::shared_timed_mutex> unique_lock(m_Stream->m_PickerMutex);

			std::map<CAfxTrackedMaterial *, CPickerMatValue> & pickerMaterials = m_Stream->m_PickerMaterials;

			for (std::map<CAfxTrackedMaterial *, CPickerMatValue>::iterator it = pickerMaterials.begin(); it != pickerMaterials.end(); ++it)
			{
				it->first->RemoveNotifyee(this);
			}
		}

		virtual void AfxMaterialFree(CAfxTrackedMaterial * material)
		{
			std::unique_lock<std::shared_timed_mutex> unique_lock(m_Stream->m_PickerMutex);

			m_Stream->m_PickerMaterials.erase(material);
		}

	private:
		CAfxBaseFxStream * m_Stream;

	} * m_PickerMaterialsRleaseNotification;

	std::map<int, CPickerEntValue> m_PickerEntities;
	bool m_PickingEntities;
	bool m_PickerEntitiesAlerted;

	std::atomic_bool m_PickerActive = false;
	bool m_PickerCollecting;
	std::shared_timed_mutex m_PickerMutex;

	CAction * CAfxBaseFxStream::GetAction(CAfxTrackedMaterial * trackedMaterial);
	CAction * CAfxBaseFxStream::GetAction(CAfxTrackedMaterial * trackedMaterial, CAction * action);

	/*
	void ConvertDepthAction(CAction * & action, bool to24);
	*/

	bool Picker_GetHidden(SOURCESDK::CSGO::CBaseHandle const & entityHandle, CAfxTrackedMaterial * material);
};

class CAfxDepthStream
: public CAfxBaseFxStream
{
public:
	CAfxDepthStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DepthAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DepthAction_get());
		SetAction(m_StatTrakAction, m_Shared.DepthAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DepthAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());

		DrawHud_set(DT_NoDraw);
	}

protected:
	virtual ~CAfxDepthStream() {}
};

class CAfxZDepthStream
	: public CAfxBaseFxStream
{
public:
	CAfxZDepthStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);
		DrawHud_set(DT_Draw);
		DrawDepth_set(EDrawDepth_Gray);
	}
};

class CAfxMatteWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxMatteWorldStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction,  m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());

		DrawHud_set(DT_NoDraw);
	}

protected:
	virtual ~CAfxMatteWorldStream() {}
};

class CAfxDepthWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxDepthWorldStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());

		DrawHud_set(DT_NoDraw);
	}

protected:
	virtual ~CAfxDepthWorldStream() {}
};

class CAfxZDepthWorldStream
	: public CAfxMatteWorldStream
{
public:
	CAfxZDepthWorldStream() : CAfxMatteWorldStream()
	{
		ForceBuildingCubemaps_set(true);
		DrawHud_set(DT_Draw);
		DrawDepth_set(EDrawDepth_Gray);
	}
};


class CAfxMatteEntityStream
: public CAfxBaseFxStream
{
public:
	CAfxMatteEntityStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.MaskAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.DrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.MaskAction_get());
		SetAction(m_OtherAction, m_Shared.NoDrawAction_get());

		DrawHud_set(DT_NoDraw);

		this->SmokeOverlayAlphaFactor_set(0.01f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawAction_get());
	}

protected:
	virtual ~CAfxMatteEntityStream() {}
};

class CAfxDepthEntityStream
: public CAfxDepthStream
{
};

class CAfxZDepthEntityStream
	: public CAfxZDepthStream
{
public:
	CAfxZDepthEntityStream() : CAfxZDepthStream()
	{
	}
};

class CAfxAlphaMatteStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaMatteStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_StatTrakAction, m_Shared.WhiteAction_get());
		SetAction(m_ShellModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_OtherModelsAction, m_Shared.BlackAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.WhiteAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.BlackAction_get());
		SetAction(m_OtherAction, m_Shared.BlackAction_get());

		DrawHud_set(DT_NoDraw);

		this->SmokeOverlayAlphaFactor_set(0.01f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawAction_get());
	}

protected:
	virtual ~CAfxAlphaMatteStream() {}
};

class CAfxAlphaEntityStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaEntityStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction, m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.DrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_WriteZAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());

		DrawHud_set(DT_NoDraw);
	}

protected:
	virtual ~CAfxAlphaEntityStream() {}
};

class CAfxAlphaWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaWorldStream() : CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction, m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());

		DrawHud_set(DT_NoDraw);
	}

protected:
	virtual ~CAfxAlphaWorldStream() {}
};

class CAfxHudWhiteStream
	: public CAfxBaseFxStream
{
public:
	CAfxHudWhiteStream()
		: CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherAction, m_Shared.NoDrawAction_get());

		DrawHud_set(DT_NoChange);
		ClearBeforeHud_set(EClearBeforeHud_White);
	}
};

class CAfxHudBlackStream
	: public CAfxBaseFxStream
{
public:
	CAfxHudBlackStream()
		: CAfxBaseFxStream()
	{
		ForceBuildingCubemaps_set(true);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherAction, m_Shared.NoDrawAction_get());

		DrawHud_set(DT_NoChange);
		ClearBeforeHud_set(EClearBeforeHud_Black);
	}
};

class CAfxFileTracker
{
public:
	void TrackFile(char const * filePath);

	void WaitForFiles(unsigned int maxUnfinishedFiles);

private:
	std::queue<std::string> m_FilePaths;

};

class CAfxStreams
: public IAfxBaseClientDllView_Render
{
public:
	typedef SOURCESDK::IMatRenderContext_csgo CMatQueuedRenderContext_csgo;

	CAfxImageBufferPool ImageBufferPool;

	bool m_FormatBmpAndNotTga;

	CAfxStreams();
	~CAfxStreams();

	void ShutDown(void);

	/// <summary>Carry out initalization that cannot be done in DllMain</summary>
	static void AfxStreamsInit(void);

	static void MainThreadInitialize(void)
	{
		CAfxBaseFxStream::MainThreadInitialize();
	}

	void OnMaterialSystem(SOURCESDK::IMaterialSystem_csgo * value);
	void OnAfxBaseClientDll(IAfxBaseClientDll * value);
	void OnShaderShadow(SOURCESDK::IShaderShadow_csgo * value);

#if AFX_SHADERS_CSGO
	/// <remarks>This function can be called from diffrent threads, but only one thread at a time.</remarks>
	void OnSetVertexShader(CAfx_csgo_ShaderState & state);

	/// <remarks>This function can be called from diffrent threads, but only one thread at a time.</remarks>
	void OnSetPixelShader(CAfx_csgo_ShaderState & state);
#endif

	//void OnRender(CCSViewRender_Render_t fn, void * this_ptr, const SOURCESDK::vrect_t_csgo * rect);
	void OnRenderView(CCSViewRender_RenderView_t fn, void * this_ptr, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	bool OnViewRenderShouldForceNoVis(bool orgValue);

	void OnDrawingHudBegin(void);

	void OnDrawingHudEnd(void);

	void OnDrawingSkyBoxViewBegin(void);

	void OnDrawingSkyBoxViewEnd(void);

	void Console_RecordName_set(const char * value);
	const char * Console_RecordName_get();

	void Console_PresentRecordOnScreen_set(bool value);
	bool Console_PresentRecordOnScreen_get();

	void Console_StartMovieWav_set(bool value);
	bool Console_StartMovieWav_get();	

	void Console_RecordVoices_set(bool value);
	bool Console_RecordVoices_get();

	void Console_MatPostprocessEnable_set(int value);
	int Console_MatPostprocessEnable_get();

	void Console_MatDynamicToneMapping_set(int value);
	int Console_MatDynamicToneMapping_get();

	void Console_MatMotionBlurEnabled_set(int value);
	int Console_MatMotionBlurEnabled_get();

	void Console_MatForceTonemapScale_set(float value);
	float Console_MatForceTonemapScale_get();

	void Console_RecordFormat_set(const char * value);
	const char * Console_RecordFormat_get();

	void Console_PreviewSuspend_set(bool value);
	bool Console_PreviewSuspend_get();

	void Console_Record_Start();
	void Console_Record_End();
	void Console_AddStream(const char * streamName);
	void Console_AddBaseFxStream(const char * streamName);
	void Console_AddDepthStream(const char * streamName, bool tryZDepth);
	void Console_AddMatteWorldStream(const char * streamName);
	void Console_AddDepthWorldStream(const char * streamName, bool tryZDepth);
	void Console_AddMatteEntityStream(const char * streamName);
	void Console_AddDepthEntityStream(const char * streamName, bool tryZDepth);
	void Console_AddAlphaMatteStream(const char * streamName);
	void Console_AddAlphaEntityStream(const char * streamName);
	void Console_AddAlphaWorldStream(const char * streamName);
	void Console_AddAlphaMatteEntityStream(const char * streamName);
	void Console_AddHudStream(const char * streamName);
	void Console_AddHudWhiteStream(const char * streamName);
	void Console_AddHudBlackStream(const char * streamName);
	void Console_PrintStreams();
	void Console_MoveStream(IWrpCommandArgs * args);
	void Console_RemoveStream(const char * streamName);
	void Console_EditStream(const char * streamName, IWrpCommandArgs * args);
	void Console_ListActions(void);
	void Console_Bvh(IWrpCommandArgs * args);

	bool CamExport_get(void) { return m_CamExport;  }
	void CamExport_set(bool value) { m_CamExport = value;  }

	CamExport::ScaleFov CamExportScaleFov_get(void) { return m_CamExportScaleFov;  }
	void CamExportScaleFov_set(CamExport::ScaleFov value) { m_CamExportScaleFov = value;  }

	void Console_GameRecording(IWrpCommandArgs * args);

	/// <param name="streamName">stream name to preview or empty string if to preview nothing.</param>
	/// <param name="slot">-1 means all slots if streamName is emtpy.</param>
	void Console_PreviewStream(const char * streamName, int slot);

	bool Console_ToAfxAction(char const * value, CAfxBaseFxStream::CAction * & action);
	char const * Console_FromAfxAction(CAfxBaseFxStream::CAction * action);

	void DebugDump(IAfxMatRenderContextOrg * ctxp);

	virtual SOURCESDK::IMaterialSystem_csgo * GetMaterialSystem(void);
	virtual SOURCESDK::IShaderShadow_csgo * GetShaderShadow(void);

	virtual std::wstring GetTakeDir(void);

	void LevelInitPostEntity(void);
	void LevelShutdown(void);

	virtual void View_Render(IAfxBaseClientDll * cl, SOURCESDK::vrect_t_csgo *rect);

private:
	class CEntityBvhCapture
	{
	public:
		enum Origin_e {
			O_Net,
			O_View
		};

		enum Angles_e {
			A_Net,
			A_View
		};

		CEntityBvhCapture(int entityIndex, Origin_e origin, Angles_e angles);
		~CEntityBvhCapture();

		void StartCapture(std::wstring const & takePath, double frameTime);
		void EndCapture(void);

		void CaptureFrame(void);

		int EntityIndex_get(void) { return m_EntityIndex; }
		Origin_e Origin_get(void) { return m_Origin; }
		Angles_e Angles_get(void) { return m_Angles; }

	private:
		Origin_e m_Origin;
		Angles_e m_Angles;
		int m_EntityIndex;
		BvhExport * m_BvhExport;
	};


	class CSleepFunctor
		: public CAfxFunctor
	{
	public:
		CSleepFunctor(DWORD sleep)
			: m_Sleep(sleep)
		{
		}

		virtual void operator()()
		{
			Sleep(m_Sleep);
		}

	private:
		DWORD m_Sleep;
	};

	std::string m_RecordName;
	bool m_FirstRenderAfterLevelInit = true;
	bool m_FirstStreamToBeRendered;
	bool m_SuspendPreview = false;
	bool m_PresentRecordOnScreen;
	bool m_StartMovieWav;
	bool m_StartMovieWavUsed;

	bool m_RecordVoices;
	bool m_RecordVoicesUsed;

	bool m_LastPreviewWithNoHud;

	SOURCESDK::IMaterialSystem_csgo * m_MaterialSystem;
	IAfxBaseClientDll * m_AfxBaseClientDll;
	SOURCESDK::IShaderShadow_csgo * m_ShaderShadow;
	std::list<CAfxRecordStream *> m_Streams;
	CAfxRecordStream * m_PreviewStreams[16] = { };
	bool m_Recording;
	int m_Frame;
	bool m_CamBvh;
	std::list<CEntityBvhCapture *> m_EntityBvhCaptures;
	bool m_CamExport = false;
	CamExport::ScaleFov m_CamExportScaleFov = CamExport::SF_None;
	CamExport * m_CamExportObj = 0;
	bool m_GameRecording;

	WrpConVarRef * m_HostFrameRate = nullptr;

	WrpConVarRef * m_MatPostProcessEnableRef = nullptr;
	int m_OldMatPostProcessEnable;
	int m_NewMatPostProcessEnable = -1;

	WrpConVarRef * m_MatDynamicTonemappingRef = nullptr;
	int m_OldMatDynamicTonemapping;
	int m_NewMatDynamicTonemapping = -1;

	WrpConVarRef * m_MatMotionBlurEnabledRef = nullptr;
	int m_OldMatMotionBlurEnabled;
	int m_NewMatMotionBlurEnabled = -1;

	WrpConVarRef * m_MatForceTonemapScale = nullptr;
	float m_OldMatForceTonemapScale;
	float m_NewMatForceTonemapScale = -1;

	WrpConVarRef * m_SndMuteLosefocus = nullptr;
	int m_OldSndMuteLosefocus;

	WrpConVarRef * m_BuildingCubemaps = nullptr;
	int m_OldBuildingCubemaps;

	WrpConVarRef * m_PanoramaDisableLayerCache = nullptr;
	int m_OldPanoramaDisableLayerCache;

	std::wstring m_TakeDir;
	//ITexture_csgo * m_RgbaRenderTarget;
	SOURCESDK::ITexture_csgo * m_RenderTargetDepthF;
	//CAfxMaterial * m_ShowzMaterial;
	DWORD m_Current_View_Render_ThreadId;
	bool m_PresentBlocked = false;
	bool m_ShutDown = false;

	bool m_BlockHud = false;

	void SetCurrent_View_Render_ThreadId(DWORD id);

	DWORD GetCurrent_View_Render_ThreadId();

	void OnAfxBaseClientDll_Free(void);

	void Console_EditStream(CAfxStream * stream, IWrpCommandArgs * args);

	bool Console_EditStream(CAfxRenderViewStream * stream, IWrpCommandArgs * args);

	bool Console_CheckStreamName(char const * value);

	bool Console_ToStreamCombineType(char const * value, CAfxTwinStream::StreamCombineType & streamCombineType);
	char const * Console_FromStreamCombineType(CAfxTwinStream::StreamCombineType streamCombineType);

	bool Console_ToStreamCaptureType(char const * value, CAfxRenderViewStream::StreamCaptureType & StreamCaptureType);
	char const * Console_FromStreamCaptureType(CAfxRenderViewStream::StreamCaptureType StreamCaptureType);

	bool CheckCanFeedStreams(void);

	void BackUpMatVars();
	void SetMatVarsForStreams();
	void RestoreMatVars();
	void EnsureMatVars();

	void AddStream(CAfxRecordStream * stream);

	void CreateRenderTargets(SOURCESDK::IMaterialSystem_csgo * materialSystem);

	IAfxMatRenderContextOrg * CaptureStream(IAfxMatRenderContextOrg * ctxp, CAfxRecordStream * stream, CCSViewRender_RenderView_t fn, void * this_ptr, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);
	IAfxMatRenderContextOrg * CaptureStreamToBuffer(IAfxMatRenderContextOrg * ctxp, CAfxRenderViewStream * stream, CAfxRecordStream * captureTarget, bool first, bool last, CCSViewRender_RenderView_t fn, void * this_ptr, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	IAfxMatRenderContextOrg * PreviewStream(IAfxMatRenderContextOrg * ctxp, CAfxRenderViewStream * previewStream, bool isLast, int slot, int cols, bool & hudDrawn, CCSViewRender_RenderView_t fn, void * this_ptr, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	IAfxStreamContext * FindStreamContext(IAfxMatRenderContext * ctx);

	void BlockPresent(IAfxMatRenderContextOrg * ctx, bool value);
};
