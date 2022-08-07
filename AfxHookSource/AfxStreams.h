#pragma once

#define AFXSTREAMS_REFTRACKER 0

#include "SourceInterfaces.h"
#include "AfxInterfaces.h"
#include "AfxClasses.h"
#include "WrpConsole.h"
#include "AfxShaders.h"
#include "csgo_Stdshader_dx9_Hooks.h"
#include "csgo_Stdshader_dx9_Hooks.h"
#include "CamIO.h"
#include "MatRenderContextHook.h"
#include "AfxWriteFileLimiter.h"
#include "AfxThreadedRefCounted.h"
#include "MirvCalcs.h"
#include "d3d9Hooks.h"

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

#include <shared/AfxImageBuffer.h>
#include <shared/AfxOutStreams.h>
#include <shared/bvhexport.h>
#include <shared/AfxColorLut.h>

#include <cctype>

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
#include <memory>

//typedef void(__fastcall * CCSViewRender_Render_t)(void * This, void* Edx, const SOURCESDK::vrect_t_csgo * rect);
typedef void(__fastcall* CCSViewRender_RenderView_t)(void * This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw);

class CAfxStreams;

extern CAfxStreams g_AfxStreams;

class IAfxBasefxStreamModifier;

struct CEntityMeta
{
	SOURCESDK::IHandleEntity_csgo * EntityPtr;
	SOURCESDK::CSGO::CBaseHandle Handle;
	std::string ClassName;
	std::string ModelName;
	bool IsPlayer;
	int TeamNumber;

	CEntityMeta(SOURCESDK::IHandleEntity_csgo * entityPtr)
		: EntityPtr(entityPtr)
		, Handle(SOURCESDK_CSGO_INVALID_EHANDLE_INDEX)
		, ClassName("")
		, ModelName("")
		, IsPlayer(false)
		, TeamNumber(0)
	{

	}

	CEntityMeta(const CEntityMeta& other)
		: EntityPtr(other.EntityPtr)
		, Handle(other.Handle)
		, ClassName(other.ClassName)
		, ModelName(other.ModelName)
		, IsPlayer(other.IsPlayer)
		, TeamNumber(other.TeamNumber)
	{
	}

	CEntityMeta& operator=(const CEntityMeta& x) {
		EntityPtr = x.EntityPtr;
		Handle = x.Handle;
		ClassName = x.ClassName;
		ModelName = x.ModelName;
		IsPlayer = x.IsPlayer;
		TeamNumber = x.TeamNumber;
		return *this;
	}

	bool operator==(const CEntityMeta& other) const
	{
		return EntityPtr == other.EntityPtr;
	}

	bool operator<(const CEntityMeta& other) const
	{
		return EntityPtr < other.EntityPtr;
	}
};

class CEntityMetaRef
{
public:
	CEntityMetaRef() {

	}

	CEntityMetaRef(std::unique_ptr<CEntityMeta> &&entityMeta) : m_SharedPtr(std::move(entityMeta)) {

	}

	CEntityMetaRef& operator=(const CEntityMetaRef& x) {
		m_SharedPtr = x.m_SharedPtr;
		return *this;
	}

	CEntityMetaRef& operator=(std::unique_ptr<CEntityMeta> &&entityMeta) {
		m_SharedPtr = std::move(entityMeta);
		return *this;
	}

	explicit operator bool() const noexcept {
		return m_SharedPtr.operator bool();
	}

	bool operator!=(const CEntityMetaRef& other) const
	{
		if(nullptr == m_SharedPtr) return nullptr != other.m_SharedPtr;
		if(nullptr == other.m_SharedPtr) return true;

		return !(*m_SharedPtr == *(other.m_SharedPtr));
	}

	bool operator==(const CEntityMetaRef& other) const
	{
		if(nullptr == m_SharedPtr) return nullptr == other.m_SharedPtr;
		if(nullptr == other.m_SharedPtr) return false;

		return *m_SharedPtr == *(other.m_SharedPtr);
	}

	bool operator<(const CEntityMetaRef& other) const
	{
		if(nullptr == m_SharedPtr) return nullptr != other.m_SharedPtr;
		if(nullptr == other.m_SharedPtr) return false;

		return *m_SharedPtr < *(other.m_SharedPtr);
	}

	CEntityMeta& operator*() const noexcept{
		return m_SharedPtr.operator*();
	}

	CEntityMeta * operator->() const noexcept{
		return m_SharedPtr.operator->();
	}

private:
	std::shared_ptr<CEntityMeta> m_SharedPtr;
};

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

	virtual void Set_In_CModelRenderSystem_SetupBones(bool value) = 0;
	
	virtual bool Get_In_CModelRenderSystem_SetupBones(void)  = 0;

	virtual SOURCESDK::IMaterial_csgo * MaterialHook(IAfxMatRenderContext* ctx, SOURCESDK::IMaterial_csgo * material, void * proxyData) = 0;

	virtual void OnLockRenderData(IAfxMatRenderContext* ctx, int nSizeInBytes, void * ptr) = 0;

	virtual void DrawInstances(IAfxMatRenderContext* ctx, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance) = 0;

	virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0) = 0;

	virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists) = 0;

	virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0) = 0;

	virtual void UnlockMesh(IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc) = 0;

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

class CAfxRecordStream;

class CAfxStreamShared : public CAfxThreadedRefCounted
{
public:
	//
	// Reference counting:

	virtual int AddRef(bool keepLocked = false) override
	{
		AFXSTREAMS_REFTRACKER_INC

		return CAfxThreadedRefCounted::AddRef(keepLocked);
	}

	virtual int Release(bool isLocked = false) override
	{
		int result = CAfxThreadedRefCounted::Release(isLocked);

		AFXSTREAMS_REFTRACKER_DEC

		return result;
	}

private:
	std::mutex m_RefMutex;
	int m_RefCount = 0;
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


template<typename T> class CAfxOverrideable
{
public:
	CAfxOverrideable()
		: m_Override(false)
	{

	}

	CAfxOverrideable(T value)
		: m_Override(true)
		, m_Value(value)
	{
	}

	CAfxOverrideable & operator= (const CAfxOverrideable & x)
	{
		m_Override = x.m_Override;
		m_Value = x.m_Value;

		return *this;
	}

	CAfxOverrideable & operator= (const T value)
	{
		m_Override = true;
		m_Value = value;

		return *this;
	}

	CAfxOverrideable & AssignNoOverride()
	{
		m_Override = false;

		return *this;
	}

	bool GetOverride() const
	{
		return m_Override;
	}

	bool Get(T & outValue) const
	{
		if (m_Override)
		{
			outValue = m_Value;
			return true;
		}

		return false;
	}

private:
	bool m_Override;
	T m_Value;
};

typedef CAfxOverrideable<bool> CAfxBoolOverrideable;

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
		SCT_Invalid,
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

	virtual void SetActive(bool value) {}

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

	StreamCaptureType StreamCaptureType_get(void) const;
	void StreamCaptureType_set(StreamCaptureType value);

	bool ForceBuildingCubemaps_get(void)
	{
		return m_ForceBuildingCubemaps;
	}

	void ForceBuildingCubemaps_set(bool value)
	{
		m_ForceBuildingCubemaps = value;
	}

	CAfxBoolOverrideable DoBloomAndToneMapping;
	CAfxBoolOverrideable DoDepthOfField;
	CAfxBoolOverrideable DrawWorldNormal;
	CAfxBoolOverrideable CullFrontFaces;
	CAfxBoolOverrideable RenderFlashlightDepthTranslucents;

	//bool GetDisableFastPath()
	//{
	//	return m_DisableFastPath;
	//}

	//void SetDisableFastPath(bool value)
	//{
	//	m_DisableFastPath = value;
	//}

	//void Console_DisableFastPathRequired();

	virtual void OnEntityDeleted(SOURCESDK::IHandleEntity_csgo *) {

	}

	virtual void LevelShutdown(void)
	{
	}

	void QueueCapture(IAfxMatRenderContextOrg * ctx, CAfxRecordStream * captureTarget, size_t streamIndex, int x, int y, int width, int height);

	//
	// State information:

	virtual void OnRenderBegin(IAfxBasefxStreamModifier * modifier, const AfxViewportData_t & viewport, const SOURCESDK::VMatrix & projectionMatrix, const SOURCESDK::VMatrix & projectionMatrixSky)
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
	//bool m_DisableFastPath = false;

	virtual ~CAfxRenderViewStream();

private:
	class CCaptureFunctor :
		public CAfxFunctor
	{
	public:
		CCaptureFunctor(CAfxRenderViewStream & stream, CAfxRecordStream * captureTarget, size_t streamIndex, int x, int y, int width, int height);

		void operator()();

	private:
		CAfxRenderViewStream & m_Stream;
		CAfxRecordStream * m_CaptureTarget;
		size_t m_StreamIndex;
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

	void Capture(CAfxRecordStream * captureTarget, size_t streamIndex, int x, int y, int width, int height);
};

class CAfxSingleStream;
class CAfxTwinStream;

class CAfxRecordingSettings : public CAfxThreadedRefCounted
{
public:
	static CAfxRecordingSettings * GetDefault()
	{
		return m_Shared.m_DefaultSettings;
	}

	static CAfxRecordingSettings * GetByName(const char * name)
	{
		auto it = m_Shared.m_NamedSettings.find(CNamedSettingKey(name));

		if (m_Shared.m_NamedSettings.end() != it)
			return it->second.Settings;
		else
			return nullptr;
	}

	static void Console(IWrpCommandArgs * args);

	CAfxRecordingSettings(const char * name, bool bProtected)
		: m_Name(name)
		, m_Protected(bProtected)
	{
	}

	const char * GetName() const
	{
		return m_Name.c_str();
	}

	bool GetProtected() const
	{
		return m_Protected;
	}

	virtual void Console_Edit(IWrpCommandArgs * args) = 0;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const = 0;

	virtual bool InheritsFrom(CAfxRecordingSettings * setting) const
	{
		if (setting == this) return true;

		return false;
	}

protected:
	std::string m_Name;
	bool m_Protected;

	struct CNamedSettingValue {
		CAfxRecordingSettings * Settings;

		CNamedSettingValue()
			: Settings(nullptr)
		{
		}

		CNamedSettingValue(CAfxRecordingSettings * settings)
			: Settings(settings)
		{
			Settings->AddRef();
		}

		CNamedSettingValue(const CNamedSettingValue & copyFrom)
			: Settings(copyFrom.Settings)
		{
			if (Settings) Settings->AddRef();
		}

		~CNamedSettingValue()
		{
			if(Settings) Settings->Release();
		}
	};

	class CNamedSettingKey
	{
	public:
		CNamedSettingKey(const char * value)
			: m_Value(value)
		{
			std::transform(m_Value.begin(), m_Value.end(), m_Value.begin(), [](unsigned char c) { return std::tolower(c); });
		}

		bool operator < (const CNamedSettingKey & y) const
		{
			return m_Value.compare(y.m_Value) < 0;
		}

	private:
		std::string m_Value;
	};

	static struct CShared {
		std::map<CNamedSettingKey, CNamedSettingValue> m_NamedSettings;
		CAfxRecordingSettings * m_DefaultSettings;

		CShared();
		~CShared();

		bool DeleteIfUnrefrenced(std::map<CNamedSettingKey, CNamedSettingValue>::iterator it)
		{
			if (it->second.Settings)
			{
				it->second.Settings->Lock();
				if (1 == it->second.Settings->GetRefCount())
				{
					it->second.Settings->Release(true);
					it->second.Settings = nullptr;
					m_NamedSettings.erase(it);
					return true;
				}
				else
				{
					it->second.Settings->Unlock();
					return false;
				}
			}

			m_NamedSettings.erase(it);
			return true;
		}
	} m_Shared;
};

class CAfxDefaultRecordingSettings : public CAfxRecordingSettings
{
public:
	CAfxDefaultRecordingSettings(CAfxRecordingSettings * useSettings)
		: CAfxRecordingSettings("afxDefault", true)
		, m_DefaultSettings(useSettings)
	{
		if (m_DefaultSettings) m_DefaultSettings->AddRef();
	}

	virtual void Console_Edit(IWrpCommandArgs * args) override;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const override
	{
		if (m_DefaultSettings)
			return m_DefaultSettings->CreateOutVideoStream(streams, stream, imageFormat, fps, pathSuffix);

		return nullptr;
	}

	virtual bool InheritsFrom(CAfxRecordingSettings * setting) const override
	{
		if (CAfxRecordingSettings::InheritsFrom(setting)) return true;

		if (m_DefaultSettings) if (m_DefaultSettings->InheritsFrom(setting)) return true;

		return false;
	}

protected:
	virtual ~CAfxDefaultRecordingSettings()
	{
		if (m_DefaultSettings)
		{
			m_DefaultSettings->Release();
			m_DefaultSettings = nullptr;
		}
	}
private:
	CAfxRecordingSettings * m_DefaultSettings;
};

class CAfxMultiRecordingSettings : public CAfxRecordingSettings
{
public:
	CAfxMultiRecordingSettings(const char * name, bool bProtected)
		: CAfxRecordingSettings(name, bProtected)
	{
	}

	virtual void Console_Edit(IWrpCommandArgs * args) override;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const override
	{
		std::list<advancedfx::COutVideoStream * > outVideoStreams;

		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CAfxRecordingSettings * setting = *it)
			{
				std::string mySuffix(pathSuffix);
				mySuffix.append("\\");
				mySuffix.append(setting->GetName());

				outVideoStreams.push_back(setting->CreateOutVideoStream(streams, stream, imageFormat, fps, mySuffix.c_str()));
			}
		}

		return new advancedfx::COutMultiVideoStream(imageFormat, std::move(outVideoStreams));
	}

	virtual bool InheritsFrom(CAfxRecordingSettings * setting) const override
	{
		if (CAfxRecordingSettings::InheritsFrom(setting)) return true;

		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CAfxRecordingSettings * itSetting = *it) if (itSetting->InheritsFrom(setting)) return true;
		}

		return false;
	}

protected:
	virtual ~CAfxMultiRecordingSettings() override
	{
		for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
		{
			if (CAfxRecordingSettings * setting = *it)
				setting->Release();
		}
	}
private:
	std::list<CAfxRecordingSettings *> m_Settings;
};

class CAfxClassicRecordingSettings : public CAfxRecordingSettings
{
public:
	CAfxClassicRecordingSettings()
		: CAfxRecordingSettings("afxClassic", true)
	{
	}

	virtual void Console_Edit(IWrpCommandArgs * args) override;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const override;
};

class CAfxFfmpegRecordingSettings : public CAfxRecordingSettings
{
public:
	CAfxFfmpegRecordingSettings(const char * name, bool bProtected, const char * szFfmpegOptions)
		: CAfxRecordingSettings(name, bProtected)
		, m_FfmpegOptions(szFfmpegOptions)
	{

	}

	virtual void Console_Edit(IWrpCommandArgs * args) override;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const override;

private:
	std::string m_FfmpegOptions;
};


class CAfxSamplingRecordingSettings : public CAfxRecordingSettings
{
public:
	CAfxSamplingRecordingSettings(const char * name, bool bProtected, CAfxRecordingSettings * outputSettings, EasySamplerSettings::Method method, float outFps, double exposure, float frameStrength)
		: CAfxRecordingSettings(name, bProtected)
		, m_OutputSettings(outputSettings)
		, m_Method(method)
		, m_OutFps(outFps)
		, m_Exposure(exposure)
		, m_FrameStrength(frameStrength)
	{
		if (m_OutputSettings) m_OutputSettings->AddRef();
	}

	virtual void Console_Edit(IWrpCommandArgs * args) override;

	virtual advancedfx::COutVideoStream * CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float fps, const char * pathSuffix) const override;

protected:
	virtual ~CAfxSamplingRecordingSettings()
	{
		if (m_OutputSettings)
		{
			m_OutputSettings->Release();
			m_OutputSettings = nullptr;
		}
	}
private:
	CAfxRecordingSettings * m_OutputSettings;
	EasySamplerSettings::Method m_Method;
	float m_OutFps;
	double m_Exposure;
	float m_FrameStrength;
};

class CAfxRecordStream abstract
: public CAfxStream
{
public:
	CAfxRecordStream(char const * streamName, std::vector<CAfxRenderViewStream *>&& streams);

	virtual CAfxRecordStream * AsAfxRecordStream(void) { return this; }

	virtual IAfxBasefxStreamModifier * GetBasefxStreamModifier(size_t streamIndex) const {
		return nullptr;
	}

	CAfxRecordingSettings * GetSettings() const
	{
		return m_Settings;
	}

	void SetSettings(CAfxRecordingSettings * settings)
	{
		settings->AddRef();

		m_Settings->Release();
		m_Settings = settings;
	}

	char const * StreamName_get(void) const;

	bool Record_get(void);
	void Record_set(bool value);

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordStart();

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordEnd();

	void QueueCaptureStart(IAfxMatRenderContextOrg * ctx);

	void QueueCaptureEnd(IAfxMatRenderContextOrg * ctx);

	/// <remarks>This is not guaranteed to be called, i.e. not called upon buffer re-allocation error.</remarks>
	void OnImageBufferCaptured(size_t index, advancedfx::CImageBuffer * buffer);

	virtual CAfxRenderViewStream::StreamCaptureType GetCaptureType() const = 0;

	size_t GetStreamCount() const {
		return m_Streams.size();
	}

	CAfxRenderViewStream * GetStream(size_t index) const
	{
		if (index < 0 || index > m_Streams.size()) return nullptr;
		
		return m_Streams[index];
	}

	void LevelShutdown(void)
	{
		for (size_t i = 0; i < m_Streams.size(); ++i)
		{
			m_Streams[i]->LevelShutdown();
		}
	}

	void SetActive(bool value) {
		for (size_t i = 0; i < m_Streams.size(); ++i)
		{
			m_Streams[i]->SetActive(value);
		}
	}

	virtual bool Console_Edit_Head(IWrpCommandArgs * args);
	virtual void Console_Edit_Tail(IWrpCommandArgs * args);

protected:
	std::vector<CAfxRenderViewStream *> m_Streams;

	std::vector<advancedfx::CImageBuffer *> m_Buffers;

	CAfxRecordingSettings * m_Settings;
	advancedfx::COutVideoStream * m_OutVideoStream;

	virtual ~CAfxRecordStream() override;

	virtual void CaptureStart(void)
	{

	}

	virtual void CaptureEnd();

private:
	class CCaptureStartFunctor
		: public CAfxFunctor
	{
	public:
		CCaptureStartFunctor(CAfxRecordStream & stream)
			: m_Stream(stream)
		{
			m_Stream.AddRef();
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
		CCaptureEndFunctor(CAfxRecordStream & stream)
			: m_Stream(stream)
		{
		}

		void operator()()
		{
			m_Stream.CaptureEnd();
			m_Stream.Release();
		}

	private:
		CAfxRecordStream & m_Stream;
		std::wstring m_OutPath;
		bool m_OutPathOk;
	};

	std::string m_StreamName;
	bool m_Record;
};

class CAfxSingleStream
	: public CAfxRecordStream
{
public:
	CAfxSingleStream(char const * streamName, CAfxRenderViewStream * stream);

	virtual CAfxRenderViewStream::StreamCaptureType GetCaptureType() const override;

	virtual bool Console_Edit_Head(IWrpCommandArgs * args) override;
	virtual void Console_Edit_Tail(IWrpCommandArgs * args) override;

protected:
	virtual void CaptureEnd() override;

private:

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

	StreamCombineType StreamCombineType_get(void);
	void StreamCombineType_set(StreamCombineType value);

	virtual CAfxRenderViewStream::StreamCaptureType GetCaptureType() const override;

	virtual bool Console_Edit_Head(IWrpCommandArgs * args) override;
	virtual void Console_Edit_Tail(IWrpCommandArgs * args) override;

protected:
	virtual void CaptureEnd() override;

private:
	StreamCombineType m_StreamCombineType;
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

		virtual void Console_Edit(IWrpCommandArgs* args);

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

		virtual CAction * ResolveAction(const CAfxTrackedMaterialRef& trackedMaterial)
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

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial);

		virtual void DrawInstances(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance )
		{
			ctx->GetOrg()->DrawInstances(nInstanceCount, pInstance);
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

		virtual void UnlockMesh(CAfxBaseFxStreamContext* ch, IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc)
		{
			am->GetParent()->UnlockMesh(numVerts, numIndices, desc);
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

		virtual CAction * SafeSubResolveAction(CAction * action, const CAfxTrackedMaterialRef& trackedMaterial)
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

	static void Console_AddGlowColorMapAction(IWrpCommandArgs* args)
	{
		m_Shared.Console_AddGlowColorMapAction(args);
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

	virtual void OnRenderBegin(IAfxBasefxStreamModifier * modifier, const AfxViewportData_t & viewport, const SOURCESDK::VMatrix & projectionMatrix, const SOURCESDK::VMatrix & projectionMatrixSky) override;

	virtual void OnRenderEnd(void) override;

	void Console_ActionFilter_Add(const char * expression, CAction * action);
	void Console_ActionFilter_AddEx(CAfxStreams * streams, IWrpCommandArgs * args);
	void Console_ActionFilter_Print(void);
	void Console_ActionFilter_Remove(int id);
	void Console_ActionFilter_Move(int id, int moveBeforeId);
	void Console_ActionFilter_Clear();

	virtual CAfxBaseFxStream * AsAfxBaseFxStream(void) { return this; }

	virtual void SetActive(bool value) override;

	virtual void OnEntityDeleted(SOURCESDK::IHandleEntity_csgo * entity) override;

	virtual void LevelShutdown(void) override;

	CAction * RetrieveAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity);

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
	void VguiAction_set(CAction *);

	bool GetClearBeforeRender(void) const
	{
		return m_ClearBeforeRender;
	}

	void SetClearBeforeRender(bool value)
	{
		m_ClearBeforeRender = value;
	}

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
		EDrawDepthMode_LogE,
		EDrawDepthMode_PyramidalLinear,
		EDrawDepthMode_PyramidalLogE
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

	void Picker_Pick(bool pickEntityNotMaterial, bool wasVisible);
	void Picker_Stop(void);
	void Picker_Print(void);

	void InvalidateMap();

	/*
	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	/// <pram name="depth24ZIP">if not to24, then set SCT_Normal otherwise: false: set capturetype SCT_Depth24; true: set capturetype SCT_Depth24ZIP</param>
	void ConvertStreamDepth(bool to24, bool depth24ZIP);

	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	void ConvertDepthActions(bool to24);
	*/

	bool ReShadeEnabled_get() {
		return m_ReShadeEnabled;
	}

	void ReShadeEnabled_set(bool value) {
		m_ReShadeEnabled = value;
	}

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
		void Console_AddGlowColorMapAction(IWrpCommandArgs* args);
		CAction * GetAction(CActionKey const & key);
		bool RemoveAction(CActionKey const & key);

		void MainThreadInitialize(void);

		void LevelShutdown(void);

		CAction * DrawAction_get(void);
		CAction * NoDrawAction_get(void);
		CAction * DrawMatteAction_get(void);
		CAction * NoDrawMatteAction_get(void);
		CAction * DepthAction_get(void);
		//CAction * Depth24Action_get(void);
		CAction * MaskAction_get(void);
		CAction * WhiteAction_get(void);
		CAction * BlackAction_get(void);
	
	private:
		int m_RefCount = 0;
		int m_MainThreadInitalizeLevel = 0;
		int m_ShutDownLevel = 0;
		std::map<CActionKey, CAction *> m_Actions;
		CAction * m_DrawAction = 0;
		CAction * m_DrawMatteAction = 0;
		CAction * m_NoDrawMatteAction = 0;
		CAction * m_NoDrawAction = 0;
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

	bool m_ReShadeEnabled = false;

	virtual ~CAfxBaseFxStream();

	void SetActionAndInvalidateMap(CAction * & target, CAction * src);
	void SetAction(CAction * & target, CAction * src);

private:

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

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch) override;

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial) override;

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

	class CActionZOnly
		: public CAction
	{
	public:
		CActionZOnly()
			: CAction()
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch) override;

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial) override;
	};

	class CActionDraw
	: public CAction
	{
	public:
		CActionDraw()
		: CAction()
		{
		}

		//virtual bool CheckIsCompatible(const CAfxTrackedMaterialRef& tackedMaterial)
		//{
		//	return true;
		//}
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
			m_ModulationColorBlendOverride.m_OverrideColor = true;
			m_ModulationColorBlendOverride.m_Color[0] = color[0];
			m_ModulationColorBlendOverride.m_Color[1] = color[1];
			m_ModulationColorBlendOverride.m_Color[2] = color[2];
		}

		void OverrideBlend(float blend)
		{
			m_ModulationColorBlendOverride.m_OverrideBlend = true;
			m_ModulationColorBlendOverride.m_Blend = blend;
		}

		void OverrideDepthWrite(bool depthWrite)
		{
			m_OverrideDepthWrite = true;
			m_DepthWrite = depthWrite;
		}

		void OverrideLigthScale(float const color[4])
		{
			m_LightScaleOverride.m_Override = true;
			m_LightScaleOverride.m_Value[0] = color[0];
			m_LightScaleOverride.m_Value[1] = color[1];
			m_LightScaleOverride.m_Value[2] = color[2];
			m_LightScaleOverride.m_Value[3] = color[3];
		}

		virtual CAction * ResolveAction(const CAfxTrackedMaterialRef& trackedMaterial);

		virtual void MainThreadInitialize(void);

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch) override;

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial) override;

 		virtual void DrawInstances(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance ) override
		{
			if(m_ModulationColorBlendOverride.m_OverrideColor || m_ModulationColorBlendOverride.m_OverrideBlend)
			{
				SOURCESDK::MeshInstanceData_t_csgo * first = const_cast<SOURCESDK::MeshInstanceData_t_csgo *>(pInstance);

				for(int i = 0; i < nInstanceCount; ++i)
				{
					if(m_ModulationColorBlendOverride.m_OverrideColor)
					{
						first->m_DiffuseModulation.x = m_ModulationColorBlendOverride.m_Color[0];
						first->m_DiffuseModulation.y = m_ModulationColorBlendOverride.m_Color[1];
						first->m_DiffuseModulation.z = m_ModulationColorBlendOverride.m_Color[2];
					}
					if(m_ModulationColorBlendOverride.m_OverrideBlend)
					{
						first->m_DiffuseModulation.w = m_ModulationColorBlendOverride.m_Blend;
					}
 
					++first;
				}
			}
 
			ctx->GetOrg()->DrawInstances(nInstanceCount, pInstance); 
		}

	protected:
		virtual ~CActionReplace();

	private:
		class CModulationColorBlendOverride : public ID3d9HooksFloat4ParamOverride
		{
		public:
			bool m_OverrideColor = false;
			float m_Color[3] = { 0.0f, 0.0f, 0.0f };
			bool m_OverrideBlend = false;
			float m_Blend = 0.0f;

			virtual void D3d9HooksFloat4ParamOverride(float value[4]) override
			{
				if (m_OverrideColor) {
					value[0] = m_Color[0]; value[1] = m_Color[1]; value[2] = m_Color[2];
				}

				if (m_OverrideBlend) {
					value[3] = m_Blend;
				}
			}
		private:
		} m_ModulationColorBlendOverride;

		class CClightScaleOverride : public ID3d9HooksFloat4ParamOverride
		{
		public:
			bool m_Override = false;
			float m_Value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			virtual void D3d9HooksFloat4ParamOverride(float value[4]) override
			{
				if (m_Override)
				{
					value[0] = m_Value[0];
					value[1] = m_Value[1];
					value[2] = m_Value[2];
					value[3] = m_Value[3];
				}
			}
		private:
		} m_LightScaleOverride;

		CAfxOwnedMaterial * m_Material;
		std::string m_MaterialName;
		CAction * m_FallBackAction;
		bool m_OverrideDepthWrite;
		bool m_DepthWrite;
		CAfxTrackedMaterial * m_TrackedMaterial;

		void ExamineMaterial(SOURCESDK::IMaterial_csgo *, bool & outSplinetype, bool & outUseinstancing);
	};

	// Todo: Speed up with cubetrees.
	class CActionGlowColorMap
		: public CAction, public ID3d9HooksFloat4ParamOverride
	{
	public:
		virtual void Console_Edit(IWrpCommandArgs* args) override;

		virtual void AfxUnbind(CAfxBaseFxStreamContext* ch) override;

		virtual void MaterialHook(CAfxBaseFxStreamContext* ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial) override;

		virtual void UnlockMesh(CAfxBaseFxStreamContext* ch, IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc) override;

		virtual void D3d9HooksFloat4ParamOverride(float color[4]) override
		{
			if(IDirect3DDevice9 * device = AfxGetDirect3DDevice9())
			{
				DWORD value;
				if(SUCCEEDED(device->GetRenderState(D3DRS_STENCILENABLE, &value)))
				{
					if(value)
					{
						// Stencil active.

						// Edge case for cut-out mask.
						return; // we don't want to tamper with the mask.
					}

				}
			}

			if (1 == m_DebugColor)
			{
				color[3] = 1;
			}
			else if (2 == m_DebugColor)
			{
				color[0] = color[3];
				color[1] = color[3];
				color[2] = color[3];
				color[3] = 1;
			}
			else
			{
				float fac = color[3];
				if (m_Normalize && fac)
				{
					color[0] /= fac;
					color[1] /= fac;
					color[2] /= fac;
					color[3] = 1;

					RemapColor(color[0], color[1], color[2], color[3]);

					color[0] *= fac;
					color[1] *= fac;
					color[2] *= fac;
					color[3] = fac;
				}
				else
				{
					RemapColor(color[0], color[1], color[2], color[3]);
				}
			}
		}

	protected:
		virtual ~CActionGlowColorMap();

	private:
		struct CRecentEntry
		{
			int Count;
			CAfxColorLut::CRgba Result;

			CRecentEntry(const CAfxColorLut::CRgba & result)
				: Count(1)
				, Result(result)
			{
			}

			CRecentEntry(const CRecentEntry& other)
				: Count(other.Count)
				, Result(other.Result)
			{
			}
		};

		std::shared_timed_mutex m_EditMutex;
		int m_DebugColor = 0;
		bool m_Normalize = false;
		std::queue<CAfxColorLut::CRgba> m_Queue;
		std::map<CAfxColorLut::CRgba, CRecentEntry> m_Cache;
		CAfxColorLut* m_AfxColorLut = nullptr;

		void RemapColor(float& r, float& g, float& b, float &a);
	};

	class CActionDebugDepth
	: public CAction
	{
	public:
		CActionDebugDepth(CAction * fallBackAction);

		virtual CAction * ResolveAction(const CAfxTrackedMaterialRef& trackedMaterial);

		virtual void MainThreadInitialize(void);

		virtual void AfxUnbind(CAfxBaseFxStreamContext * ch);

		virtual void MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial) override;

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
			, m_UseClassName(false)
			, m_UseModelName(false)
			, m_UseIsPlayer(false)
			, m_UseTeamNumber(false)
			, m_IsErrorMaterial(TS_DontCare)
			, m_MatchAction(0)
		{
		}

		CActionFilterValue(
			bool useHandle,
			SOURCESDK::CSGO::CBaseHandle handle,
			const char* className,
			const char* modelName,
			bool useIsPlayer,
			bool isPlayer,
			bool useTeamNumber,
			int teamNumber,
			char const* name,
			char const* textureGroupName,
			char const* shaderName,
			TriState isErrorMaterial,
			CAction* matchAction)
			: m_UseHandle(useHandle)
			, m_Handle(handle)
			, m_UseClassName(0 != strcmp(className, "\\*"))
			, m_ClassName(className)
			, m_UseModelName(0 != strcmp(modelName, "\\*"))
			, m_ModelName(modelName)
			, m_UseIsPlayer(useIsPlayer)
			, m_IsPlayer(isPlayer)
			, m_UseTeamNumber(useTeamNumber)
			, m_TeamNumber(teamNumber)
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
		, m_UseClassName(false)
		, m_ClassName("\\*")
		, m_UseModelName(false)
		, m_ModelName("\\*")
		, m_UseIsPlayer(false)
		, m_IsPlayer(false)
		, m_UseTeamNumber(false)
		, m_TeamNumber(0)
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
		, m_UseClassName(x.m_UseClassName)
		, m_ClassName(x.m_ClassName)
		, m_UseModelName(x.m_UseModelName)
		, m_ModelName(x.m_ModelName)
		, m_UseIsPlayer(x.m_UseIsPlayer)
		, m_IsPlayer(x.m_IsPlayer)
		, m_UseTeamNumber(x.m_UseTeamNumber)
		, m_TeamNumber(x.m_TeamNumber)
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
			this->m_UseClassName = x.m_UseClassName;
			this->m_ClassName = x.m_ClassName;
			this->m_UseModelName = x.m_UseModelName;
			this->m_ModelName = x.m_ModelName;
			this->m_UseIsPlayer = x.m_UseIsPlayer;
			this->m_IsPlayer = x.m_IsPlayer;
			this->m_UseTeamNumber = x.m_UseTeamNumber;
			this->m_TeamNumber = x.m_TeamNumber;
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
			std::string handleStr("\\*");
			std::string teamNumberStr("\\*");

			if (m_UseHandle)
			{
				handleStr = std::to_string(m_Handle.ToInt());
			}

			if (m_UseTeamNumber)
			{
				teamNumberStr = std::to_string(m_TeamNumber);
			}

			Tier0_Msg("id=%i, \"handle=%s\",\"name=%s\", \"textureGroup=%s\", \"shader=%s\", \"isErrrorMaterial=%s\",  \"className=%s\",  \"modelName=%s\", \"isPlayer=%s\", \"teamNumber=%s\", \"action=%s\"\n",
				id,
				handleStr.c_str(),
				m_Name.c_str(),
				m_TextureGroupName.c_str(),
				m_ShaderName.c_str(),
				m_IsErrorMaterial == TS_True ? "1" : (m_IsErrorMaterial == TS_False ? "0" : "\\*"),
				m_ClassName.c_str(),
				m_ModelName.c_str(),
				m_UseIsPlayer ? (m_IsPlayer ? "1" : "0") : "\\*",
				teamNumberStr.c_str(),
				m_MatchAction ? m_MatchAction->Key_get().m_Name.c_str() : "(null)"
			);
		}

		CAction * GetMatchAction(void)
		{
			return m_MatchAction;
		}

		bool GetUseEntity(void)
		{
			return m_UseHandle || m_UseClassName || m_UseModelName || m_UseIsPlayer || m_UseTeamNumber;
		}

		bool CalcMatch_Material(const CAfxTrackedMaterialRef& trackedMaterial);

		bool CalcMatch_Entity(const CEntityMeta & info);

	private:
		bool m_UseHandle;
		SOURCESDK::CSGO::CBaseHandle m_Handle;
		bool m_UseClassName;
		std::string m_ClassName;
		bool m_UseModelName;
		std::string m_ModelName;
		bool m_UseIsPlayer;
		bool m_IsPlayer;
		bool m_UseTeamNumber;
		int m_TeamNumber;
		std::string m_Name;
		std::string m_TextureGroupName;
		std::string m_ShaderName;
		TriState m_IsErrorMaterial;
		CAction * m_MatchAction;
	};

	class CAfxBaseFxStreamData
	{
	public:
		CAfxBaseFxStreamData()
		{
		}

		CAfxBaseFxStreamData(IAfxBasefxStreamModifier* modifier, const AfxViewportData_t& viewport, const SOURCESDK::VMatrix& projectionMatrix, const SOURCESDK::VMatrix& projectionMatrixSky, bool invalidateMap)
			: Modifier(modifier)
			, Viewport(viewport)
			, ProjectionMatrix(projectionMatrix)
			, ProjectionMatrixSky(projectionMatrixSky)
			, InvalidateMap(invalidateMap)
		{
		}

		AfxViewportData_t Viewport;
		SOURCESDK::VMatrix ProjectionMatrix;
		SOURCESDK::VMatrix ProjectionMatrixSky;
		IAfxBasefxStreamModifier* Modifier;
		bool InvalidateMap;
	} m_Data;

	class CAfxBaseFxStreamContext
		: public IAfxStreamContext
	{
	public:
		CAfxBaseFxStreamContext(CAfxBaseFxStream * stream)
			: m_CurrentAction(0)
			, m_Stream(stream)
		{
			m_MapRleaseNotification = new CMapRleaseNotification(this);
		}

		~CAfxBaseFxStreamContext()
		{
			delete m_MapRleaseNotification;
		}

		CAfxBaseFxStream * GetStream()
		{
			return m_Stream;
		}

		bool DrawingSkyBoxView_get(void)
		{
			return m_DrawingSkyBoxView;
		}

		IAfxBasefxStreamModifier * GetModifier() const
		{
			return GetModifier();
		}

		void InvalidateMap();

		void UpdateCurrentEntity(CEntityMetaRef currentEntity);

		void QueueBegin(const CAfxBaseFxStreamData& data, bool isRoot = false);
		void QueueEnd(bool isRoot = false);


		void OnEntityDeleted(SOURCESDK::IHandleEntity_csgo * entity) {

			CEntityMetaRef entMetaRef(std::unique_ptr<CEntityMeta>(new CEntityMeta(entity)));

			auto result = m_EntityToCacheEntry.find(entMetaRef);
			if(result != m_EntityToCacheEntry.end()) {
				for(auto it = result->second.begin(); it != result->second.end(); ++it) {
					(*it)->EntityActions.erase(entMetaRef);
				}

				m_EntityToCacheEntry.erase(result);
			}
		}

		//
		// IAfxStreamContext:

		virtual bool ViewRenderShouldForceNoVis(bool orgValue);

		virtual void DrawingHudBegin(void);

		virtual void DrawingHudEnd(void);

		virtual void DrawingSkyBoxViewBegin(void);

		virtual void DrawingSkyBoxViewEnd(void);

		//virtual void Viewport(int x, int y, int width, int height);

		virtual void Set_In_CModelRenderSystem_SetupBones(bool value) override {
			m_In_CModelRenderSystem_SetupBones = value;
		}

		virtual bool Get_In_CModelRenderSystem_SetupBones(void) override {
			return m_In_CModelRenderSystem_SetupBones;
		}

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(IAfxMatRenderContext* ctx, SOURCESDK::IMaterial_csgo * material, void * proxyData);

		virtual void OnLockRenderData(IAfxMatRenderContext* ctx, int nSizeInBytes, void * ptr);

		virtual void DrawInstances(IAfxMatRenderContext* ctx, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance);

		virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0);

		virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists);

		virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0);

		virtual void UnlockMesh(IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc);

		virtual void QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor);

#if AFX_SHADERS_CSGO
		virtual void SetVertexShader(CAfx_csgo_ShaderState & state);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);
#endif

	private:
		CAfxBaseFxStreamData m_Data;
		bool m_DrawingHud;
		bool m_DrawingSkyBoxView;
		bool m_IsNextDepth;
		CEntityMetaRef m_CurrentEntityMeta;
		CEntityMetaRef m_CurrentEntityMetaOrg;
		bool m_In_CModelRenderSystem_SetupBones = false;
		SOURCESDK::IMaterial_csgo * m_CurrentMaterial = nullptr;
		SOURCESDK::IMaterial_csgo * m_CurrentMaterialOrg = nullptr;
		void * m_CurrentProxyData = nullptr;

		std::atomic<IAfxMatRenderContext *> m_RootContext = nullptr;

		class CQueueBeginFunctor
			: public CAfxFunctor
		{
		public:
			CQueueBeginFunctor(CAfxBaseFxStreamContext * streamContext, const CAfxBaseFxStreamData& data)
				: m_StreamContext(streamContext)
				, m_Data(data)
			{
			}

			virtual void operator()();

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
			CAfxBaseFxStreamData m_Data;
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
			CUpdateCurrentEnitityHandleFunctor(CAfxBaseFxStreamContext * streamContext, CEntityMetaRef currentEntity)
				: m_StreamContext(streamContext)
				, m_CurrentEntity(currentEntity)
			{
			}

			virtual void operator()()
			{
				m_StreamContext->UpdateCurrentEntity(m_CurrentEntity);
			}

		private:
			CAfxBaseFxStreamContext * m_StreamContext;
			CEntityMetaRef m_CurrentEntity;
		};

		CAfxBaseFxStream * m_Stream;
		CAfxBaseFxStream::CAction * m_CurrentAction;
		//std::map<void *, SOURCESDK::CSGO::CBaseHandle> m_ProxyDataToEntityHandle;

		struct CCacheEntry
		{
			struct CachedData
			{
				CAction* Action = nullptr;

				CachedData()
				{
				}

				CachedData(const CachedData& other)
					: Action(other.Action)
				{
					if (Action) Action->AddRef();
				}

				~CachedData()
				{
					if (Action) Action->Release();
				}
			};

			std::map<CEntityMetaRef, CachedData> EntityActions;
		};

		std::mutex m_MapMutex;
		std::map<CAfxTrackedMaterialRef, CCacheEntry> m_Map;
		std::map<CEntityMetaRef, std::list<CCacheEntry*>> m_EntityToCacheEntry;

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

		class CMapRleaseNotification : public IAfxMaterialFree
		{
		public:
			CMapRleaseNotification(CAfxBaseFxStreamContext * context)
				: m_Context(context)
			{
			}

			~CMapRleaseNotification()
			{
				std::unique_lock<std::mutex> unique_lock(m_Context->m_MapMutex);

				std::map<CAfxTrackedMaterialRef, CCacheEntry>& map = m_Context->m_Map;

				for (std::map<CAfxTrackedMaterialRef, CCacheEntry>::iterator it = map.begin(); it != map.end(); ++it)
				{
					it->first.Get()->RemoveNotifyee(this);
				}
			}

			virtual void AfxMaterialFree(CAfxTrackedMaterial* trackedMaterial)
			{
				// Happens only from drawing context or when not drawing, so no need to lock.

				std::map<CAfxTrackedMaterialRef, CCacheEntry>::iterator it = m_Context->m_Map.find(trackedMaterial);

				if (it != m_Context->m_Map.end())
				{
					m_Context->m_Map.erase(it);
				}
			}

		private:
			CAfxBaseFxStreamContext* m_Context;


		} *m_MapRleaseNotification;

		CAction* RetrieveAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity);

		void IfRootThenUpdateCurrentEntity(void *proxyData)
		{
			IAfxMatRenderContext* rootContext = m_RootContext;

			if (rootContext && rootContext == GetCurrentContext())
			{
				m_Stream->UpdateCurrentEntity(proxyData);
			}
		}
	};

	CAfxBaseFxStreamContext * m_Context = nullptr;

	CEntityMetaRef m_CurrentEntityMeta = nullptr;

	void UpdateCurrentEntity(void* proxyData);

	bool m_DebugPrint;

	class CPickerEntValue : public IAfxMaterialFree
	{
	public:
		int Index;
		std::set<CAfxTrackedMaterialRef> Materials;

		virtual void AfxMaterialFree(CAfxTrackedMaterial * trackedMaterial)
		{
			// Happens only from drawing context or when not drawing, so no need to lock.

			Materials.erase(trackedMaterial);
		}

		CPickerEntValue(CAfxBaseFxStream * stream, int index, const CAfxTrackedMaterialRef& trackedMaterial)
			: m_Stream(stream)
			, Index(index)
		{
			Materials.insert(trackedMaterial);

			trackedMaterial.Get()->AddNotifyee(this);
		}

		~CPickerEntValue()
		{
			for (std::set<CAfxTrackedMaterialRef>::iterator it = Materials.begin(); it != Materials.end(); ++it)
			{
				(*it).Get()->RemoveNotifyee(this);
			}
		}

	private:
		CAfxBaseFxStream * m_Stream;
	};

	std::list<CActionFilterValue> m_ActionFilter;

	bool m_InvalidateMap = false;

	struct CPickerMatValue
	{
		int Index;
		std::set<CEntityMetaRef> Entities;

		CPickerMatValue(int index, CEntityMetaRef entity)
		{
			Index = index;
			Entities.insert(entity);
		}
	};
	std::map<CAfxTrackedMaterialRef, CPickerMatValue> m_PickerMaterials;
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

			std::map<CAfxTrackedMaterialRef, CPickerMatValue> & pickerMaterials = m_Stream->m_PickerMaterials;

			for (std::map<CAfxTrackedMaterialRef, CPickerMatValue>::iterator it = pickerMaterials.begin(); it != pickerMaterials.end(); ++it)
			{
				it->first.Get()->RemoveNotifyee(this);
			}
		}

		virtual void AfxMaterialFree(CAfxTrackedMaterial * trackedMaterial)
		{
			// Happens only from drawing context or when not drawing, so no need to lock.

			m_Stream->m_PickerMaterials.erase(trackedMaterial);
		}

	private:
		CAfxBaseFxStream * m_Stream;

	} * m_PickerMaterialsRleaseNotification;

	std::map<CEntityMetaRef, CPickerEntValue> m_PickerEntities;
	bool m_PickingEntities;
	bool m_PickerEntitiesAlerted;

	std::atomic_bool m_PickerActive = false;
	bool m_PickerCollecting;
	std::shared_timed_mutex m_PickerMutex;

	bool m_ClearBeforeRender = false;

	bool m_Active = false;

	CAction * CAfxBaseFxStream::GetAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity);
	CAction * CAfxBaseFxStream::GetAction(const CAfxTrackedMaterialRef& trackedMaterial, CAction * action);

	/*
	void ConvertDepthAction(CAction * & action, bool to24);
	*/

	bool Picker_GetHidden(const CAfxTrackedMaterialRef& tackedMaterial,CEntityMetaRef currentEntity);
};

class __declspec(novtable) IAfxBasefxStreamModifier abstract
{
public:
	virtual void OverrideClearColor(unsigned char & outR, unsigned char & outG, unsigned char & outB, unsigned char & outA) = 0;
	virtual CAfxBaseFxStream::CAction * OverrideAction(CAfxBaseFxStream::CAction * action) = 0;
};

class CAfxMatteStream
	: public CAfxRecordStream
{
public:
	/// <remarks>Takes ownership of given streams.</remarks>
	CAfxMatteStream(char const * streamName, CAfxRenderViewStream * stream);

	virtual IAfxBasefxStreamModifier * GetBasefxStreamModifier(size_t streamIndex) const override
	{
		if (streamIndex >= m_Modifiers.size()) return nullptr;

		return m_Modifiers[streamIndex];
	}

	virtual CAfxRenderViewStream::StreamCaptureType GetCaptureType() const override;

	virtual bool Console_Edit_Head(IWrpCommandArgs * args) override;
	virtual void Console_Edit_Tail(IWrpCommandArgs * args) override;

	void OverrideClearColor(size_t streamIndex, unsigned char & outR, unsigned char & outG, unsigned char & outB, unsigned char & outA);
	CAfxBaseFxStream::CAction * OverrideAction(size_t streamIndex, CAfxBaseFxStream::CAction * action);

protected:
	virtual ~CAfxMatteStream();

	virtual void CaptureEnd() override;

private:
	std::vector<IAfxBasefxStreamModifier *> m_Modifiers;
	std::set<CAfxBaseFxStream::CAction *> m_MatteActions;
	std::set<CAfxBaseFxStream::CAction *> m_NoMatteActions;
	CAfxBaseFxStream::CAction * m_ActionBlack;
	CAfxBaseFxStream::CAction * m_ActionWhite;
	CAfxBaseFxStream::CAction * m_ActionNoDraw;
	bool m_HandleMaskAction;
};

class CAfxDepthStream
: public CAfxBaseFxStream
{
public:
	CAfxDepthStream() : CAfxBaseFxStream()
	{
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

		this->SmokeOverlayAlphaFactor_set(0.0f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawAction_get());
	}

protected:
	virtual ~CAfxMatteEntityStream() {}
};

class CAfxMatteFxStream
	: public CAfxBaseFxStream
{
public:
	CAfxMatteFxStream() : CAfxBaseFxStream()
	{
		SetAction(m_PlayerModelsAction, m_Shared.DrawMatteAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawMatteAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawMatteAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawMatteAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawMatteAction_get());
		SetAction(m_StickerAction, m_Shared.DrawMatteAction_get());

		// Temporary workaround:
		SetAction(m_WriteZAction, m_Shared.DrawMatteAction_get());
		SetAction(m_DevAction, m_Shared.DrawMatteAction_get());
		SetAction(m_OtherEngineAction, m_Shared.DrawMatteAction_get());
		SetAction(m_OtherSpecialAction, m_Shared.DrawMatteAction_get());

		DrawHud_set(DT_NoDraw);

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawMatteAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawMatteAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawMatteAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawMatteAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawMatteAction_get());
		SetAction(m_OtherAction, m_Shared.NoDrawMatteAction_get());

		this->SmokeOverlayAlphaFactor_set(0.0f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawMatteAction_get());
	}

protected:
	virtual ~CAfxMatteFxStream() {}
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

		this->SmokeOverlayAlphaFactor_set(0.0f);
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

class CAfxStreams
: public IAfxBaseClientDllView_Render
{
public:
	typedef SOURCESDK::IMatRenderContext_csgo CMatQueuedRenderContext_csgo;

	advancedfx::CImageBufferPool ImageBufferPool;

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
	void OnRenderView(CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	bool OnViewRenderShouldForceNoVis(bool orgValue);

	void OnDrawingHudBegin(void);

	void OnDrawingHudEnd(void);

	void On_DrawTranslucentRenderables(SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall);

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
	void Console_AddMatteStream(const char * streamName);
	void Console_AddHudStream(const char * streamName);
	void Console_AddHudWhiteStream(const char * streamName);
	void Console_AddHudBlackStream(const char * streamName);
	void Console_PrintStreams();
	void Console_MoveStream(IWrpCommandArgs * args);
	void Console_RemoveStream(const char * streamName);
	void Console_EditStream(const char * streamName, IWrpCommandArgs * args);
	void Console_EditStream(CAfxStream * stream, IWrpCommandArgs * args);
	bool Console_EditStream(CAfxRenderViewStream * stream, IWrpCommandArgs * args);
	void Console_ListActions(void);
	void Console_Bvh(IWrpCommandArgs * args);
	bool Console_ToStreamCombineType(char const * value, CAfxTwinStream::StreamCombineType & streamCombineType);
	char const * Console_FromStreamCombineType(CAfxTwinStream::StreamCombineType streamCombineType);

	bool CamExport_get(void) { return m_CamExport;  }
	void CamExport_set(bool value) { m_CamExport = value;  }

	void Console_GameRecording(IWrpCommandArgs * args);

	/// <param name="streamName">stream name to preview or empty string if to preview nothing.</param>
	/// <param name="slot">-1 means all slots if streamName is emtpy.</param>
	void Console_PreviewStream(const char * streamName, int slot);

	bool Console_ToAfxAction(char const * value, CAfxBaseFxStream::CAction * & action);
	char const * Console_FromAfxAction(CAfxBaseFxStream::CAction * action);

	virtual SOURCESDK::IMaterialSystem_csgo * GetMaterialSystem(void);
	virtual SOURCESDK::IShaderShadow_csgo * GetShaderShadow(void);

	const std::wstring & GetTakeDir(void) const;

	void LevelInitPostEntity(void);
	void LevelShutdown(void);

	virtual void View_Render(IAfxBaseClientDll * cl, SOURCESDK::vrect_t_csgo *rect);

	float GetStartHostFrameRate()
	{
		return m_StartHostFrameRateValue;
	}

	void Console_MainStream(IWrpCommandArgs * args);

	bool DrawPhiGrid = false;
	bool DrawRuleOfThirds = false;

	void BeforeFrameStart()
	{
	}

	void OnClientEntityCreated(SOURCESDK::C_BaseEntity_csgo* ent);

	void OnClientEntityDeleted(SOURCESDK::C_BaseEntity_csgo* ent);

	bool IsRecording() {
		return m_Recording;
	}

	bool IsQueuedThreaded();

	bool IsSingleThreaded();

	bool OnEngineThread();

	IAfxStreamContext * FindStreamContext(IAfxMatRenderContext * ctx);

private:

	enum MainStreamMode_e
	{
		MainStreamMode_None,
		MainStreamMode_FirstActive,
		MainStreamMode_First,
		MainStreamMode_Set
	} m_MainStreamMode = MainStreamMode_FirstActive;
	CAfxRecordStream * m_MainStream = nullptr;

	bool m_ForceCacheFullSceneState = false;

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

	const SOURCESDK::CViewSetup_csgo * m_CurrentView;

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
	CamExport * m_CamExportObj = 0;
	bool m_GameRecording;

	WrpConVarRef * m_HostFrameRate = nullptr;
	float m_StartHostFrameRateValue = 0.0f;

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

	//WrpConVarRef * m_cl_modelfastpath = nullptr;
	//int m_Old_cl_modelfastpath;

	//WrpConVarRef * m_cl_tlucfastpath = nullptr;
	//int m_Old_cl_tlucfastpath;

	//WrpConVarRef * m_cl_brushfastpath = nullptr;
	int m_Old_cl_brushfastpath;

	//WrpConVarRef * m_r_drawstaticprops = nullptr;
	//int m_Old_r_drawstaticprops;

	std::wstring m_TakeDir;
	//ITexture_csgo * m_RgbaRenderTarget;
	SOURCESDK::ITexture_csgo * m_RenderTargetDepthF;
	//CAfxMaterial * m_ShowzMaterial;
	DWORD m_View_Render_ThreadId;
	bool m_PresentBlocked = false;
	bool m_ShutDown = false;

	bool m_HudDrawn = false;

	void Set_View_Render_ThreadId(DWORD id);

	DWORD Get_View_Render_ThreadId();

	void OnAfxBaseClientDll_Free(void);

	bool Console_CheckStreamName(char const * value);

	bool Console_ToStreamCaptureType(char const * value, CAfxRenderViewStream::StreamCaptureType & StreamCaptureType);
	char const * Console_FromStreamCaptureType(CAfxRenderViewStream::StreamCaptureType StreamCaptureType);

	bool CheckCanFeedStreams(void);

	void BackUpMatVars();
	void SetMatVarsForStreams();
	void RestoreMatVars();
	void EnsureMatVars();

	//void DisableFastPath();
	//void RestoreFastPath();

	void AddStream(CAfxRecordStream * stream);

	void CreateRenderTargets(SOURCESDK::IMaterialSystem_csgo * materialSystem);

	IAfxMatRenderContextOrg * CaptureStream(IAfxMatRenderContextOrg * ctxp, CAfxRecordStream * stream, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);
	IAfxMatRenderContextOrg * CaptureStreamToBuffer(IAfxMatRenderContextOrg * ctxp, size_t streamIndex, CAfxRenderViewStream * stream, CAfxRecordStream * captureTarget, bool first, bool last, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	IAfxMatRenderContextOrg * PreviewStream(IAfxMatRenderContextOrg * ctxp, CAfxRenderViewStream * previewStream, bool isLast, int slot, int cols, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer);

	void BlockPresent(IAfxMatRenderContextOrg * ctx, bool value);

	void DoRenderView(CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw);

	void CalcMainStream();

	void UpdateStreamDeps();
};
