#include "stdafx.h"

#include "AfxStreams.h"

#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "csgo_CSkyBoxView.h"
#include "csgo_view.h"
#include "csgo_CViewRender.h"
#include "RenderView.h"
#include "ClientTools.h"
#include "d3d9Hooks.h"
#include "D3D9ImageBuffer.h"
#include "csgo_GlowOverlay.h"
#include "MirvPgl.h"
#include "AfxInterop.h"
#include "csgo_Audio.h"
#include "mirv_voice.h"
#include "addresses.h"
#include "MirvTime.h"
#include "SourceInterfaces.h"
//#include "csgo/hooks/c_basentity.h"
//#include "csgo/hooks/c_baseanimating.h"
//#include "csgo/hooks/c_basecombatweapon.h"
//#include "csgo/hooks/staticpropmgr.h"
#include "csgo/ClientToolsCsgo.h"
#include "ReShadeAdvancedfx.h"
#include "AfxCommandLine.h"

#include <shared/StringTools.h>
#include <shared/FileTools.h>
#include <shared/ThreadPool.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>

#undef min
#undef max

//#define CAFXBASEFXSTREAM_STREAMCOMBINETYPES "aRedAsAlphaBColor|aColorBRedAsAlpha|aHudWhiteBHudBlack"
#define CAFXBASEFXSTREAM_STREAMCOMBINETYPES "aRedAsAlphaBColor|aColorBRedAsAlpha"
#define CAFXBASEFXSTREAM_STREAMCAPTURETYPES "normal|depth24|depth24ZIP|depthF|depthFZIP"
#define CAFXSTREAMS_ACTIONSUFFIX " <actionName> - Set action with name <actionName> (see mirv_streams actions)."


#if AFXSTREAMS_REFTRACKER

#include <atomic>

std::atomic_int g_AfxStreams_RefTracker_Count = 0;

void AfxStreams_RefTracker_Inc(void)
{
	++g_AfxStreams_RefTracker_Count;
}

void AfxStreams_RefTracker_Dec(void)
{
	--g_AfxStreams_RefTracker_Count;
}

int AfxStreams_RefTracker_Get(void)
{
	return g_AfxStreams_RefTracker_Count;
}

#endif


extern WrpVEngineClient * g_VEngineClient;
extern SOURCESDK::IMaterialSystem_csgo * g_MaterialSystem_csgo;
extern SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo;
extern SOURCESDK::CSGO::IVModelInfoClient* g_pModelInfo;

static AfxInterop::EnabledFeatures_t g_InteropFeatures;

IAfxMatRenderContext * GetCurrentContext()
{
	if (!g_MaterialSystem_csgo)
		return nullptr;

	return MatRenderContextHook(g_MaterialSystem_csgo);
}

CAfxStreams g_AfxStreams;


int GetMaterialSystemThread() {
	static bool bRun = false;
	static int iFirstResult = 0;

	if (bRun) return iFirstResult;
	iFirstResult = g_CommandLine->FindParam(L"-swapcores") ? 1 : 0;
	bRun = true;
	return iFirstResult;
}

////////////////////////////////////////////////////////////////////////////////

class CCaptureNode::CGpuLockQueue CCaptureNode::s_GpuLockQueue;
class CCaptureNode::CGpuReleaseQueue CCaptureNode::s_GpuReleaseQueue;

std::map<SOURCESDK::matrix3x4_t *, CEntityMetaRef> g_DrawingThread_BonesPtr_To_Meta;
std::map<SOURCESDK::IHandleEntity_csgo *, SOURCESDK::matrix3x4_t *> g_DrawingThread_EntityPtr_To_BonesPtr;

std::set<CAfxRenderViewStream *> m_DrawingThread_NotifyStreams;

std::list<CEntityMetaRef> g_DrawingThread_CurrentMeta;

class CAfxNotifyStreamAdd
	: public CAfxFunctor
{
public:
	CAfxNotifyStreamAdd(CAfxRenderViewStream * stream)
		: m_Stream(stream)
	{
		stream->AddRef();
	}

	virtual void operator()() {
		m_DrawingThread_NotifyStreams.emplace(m_Stream);
	}

private:
	CAfxRenderViewStream * m_Stream;
};

class CAfxNotifyStreamRemove
	: public CAfxFunctor
{
public:
	CAfxNotifyStreamRemove(CAfxRenderViewStream * stream)
		: m_Stream(stream)
	{
	}

	virtual void operator()() {
		 m_DrawingThread_NotifyStreams.erase(m_Stream);
		 m_Stream->Release();
	}

private:
	CAfxRenderViewStream * m_Stream;
};


class CAfxUpdateEntityMetaFunctor
	: public CAfxFunctor
{
public:
	CAfxUpdateEntityMetaFunctor(SOURCESDK::matrix3x4_t * bonesPtr, CEntityMetaRef entityMetaRef)
		: m_BonesPtr(bonesPtr)
		, m_EntityMetaRef(entityMetaRef)
	{

	}

	virtual void operator()() {
		auto result = g_DrawingThread_EntityPtr_To_BonesPtr.find(m_EntityMetaRef->EntityPtr);
		if(result != g_DrawingThread_EntityPtr_To_BonesPtr.end()) {
			if(result->second != m_BonesPtr) {
				g_DrawingThread_BonesPtr_To_Meta.erase(result->second);
				result->second = m_BonesPtr;
				g_DrawingThread_BonesPtr_To_Meta.emplace(m_BonesPtr, m_EntityMetaRef);
			}
		} else {
			g_DrawingThread_EntityPtr_To_BonesPtr[m_EntityMetaRef->EntityPtr] = m_BonesPtr;
			g_DrawingThread_BonesPtr_To_Meta.emplace(m_BonesPtr, m_EntityMetaRef);
		}
	}

private:
	SOURCESDK::matrix3x4_t * m_BonesPtr;
	CEntityMetaRef m_EntityMetaRef;
};

class CAfxDeleteEntityMetaFunctor
	: public CAfxFunctor
{
public:
	CAfxDeleteEntityMetaFunctor(SOURCESDK::IHandleEntity_csgo * handleEntity)
		: m_HandleEntity(handleEntity)
	{

	}

	virtual void operator()() {
		for(auto it=m_DrawingThread_NotifyStreams.begin(); it != m_DrawingThread_NotifyStreams.end(); ++it) {
			(*it)->OnEntityDeleted(m_HandleEntity);
		}
		auto result = g_DrawingThread_EntityPtr_To_BonesPtr.find(m_HandleEntity);
		if(result != g_DrawingThread_EntityPtr_To_BonesPtr.end()) {
			g_DrawingThread_BonesPtr_To_Meta.erase(result->second);			
			g_DrawingThread_EntityPtr_To_BonesPtr.erase(result);
		}
	}

private:
	SOURCESDK::IHandleEntity_csgo *  m_HandleEntity;
};


typedef void (__fastcall * csgo_IHandleEntity_Dtor_t)( SOURCESDK::IHandleEntity_csgo * This, void* Edx, int flags);
std::map<csgo_IHandleEntity_Dtor_t,csgo_IHandleEntity_Dtor_t> csgo_IHandleEntity_Dtors;

void __fastcall My_csgo_IHandleEntity_Dtor( SOURCESDK::IHandleEntity_csgo * This, void* Edx, int flags) {
	void **vtable = *(void ***)This;
	csgo_IHandleEntity_Dtors[(csgo_IHandleEntity_Dtor_t)(vtable[0])](This,Edx,flags);
}

void TrackRenderableLifeTime(SOURCESDK::IClientRenderable_csgo * This) {
	SOURCESDK::IHandleEntity_csgo * base = This->GetIClientUnknown();
	void **vtable = *(void ***)base;
	csgo_IHandleEntity_Dtor_t dtor = (csgo_IHandleEntity_Dtor_t)(vtable[0]);

	if(csgo_IHandleEntity_Dtors.find(dtor) != csgo_IHandleEntity_Dtors.end()) return; // already tracked

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)dtor, My_csgo_IHandleEntity_Dtor);
	LONG error = DetourTransactionCommit();
	if(NO_ERROR == error) {
		csgo_IHandleEntity_Dtors.emplace((csgo_IHandleEntity_Dtor_t)(vtable[0]), dtor);
	}
}


typedef void (__fastcall * csgo_CModelRenderSystem_SetupBones_t)( void * This, void* Edx, int nModelTypeCount, void *pModelList );
csgo_CModelRenderSystem_SetupBones_t True_csgo_CModelRenderSystem_SetupBones = nullptr;
void __fastcall My_csgo_CModelRenderSystem_SetupBones( void * This, void* Edx, int nModelTypeCount, void *pModelList ) {
	auto ctx = GetCurrentContext();
	IAfxStreamContext * hook = g_AfxStreams.FindStreamContext(ctx);
	if (hook) {
		hook->Set_In_CModelRenderSystem_SetupBones(true);
		for(int i=0; i < nModelTypeCount; ++i) {
			unsigned char * pI = (unsigned char *)pModelList + 0x1c +0x30*i;
			int count = *(int *)pI;
			for(int j=0; j<count;++j) {
				SOURCESDK::IClientRenderable_csgo *clientRenderAble = *(SOURCESDK::IClientRenderable_csgo **)(*(unsigned char **)(pI + 0x0c) +0x60 * j +0x48);
				if(clientRenderAble) {
					CEntityMetaRef entMeta(std::unique_ptr<CEntityMeta>(new CEntityMeta(clientRenderAble->GetIClientUnknown())));
					SOURCESDK::IClientUnknown_csgo * clientUnknown = clientRenderAble->GetIClientUnknown();
					SOURCESDK::IClientUnknown_csgo * clientEntity = clientUnknown ? clientUnknown->GetIClientEntity() : nullptr;
					SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : nullptr;
					entMeta->Handle = clientUnknown ? clientUnknown->GetRefEHandle() : SOURCESDK_CSGO_INVALID_EHANDLE_INDEX;
					entMeta->ClassName = baseEntity ? baseEntity->GetClassname() : "";
					entMeta->IsPlayer = baseEntity ? baseEntity->IsPlayer() : false;
					entMeta->TeamNumber = baseEntity ? baseEntity->GetTeamNumber() : 0;
					if(g_pModelInfo) {
						const SOURCESDK::CSGO::model_t * pModel = clientRenderAble->GetModel();
						if(pModel) {
							entMeta->ModelName = g_pModelInfo->GetModelName(pModel);
						}
					}
					g_DrawingThread_CurrentMeta.push_back(entMeta);
					TrackRenderableLifeTime(clientRenderAble);
				} else {
					CEntityMetaRef entMeta = nullptr;
					g_DrawingThread_CurrentMeta.push_back(entMeta);
				}
			}
		}
	}
	True_csgo_CModelRenderSystem_SetupBones(This,Edx,nModelTypeCount,pModelList);
	if (hook) {
		hook->Set_In_CModelRenderSystem_SetupBones(false);
	}
	g_DrawingThread_CurrentMeta.clear();
}

bool csgo_CModelRenderSystem_SetupBones_Install(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_client_CModelRenderSystem_SetupBones))
	{
		LONG error = NO_ERROR;

		True_csgo_CModelRenderSystem_SetupBones = (csgo_CModelRenderSystem_SetupBones_t)AFXADDR_GET(csgo_client_CModelRenderSystem_SetupBones);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_csgo_CModelRenderSystem_SetupBones, My_csgo_CModelRenderSystem_SetupBones);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

////////////////////////////////////////////////////////////////////////////////

bool AfxOverrideable_FromConsole(const char * arg, CAfxBoolOverrideable & outValue)
{
	if (0 == _stricmp("default", arg))
	{
		outValue.AssignNoOverride();
		return true;
	}
	else if (StringIsDigits(arg))
	{
		outValue = 0 != atoi(arg);
		return true;
	}

	Tier0_Warning("AFXERROR: %s is not a valid value.\n");
	return false;
}

void AfxOverridable_ToConsole(const CAfxBoolOverrideable & value)
{
	bool boolValue;
	if (value.Get(boolValue)) Tier0_Msg("%i", boolValue ? 1 : 0);
	else Tier0_Msg("default");
}

void AfxOverrideable_Console(IWrpCommandArgs * args, CAfxBoolOverrideable & value, const char * description)
{
	int argC = args->ArgC();

	if (2 == argC)
	{
		const char * argValue = args->ArgV(1);
		AfxOverrideable_FromConsole(argValue, value);
		return;
	}

	Tier0_Msg("%s default|0|1", args->ArgV(0));
	if (description) Tier0_Msg(" - %s", description);
	Tier0_Msg("\n");
	Tier0_Msg("Current value: ");
	AfxOverridable_ToConsole(value);
	Tier0_Msg("\n");
}

/* Doesn't work for some reason.
void DebugDepthFixDraw(IMesh_csgo * pMesh)
{
	MeshDesc_t_csgo meshDesc;
	VertexDesc_t_csgo vertexDesc;

	int nMaxVertexCount, nMaxIndexCount;
	nMaxVertexCount =  nMaxIndexCount = 4;

	pMesh->SetPrimitiveType( MATERIAL_POINTS );

	pMesh->LockMesh(nMaxVertexCount, nMaxIndexCount, meshDesc, 0);

	IIndexBuffer_csgo * m_pIndexBuffer = pMesh;
	int m_nIndexCount = 0;
	int m_nMaxIndexCount = nMaxIndexCount;

	int m_nIndexOffset = meshDesc.m_nFirstVertex;
	unsigned short * m_pIndices = meshDesc.m_pIndices;
	unsigned int m_nIndexSize = meshDesc.m_nIndexSize;
	int m_nCurrentIndex = 0;

	IVertexBuffer_csgo * m_pVertexBuffer = pMesh;
	memcpy( static_cast<VertexDesc_t_csgo*>( &vertexDesc ), static_cast<const VertexDesc_t_csgo*>( &meshDesc ), sizeof(VertexDesc_t_csgo) );
	int m_nMaxVertexCount = nMaxVertexCount;

	unsigned int m_nTotalVertexCount = 0;
	//unsigned int m_nBufferOffset = static_cast< const VertexDesc_t_csgo* >( &meshDesc )->m_nOffset;
	//unsigned int m_nBufferFirstVertex = meshDesc.m_nFirstVertex;

	int m_nVertexCount = 0;

	int m_nCurrentVertex = 0;

	float * m_pCurrPosition = vertexDesc.m_pPosition;
	float * m_pCurrNormal = vertexDesc.m_pNormal;
	float * m_pCurrTexCoord[VERTEX_MAX_TEXTURE_COORDINATES_csgo];
	for ( size_t i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES_csgo; i++ )
	{
		m_pCurrTexCoord[i] = vertexDesc.m_pTexCoord[i];
	}
	unsigned char * m_pCurrColor = vertexDesc.m_pColor;
	// BEGIN actual "drawing":

	// Position:
	{
		float *pDst = m_pCurrPosition;
		*pDst++ = 0.0f;
		*pDst++ = 0.0f;
		*pDst = 0.0f;
	}

	// Color:
	if(false) {
		int r = 0;
		int g = 0;
		int b = 0;
		int a = 0;

		int col = b | (g << 8) | (r << 16) | (a << 24);

		*(int*)m_pCurrColor = col;
	}


	// Normal:
	{
		float *pDst = m_pCurrNormal;
		*pDst++ = 0.0f;
		*pDst++ = 0.0f;
		*pDst = 0.0f;
	}


	// TextCoord:
	{
		float *pDst = m_pCurrTexCoord[0];
		*pDst++ = 1.0f;
		*pDst++ = 0.0f;
		*pDst = 0.0f;
	}

	// AdvanceVertex:
	{
		if ( ++m_nCurrentVertex > m_nVertexCount )
		{
			m_nVertexCount = m_nCurrentVertex;
		}

		//m_pCurrPosition = reinterpret_cast<float*>( reinterpret_cast<unsigned char*>( m_pCurrPosition ) + vertexDesc.m_VertexSize_Position );
		//m_pCurrColor += vertexDesc.m_VertexSize_Color;
	}

	// End drawing:

	{
		int nIndexCount = 1;
		if(0 != m_nIndexSize)
		{
			int nMaxIndices = m_nMaxIndexCount - m_nCurrentIndex;
			nIndexCount = min( nMaxIndices, nIndexCount );
			if ( nIndexCount != 0 )
			{
				unsigned short *pIndices = &m_pIndices[m_nCurrentIndex];

				//GenerateSequentialIndexBuffer( pIndices, nIndexCount, m_nIndexOffset );
				// What about m_IndexOffset? -> dunno.

				*pIndices = m_nIndexOffset;
			}

			m_nCurrentIndex += nIndexCount * m_nIndexSize;
			if ( m_nCurrentIndex > m_nIndexCount )
			{
				m_nIndexCount = m_nCurrentIndex; 
			}
		}
	}
	
//	Tier0_Msg("Spew: ");
//	pMesh->Spew( m_nVertexCount, m_nIndexCount, meshDesc );

//	pMesh->ValidateData( m_nVertexCount ,m_nIndexCount, meshDesc );

	pMesh->UnlockMesh( m_nVertexCount, m_nIndexCount, meshDesc );

	// Draw!!!!
	pMesh->Draw();
}
*/

void QueueOrExecute(IAfxMatRenderContextOrg * ctx, SOURCESDK::CSGO::CFunctor * functor)
{
	SOURCESDK::CSGO::ICallQueue * queue = ctx->GetCallQueue();

	if (!queue)
	{	
		functor->AddRef();
		(*functor)();
		functor->Release();
	}
	else
	{
		queue->QueueFunctor(functor);
	}
}

void CAfxBlockFunctor::operator()()
{
	if (m_Block)
	{
		AfxD3D9PushOverrideState(false);

		AfxD3D9OverrideBegin_D3DRS_ALPHABLENDENABLE(TRUE);
		AfxD3D9OverrideBegin_D3DRS_SRCBLEND(D3DBLEND_ZERO);
		AfxD3D9OverrideBegin_D3DRS_DESTBLEND(D3DBLEND_ONE);
		AfxD3D9OverrideBegin_D3DRS_ZWRITEENABLE(FALSE);
		AfxD3D9OverrideBegin_D3DRS_COLORWRITEENABLE(0);
	}
	else
	{
		AfxD3D9OverrideEnd_D3DRS_COLORWRITEENABLE();
		AfxD3D9OverrideEnd_D3DRS_ZWRITEENABLE();
		AfxD3D9OverrideEnd_D3DRS_DESTBLEND();
		AfxD3D9OverrideEnd_D3DRS_SRCBLEND();
		AfxD3D9OverrideEnd_D3DRS_ALPHABLENDENABLE();

		AfxD3D9PopOverrideState();
	}
}

class CAfxD3D9PushOverrideState_Functor
	: public CAfxFunctor
{
public:
	CAfxD3D9PushOverrideState_Functor(bool clean)
		: m_Clean(clean)
	{
	}

	virtual void operator()()
	{
		AfxD3D9PushOverrideState(m_Clean);
	}

private:
	bool m_Clean;
};

class CAfxD3D9PopOverrideState_Functor
	: public CAfxFunctor
{
public:
	virtual void operator()()
	{
		AfxD3D9PopOverrideState();
	}
};


class AfxD3D9BlockPresent_Functor
	: public CAfxFunctor
{
public:
	AfxD3D9BlockPresent_Functor(bool value)
		: m_Value(value)
	{
	}

	virtual void operator()()
	{
		AfxD3D9_Block_Present(m_Value);
	}

private:
	bool m_Value;
};

class AfxDrawGuidesFunctor
	: public CAfxFunctor
{
public:
	AfxDrawGuidesFunctor(bool phiGrid, bool ruleOfThirds)
		: m_PhiGrid(phiGrid)
		, m_RuleOfThrids(ruleOfThirds)
	{
	}

	virtual void operator()()
	{
		IAfxMatRenderContextOrg * context = GetCurrentContext()->GetOrg();

		int x, y, width, height;
		context->GetViewport(x, y, width, height);

		AfxDrawGuides(x, y, width, height, m_PhiGrid, m_RuleOfThrids);
	}

private:
	bool m_PhiGrid;
	bool m_RuleOfThrids;
};

#ifdef AFX_INTEROP

class CAfxInteropDrawDepth_Functor
	: public CAfxFunctor
{
public:
	CAfxInteropDrawDepth_Functor(bool isNextDepth, float outZNear, float outZFar, float zNear, float zFar)
		: m_IsNextDepth(isNextDepth)
		, m_OutZNear(outZNear)
		, m_OutZFar(outZFar)
		, m_ZNear(zNear)
		, m_ZFar(zFar)
	{
	}

	virtual void operator()()
	{
		IAfxMatRenderContextOrg * context = GetCurrentContext()->GetOrg();

		int x, y, width, height;
		context->GetViewport(x, y, width, height);

		AfxInterop::DrawingThread_DrawDepth(m_IsNextDepth, m_OutZNear, m_OutZFar, x, y, width, height, m_ZNear, m_ZFar);
	}

private:
	bool m_IsNextDepth;
	float m_OutZNear;
	float m_OutZFar;
	float m_ZNear;
	float m_ZFar;
};

class CAfxInteropOverrideDepthBegin_Functor
	: public CAfxFunctor
{
public:
	CAfxInteropOverrideDepthBegin_Functor()
	{
	}

	virtual void operator()()
	{
		g_AfxStreams.DrawingThread_SetRenderTargetNoMsaa();
		g_AfxStreams.DrawingThread_SetIntZTextureSurface();
	}

private:
};

class CAfxInteropOverrideDepthEnd_Functor
	: public CAfxFunctor
{
public:
	CAfxInteropOverrideDepthEnd_Functor()
	{
	}

	virtual void operator()()
	{
		g_AfxStreams.DrawingThread_UnsetIntZTextureSurface();
		g_AfxStreams.DrawingThread_UnsetRenderTargetNoMsaa(false);
	}

private:
};

#endif


// CAfxRenderViewStream ////////////////////////////////////////////////////////

CAfxRenderViewStream * CAfxRenderViewStream::m_EngineThreadStream = 0;

CAfxRenderViewStream::CAfxRenderViewStream()
: m_DrawViewModel(DT_NoChange)
, m_DrawHud(DT_NoChange)
, m_StreamCaptureType(SCT_Normal)
{
}

CAfxRenderViewStream::~CAfxRenderViewStream()
{
}

char const * CAfxRenderViewStream::AttachCommands_get(void)
{
	return m_AttachCommands.c_str();
}

void CAfxRenderViewStream::AttachCommands_set(char const * value)
{
	m_AttachCommands.assign(value);
}

char const * CAfxRenderViewStream::DetachCommands_get(void)
{
	return m_DetachCommands.c_str();
}

void CAfxRenderViewStream::DetachCommands_set(char const * value)
{
	m_DetachCommands.assign(value);
}

CAfxRenderViewStream::DrawType CAfxRenderViewStream::DrawHud_get(void)
{
	return m_DrawHud;
}

void CAfxRenderViewStream::DrawHud_set(DrawType value)
{
	m_DrawHud = value;
}

CAfxRenderViewStream::DrawType CAfxRenderViewStream::DrawViewModel_get(void)
{
	return m_DrawViewModel;
}

void CAfxRenderViewStream::DrawViewModel_set(DrawType value)
{
	m_DrawViewModel = value;
}

CAfxRenderViewStream::StreamCaptureType CAfxRenderViewStream::StreamCaptureType_get(void) const
{
	return m_StreamCaptureType;
}

void CAfxRenderViewStream::StreamCaptureType_set(StreamCaptureType value)
{
	m_StreamCaptureType = value;
}

extern class advancedfx::CThreadPool* g_pThreadPool;

class CAfxTransformer {
public:
/*
	static class ICapture* DummyCapture() {
		advancedfx::CImageFormat format(advancedfx::ImageFormat::BGRA, 1280, 720);
		format.SetOrigin(advancedfx::ImageOrigin::TopLeft);
		CAfxImageBufferCapture * result = CAfxImageBufferCapture::Create(format);
		result->AddRef();
		return result;
	}
*/
	static class ICapture* TransformStripAlpha(class ICapture* capture) {

		if (nullptr == capture) return nullptr;

		if (const class advancedfx::IImageBuffer* pBuffer = capture->GetBuffer()) {
			if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
				if (pFormat->Format == advancedfx::ImageFormat::BGR) return capture;
			}
		}

		CTransformStripAlpha transform(capture);
		return Transform(&transform);
	}

	static class ICapture* TransformDepthF(class ICapture* capture, float depthScale, float depthOfs) {
		CTransformDepthF transform(capture, depthScale, depthOfs);
		return Transform(&transform);
	}

	static class ICapture* TransformDepth24(class ICapture* capture, float depthScale, float depthOfs) {
		CTransformDepth24 transform(capture, depthScale, depthOfs);
		return Transform(&transform);
	}

	static class ICapture* TransformMatte(class ICapture* captureEntBlack, class ICapture* captureEntWhite) {
		CTransformMatte transform(captureEntBlack, captureEntWhite);
		return Transform(&transform);
	}

	static class ICapture* TransformAColorBRedAsAlpha(class ICapture* aColor, class ICapture* bRedAsAlpha) {
		CTransformAColorBRedAsAlpha transform(aColor, bRedAsAlpha);
		return Transform(&transform);
	}

private:
	class CAfxImageBufferCapture
		: public advancedfx::CRefCountedThreadSafe
		, public ICapture
	{
	public:
		static class CAfxImageBufferCapture* Create(const class advancedfx::CImageFormat& format) {
			class CAfxImageBufferCapture* result = new CAfxImageBufferCapture();
			result->AddRef();
			if (!result->AutoRealloc(format)) {
				result->Release();
				Tier0_Warning("CAfxImageBufferCapture::Create: Failed to reallocate buffer.\n");
				return nullptr;
			}
			return result;
		}

		virtual void AddRef() override {
			advancedfx::CRefCountedThreadSafe::AddRef();
		}

		virtual void Release() override {
			advancedfx::CRefCountedThreadSafe::Release();
		}

		virtual const advancedfx::IImageBuffer* GetBuffer() const {
			return m_ImageBuffer;
		}

		void * GetImageBufferDataRw() const {
			return m_ImageBuffer->Buffer;
		}

	protected:
		CAfxImageBufferCapture()
			: advancedfx::CRefCountedThreadSafe()
		{
			m_ImageBuffer = g_AfxStreams.ImageBufferPoolThreadSafe.AquireBuffer();
		}

		virtual ~CAfxImageBufferCapture() {
			g_AfxStreams.ImageBufferPoolThreadSafe.ReleaseBuffer(m_ImageBuffer);
		}

	private:
		advancedfx::CImageBuffer* m_ImageBuffer;

		bool AutoRealloc(const class advancedfx::CImageFormat& format) {
			return m_ImageBuffer->AutoRealloc(format);
		}
	};

	class CTranformTask
		: public advancedfx::CThreadPool::CTask
	{
	public:
		CTranformTask(std::atomic_int& task_counter)
			: task_counter(task_counter)			
		{
			task_counter++;
		}

		virtual ~CTranformTask() {
			task_counter--;
		}

	protected:

	private:
		std::atomic_int& task_counter;
	};

	class ITransform {
	public:
		virtual CAfxImageBufferCapture* CreateOutput(void) = 0;
		virtual size_t GetTaskSize() = 0;
		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) = 0;
	};
	class CTransformAColorBRedAsAlpha
		: public ITransform {
	public:
		CTransformAColorBRedAsAlpha(class ICapture* aColor, class ICapture* bRedAsAlpha)
			: m_CaptureA(aColor)
			, m_CaptureB(bRedAsAlpha)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(void) {
			if (nullptr != m_CaptureA && nullptr != m_CaptureB) {

				if (const class advancedfx::IImageBuffer* pBufferA = m_CaptureA->GetBuffer()) {
					if (const unsigned char* pDataA = static_cast<const unsigned char*>(pBufferA->GetImageBufferData())) {
						m_pInDataA = pDataA;
						if (const class advancedfx::CImageFormat* pFormatA = pBufferA->GetImageBufferFormat()) {
							m_InFormatA = *pFormatA;
							if (m_InFormatA.Format == advancedfx::ImageFormat::BGRA || m_InFormatA.Format == advancedfx::ImageFormat::BGR) {

								if (const class advancedfx::IImageBuffer* pBufferB = m_CaptureB->GetBuffer()) {
									if (const unsigned char* pDataB = static_cast<const unsigned char*>(pBufferB->GetImageBufferData())) {
										m_pInDataB = pDataB;
										if (const class advancedfx::CImageFormat* pFormatB = pBufferB->GetImageBufferFormat()) {
											m_InFormatB = *pFormatB;
											if (
												(m_InFormatB.Format == advancedfx::ImageFormat::BGRA || m_InFormatB.Format == advancedfx::ImageFormat::BGR)
												&& m_InFormatB.Width == m_InFormatA.Width
												&& m_InFormatB.Height == m_InFormatA.Height
												&& m_InFormatB.Origin == m_InFormatA.Origin)
											{
												m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatA.Width, m_InFormatA.Height);
												m_OutFormat.SetOrigin(m_InFormatA.Origin);
												class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
												if (pOutCapture) {
													m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
												}
												return pOutCapture;
											}
										}
									}
								}

							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_OutFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter,
				m_pInDataA + taskIndex * m_InFormatA.Pitch, m_InFormatA.Pitch, m_InFormatA.GetPixelStride(),
				m_pInDataB + taskIndex * m_InFormatB.Pitch, m_InFormatB.Pitch, m_InFormatB.GetPixelStride(),
				m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pDataA, size_t pitchA, size_t pixelPichtA, const unsigned char* pDataB, size_t pitchB, size_t pixelPichtB, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pDataA(pDataA)
				, pitchA(pitchA)
				, pixelPichtA(pixelPichtA)
				, pDataB(pDataB)
				, pitchB(pitchB)
				, pixelPichtB(pixelPichtB)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 4 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char* pInA = (const unsigned char*)(pDataA + y * pitchA + x * pixelPichtA);
						const unsigned char* pInB = (const unsigned char*)(pDataB + y * pitchB + x * pixelPichtB);
						unsigned char* pOut = (unsigned char*)(pOutData + y * targetPitch + x * 4 * sizeof(unsigned char));

						pOut[0] = pInA[0];
						pOut[1] = pInA[1];
						pOut[2] = pInA[2];
						pOut[3] = pInB[0];

					}
				}
			}

		private:
			const unsigned char* pDataA;
			size_t pitchA;
			size_t pixelPichtA;
			const unsigned char* pDataB;
			size_t pitchB;
			size_t pixelPichtB;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_CaptureA;
		class ICapture* m_CaptureB;
		advancedfx::CImageFormat m_InFormatA;
		advancedfx::CImageFormat m_InFormatB;
		const unsigned char* m_pInDataA;
		const unsigned char* m_pInDataB;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformMatte
		: public ITransform {
	public:
		CTransformMatte(class ICapture* captureEntBlack, class ICapture* captureEntWhite)
			: m_CaptureEntBlack(captureEntBlack)
			, m_CaptureEntWhite(captureEntWhite)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(void) {
			if (nullptr != m_CaptureEntBlack && nullptr != m_CaptureEntWhite) {

				if (const class advancedfx::IImageBuffer* pBufferEntBlack = m_CaptureEntBlack->GetBuffer()) {
					if (const unsigned char* pDataEntBlack = static_cast<const unsigned char*>(pBufferEntBlack->GetImageBufferData())) {
						m_pInDataEntBlack = pDataEntBlack;
						if (const class advancedfx::CImageFormat* pFormatEntBlack = pBufferEntBlack->GetImageBufferFormat()) {
							m_InFormatEntBlack = *pFormatEntBlack;
							if (m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGR) {

								if (const class advancedfx::IImageBuffer* pBufferEntWhite = m_CaptureEntWhite->GetBuffer()) {
									if (const unsigned char* pDataEntWhite = static_cast<const unsigned char*>(pBufferEntWhite->GetImageBufferData())) {
										m_pInDataEntWhite = pDataEntWhite;
										if (const class advancedfx::CImageFormat* pFormatEntWhite = pBufferEntWhite->GetImageBufferFormat()) {
											m_InFormatEntWhite = *pFormatEntWhite;
											if (
												(m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGR)
												&& m_InFormatEntWhite.Width == m_InFormatEntBlack.Width
												&& m_InFormatEntWhite.Height == m_InFormatEntBlack.Height
												&& m_InFormatEntWhite.Origin == m_InFormatEntBlack.Origin)
											{
												m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatEntBlack.Width, m_InFormatEntBlack.Height);
												m_OutFormat.SetOrigin(m_InFormatEntBlack.Origin);
												class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
												if (pOutCapture) {
													m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
												}
												return pOutCapture;
											}
										}
									}
								}

							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_OutFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter,
				m_pInDataEntBlack + taskIndex * m_InFormatEntBlack.Pitch, m_InFormatEntBlack.Pitch, m_InFormatEntBlack.GetPixelStride(),
				m_pInDataEntWhite + taskIndex * m_InFormatEntWhite.Pitch, m_InFormatEntWhite.Pitch, m_InFormatEntWhite.GetPixelStride(),
				m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pDataEntBlack, size_t pitchEntBlack, size_t pixelPichtEntBlack, const unsigned char* pDataEntWhite, size_t pitchEntWhite, size_t pixelPichtEntWhite, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pDataEntBlack(pDataEntBlack)
				, pitchEntBlack(pitchEntBlack)
				, pixelPichtEntBlack(pixelPichtEntBlack)
				, pDataEntWhite(pDataEntWhite)
				, pitchEntWhite(pitchEntWhite)
				, pixelPichtEntWhite(pixelPichtEntWhite)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 4 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char* pInEntBlack = (const unsigned char*)(pDataEntBlack + y * pitchEntBlack + x * pixelPichtEntBlack);
						const unsigned char* pInEntWhite = (const unsigned char*)(pDataEntWhite + y * pitchEntWhite + x * pixelPichtEntWhite);
						unsigned char* pOut = (unsigned char*)(pOutData + y * targetPitch + x * 4 * sizeof(unsigned char));

						unsigned char entBlack_b = pInEntBlack[0];
						unsigned char entBlack_g = pInEntBlack[1];
						unsigned char entBlack_r = pInEntBlack[2];

						unsigned char entWhite_b = pInEntWhite[0];
						unsigned char entWhite_g = pInEntWhite[1];
						unsigned char entWhite_r = pInEntWhite[2];

						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 0] = y < 1 * height / 3 ? entBlack_b : (y < 2 * height / 3 ? entWhite_b : (unsigned char)(((int)entBlack_b + (int)entWhite_b)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 1] = y < 1 * height / 3 ? entBlack_g : (y < 2 * height / 3 ? entWhite_g : (unsigned char)(((int)entBlack_g + (int)entWhite_g)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 2] = y < 1 * height / 3 ? entBlack_r : (y < 2 * height / 3 ? entWhite_r : (unsigned char)(((int)entBlack_r + (int)entWhite_r)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 3] = y < 1 * height / 3 ? 255 : (y < 2 * height / 3 ? 255 : (unsigned char)min(max((255l - (int)entWhite_b + (int)entBlack_b + 255l - (int)entWhite_g + (int)entBlack_g + 255l - (int)entWhite_r + (int)entBlack_r) / 3l, 0), 255));
						pOut[0] = (unsigned char)(((int)entBlack_b + (int)entWhite_b) / 2);
						pOut[1] = (unsigned char)(((int)entBlack_g + (int)entWhite_g) / 2);
						pOut[2] = (unsigned char)(((int)entBlack_r + (int)entWhite_r) / 2);
						pOut[3] = (unsigned char)std::min(std::max((255l - (int)entWhite_b + (int)entBlack_b + 255l - (int)entWhite_g + (int)entBlack_g + 255l - (int)entWhite_r + (int)entBlack_r) / 3l, 0l), 255l);

					}
				}
			}

		private:
			const unsigned char* pDataEntBlack;
			size_t pitchEntBlack;
			size_t pixelPichtEntBlack;
			const unsigned char* pDataEntWhite;
			size_t pitchEntWhite;
			size_t pixelPichtEntWhite;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_CaptureEntBlack;
		class ICapture* m_CaptureEntWhite;
		advancedfx::CImageFormat m_InFormatEntBlack;
		advancedfx::CImageFormat m_InFormatEntWhite;
		const unsigned char* m_pInDataEntBlack;
		const unsigned char* m_pInDataEntWhite;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformStripAlpha
		: public ITransform {
	public:
		CTransformStripAlpha(class ICapture* capture)
			: m_Capture(capture)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(void) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGR, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 3 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char * pIn = (const unsigned char *)(pData + y * pitch + x * 4 * sizeof(unsigned char));
						unsigned char * pOut = (unsigned char*)(pOutData + y * targetPitch + x * 3 * sizeof(unsigned char));
	
						pOut[0] = pIn[0];
						pOut[1] = pIn[1];
						pOut[2] = pIn[2];
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformDepthF 
		: public ITransform {
	public:
		CTransformDepthF(class ICapture* capture, float depthScale, float depthOfs)
			: m_Capture(capture)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(void) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::ZFloat) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual CAfxImageBufferCapture* CreateCapture(void) {
			class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
			if(pOutCapture) {
				m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
			}
			return pOutCapture;							
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize, m_DepthScale, m_DepthOfs);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, unsigned char* pOutData, size_t width, size_t height, float depthScale, float depthOfs)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
				, depthScale(depthScale)
				, depthOfs(depthOfs)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * sizeof(float);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						float depth = *(const float*)(pData + y * pitch + x * sizeof(float));

						depth *= depthScale;
						depth += depthOfs;

						*(float*)(pOutData + y * targetPitch + x * sizeof(float))
							= depth;
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
			float depthScale;
			float depthOfs;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
		float m_DepthScale;
		float m_DepthOfs;
	};

	class CTransformDepth24
		: public ITransform {
	public:
		CTransformDepth24(class ICapture* capture, float depthScale, float depthOfs)
			: m_Capture(capture)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(void) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::BGR || m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_InFormat.GetPixelStride(), m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize, m_DepthScale, m_DepthOfs);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, size_t pixelPitch, unsigned char* pOutData, size_t width, size_t height, float depthScale, float depthOfs)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pixelPitch(pixelPitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
				, depthScale(depthScale)
				, depthOfs(depthOfs)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * sizeof(float);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char b = pData[y * pitch + x * pixelPitch + 0];
						const unsigned char g = pData[y * pitch + x * pixelPitch + 1];
						const unsigned char r = pData[y * pitch + x * pixelPitch + 2];

						float depth;

						depth = (1.0f / 16777215.0f) * r + (256.0f / 16777215.0f) * g + (65536.0f / 16777215.0f) * b;

						depth *= depthScale;
						depth += depthOfs;

						*(float*)(pOutData + y * targetPitch + x * sizeof(float)) = depth;
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			size_t pixelPitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
			float depthScale;
			float depthOfs;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
		float m_DepthScale;
		float m_DepthOfs;
	};

	static class ICapture* Transform(class ITransform* transform) {
		if (class CAfxImageBufferCapture* pOutCapture = transform->CreateOutput()) {
			size_t outTaskSize = transform->GetTaskSize();
			size_t thread_count = std::min(g_pThreadPool->GetThreadCount() + 1, outTaskSize);
			size_t lines_per_task = outTaskSize / thread_count;
			size_t lines_per_task_remainder = outTaskSize % thread_count;
			std::atomic_int task_counter(0);
			size_t line = 0;
			if (1 < thread_count) {
				std::vector<advancedfx::CThreadPool::CTask*> tasks(thread_count - 1);
				for (size_t i = 0; i < tasks.size(); i++) {
					size_t cur_task_lines = lines_per_task;
					if (0 < lines_per_task_remainder) {
						cur_task_lines += 1;
						lines_per_task_remainder--;
					}
					tasks[i] = transform->CreateTask(task_counter, line, cur_task_lines);
					line += cur_task_lines;
				}
				g_pThreadPool->QueueTasks(tasks);
			}
			{
				advancedfx::CThreadPool::CTask* lastTask = transform->CreateTask(task_counter, line, lines_per_task);
				lastTask->Execute();
				delete lastTask;
			}
			while (0 < task_counter) {}

			return pOutCapture;
		}

		return nullptr;
	}
};

/*
void CAfxRenderViewStream::Console_DisableFastPathRequired()
{
	if (!GetDisableFastPath())
	{
		Tier0_Warning("You are using a feature that might require disabling fastpath, autoamtically setting disableFastPath 1 on this stream.\n");
		SetDisableFastPath(true);
	}
}
*/

// CAfxRecordStream::CCaptureFunctor ///////////////////////////////////////////

CAfxRecordStream::CCaptureFunctor::CCaptureFunctor(CAfxRecordStream& stream, size_t index)
	: m_Stream(stream)
	, m_Index(index)
{
	m_Stream.AddRef();
}

void CAfxRecordStream::CCaptureFunctor::operator()()
{
	bool bDepthF = false;
	switch (m_Stream.m_Streams[m_Index]->StreamCaptureType_get()) {
	case CAfxRenderViewStream::SCT_DepthF:
	case CAfxRenderViewStream::SCT_DepthFZIP:
		bDepthF = true;
		break;
	}
	IDirect3DSurface9* oldRenderTarget = nullptr;
	if (bDepthF) oldRenderTarget = AfxSetRenderTargetR32FDepthTexture();
	m_Stream.Capture(m_Index);
	if (bDepthF) AfxSetRenderTargetR32FDepthTexture_Restore(oldRenderTarget);

	m_Stream.Release();
}

// CAfxRecordStream ////////////////////////////////////////////////////////////

CAfxRecordStream::CAfxRecordStream(char const * streamName, std::vector<CAfxRenderViewStream *>&& streams)
	: CAfxStream()
	, m_StreamName(streamName)
	, m_Record(true)
	, m_OutVideoStream(nullptr)
	, m_Streams(streams)
{
	for (size_t i = 0; i < m_Streams.size(); ++i)
	{
		m_Streams[i]->AddRef();
	}
	m_CaptureNodes.resize(m_Streams.size());
	m_Buffers.resize(m_Streams.size());
}

CAfxRecordStream::~CAfxRecordStream()
{
	for (size_t i = 0; i < m_Streams.size(); ++i)
	{
		if (ICapture * capture = m_Buffers[i])
		{
			// TODO: This is probably too late.
			capture->Release();
		}
	}

	for (size_t i = 0; i < m_Streams.size(); ++i)
	{
		m_Streams[i]->Release();
	}

	m_Settings->Release();
}

void CAfxRecordStream::CaptureEnd()
{
	for (size_t i = 0; i < m_Buffers.size(); ++i)
	{
		if (ICapture*& captureRef = m_Buffers[i])
		{
			captureRef->Release();
			captureRef = nullptr;
		}
	}

	m_CapturesLeft--;
}

bool CAfxRecordStream::Record_get(void)
{
	return m_Record;
}

void CAfxRecordStream::Record_set(bool value)
{
	m_Record = value;
}

void CAfxRecordStream::RecordStart()
{
	if (m_Record)
	{
		m_Recording = true;
		m_FirstCapture = true;
		m_ProcessingThread = std::thread(&CAfxRecordStream::ProcessingThreadFunc, this);
	}
}

void CAfxRecordStream::RecordEnd()
{
	if (m_Recording) {
		{
			std::unique_lock<std::mutex> lock(m_ProcessingThreadMutex);
			m_Recording = false;
			m_ProcessingThreadCv.notify_one();
		}
		m_ProcessingThread.join();

		for (size_t i = 0; i < m_CaptureNodes.size(); ++i) {
			if (class CCaptureNode*& nodeRef = m_CaptureNodes[i]) {
				nodeRef->CpuQueueGpuRelease();
				nodeRef->Release();
				nodeRef = nullptr;
			}
		}

		if (m_OutVideoStream)
		{
			m_OutVideoStream->Release();
			m_OutVideoStream = nullptr;
		}
	}
}

char const * CAfxRecordStream::StreamName_get(void) const
{
	return m_StreamName.c_str();
}

void CAfxRecordStream::DoCaptureStart(IAfxMatRenderContextOrg * ctx, const AfxViewportData_t& viewport)
{
	m_CapturesLeft++;
	CaptureStart(m_FirstCapture, viewport);
	m_FirstCapture = false;
}

void CAfxRecordStream::OnImageBufferCaptured(size_t index, class ICapture* buffer)
{
	std::unique_lock<std::mutex> lock(m_ProcessingThreadMutex);
	if (buffer) buffer->AddRef();
	m_Buffers[index] = buffer;
	m_CapturesReady++;
	if (m_CapturesReady == m_Streams.size()) {
		m_ProcessingThreadCv.notify_one();
	}
}

bool CAfxRecordStream::Console_Edit_Head(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		char const * arg1 = args->ArgV(1);

		if (!_stricmp(arg1, "record"))
		{
			if (3 <= argC)
			{
				char const * arg2 = args->ArgV(2);

				this->Record_set(atoi(arg2) != 0 ? true : false);

				return true;
			}

			Tier0_Msg(
				"%s record 0|1 - Whether to record this stream with mirv_streams record - 0 = record off, 1 = RECORD ON.\n"
				"Current value: %s.\n"
				, arg0
				, this->Record_get() ? "1" : "0"
			);
			return true;
		}
		else if (0 == _stricmp("settings", arg1))
		{
			if (3 <= argC)
			{
				char const * arg2 = args->ArgV(2);

				if (CAfxRecordingSettings * settings = CAfxRecordingSettings::GetByName(arg2))
				{
					this->SetSettings(settings);
				}
				else
				{
					Tier0_Warning("AFXERROR: There is no recording setting named %s\n", arg2);
				}

				return true;
			}

			Tier0_Msg(
				"%s settings <name> - Set recording settings to use from mirv_streams settings.\n"
				"Current value: %s\n"
				, arg0
				, this->GetSettings()->GetName()
			);

			return true;
		}
	}

	return false;
}

void CAfxRecordStream::Console_Edit_Tail(IWrpCommandArgs * args)
{
	char const * arg0 = args->ArgV(0);

	Tier0_Msg("-- record properties --\n");
	Tier0_Msg("%s record [...] - Controls whether or not this stream is recorded with mirv_streams record.\n", arg0);
	Tier0_Msg("%s settings [...] - Recording settings to use.\n", arg0);
}


void CAfxRecordStream::QueueCapture(IAfxMatRenderContextOrg* ctx, size_t index)
{
	QueueOrExecute(ctx, new CAfxLeafExecute_Functor(new CCaptureFunctor(*this, index)));
}

// CAfxSingleStream ////////////////////////////////////////////////////////////

CAfxSingleStream::CAfxSingleStream(char const * streamName, CAfxRenderViewStream * stream)
	: CAfxRecordStream(streamName, std::vector<CAfxRenderViewStream *>({ stream }))
{
	m_Settings = CAfxRecordingSettings::GetDefault();
	m_Settings->AddRef();
}


void CAfxSingleStream::CaptureStart(bool bFirstCapture, const AfxViewportData_t& viewport) {
	CAfxRecordStream::CaptureStart(bFirstCapture, viewport);

	if (bFirstCapture) {
		/*bool isDepthCapture = false;
		switch (m_Streams[0]->StreamCaptureType_get()) {
		case CAfxRenderViewStream::SCT_DepthF:
		case CAfxRenderViewStream::SCT_DepthFZIP:
			isDepthCapture = true;
			break;
		}*/
		class CCaptureNode* node0 = new CCaptureNode(
			new CCaptureInputRenderTarget(),
			new CAfxStreamsCaptureOutput(this, 0));
		node0->AddRef();
		m_CaptureNodes[0] = node0;
	}
}

void CAfxSingleStream::CaptureEnd()
{
	if (ICapture *& capture = m_Buffers[0])
	{
		CAfxRenderViewStream::StreamCaptureType streamCaptureType = m_Streams[0]->StreamCaptureType_get();

		if (streamCaptureType == CAfxRenderViewStream::SCT_DepthF || streamCaptureType == CAfxRenderViewStream::SCT_DepthFZIP) {
			float depthScale = 1.0f;
			float depthOfs = 0.0f;
			if (CAfxBaseFxStream* baseFx = m_Streams[0]->AsAfxBaseFxStream())
			{
				depthScale = baseFx->DepthValMax_get() - baseFx->DepthVal_get();
				depthOfs = baseFx->DepthVal_get();
			}

			ICapture* outCapture = CAfxTransformer::TransformDepthF(capture, depthScale, depthOfs);
			if (capture) capture->Release();
			capture = outCapture;
		}
		else if (CAfxRenderViewStream::SCT_Depth24 == streamCaptureType || CAfxRenderViewStream::SCT_Depth24ZIP == streamCaptureType) {
			float depthScale = 1.0f;
			float depthOfs = 0.0f;
			if (CAfxBaseFxStream* baseFx = m_Streams[0]->AsAfxBaseFxStream())
			{
				depthScale = baseFx->DepthValMax_get() - baseFx->DepthVal_get();
				depthOfs = baseFx->DepthVal_get();
			}

			ICapture* outCapture = CAfxTransformer::TransformDepth24(capture, depthScale, depthOfs);
			if (capture) capture->Release();
			capture = outCapture;
		}
		else {
			ICapture* outCapture = CAfxTransformer::TransformStripAlpha(capture);
			if (capture) capture->Release();
			capture = outCapture;
		}

		if (capture) {
			if (const advancedfx::IImageBuffer* buffer = capture->GetBuffer()) {

				if (nullptr == m_OutVideoStream)
				{
					m_OutVideoStream = m_Settings->CreateOutVideoStream(g_AfxStreams, *this, *buffer->GetImageBufferFormat(), g_AfxStreams.GetStartHostFrameRate(), "");
					if (nullptr == m_OutVideoStream)
					{
						Tier0_Warning("AFXERROR: Failed to create image stream for %s.\n", this->StreamName_get());
					}
					else
					{
						m_OutVideoStream->AddRef();
					}
				}

				if (nullptr != m_OutVideoStream && !m_OutVideoStream->SupplyImageBuffer(buffer))
				{
					Tier0_Warning("AFXERROR: Failed writing image for stream %s.\n", this->StreamName_get());
				}
			}
			else {
				Tier0_Warning("AFXERROR: Could not get capture buffer for stream %s.\n", this->StreamName_get());
			}
		}
		else {
			Tier0_Warning("AFXERROR: Captured image transform failed for stream %s.\n", this->StreamName_get());
		}
	}

	CAfxRecordStream::CaptureEnd();
}

CAfxRenderViewStream::StreamCaptureType CAfxSingleStream::GetCaptureType() const
{
	return m_Streams[0]->StreamCaptureType_get();
}

bool CAfxSingleStream::Console_Edit_Head(IWrpCommandArgs * args)
{
	if (CAfxRecordStream::Console_Edit_Head(args))
		return true;

	if (g_AfxStreams.Console_EditStream(m_Streams[0], args))
		return true;

	return false;
}

void CAfxSingleStream::Console_Edit_Tail(IWrpCommandArgs * args)
{
	// ...

	CAfxRecordStream::Console_Edit_Tail(args);
}



// CAfxTwinStream //////////////////////////////////////////////////////////////

CAfxTwinStream::CAfxTwinStream(char const * streamName, CAfxRenderViewStream * streamA, CAfxRenderViewStream * streamB, StreamCombineType streamCombineType)
	: CAfxRecordStream(streamName, std::vector<CAfxRenderViewStream *>({streamA, streamB}))
	, m_StreamCombineType(streamCombineType)
{
	m_Settings = CAfxRecordingSettings::GetDefault();
	m_Settings->AddRef();
}

CAfxTwinStream::StreamCombineType CAfxTwinStream::StreamCombineType_get(void)
{
	return m_StreamCombineType;
}

void CAfxTwinStream::StreamCombineType_set(StreamCombineType value)
{
	m_StreamCombineType = value;
}

CAfxRenderViewStream::StreamCaptureType CAfxTwinStream::GetCaptureType() const
{
	if (CAfxTwinStream::SCT_ARedAsAlphaBColor == m_StreamCombineType)
	{
		return m_Streams[1]->StreamCaptureType_get();
	}
	else if (CAfxTwinStream::SCT_AColorBRedAsAlpha == m_StreamCombineType)
	{
		return m_Streams[0]->StreamCaptureType_get();
	}

	return CAfxRenderViewStream::SCT_Invalid;
}

void CAfxTwinStream::CaptureStart(bool bFirstCapture, const AfxViewportData_t& viewport) {
	CAfxRecordStream::CaptureStart(bFirstCapture, viewport);

	if (bFirstCapture) {
		class CCaptureNode* node0 = new CCaptureNode(static_cast<class ICaptureInput*>(new CCaptureInputRenderTarget()), new CAfxStreamsCaptureOutput(this, 0));
		node0->AddRef();
		m_CaptureNodes[0] = node0;
		class CCaptureNode* node1 = new CCaptureNode(static_cast<class ICaptureInput*>(new CCaptureInputRenderTarget()), new CAfxStreamsCaptureOutput(this, 1));
		node1->AddRef();
		m_CaptureNodes[1] = node1;
	}
}

void CAfxTwinStream::CaptureEnd()
{
	class ICapture*& captureA = m_Buffers[0];
	class ICapture*& captureB = m_Buffers[1];
	ICapture* outCapture = nullptr;

	if (CAfxTwinStream::SCT_ARedAsAlphaBColor == m_StreamCombineType)
	{
		outCapture = CAfxTransformer::TransformAColorBRedAsAlpha(captureB, captureA);
	}
	else if (CAfxTwinStream::SCT_AColorBRedAsAlpha == m_StreamCombineType)
	{
		outCapture = CAfxTransformer::TransformAColorBRedAsAlpha(captureA, captureB);
	}

	if (outCapture) {

		if (captureA) captureA->Release();
		captureA = nullptr;
		if (captureB) captureB->Release();
		captureB = nullptr;

		if (const advancedfx::IImageBuffer* buffer = outCapture->GetBuffer()) {

			if (nullptr == m_OutVideoStream)
			{
				m_OutVideoStream = m_Settings->CreateOutVideoStream(g_AfxStreams, *this, *buffer->GetImageBufferFormat(), g_AfxStreams.GetStartHostFrameRate(), "");
				if (nullptr == m_OutVideoStream)
				{
					Tier0_Warning("AFXERROR: Failed to create image stream for %s.\n", this->StreamName_get());
				}
				else
				{
					m_OutVideoStream->AddRef();
				}
			}

			if (nullptr != m_OutVideoStream && !m_OutVideoStream->SupplyImageBuffer(buffer))
			{
				Tier0_Warning("AFXERROR: Failed writing image for stream %s.\n", this->StreamName_get());
			}
		}
		else {
			Tier0_Warning("AFXERROR: Could not get capture buffer for stream %s.\n", this->StreamName_get());
		}

		outCapture->Release();

	}
	else {
		Tier0_Warning("CAfxTwinStream::CaptureEnd: Combining sub-streams for stream %s, failed.\n", this->StreamName_get());
	}

	CAfxRecordStream::CaptureEnd();
}

bool CAfxTwinStream::Console_Edit_Head(IWrpCommandArgs * args)
{
	if (CAfxRecordStream::Console_Edit_Head(args))
		return true;

	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp(arg1, "streamA"))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			g_AfxStreams.Console_EditStream(m_Streams[0], &subArgs);
			return true;
		}
		else if (0 == _stricmp(arg1, "streamB"))
		{
			CSubWrpCommandArgs subArgs(args,2);

			g_AfxStreams.Console_EditStream(m_Streams[1], &subArgs);
			return true;
		}
		else if (0 == _stricmp(arg1, "streamCombineType"))
		{
			if (3 <= argC)
			{
				char const * arg2 = args->ArgV(2);
				CAfxTwinStream::StreamCombineType value;

				if (g_AfxStreams.Console_ToStreamCombineType(arg2, value))
				{
					this->StreamCombineType_set(value);
					return true;
				}
			}

			Tier0_Msg(
				"%s streamCombineType " CAFXBASEFXSTREAM_STREAMCOMBINETYPES " - Set new combine type.\n"
				"Current value: %s.\n"
				, arg0
				, g_AfxStreams.Console_FromStreamCombineType(this->StreamCombineType_get())
			);
			return true;
		}
	}

	return false;
}

void CAfxTwinStream::Console_Edit_Tail(IWrpCommandArgs * args)
{
	char const * arg0 = args->ArgV(0);

	Tier0_Msg("-- twin properties --\n");
	Tier0_Msg("%s streamA [...] - Edit sub stream A.\n", arg0);
	Tier0_Msg("%s streamB [...] - Edit sub stream B.\n", arg0);
	Tier0_Msg("%s streamCombineType [...] - Controls how streams are combined.\n", arg0);

	CAfxRecordStream::Console_Edit_Tail(args);
}


// CAfxMatteStream /////////////////////////////////////////////////////////////

/*

entWhite = a * ent + (1 - a) * 1
entBlack = a * ent + (1 - a) * 0

entWhite - entBlack = 1 - a

---

entBlack = a * ent + (1 - a) * 0

entWhiteInv = 1 - entWhite
= 1 - [a*ent + (1 - a) * 1]
= 1 - a * ent - 1 + a
= a - a * ent
= (1 - a) * ent + a * 0

*/

class CAfxMatteStreamModifier : public IAfxBasefxStreamModifier
{
public:
	CAfxMatteStreamModifier(CAfxMatteStream * matteStream, size_t streamIndex)
		: m_MatteStream(matteStream)
		, m_StreamIndex(streamIndex)
	{
	}

	virtual void OverrideClearColor(unsigned char & outR, unsigned char & outG, unsigned char & outB, unsigned char & outA) override
	{
		m_MatteStream->OverrideClearColor(m_StreamIndex, outR, outG, outB, outA);
	}

	virtual CAfxBaseFxStream::CAction * OverrideAction(CAfxBaseFxStream::CAction * action) override
	{
		return m_MatteStream->OverrideAction(m_StreamIndex, action);
	}

private:
	CAfxMatteStream * m_MatteStream;
	size_t m_StreamIndex;
};

CAfxMatteStream::CAfxMatteStream(char const * streamName, CAfxRenderViewStream * stream)
	: CAfxRecordStream(streamName, std::vector<CAfxRenderViewStream *>({ stream, stream }))
	, m_HandleMaskAction(true)
{
	m_Settings = CAfxRecordingSettings::GetDefault();
	m_Settings->AddRef();

	m_Modifiers.resize(2);
	m_Modifiers[0] = new CAfxMatteStreamModifier(this, 0);
	m_Modifiers[1] = new CAfxMatteStreamModifier(this, 1);

	if (CAfxBaseFxStream::CAction * drawMatteAction = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey("drawMatte")))
	{
		drawMatteAction->AddRef();
		m_MatteActions.emplace(drawMatteAction);
	}

	if (CAfxBaseFxStream::CAction * noDrawMatteAction = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey("noDrawMatte")))
	{
		noDrawMatteAction->AddRef();
		m_NoMatteActions.emplace(noDrawMatteAction);
	}

	m_ActionBlack = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey("black"));
	if (m_ActionBlack) m_ActionBlack->AddRef();

	m_ActionWhite = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey("white"));
	if (m_ActionWhite) m_ActionWhite->AddRef();

	m_ActionNoDraw = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey("noDraw"));
	if (m_ActionNoDraw) m_ActionNoDraw->AddRef();
}

CAfxMatteStream::~CAfxMatteStream()
{
	for (size_t i = 0; i < m_Modifiers.size(); ++i) delete m_Modifiers[i];

	for (auto it = m_NoMatteActions.begin(); it != m_NoMatteActions.end(); ++it)
	{
		(*it)->Release();
	}

	for (auto it = m_MatteActions.begin(); it != m_MatteActions.end(); ++it)
	{
		(*it)->Release();
	}

	if (m_ActionWhite) m_ActionWhite->Release();
	if (m_ActionBlack) m_ActionBlack->Release();
	if (m_ActionNoDraw) m_ActionNoDraw->Release();
}

CAfxRenderViewStream::StreamCaptureType CAfxMatteStream::GetCaptureType() const
{
	return m_Streams[0]->StreamCaptureType_get();
}

void CAfxMatteStream::OverrideClearColor(size_t streamIndex, unsigned char & outR, unsigned char & outG, unsigned char & outB, unsigned char & outA)
{
	switch (streamIndex)
	{
	case 0: // entBlackBg
		outR = 0;
		outG = 0;
		outB = 0;
		outA = 255;
		return;
	case 1: // entWhiteBg
		outR = 255;
		outG = 255;
		outB = 255;
		outA = 255;
		return;
	}
}

CAfxBaseFxStream::CAction * CAfxMatteStream::OverrideAction(size_t streamIndex, CAfxBaseFxStream::CAction * action)
{
	if (m_MatteActions.end() == m_MatteActions.find(action))
	{
		if (m_NoMatteActions.end() != m_NoMatteActions.find(action))
			return m_ActionNoDraw;

		switch (streamIndex)
		{
		case 0: // entBlackBg
			return m_ActionBlack;
		default:
		case 1: // entWhiteBg
			return m_ActionWhite;
		}
	}

	return action;
}

bool CAfxMatteStream::Console_Edit_Head(IWrpCommandArgs * args)
{
	if (CAfxRecordStream::Console_Edit_Head(args))
		return true;

	if (g_AfxStreams.Console_EditStream(m_Streams[0], args))
		return true;

	return false;
}

void CAfxMatteStream::Console_Edit_Tail(IWrpCommandArgs * args)
{
	// ...

	CAfxRecordStream::Console_Edit_Tail(args);
}

void CAfxMatteStream::CaptureStart(bool bFirstCapture, const AfxViewportData_t& viewport) {
	CAfxRecordStream::CaptureStart(bFirstCapture, viewport);

	if (bFirstCapture) {
		class CCaptureNode* node0 = new CCaptureNode(static_cast<class ICaptureInput*>(new CCaptureInputRenderTarget()), new CAfxStreamsCaptureOutput(this, 0));
		node0->AddRef();
		m_CaptureNodes[0] = node0;
		class CCaptureNode* node1 = new CCaptureNode(static_cast<class ICaptureInput*>(new CCaptureInputRenderTarget()), new CAfxStreamsCaptureOutput(this, 1));
		node1->AddRef();
		m_CaptureNodes[1] = node1;
	}
}


void CAfxMatteStream::CaptureEnd()
{
	class ICapture*& captureA = m_Buffers[0];
	class ICapture*& captureB = m_Buffers[1];

	if(ICapture* outCapture = CAfxTransformer::TransformMatte(captureA, captureB)) {

		if (captureA) captureA->Release();
		captureA = nullptr;
		if (captureB) captureB->Release();
		captureB = nullptr;

		if (const advancedfx::IImageBuffer* buffer = outCapture->GetBuffer()) {

			if (nullptr == m_OutVideoStream)
			{
				m_OutVideoStream = m_Settings->CreateOutVideoStream(g_AfxStreams, *this, *buffer->GetImageBufferFormat(), g_AfxStreams.GetStartHostFrameRate(), "");
				if (nullptr == m_OutVideoStream)
				{
					Tier0_Warning("AFXERROR: Failed to create image stream for %s.\n", this->StreamName_get());
				}
				else
				{
					m_OutVideoStream->AddRef();
				}
			}

			if (nullptr != m_OutVideoStream && !m_OutVideoStream->SupplyImageBuffer(buffer))
			{
				Tier0_Warning("AFXERROR: Failed writing image for stream %s.\n", this->StreamName_get());
			}
		}
		else {
			Tier0_Warning("AFXERROR: Could not get capture buffer for stream %s.\n", this->StreamName_get());
		}

		outCapture->Release();

	} else {
		Tier0_Warning("CAfxMatteStream::CaptureEnd: Combining sub-streams for stream %s, failed.\n", this->StreamName_get());
	}

	CAfxRecordStream::CaptureEnd();
}

// CAfxBaseFxStream ////////////////////////////////////////////////////////////

CAfxBaseFxStream::CShared CAfxBaseFxStream::m_Shared;

CAfxBaseFxStream::CAfxBaseFxStream()
: CAfxRenderViewStream()
, m_TestAction(false)
, m_DepthVal(7)
, m_DepthValMax(2100)
, m_SmokeOverlayAlphaFactor(1)
, m_ShouldForceNoVisOverride(false)
, m_DebugPrint(false)
, m_ClientEffectTexturesAction(0)
, m_WorldTexturesAction(0)
, m_SkyBoxTexturesAction(0)
, m_StaticPropTexturesAction(0)
, m_CableAction(0)
, m_PlayerModelsAction(0)
, m_WeaponModelsAction(0)
, m_StatTrakAction(0)
, m_ShellModelsAction(0)
, m_OtherModelsAction(0)
, m_DecalTexturesAction(0)
, m_EffectsAction(0)
, m_ShellParticleAction(0)
, m_OtherParticleAction(0)
, m_StickerAction(0)
, m_ErrorMaterialAction(0)
, m_OtherAction(0)
, m_WriteZAction(0)
, m_DevAction(0)
, m_OtherEngineAction(0)
, m_OtherSpecialAction(0)
, m_VguiAction(0)
, m_InvalidateMap(false)
{
	m_Context = new CAfxBaseFxStreamContext(this);

	m_PickerMaterialsRleaseNotification = new CPickerMaterialsRleaseNotification(this);

	m_Shared.AddRef();

	DoBloomAndToneMapping = false;
	DoDepthOfField = false;

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
	SetAction(m_OtherAction, m_Shared.DrawAction_get());
	SetAction(m_WriteZAction, m_Shared.DrawAction_get());
	SetAction(m_DevAction, m_Shared.DrawAction_get());
	SetAction(m_OtherEngineAction, m_Shared.DrawAction_get());
	SetAction(m_OtherSpecialAction, m_Shared.DrawAction_get());
	SetAction(m_VguiAction, m_Shared.DrawAction_get());
}

CAfxBaseFxStream::~CAfxBaseFxStream()
{
	InvalidateMap();

	SetAction(m_ClientEffectTexturesAction, 0);
	SetAction(m_WorldTexturesAction, 0);
	SetAction(m_SkyBoxTexturesAction, 0);
	SetAction(m_StaticPropTexturesAction, 0);
	SetAction(m_CableAction, 0);
	SetAction(m_PlayerModelsAction, 0);
	SetAction(m_WeaponModelsAction, 0);
	SetAction(m_StatTrakAction, 0);
	SetAction(m_ShellModelsAction, 0);
	SetAction(m_OtherModelsAction, 0);
	SetAction(m_DecalTexturesAction, 0);
	SetAction(m_EffectsAction, 0);
	SetAction(m_ShellParticleAction, 0);
	SetAction(m_OtherParticleAction, 0);
	SetAction(m_StickerAction, 0);
	SetAction(m_ErrorMaterialAction, 0);
	SetAction(m_OtherAction, 0);
	SetAction(m_WriteZAction, 0);
	SetAction(m_DevAction, 0);
	SetAction(m_OtherEngineAction, 0);
	SetAction(m_OtherSpecialAction, 0);
	SetAction(m_VguiAction, 0);

	m_Shared.Release();

	delete m_PickerMaterialsRleaseNotification;

	delete m_Context;
}

void CAfxBaseFxStream::AfxStreamsInit(void)
{
	m_Shared.AfxStreamsInit();
}

void CAfxBaseFxStream::AfxStreamsShutdown(void)
{
	m_Shared.AfxStreamsShutdown();
}


void CAfxBaseFxStream::Console_ActionFilter_Add(const char * expression, CAction * action)
{
	InvalidateMap();
	m_ActionFilter.push_back(CActionFilterValue(expression,action));
}

void CAfxBaseFxStream::Console_ActionFilter_AddEx(CAfxStreams * streams, IWrpCommandArgs * args)
{
	CActionFilterValue * value = CActionFilterValue::Console_Parse(streams, args);
	if (value)
	{
		InvalidateMap();
		//if (value->GetUseEntity()) this->Console_DisableFastPathRequired();
		m_ActionFilter.push_back(*value);
		delete value;
	}
}

void CAfxBaseFxStream::Console_ActionFilter_Print(void)
{
	int id = 0;
	for(std::list<CActionFilterValue>::iterator it = m_ActionFilter.begin(); it != m_ActionFilter.end(); ++it)
	{
		it->Console_Print(id);

		++id;
	}
}

void CAfxBaseFxStream::Console_ActionFilter_Remove(int id)
{
	int curId = 0;
	for(std::list<CActionFilterValue>::iterator it = m_ActionFilter.begin(); it != m_ActionFilter.end(); ++it)
	{
		if(curId == id)
		{
			InvalidateMap();
			m_ActionFilter.erase(it);
			return;
		}

		++curId;
	}

	Tier0_Warning("Error: %i is not a valid actionFilter id!\n", id);
}

void CAfxBaseFxStream::Console_ActionFilter_Move(int id, int moveBeforeId)
{
	CActionFilterValue val;

	if(moveBeforeId < 0 || moveBeforeId > (int)m_ActionFilter.size())
	{
		Tier0_Msg("Error: %i is not in valid range for <targetId>\n");
		return;
	}

	{
		int curId = 0;
		std::list<CActionFilterValue>::iterator it = m_ActionFilter.begin();
		while(curId < id && it != m_ActionFilter.end())
		{
			++curId;
			++it;
		}

		if(it == m_ActionFilter.end())
		{
			Tier0_Warning("Error: %i is not a valid actionFilter id!\n");
			return;
		}

		val = *it;

		InvalidateMap();

		m_ActionFilter.erase(it);
	}

	{
		int curId = 0;
		std::list<CActionFilterValue>::iterator it = m_ActionFilter.begin();
		while(curId < moveBeforeId && it != m_ActionFilter.end())
		{
			++curId;
			++it;
		}

		m_ActionFilter.insert(it,val);
	}
}

void CAfxBaseFxStream::Console_ActionFilter_Clear()
{
	InvalidateMap();
	m_ActionFilter.clear();
}

void CAfxBaseFxStream::MainThreadInitialize(void)
{
	m_Shared.MainThreadInitialize();
}

void CAfxBaseFxStream::SetActive(bool value) {
	if(m_Active != value) {
		m_Active = value;
		InvalidateMap();
		if(value) {
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxNotifyStreamAdd(this)));
		} else {
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxNotifyStreamRemove(this)));
		}
	}
}

void CAfxBaseFxStream::OnEntityDeleted(SOURCESDK::IHandleEntity_csgo * entity) {
	m_Context->OnEntityDeleted(entity);
}

void CAfxBaseFxStream::LevelShutdown(void)
{
	Picker_Stop();
	InvalidateMap();
	if (m_Context) m_Context->InvalidateMap();
	m_Shared.LevelShutdown();
}

void CAfxBaseFxStream::OnRenderBegin(IAfxBasefxStreamModifier* modifier, const AfxViewportData_t& viewport, const SOURCESDK::VMatrix& projectionMatrix, const SOURCESDK::VMatrix& projectionMatrixSky)
{
	CAfxRenderViewStream::OnRenderBegin(modifier, viewport, projectionMatrix, projectionMatrixSky);

	m_Context->QueueBegin(CAfxBaseFxStreamData(
		modifier,
		viewport,
		projectionMatrix,
		projectionMatrixSky,
		m_InvalidateMap
	), true);
	m_InvalidateMap = false;
}

void CAfxBaseFxStream::OnRenderEnd()
{
	m_Context->QueueEnd(true);

	CAfxRenderViewStream::OnRenderEnd();
}

CAfxBaseFxStream::CAction* CAfxBaseFxStream::RetrieveAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity)
{
	CAction* action = nullptr;

	if (Picker_GetHidden(trackedMaterial, currentEntity))
	{
		action = GetAction(trackedMaterial, m_Shared.NoDrawAction_get());
	}

	return action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CAfxBaseFxStreamContext::RetrieveAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity)
{
	CAction * action = nullptr;

	std::map<CAfxTrackedMaterialRef, CCacheEntry>::iterator it = m_Map.find(trackedMaterial);

	if (it == m_Map.end())
	{
		it = m_Map.emplace(std::piecewise_construct, std::forward_as_tuple(trackedMaterial), std::forward_as_tuple()).first;
		trackedMaterial.Get()->AddNotifyee(m_MapRleaseNotification);
	}

	std::map<CEntityMetaRef, CCacheEntry::CachedData>::iterator itEnt = it->second.EntityActions.find(currentEntity);

	if (itEnt == it->second.EntityActions.end())
	{
		itEnt = it->second.EntityActions.emplace(std::piecewise_construct, std::forward_as_tuple(currentEntity), std::forward_as_tuple()).first;
		if(currentEntity) {
			m_EntityToCacheEntry[currentEntity].push_back(&(it->second));
		}
	}

	action = itEnt->second.Action;

	if (!action)
	{
		for (std::list<CActionFilterValue>::iterator itAf = m_Stream->m_ActionFilter.begin(); itAf != m_Stream->m_ActionFilter.end(); ++itAf)
		{
			if (itAf->CalcMatch_Material(trackedMaterial))
			{
				if (itAf->GetUseEntity())
				{
					if (currentEntity && itAf->CalcMatch_Entity(*currentEntity))
					{
						action = m_Stream->GetAction(trackedMaterial, itAf->GetMatchAction());
						action->AddRef();
						itEnt->second.Action = action;
						break;
					}
				}
				else
				{
					action = m_Stream->GetAction(trackedMaterial, itAf->GetMatchAction());
					action->AddRef();
					itEnt->second.Action = action;

					break;
				}
			}
		}

		if (!action)
		{
			action = m_Stream->GetAction(trackedMaterial, currentEntity);
			action->AddRef();
			itEnt->second.Action = action;
		}

		Assert(0 != action);

		if (m_Stream->m_DebugPrint)
		{
			SOURCESDK::IMaterialInternal_csgo* material = trackedMaterial.Get()->GetMaterial();

			const char* name = material->GetName();
			const char* groupName = material->GetTextureGroupName();
			const char* shaderName = material->GetShaderName();
			bool isErrorMaterial = material->IsErrorMaterial();
			const char* className = currentEntity ? currentEntity->ClassName.c_str() : "\\*";
			const char* modelName = currentEntity ? currentEntity->ModelName.c_str() : "\\*";
			bool isPlayer = currentEntity ? currentEntity->IsPlayer : false;
			int teamNumber = currentEntity ? currentEntity->TeamNumber : 0;

			Tier0_Msg("Stream: RetrieveAction: Material action cache miss: \"handle=%i\" \"name=%s\" \"textureGroup=%s\" \"shader=%s\" \"isErrrorMaterial=%u\" \"className=%s\" \"modelName=%s\" \"isPlayer=%i\" \"teamNumber=%i\" -> %s\n"
				, (currentEntity ? currentEntity->Handle : SOURCESDK_CSGO_INVALID_EHANDLE_INDEX)
				, name
				, groupName
				, shaderName
				, isErrorMaterial ? 1 : 0
				, className
				, modelName
				, isPlayer ? 1 : 0
				, teamNumber
				, action ? action->Key_get().m_Name.c_str() : "(null)");
		}
	}

	if(false)
	{
		SOURCESDK::IMaterialInternal_csgo* material = trackedMaterial.Get()->GetMaterial();

		const char * name = material->GetName();
		const char * groupName = material->GetTextureGroupName();
		const char * shaderName = material->GetShaderName();
		bool isErrorMaterial = material->IsErrorMaterial();

		Tier0_Msg("Info: \"handle=%i\" (not used) \"name=%s\" \"textureGroup=%s\" \"shader=%s\" \"isErrrorMaterial=%u\" -> %s\n"
			, (currentEntity ? currentEntity->Handle : SOURCESDK_CSGO_INVALID_EHANDLE_INDEX)
			, name
			, groupName
			, shaderName
			, isErrorMaterial ? 1 : 0
			, action ? action->Key_get().m_Name.c_str() : "(null)");
	}

	return action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::GetAction(const CAfxTrackedMaterialRef& trackedMaterial, CEntityMetaRef currentEntity)
{
	SOURCESDK::IMaterial_csgo * material = trackedMaterial.Get()->GetMaterial();

	const char * groupName =  material->GetTextureGroupName();
	const char * name = material->GetName();
	const char * shaderName = material->GetShaderName();
	bool isErrorMaterial = material->IsErrorMaterial();

	if(isErrorMaterial)
		return GetAction(trackedMaterial, m_ErrorMaterialAction);

	if (0 == strcmp(shaderName, "Character"))
		return GetAction(trackedMaterial, m_PlayerModelsAction);

	if(!strcmp("ClientEffect textures", groupName))
		return GetAction(trackedMaterial, m_ClientEffectTexturesAction);
	else
	if(!strcmp("Decal textures", groupName))
		return GetAction(trackedMaterial, m_DecalTexturesAction);
	else
	if(!strcmp("World textures", groupName))
		return GetAction(trackedMaterial, m_WorldTexturesAction);
	else
	if(!strcmp("SkyBox textures", groupName))
		return GetAction(trackedMaterial, m_SkyBoxTexturesAction);
	else
	if(!strcmp("StaticProp textures", groupName))
	{
		if(StringBeginsWith(name, "models/weapons/"))
			return GetAction(trackedMaterial, m_WeaponModelsAction);
		else
			return GetAction(trackedMaterial, m_StaticPropTexturesAction);
	}
	else
	if(!strcmp("Model textures", groupName))
	{
		if (StringBeginsWith(name, "models/player/"))
		{
			if(StringBeginsWith(name, "models/player/contactshadows/"))
				return GetAction(trackedMaterial, m_WorldTexturesAction);

			return GetAction(trackedMaterial, m_PlayerModelsAction);
		}
		else
		if(StringBeginsWith(name, "models/weapons/"))
		{
			if(StringBeginsWith(name, "models/weapons/stattrack/"))
				return GetAction(trackedMaterial, m_StatTrakAction);
			else if(StringBeginsWith(name, "models/weapons/w_models/arms/") || StringBeginsWith(name, "models/weapons/v_models/arms/"))
				return GetAction(trackedMaterial, m_PlayerModelsAction);
			else if(StringBeginsWith(name, "models/weapons/shared/shells/"))
				return GetAction(trackedMaterial, m_ShellModelsAction);
			else
				return GetAction(trackedMaterial, m_WeaponModelsAction);
		}
		else
		if(StringBeginsWith(name, "models/shells/"))
			return GetAction(trackedMaterial, m_ShellModelsAction);
		else
			return GetAction(trackedMaterial, m_OtherModelsAction);
	}
	else
	if(!strcmp("Other textures", groupName)
		||!strcmp("Precached", groupName))
	{
		if(StringBeginsWith(name, "__"))
			return GetAction(trackedMaterial, m_OtherSpecialAction);
		else
		if(StringBeginsWith(name, "cable/"))
			return GetAction(trackedMaterial, m_CableAction);
		else if (StringBeginsWith(name, "cs_custom_material_"))
			return GetAction(trackedMaterial, m_WeaponModelsAction);
		else
		if(StringBeginsWith(name, "engine/"))
		{
			if(!strcmp(name, "engine/writez"))
				return GetAction(trackedMaterial, m_WriteZAction);
			else
				return GetAction(trackedMaterial, m_OtherEngineAction);
		}
		else
		if(StringBeginsWith(name, "effects/"))
			return GetAction(trackedMaterial, m_EffectsAction);
		else
		if(StringBeginsWith(name, "dev/"))
			return GetAction(trackedMaterial, m_DevAction);
		else
		if(StringBeginsWith(name, "particle/"))
		{
			if(StringBeginsWith(name, "particle/shells/"))
				return GetAction(trackedMaterial, m_ShellParticleAction);
			else
				return GetAction(trackedMaterial, m_OtherParticleAction);
		}
		else
		if(StringBeginsWith(name, "sticker_"))
			return GetAction(trackedMaterial, m_StickerAction);
		else
		if(StringBeginsWith(name, "vgui"))
			return GetAction(trackedMaterial, m_VguiAction);
		else
			return GetAction(trackedMaterial, m_OtherAction);
	}
	else
	if(!strcmp("Particle textures", groupName))
	{
		if(StringBeginsWith(name, "particle/shells/"))
			return GetAction(trackedMaterial, m_ShellParticleAction);
		else
			return GetAction(trackedMaterial, m_OtherParticleAction);
	}
	else
	if(!strcmp(groupName, "VGUI textures"))
		return GetAction(trackedMaterial, m_VguiAction);

	if (currentEntity->IsPlayer)
		return GetAction(trackedMaterial, m_PlayerModelsAction);

	return GetAction(trackedMaterial, 0);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::GetAction(const CAfxTrackedMaterialRef& trackedMaterial, CAction * action)
{
	if(!action) action = m_Shared.DrawAction_get();
	action = action->ResolveAction(trackedMaterial);
	return action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::ClientEffectTexturesAction_get(void)
{
	return m_ClientEffectTexturesAction;
}

void CAfxBaseFxStream::ClientEffectTexturesAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_ClientEffectTexturesAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::WorldTexturesAction_get(void)
{
	return m_WorldTexturesAction;
}

void CAfxBaseFxStream::WorldTexturesAction_set(CAction *  value)
{
	SetActionAndInvalidateMap(m_WorldTexturesAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::SkyBoxTexturesAction_get(void)
{
	return m_SkyBoxTexturesAction;
}

void CAfxBaseFxStream::SkyBoxTexturesAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_SkyBoxTexturesAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::StaticPropTexturesAction_get(void)
{
	return m_StaticPropTexturesAction;
}

void CAfxBaseFxStream::StaticPropTexturesAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_StaticPropTexturesAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CableAction_get(void)
{
	return m_CableAction;
}

void CAfxBaseFxStream::CableAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_CableAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::PlayerModelsAction_get(void)
{
	return m_PlayerModelsAction;
}

void CAfxBaseFxStream::PlayerModelsAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_PlayerModelsAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::WeaponModelsAction_get(void)
{
	return m_WeaponModelsAction;
}

void CAfxBaseFxStream::WeaponModelsAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_WeaponModelsAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::StatTrakAction_get(void)
{
	return m_StatTrakAction;
}

void CAfxBaseFxStream::StatTrakAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_StatTrakAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::ShellModelsAction_get(void)
{
	return m_ShellModelsAction;
}

void CAfxBaseFxStream::ShellModelsAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_ShellModelsAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::OtherModelsAction_get(void)
{
	return m_OtherModelsAction;
}

void CAfxBaseFxStream::OtherModelsAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_OtherModelsAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::DecalTexturesAction_get(void)
{
	return m_DecalTexturesAction;
}

void CAfxBaseFxStream::DecalTexturesAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_DecalTexturesAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::EffectsAction_get(void)
{
	return m_EffectsAction;
}

void CAfxBaseFxStream::EffectsAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_EffectsAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::ShellParticleAction_get(void)
{
	return m_ShellParticleAction;
}

void CAfxBaseFxStream::ShellParticleAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_ShellParticleAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::OtherParticleAction_get(void)
{
	return m_OtherParticleAction;
}

void CAfxBaseFxStream::OtherParticleAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_OtherParticleAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::StickerAction_get(void)
{
	return m_StickerAction;
}

void CAfxBaseFxStream::StickerAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_StickerAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::ErrorMaterialAction_get(void)
{
	return m_ErrorMaterialAction;
}

void CAfxBaseFxStream::ErrorMaterialAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_ErrorMaterialAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::OtherAction_get(void)
{
	return m_OtherAction;
}

void CAfxBaseFxStream::OtherAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_OtherAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::WriteZAction_get(void)
{
	return m_WriteZAction;
}

void CAfxBaseFxStream::WriteZAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_WriteZAction, value);
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::DevAction_get(void)
{
	return m_DevAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::OtherEngineAction_get(void)
{
	return m_OtherEngineAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::OtherSpecialAction_get(void)
{
	return m_OtherSpecialAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::VguiAction_get(void)
{
	return m_VguiAction;
}

void CAfxBaseFxStream::VguiAction_set(CAction * value)
{
	SetActionAndInvalidateMap(m_VguiAction, value);
}

CAfxBaseFxStream::EClearBeforeHud CAfxBaseFxStream::ClearBeforeHud_get(void)
{
	return m_ClearBeforeHud;
}

void CAfxBaseFxStream::ClearBeforeHud_set(EClearBeforeHud value)
{
	m_ClearBeforeHud = value;
}

bool CAfxBaseFxStream::TestAction_get(void)
{
	return m_TestAction;
}

void CAfxBaseFxStream::TestAction_set(bool value)
{
	InvalidateMap();
	m_TestAction = value;
}

float CAfxBaseFxStream::DepthVal_get(void)
{
	return m_DepthVal;
}

void CAfxBaseFxStream::DepthVal_set(float value)
{
	m_DepthVal = value;
}

float CAfxBaseFxStream::DepthValMax_get(void)
{
	return m_DepthValMax;
}

void CAfxBaseFxStream::DepthValMax_set(float value)
{
	m_DepthValMax = value;
}

float CAfxBaseFxStream::SmokeOverlayAlphaFactor_get(void)
{
	return m_SmokeOverlayAlphaFactor;
}

void CAfxBaseFxStream::SmokeOverlayAlphaFactor_set(float value)
{
	m_SmokeOverlayAlphaFactor = value;
}

bool CAfxBaseFxStream::ShouldForceNoVisOverride_get(void)
{
	return m_ShouldForceNoVisOverride;
}

void CAfxBaseFxStream::ShouldForceNoVisOverride_set(bool value)
{
	m_ShouldForceNoVisOverride = value;
}


bool CAfxBaseFxStream::DebugPrint_get(void)
{
	return m_DebugPrint;
}

void CAfxBaseFxStream::DebugPrint_set(bool value)
{
	m_DebugPrint = value;
}

void CAfxBaseFxStream::InvalidateMap()
{
	m_InvalidateMap = true;
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::InvalidateMap()
{
	if(m_Stream->m_DebugPrint) Tier0_Msg("Stream: Invalidating material cache.\n");

	for(std::map<CAfxTrackedMaterialRef, CCacheEntry>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->first.Get()->RemoveNotifyee(m_MapRleaseNotification);
	}
	m_Map.clear();
	m_EntityToCacheEntry.clear();
}

void CAfxBaseFxStream::Picker_Stop(void)
{
	std::unique_lock<std::shared_timed_mutex> lock(m_PickerMutex);

	if(m_PickerActive)
	{
		for (auto it = m_PickerMaterials.begin(); it != m_PickerMaterials.end(); ++it)
		{
			it->first.Get()->RemoveNotifyee(m_PickerMaterialsRleaseNotification);
		}
		m_PickerMaterials.clear();

		m_PickerEntities.clear();

		m_PickerActive = false;

		Tier0_Msg("Picker stopped.\n");
	}
}

void CAfxBaseFxStream::Picker_Print(void)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_PickerMutex);

	Tier0_Msg("---- Materials: ----\n");
	for (auto it = m_PickerMaterials.begin(); it != m_PickerMaterials.end(); ++it)
	{
		int idx = it->second.Index;

		const CAfxTrackedMaterialRef& trackedMat = it->first;

		SOURCESDK::IMaterial_csgo * material = trackedMat.Get()->GetMaterial();

		Tier0_Msg("\"name=%s\" \"textureGroup=%s\" \"shader=%s\" \"isErrorMaterial=%i\" (%s)\n", material->GetName(), material->GetTextureGroupName(), material->GetShaderName(), material->IsErrorMaterial() ? 1 : 0, m_PickingMaterials && (1 == (idx & 0x1)) ? "hidden" : "visible");
	}
	Tier0_Msg("---- Entities: ----\n");
	for (auto it = m_PickerEntities.begin(); it != m_PickerEntities.end(); ++it)
	{
		int idx = it->second.Index;
		Tier0_Msg("\"handle=%i\" \"className=%s\" \"modelName=%s\" \"isPlayer=%i\" \"teamNumber=%i\" (%s)\n", (it->first ? it->first->Handle : SOURCESDK_CSGO_INVALID_EHANDLE_INDEX), (it->first ? it->first->ClassName.c_str() : "\\*"), (it->first ? it->first->ModelName.c_str() : "\\*"), (it->first ? (it->first->IsPlayer?1:0) : 0), (it->first ? it->first->TeamNumber : 0), m_PickingEntities && (1 == (idx & 0x1)) ? "hidden" : "visible");
	}
	Tier0_Msg("---- END ----\n");
}

void CAfxBaseFxStream::Picker_Pick(bool pickEntityNotMaterial, bool wasVisible)
{
	std::unique_lock<std::shared_timed_mutex> lock(m_PickerMutex);

	if (!m_PickerActive)
	{
		m_PickerActive = true;
		m_PickerCollecting = true;
	}
	else
	{
		if (m_PickerCollecting)
			m_PickerCollecting = false;

		if (m_PickingEntities)
		{
			std::set<CAfxTrackedMaterialRef> usedMats;
			int index = 0;

			for (auto it = m_PickerEntities.begin(); it != m_PickerEntities.end(); )
			{
				int oldIndex = it->second.Index;

				if ((1 == (oldIndex & 0x1)) == wasVisible)
				{
					it = m_PickerEntities.erase(it);
				}
				else
				{
					usedMats.insert(it->second.Materials.begin(), it->second.Materials.end());
					it->second.Index = index;
					++index;
					++it;
				}
			}

			for(auto it = m_PickerMaterials.begin(); it != m_PickerMaterials.end(); )
			{
				if (usedMats.end() != usedMats.find(it->first))
					++it;
				else
				{
					it->first.Get()->RemoveNotifyee(m_PickerMaterialsRleaseNotification);
					it = m_PickerMaterials.erase(it);
				}
			}
		}

		if (m_PickingMaterials)
		{
			std::set<CEntityMetaRef> usedEnts;
			int index = 0;

			for (auto it = m_PickerMaterials.begin(); it != m_PickerMaterials.end(); )
			{
				int oldIndex = it->second.Index;

				if ((1 == (oldIndex & 0x1)) == wasVisible)
				{
					it->first.Get()->RemoveNotifyee(m_PickerMaterialsRleaseNotification);
					it = m_PickerMaterials.erase(it);
				}
				else
				{
					usedEnts.insert(it->second.Entities.begin(), it->second.Entities.end());
					it->second.Index = index;
					++index;
					++it;
				}
			}

			for (auto it = m_PickerEntities.begin(); it != m_PickerEntities.end(); )
			{
				if (usedEnts.end() != usedEnts.find(it->first))
					++it;
				else
					it = m_PickerEntities.erase(it);
			}
		}

		bool determinedEntities = m_PickerEntities.size() <= 1;
		bool determinedMaterials = m_PickerMaterials.size() <= 1;

		if (pickEntityNotMaterial)
		{
			if (determinedEntities)
			{
				m_PickingEntities = false;
				m_PickingMaterials = false;

				lock.unlock();
				Picker_Print();
				Tier0_Warning("==== Entity%s determined! ====\n", determinedMaterials ? " and Material" : "");
				if (g_VEngineClient && !g_VEngineClient->Con_IsVisible()) g_VEngineClient->ClientCmd_Unrestricted("toggleconsole");
				return;
			}
		}
		else
		{
			if (determinedMaterials)
			{
				m_PickingEntities = false;
				m_PickingMaterials = false;

				lock.unlock();
				Picker_Print();
				Tier0_Warning("==== %sMaterial determined! ====\n", determinedEntities ? "Entity and " : "");
				if (g_VEngineClient && !g_VEngineClient->Con_IsVisible()) g_VEngineClient->ClientCmd_Unrestricted("toggleconsole");
				return;
			}
		}
	}

	m_PickingEntities = pickEntityNotMaterial;
	m_PickingMaterials = !pickEntityNotMaterial;
}

bool CAfxBaseFxStream::Picker_GetHidden(const CAfxTrackedMaterialRef& tackedMaterial, CEntityMetaRef currentEntity)
{
	if (!m_PickerActive)
		return false;

	m_PickerMutex.lock_shared();

	if (!m_PickerActive)
	{
		m_PickerMutex.unlock_shared();
		return false;
	}

	bool uniqueLocked = false;

	if (m_PickerCollecting)
	{
		m_PickerMutex.unlock_shared();
		m_PickerMutex.lock();

		uniqueLocked = true;
	}

	bool hidden = false;

	if (m_PickerActive)
	{
		if (!m_PickerCollecting)
		{
			if (!hidden && m_PickingMaterials)
			{
				std::map<CAfxTrackedMaterialRef, CPickerMatValue>::iterator itMat = m_PickerMaterials.find(tackedMaterial);
				hidden = (m_PickerMaterials.end() != itMat) && (((itMat->second.Index) & 0x1) == 1);
			}
			if (!hidden && m_PickingEntities)
			{
				auto itEnt = m_PickerEntities.find(currentEntity);
				hidden = (m_PickerEntities.end() != itEnt) && (((itEnt->second.Index) & 0x1) == 1);
			}
		}
		else
		{
			std::map<CAfxTrackedMaterialRef, CPickerMatValue>::iterator itMat = m_PickerMaterials.lower_bound(tackedMaterial);
			if (itMat == m_PickerMaterials.end() || (tackedMaterial < itMat->first))
			{
				itMat = m_PickerMaterials.emplace_hint(itMat,std::piecewise_construct, std::forward_as_tuple(tackedMaterial), std::forward_as_tuple(m_PickerMaterials.size(), currentEntity));
				tackedMaterial.Get()->AddNotifyee(m_PickerMaterialsRleaseNotification);
			}
			else
			{
				itMat->second.Entities.insert(currentEntity);
			}

			auto itEnt = m_PickerEntities.lower_bound(currentEntity);
			if (itEnt == m_PickerEntities.end() || (currentEntity < itEnt->first))
			{
				itEnt = m_PickerEntities.emplace_hint(itEnt, std::piecewise_construct, std::forward_as_tuple(currentEntity), std::forward_as_tuple(this, m_PickerEntities.size(), tackedMaterial));
			}
			else
			{
				std::set<CAfxTrackedMaterialRef>::iterator itEntMats = itEnt->second.Materials.lower_bound(tackedMaterial);
				if (itEntMats == itEnt->second.Materials.end() || (tackedMaterial < *itEntMats))
				{
					itEnt->second.Materials.emplace_hint(itEntMats, tackedMaterial);
					tackedMaterial.Get()->AddNotifyee(&(itEnt->second));
				}
			}

			hidden =
				(m_PickingEntities && (((itEnt->second.Index) & 0x1) == 1))
				|| (m_PickingMaterials && (((itMat->second.Index) & 0x1) == 1))
				;
		}

	}

	if (!uniqueLocked)
		m_PickerMutex.unlock_shared();
	else
		m_PickerMutex.unlock();

	return hidden;
}

/*
void CAfxBaseFxStream::ConvertStreamDepth(bool to24, bool depth24ZIP)
{
	ConvertDepthActions(to24);

	m_StreamCaptureType =
		to24 ? (depth24ZIP ? SCT_Depth24ZIP : SCT_Depth24) : SCT_Normal;
}

void CAfxBaseFxStream::ConvertDepthActions(bool to24)
{
	InvalidateMap();

	ConvertDepthAction(m_ClientEffectTexturesAction, to24);
	ConvertDepthAction(m_WorldTexturesAction, to24);
	ConvertDepthAction(m_SkyBoxTexturesAction, to24);
	ConvertDepthAction(m_StaticPropTexturesAction, to24);
	ConvertDepthAction(m_CableAction, to24);
	ConvertDepthAction(m_PlayerModelsAction, to24);
	ConvertDepthAction(m_WeaponModelsAction, to24);
	ConvertDepthAction(m_StatTrakAction, to24);
	ConvertDepthAction(m_ShellModelsAction, to24);
	ConvertDepthAction(m_OtherModelsAction, to24);
	ConvertDepthAction(m_DecalTexturesAction, to24);
	ConvertDepthAction(m_EffectsAction, to24);
	ConvertDepthAction(m_ShellParticleAction, to24);
	ConvertDepthAction(m_OtherParticleAction, to24);
	ConvertDepthAction(m_StickerAction, to24);
	ConvertDepthAction(m_ErrorMaterialAction, to24);
	ConvertDepthAction(m_OtherAction, to24);
	ConvertDepthAction(m_WriteZAction, to24);
	ConvertDepthAction(m_DevAction, to24);
	ConvertDepthAction(m_OtherEngineAction, to24);
	ConvertDepthAction(m_OtherSpecialAction, to24);
	ConvertDepthAction(m_VguiAction, to24);
}

void CAfxBaseFxStream::ConvertDepthAction(CAction * & action, bool to24)
{
	if(!action)
		return;

	char const * name = action->Key_get().m_Name.c_str();
	bool isDepth = !strcmp(name, "drawDepth");
	bool isDepth24 = !strcmp(name, "drawDepth24");

	if(to24)
	{
		if(isDepth) SetAction(action, m_Shared.Depth24Action_get());
	}
	else
	{
		if(isDepth24) SetAction(action, m_Shared.DepthAction_get());
	}
}
*/

void CAfxBaseFxStream::SetActionAndInvalidateMap(CAction * & target, CAction * src)
{
	InvalidateMap();
	SetAction(target, src);
}

void CAfxBaseFxStream::SetAction(CAction * & target, CAction * src)
{
	if(target) target->Release();
	if(src) src->AddRef();
	target = src;
}

// CAfxBaseFxStream::CAfxBaseFxStreamContext ///////////////////////////////


void CAfxBaseFxStream::CAfxBaseFxStreamContext::QueueBegin(const CAfxBaseFxStreamData& data, bool isRoot)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	ctx->Hook_set(this);

	if (isRoot)
	{
		m_RootContext = ctx;
		m_Stream->AddRef();
		m_Stream->EnterCritical();
	}

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		queue->QueueFunctor(new CQueueBeginFunctor(this, data));
	}
	else
	{
		// Is leaf context.

		if (m_Data.InvalidateMap)
		{
			m_Data.InvalidateMap = false;
			InvalidateMap();
		}

		m_MapMutex.lock();

		m_Data = data;
		m_IsNextDepth = false;
		m_DrawingHud = false;
		m_DrawingSkyBoxView = false;
		m_CurrentEntityMeta = nullptr;
		m_CurrentEntityMetaOrg = nullptr;

		bool bDrawDepth = EDrawDepth_None != this->GetStream()->m_DrawDepth;
		bool bReShade = this->GetStream()->ReShadeEnabled_get();

		if (bDrawDepth || bReShade)
		{
			if (bDrawDepth) AfxD3D9PushOverrideState(false);

			g_AfxStreams.DrawingThread_SetIntZTextureSurface();

			if (g_AfxStreams.DrawingThread_HasRenderTargetMsaa())
				g_AfxStreams.DrawingThread_SetRenderTargetNoMsaa();
			
			if(bDrawDepth) AfxD3D9OverrideBegin_D3DRS_COLORWRITEENABLE(0);

		}

		if (m_Stream->GetClearBeforeRender())
		{
			auto orgCtx = ctx->GetOrg();

			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;
			unsigned char a = 255;

			if (m_Data.Modifier)
			{
				m_Data.Modifier->OverrideClearColor(r, g, b, a);
			}

			orgCtx->ClearColor4ub(r, g, b, a);
			orgCtx->ClearBuffers(true, false, false);
		}
	}
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::QueueEnd(bool isRoot)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	// These need to happen before switching to new Queue of course:
	SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue();

	if (queue)
	{
		queue->QueueFunctor(new CQueueEndFunctor(this));
	}
	else
	{
		// Is leaf context.

		m_CurrentMaterial = nullptr;
		m_CurrentMaterialOrg = nullptr;

		BindAction(0);

		bool bDrawDepth = EDrawDepth_None != this->GetStream()->m_DrawDepth;
		bool bReShade = this->GetStream()->ReShadeEnabled_get();

		if (bDrawDepth || bReShade)
		{
			if(bDrawDepth) AfxD3D9OverrideEnd_D3DRS_COLORWRITEENABLE();

			if (g_AfxStreams.DrawingThread_HasRenderTargetMsaa())
				g_AfxStreams.DrawingThread_UnsetRenderTargetNoMsaa(bDrawDepth);

			g_AfxStreams.DrawingThread_UnsetIntZTextureSurface();

			if (bDrawDepth) AfxD3D9PopOverrideState();
		}

		m_MapMutex.unlock();

		m_Stream->ExitCritical();
		m_Stream->Release();
	}

	if (isRoot)
	{
		m_RootContext = nullptr;
	}

	ctx->Hook_set(nullptr);
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor)
{
	this->IfRootThenUpdateCurrentEntity(nullptr);
	//aq->GetParent()->QueueFunctor(new CAfxLeafExecute_Functor(new CAfxD3D9PushOverrideState_Functor(true)));
	aq->GetParent()->QueueFunctorInternal(pFunctor);
	//aq->GetParent()->QueueFunctor(new CAfxLeafExecute_Functor(new CAfxD3D9PopOverrideState_Functor()));
}

bool CAfxBaseFxStream::CAfxBaseFxStreamContext::ViewRenderShouldForceNoVis(bool orgValue)
{
	return m_Stream->m_ShouldForceNoVisOverride ? true : orgValue;
}

AfxDrawDepthMode AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthMode(CAfxBaseFxStream::EDrawDepthMode value)
{
	switch (value)
	{
	case CAfxBaseFxStream::EDrawDepthMode_Inverse:
		return AfxDrawDepthMode_Inverse;
	case CAfxBaseFxStream::EDrawDepthMode_Linear:
		return AfxDrawDepthMode_Linear;
	case CAfxBaseFxStream::EDrawDepthMode_LogE:
		return AfxDrawDepthMode_LogE;
	case CAfxBaseFxStream::EDrawDepthMode_PyramidalLinear:
		return AfxDrawDepthMode_PyramidalLinear;
	case CAfxBaseFxStream::EDrawDepthMode_PyramidalLogE:
		return AfxDrawDepthMode_PyramidalLogE;
	}

	return AfxDrawDepthMode_Linear;
}

AfxDrawDepthEncode AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthEncode(CAfxBaseFxStream::EDrawDepth value)
{
	switch (value) {
	case CAfxBaseFxStream::EDrawDepth_None:
		return AfxDrawDepthEncode_Gray;
	case CAfxBaseFxStream::EDrawDepth_Gray:
		return AfxDrawDepthEncode_Gray;
	case CAfxBaseFxStream::EDrawDepth_Rgb:
		return AfxDrawDepthEncode_Rgb;
	case CAfxBaseFxStream::EDrawDepth_Dithered:
		return AfxDrawDepthEncode_Dithered;
	}

	return AfxDrawDepthEncode_Gray;
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawingHudBegin(void)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		// Bubble into child contexts:

		queue->QueueFunctor(new CDrawingHudBeginFunctor(this));
	}
	else
	{
		// Leaf context

		m_DrawingHud = true;

		BindAction(0);

		bool bDepthF = false;
		switch (m_Stream->StreamCaptureType_get()) {
		case CAfxRenderViewStream::SCT_DepthF:
		case CAfxRenderViewStream::SCT_DepthFZIP:
			bDepthF = true;
			break;
		}
		bool bDrawDepth = EDrawDepth_None != m_Stream->m_DrawDepth;
		bool bReShade = m_Stream->ReShadeEnabled_get();

		if (bDrawDepth || bDepthF || bReShade)
		{
			float flDepthFactor = (bDrawDepth || bDepthF) ? m_Stream->m_DepthVal : m_Data.Viewport.zNear;
			float flDepthFactorMax = (bDrawDepth || bDepthF) ? m_Stream->m_DepthValMax : m_Data.Viewport.zFar;
			AfxDrawDepthEncode drawDepthEncode = bDrawDepth && !bDepthF ? AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthEncode(m_Stream->DrawDepth_get()) : AfxDrawDepthEncode_Gray;
			AfxDrawDepthMode drawDepthMode = (bDrawDepth || bDepthF) ? AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthMode(m_Stream->DrawDepthMode_get()) : AfxDrawDepthMode_Inverse;

			IDirect3DSurface9* oldRenderTarget = nullptr;
			if(bDepthF || bReShade) oldRenderTarget = AfxSetRenderTargetR32FDepthTexture();
			AfxDrawDepth(
				drawDepthEncode,
				drawDepthMode,
				m_IsNextDepth,
				flDepthFactor, flDepthFactorMax,
				m_Data.Viewport.x, m_Data.Viewport.y, m_Data.Viewport.width, m_Data.Viewport.height, m_Data.Viewport.zNear, m_Data.Viewport.zFar,
				(bDrawDepth || bDepthF) ? m_Data.ProjectionMatrix.m : nullptr);
			if (bDepthF || bReShade) AfxSetRenderTargetR32FDepthTexture_Restore(oldRenderTarget);
			m_IsNextDepth = true;
		}

		if (bReShade) {
			g_ReShadeAdvancedfx.AdvancedfxRenderEffects(AfxGetRenderTargetSurface(), AfxGetR32FDepthTexture());
		}
		
		// Do the clearing if wanted:

		switch (m_Stream->m_ClearBeforeHud)
		{
		case EClearBeforeHud_Black:
			ctx->GetOrg()->ClearColor4ub(0, 0, 0, 255);
			ctx->GetOrg()->ClearBuffers(true, false, false);
			break;
		case EClearBeforeHud_White:
			ctx->GetOrg()->ClearColor4ub(255, 255, 255, 255);
			ctx->GetOrg()->ClearBuffers(true, false, false);
			break;
		}
	}
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawingHudEnd(void)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		// Bubble into child contexts:

		queue->QueueFunctor(new CDrawingHudEndFunctor(this));
	}
	else
	{
		// Leaf context

		m_DrawingHud = false;

		BindAction(0);
	}
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawingSkyBoxViewBegin(void)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		// Bubble into child contexts:

		queue->QueueFunctor(new CDrawingSkyBoxViewBeginFunctor(this));
	}
	else
	{
		// Leaf context

		BindAction(0);

		m_DrawingSkyBoxView = true;
	}
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawingSkyBoxViewEnd(void)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		// Bubble into child contexts:

		queue->QueueFunctor(new CDrawingSkyBoxViewEndFunctor(this));
	}
	else
	{
		// Leaf context

		m_DrawingSkyBoxView = false;

		BindAction(0);

		bool bDepthF = false;
		switch (m_Stream->StreamCaptureType_get()) {
		case CAfxRenderViewStream::SCT_DepthF:
		case CAfxRenderViewStream::SCT_DepthFZIP:
			bDepthF = true;
			break;
		}
		bool bDrawDepth = EDrawDepth_None != m_Stream->m_DrawDepth;
		bool bReShade = m_Stream->ReShadeEnabled_get();

		if (bDrawDepth || bDepthF || bReShade)
		{
			int scale = csgo_CSkyBoxView_GetScale();

			float flDepthFactor = (bDrawDepth || bDepthF) ? m_Stream->m_DepthVal : m_Data.Viewport.zNear;
			float flDepthFactorMax = (bDrawDepth || bDepthF) ? m_Stream->m_DepthValMax : m_Data.Viewport.zFar;
			AfxDrawDepthEncode drawDepthEncode = bDrawDepth && !bDepthF ? AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthEncode(m_Stream->DrawDepth_get()) : AfxDrawDepthEncode_Gray;
			AfxDrawDepthMode drawDepthMode = (bDrawDepth || bDepthF) ? AfxBasefxStreamDrawDepthMode_To_AfxDrawDepthMode(m_Stream->DrawDepthMode_get()) : AfxDrawDepthMode_Inverse;

			IDirect3DSurface9* oldRenderTarget = nullptr;
			if (bDepthF || bReShade) oldRenderTarget = AfxSetRenderTargetR32FDepthTexture();
			AfxDrawDepth(
				drawDepthEncode,
				drawDepthMode,
				m_IsNextDepth,
				flDepthFactor, flDepthFactorMax,
				m_Data.Viewport.x, m_Data.Viewport.y, m_Data.Viewport.width, m_Data.Viewport.height, 2.0f * scale, (float)SOURCESDK_CSGO_MAX_TRACE_LENGTH * scale,
				(bDrawDepth || bDepthF) ? m_Data.ProjectionMatrixSky.m : nullptr);
			if (bDepthF || bReShade) AfxSetRenderTargetR32FDepthTexture_Restore(oldRenderTarget);

			m_IsNextDepth = true;
		}
	}
}

bool Pt_Inside(int x, int y, SOURCESDK::vrect_t_csgo * rect)
{
	return
		x >= rect->x
		&& y >= rect->y
		&& x < (rect->x + rect->width)
		&& y < (rect->y + rect->height);

}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::UpdateCurrentEntity(CEntityMetaRef currentEntity)
{
	IAfxMatRenderContext* ctx = GetCurrentContext();

	if (SOURCESDK::CSGO::ICallQueue * queue = ctx->GetOrg()->GetCallQueue())
	{
		// Bubble into child contexts:

		queue->QueueFunctor(new CUpdateCurrentEnitityHandleFunctor(this, currentEntity));
	}
	else
	{
		// Leaf context

		m_CurrentEntityMeta = currentEntity;
		m_CurrentEntityMetaOrg = currentEntity;
	}
}

typedef void* (__cdecl* csgo_client_dynamic_cast_t)(void* from, void* unk1, void* baseClassRtti, void* classRtti, void* unk2);

void CAfxBaseFxStream::UpdateCurrentEntity(void* proxyData)
{
	/*
	if (proxyData)
	{
		if (csgo_client_dynamic_cast_t csgo_client_dynamic_cast = (csgo_client_dynamic_cast_t)AFXADDR_GET(csgo_client_dynamic_cast))
		{
			if (void* rttiIClientRenderAble = (void*)AFXADDR_GET(csgo_client_RTTI_IClientRenderable))
			{
				if (SOURCESDK::IClientRenderable_csgo* re = (SOURCESDK::IClientRenderable_csgo*)csgo_client_dynamic_cast(proxyData, 0, rttiIClientRenderAble, rttiIClientRenderAble, 0))
				{
					if (SOURCESDK::IClientUnknown_csgo* unk = re->GetIClientUnknown())
					{
						SOURCESDK::CSGO::CBaseHandle current = unk->GetRefEHandle();
						if (m_CurrentEntity == current) return;

						info.Handle.AfxAssign(current);

						if (SOURCESDK::C_BaseEntity_csgo* be = unk->GetBaseEntity())
						{
							info.Data.ClassName = be->GetClassname();
							info.Data.IsPlayer = be->IsPlayer();
							info.Data.TeamNumber = be->GetTeamNumber();
						}
					}
				}
			}
		}
	}
	*/
	if (SOURCESDK::IViewRender_csgo* view = GetView_csgo())
	{

		if (SOURCESDK::C_BaseEntity_csgo* be = view->GetCurrentlyDrawingEntity())
		{
			SOURCESDK::CSGO::CBaseHandle current = be->GetRefEHandle();

			CEntityMetaRef info(std::unique_ptr<CEntityMeta>(new CEntityMeta(be)));

			info->Handle.AfxAssign(current);
			info->ClassName = be->GetClassname();
			info->IsPlayer = be->IsPlayer();
			info->TeamNumber = be->GetTeamNumber();
			if(g_pModelInfo) {
				const SOURCESDK::CSGO::model_t * pModel = be->GetModel();
				if(pModel) {
					info->ModelName = g_pModelInfo->GetModelName(pModel);
				}
			}
			if(m_CurrentEntityMeta != info) {
				m_Context->UpdateCurrentEntity(info);
			}
			m_CurrentEntityMeta = info;		
			return;
		}
	}

	if(m_CurrentEntityMeta) {
		m_Context->UpdateCurrentEntity(CEntityMetaRef());
	}
	m_CurrentEntityMeta = CEntityMetaRef();
}

SOURCESDK::IMaterial_csgo * CAfxBaseFxStream::CAfxBaseFxStreamContext::MaterialHook(IAfxMatRenderContext* ctx, SOURCESDK::IMaterial_csgo * material, void * proxyData)
{
	this->IfRootThenUpdateCurrentEntity(proxyData);

	if (nullptr == ctx->GetOrg()->GetCallQueue())
	{
		// This means we are on the rendering thread and can go through with the current material.

		m_CurrentMaterialOrg = material;
		
		CAfxTrackedMaterialRef trackedMaterial(CAfxTrackedMaterial::TrackMaterial(material));	

		CAction * action = m_Stream->RetrieveAction(
			trackedMaterial
			, m_CurrentEntityMeta
		);

		if (nullptr == action) action = this->RetrieveAction(trackedMaterial, m_CurrentEntityMeta);

		if (m_Data.Modifier)
		{
			action = m_Data.Modifier->OverrideAction(action);
		}

		BindAction(action);

		if (m_CurrentAction)
		{
			m_CurrentAction->MaterialHook(this, ctx, trackedMaterial);
		}

		if (SOURCESDK::IMaterial_csgo * replacementMaterial = trackedMaterial.Get()->GetReplacement())
			m_CurrentMaterial = replacementMaterial;
		else
			m_CurrentMaterial = material;

		m_CurrentProxyData = proxyData;
		return m_CurrentMaterial;
	}

	return material;
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::OnLockRenderData(IAfxMatRenderContext* ctx, int nSizeInBytes, void * ptr) {

	if(0 < g_DrawingThread_CurrentMeta.size() && g_AfxStreams.OnEngineThread()) {
		if( g_DrawingThread_CurrentMeta.size()) {
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxUpdateEntityMetaFunctor((SOURCESDK::matrix3x4_t *)ptr, g_DrawingThread_CurrentMeta.front())));			
			g_DrawingThread_CurrentMeta.pop_front();
		}
	}

}


void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawInstances(IAfxMatRenderContext* ctx, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance) {

	if(nullptr != ctx->GetOrg()->GetCallQueue()) {
		// Not leaf context.
		ctx->GetOrg()->DrawInstances(nInstanceCount, pInstance);
		return;
	}

	CAfxTrackedMaterialRef trackedMaterial(CAfxTrackedMaterial::TrackMaterial(m_CurrentMaterialOrg));		

	int m_LastDrawnCount = 0;
	m_CurrentEntityMeta = m_CurrentEntityMetaOrg;
	m_CurrentMaterial = m_CurrentMaterialOrg;
	int i;
	CAction * orgAction = m_CurrentAction;
	orgAction->AddRef();
	for(i=0;i<nInstanceCount;++i) {
		CEntityMetaRef currentEntityI = m_CurrentEntityMetaOrg;
		auto result = g_DrawingThread_BonesPtr_To_Meta.find((pInstance + i)->m_pPoseToWorld);
		if(result != g_DrawingThread_BonesPtr_To_Meta.end()) {
			currentEntityI = result->second;
		}
		if(currentEntityI != m_CurrentEntityMeta) {
			m_CurrentEntityMeta = currentEntityI;

			// BEGIN determine new action

			CAction * action = m_Stream->RetrieveAction(
				trackedMaterial
				, m_CurrentEntityMeta
			);

			if (nullptr == action) action = this->RetrieveAction(trackedMaterial, m_CurrentEntityMeta);

			if (m_Data.Modifier)
			{
				action = m_Data.Modifier->OverrideAction(action);
			}

			// END determine new action

			if(action != m_CurrentAction) {
				// Action differs, we gotta do something.

				if (m_CurrentAction)
					m_CurrentAction->DrawInstances(this, ctx, i - m_LastDrawnCount, pInstance + m_LastDrawnCount);
				else
					ctx->GetOrg()->DrawInstances(i - m_LastDrawnCount, pInstance + m_LastDrawnCount);
				m_LastDrawnCount = i;
				
				// BEGIN switch to new action

				BindAction(action);

				if (m_CurrentAction)
				{
					m_CurrentAction->MaterialHook(this, ctx, trackedMaterial);
				}

				if (SOURCESDK::IMaterial_csgo * replacementMaterial = trackedMaterial.Get()->GetReplacement())
					m_CurrentMaterial = replacementMaterial;
				else
					m_CurrentMaterial = m_CurrentMaterialOrg;

				ctx->GetOrg()->Bind(m_CurrentMaterial, m_CurrentProxyData);					

				// END SWITCH to new action
			}
		}
	}

	if(m_LastDrawnCount < nInstanceCount) {
		if (m_CurrentAction)
			m_CurrentAction->DrawInstances(this, ctx, i - m_LastDrawnCount, pInstance + m_LastDrawnCount);
		else
			ctx->GetOrg()->DrawInstances(i - m_LastDrawnCount, pInstance + m_LastDrawnCount);
		m_LastDrawnCount = i;
	}
	m_CurrentEntityMeta = m_CurrentEntityMetaOrg;

	if(orgAction != m_CurrentAction)
	{
		// BEGIN switch back

		BindAction(orgAction);

		if (m_CurrentAction)
		{
			m_CurrentAction->MaterialHook(this, ctx, trackedMaterial);
		}

		if (SOURCESDK::IMaterial_csgo * replacementMaterial = trackedMaterial.Get()->GetReplacement())
			m_CurrentMaterial = replacementMaterial;
		else
			m_CurrentMaterial = m_CurrentMaterialOrg;

		ctx->GetOrg()->Bind(m_CurrentMaterial, m_CurrentProxyData);

		// END switch back
	}
	orgAction->Release();	
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::Draw(IAfxMesh * am, int firstIndex, int numIndices)
{
	if (nullptr == am->GetContext()->GetOrg()->GetCallQueue() && m_CurrentAction)
		m_CurrentAction->Draw(this, am, firstIndex, numIndices);
	else
		am->GetParent()->Draw(firstIndex, numIndices);
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists)
{
	if (nullptr == am->GetContext()->GetOrg()->GetCallQueue() && m_CurrentAction)
		m_CurrentAction->Draw_2(this, am, pLists, nLists);
	else
		am->GetParent()->Draw(pLists, nLists);
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex, int numIndices)
{
	if (nullptr == am->GetContext()->GetOrg()->GetCallQueue() && m_CurrentAction)
		m_CurrentAction->DrawModulated(this, am, vecDiffuseModulation, firstIndex, numIndices);
	else
		am->GetParent()->DrawModulated(vecDiffuseModulation, firstIndex, numIndices);
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::UnlockMesh(IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc)
{
	if (nullptr == am->GetContext()->GetOrg()->GetCallQueue() && m_CurrentAction)
		m_CurrentAction->UnlockMesh(this, am, numVerts, numIndices, desc);
	else
		am->GetParent()->UnlockMesh(numVerts, numIndices, desc);
}

#if AFX_SHADERS_CSGO
void CAfxBaseFxStream::CAfxBaseFxStreamContext::SetVertexShader(CAfx_csgo_ShaderState & state)
{
	if (m_CurrentAction)
		m_CurrentAction->SetVertexShader(this, state);
}

void CAfxBaseFxStream::CAfxBaseFxStreamContext::SetPixelShader(CAfx_csgo_ShaderState & state)
{
	if (m_CurrentAction)
		m_CurrentAction->SetPixelShader(this, state);
}
#endif

// CAfxBaseFxStream::CAfxBaseFxStreamContext::CQueueBeginFunctor ///////////////

void CAfxBaseFxStream::CAfxBaseFxStreamContext::CQueueBeginFunctor::operator()()
{
	m_StreamContext->QueueBegin(m_Data);
}

// CAfxBaseFxStream::CAfxBaseFxStreamContext::CQueueEndFunctor /////////////////

void CAfxBaseFxStream::CAfxBaseFxStreamContext::CQueueEndFunctor::operator()()
{
	m_StreamContext->QueueEnd();
}

// CAfxBaseFxStream::CActionKey ////////////////////////////////////////////////

void CAfxBaseFxStream::CActionKey::ToLower(void)
{
	std::transform(m_Name.begin(), m_Name.end(), m_Name.begin(), ::tolower);
}

// CAfxBaseFxStream::CShared ///////////////////////////////////////////////////

CAfxBaseFxStream::CShared::CShared()
{
}

CAfxBaseFxStream::CShared::~CShared()
{

}

void CAfxBaseFxStream::CShared::AfxStreamsInit(void)
{
	CreateStdAction(m_DrawAction, CActionKey("draw"), new CAction());
	CreateStdAction(m_NoDrawAction, CActionKey("noDraw"), new CActionNoDraw());

	CreateStdAction(m_DrawMatteAction, CActionKey("drawMatte"), new CAction());
	CreateStdAction(m_NoDrawMatteAction, CActionKey("noDrawMatte"), new CAction());

	CreateStdAction(m_DepthAction, CActionKey("drawDepth"), new CActionDebugDepth(m_NoDrawAction));
	// CreateStdAction(m_DepthAction, CActionKey("drawDepth"), new CActionStandardResolve(CActionStandardResolve::RF_DrawDepth, m_NoDrawAction));

	/*
	m_Depth24Action =CreateStdAction(m_Depth24Action, CActionKey("drawDepth24"), new CActionStandardResolve(CActionStandardResolve::RF_DrawDepth24, m_NoDrawAction));
	*/
	{
		CActionReplace * action = new CActionReplace("afx/greenmatte", m_NoDrawAction);
		float color[3] = { 0,1,0 };
		action->OverrideColor(color);
		float lightScale[4] = { 1,1,1,1 };
		action->OverrideLigthScale(lightScale);
		CreateStdAction(m_MaskAction, CActionKey("mask"), action);
	}
	// CreateStdAction(m_MaskAction, CActionKey("mask"), new CActionStandardResolve(CActionStandardResolve::RF_GreenScreen, m_NoDrawAction));

	{
		CActionReplace * action = new CActionReplace("afx/white", m_NoDrawAction);
		float color[3] = { 1,1,1 };
		action->OverrideColor(color);
		float lightScale[4] = { 1,1,1,1 };
		action->OverrideLigthScale(lightScale);
		CreateStdAction(m_WhiteAction, CActionKey("white"), action);
	}
	// CreateStdAction(m_WhiteAction, CActionKey("white"), new CActionStandardResolve(CActionStandardResolve::RF_White, m_NoDrawAction));

	{
		CActionReplace * action = new CActionReplace("afx/black", m_NoDrawAction);
		float color[3] = { 0,0,0 };
		action->OverrideColor(color);
		float lightScale[4] = { 1,1,1,1 };
		action->OverrideLigthScale(lightScale);
		CreateStdAction(m_BlackAction, CActionKey("black"), action);
	}
	// CreateStdAction(m_BlackAction, CActionKey("black"), new CActionStandardResolve(CActionStandardResolve::RF_Black, m_NoDrawAction));

	// legacy actions:
	CreateAction(CActionKey("invisible"), new CActionNoDraw(), true);
	CreateAction(CActionKey("debugDepth"), new CActionDebugDepth(m_NoDrawAction), true);

	CreateAction(CActionKey("afxZOnly"), new CActionZOnly());
}

void CAfxBaseFxStream::CShared::AfxStreamsShutdown(void)
{
	for (std::map<CActionKey, CAction *>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		it->second->Release();
	}

	if (m_DrawAction) m_DrawAction->Release();
	if (m_DrawMatteAction) m_DrawMatteAction->Release();
	if (m_NoDrawMatteAction) m_NoDrawMatteAction->Release();
	if (m_NoDrawAction) m_NoDrawAction->Release();
	if (m_DepthAction) m_DepthAction->Release();
	/*
	if(m_Depth24Action) m_Depth24Action->Release();
	*/
	if (m_MaskAction) m_MaskAction->Release();
	if (m_WhiteAction) m_WhiteAction->Release();
	if (m_BlackAction) m_BlackAction->Release();
}

void CAfxBaseFxStream::CShared::AddRef()
{
	++m_RefCount;
}

void CAfxBaseFxStream::CShared::Release()
{
	--m_RefCount;
}

void CAfxBaseFxStream::CShared::Console_ListActions(void)
{
	for(std::map<CActionKey, CAction *>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		Tier0_Msg("%s%s (%i more dependencies)\n", it->second->Key_get().m_Name.c_str(), it->second->IsStockAction_get() ? " (stock action)" : "", it->second->GetRefCount() - 1);
	}
}

void CAfxBaseFxStream::CShared::Console_AddReplaceAction(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * prefix = args->ArgV(0);

	if(3 <= argc)
	{
		char const * actionName = args->ArgV(1);
		char const * materialName = args->ArgV(2);

		CActionKey key(actionName);

		if(!Console_CheckActionKey(key))
			return;

		bool overrideColor = false;
		float color[3];

		bool overrideBlend = false;
		float blend;

		bool overrideDepthWrite = false;
		bool depthWrite;

		bool overrideLightScale = false;
		float lightScale[3];

		for(int i=3; i < argc; ++i)
		{
			char const * argOpt = args->ArgV(i);

			if(StringBeginsWith(argOpt, "overrideColor="))
			{
				argOpt += strlen("overrideColor=");

				std::istringstream iss(argOpt);

				std::string word;

				int j=0;
				while(j<3 && (iss >> word))
				{
					color[j] = (float)atof(word.c_str());
					++j;
				}

				if(3 == j)
				{
					overrideColor = true;
				}
				else
				{
					Tier0_Warning("Error: overrideColor needs 3 values!\n");
					return;
				}
			}
			else
			if(StringBeginsWith(argOpt, "overrideBlend="))
			{
				argOpt += strlen("overrideBlend=");

				overrideBlend = true;
				blend = (float)atof(argOpt);
			}
			else
			if(StringBeginsWith(argOpt, "overrideDepthWrite="))
			{
				argOpt += strlen("overrideDepthWrite=");

				overrideDepthWrite = true;
				depthWrite = 0 != atoi(argOpt);
			}
			else if (StringBeginsWith(argOpt, "overrideLightScale="))
			{
				argOpt += strlen("overrideLightScale=");

				std::istringstream iss(argOpt);

				std::string word;

				int j = 0;
				while (j < 4 && (iss >> word))
				{
					lightScale[j] = (float)atof(word.c_str());
					++j;
				}

				if (4 == j)
				{
					overrideLightScale = true;
				}
				else
				{
					Tier0_Warning("Error: overrideLightScale needs 4 values!\n");
					return;
				}
			}
			else
			{
				Tier0_Warning("Error: invalid option %s\n", argOpt);
				return;
			}
		}

		CActionReplace * replaceAction = new CActionReplace(materialName, m_NoDrawAction);

		if(overrideColor) replaceAction->OverrideColor(color);
		if(overrideBlend) replaceAction->OverrideBlend(blend);
		if(overrideDepthWrite) replaceAction->OverrideDepthWrite(depthWrite);
		if (overrideLightScale) replaceAction->OverrideLigthScale(lightScale);

		CreateAction(key, replaceAction);
	}
	else
	{
		Tier0_Msg(
			"%s <actionName> <materialName> [option]*\n"
			"Options (yes you can specify multiple) can be:\n"
			"\t\"overrideColor=<rF> <gF> <bF>\" - Where <.F> is a floating point value between 0.0 and 1.0\n"
			"\t\"overrideBlend=<bF>\" - Where <bF> is a floating point value between 0.0 and 1.0\n"
			"\t\"overrideDepthWrite=<iF>\" - Where <iF> is 0 (don't write depth) or 1 (write depth)\n"
			"\t\"overrideLightScale=<fLinearLight> <fLightMap> <fEnvMap> <fGammaLight>\"\n"
			,
			prefix);
	}
}

void CAfxBaseFxStream::CShared::Console_AddGlowColorMapAction(IWrpCommandArgs* args)
{
	int argc = args->ArgC();

	char const* prefix = args->ArgV(0);

	if (2 <= argc)
	{
		char const* actionName = args->ArgV(1);

		CActionKey key(actionName);

		if (!Console_CheckActionKey(key))
			return;

		CActionGlowColorMap * action = new CActionGlowColorMap();

		CreateAction(key, action);
	}
	else
	{
		Tier0_Msg("%s <actionName>\n", prefix);
	}
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::GetAction(CActionKey const & key)
{
	CActionKey lowerKey(key, true);
	
	std::map<CActionKey, CAction *>::iterator it = m_Actions.find(lowerKey);
	if(it != m_Actions.end())
	{
		return it->second;
	}

	return 0;
}

bool CAfxBaseFxStream::CShared::RemoveAction(CActionKey const & key)
{
	CActionKey lowerKey(key, true);

	std::map<CActionKey, CAction *>::iterator it = m_Actions.find(lowerKey);
	if(it != m_Actions.end())
	{
		if(!it->second->IsStockAction_get())
		{
			if(1 == it->second->GetRefCount())
			{
				it->second->Release();
				m_Actions.erase(it);
				return true;
			}
			else
				Tier0_Warning("Action cannot be removed due to %i more dependencies.\n", it->second->GetRefCount() -1);
		}
		else
			Tier0_Warning("Stock actions cannot be removed!\n");		
	}
	else
		Tier0_Warning("Action not found.");

	return false;
}

void CAfxBaseFxStream::CShared::MainThreadInitialize(void)
{
	for (std::map<CActionKey, CAction *>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		it->second->MainThreadInitialize();
	}
}

void CAfxBaseFxStream::CShared::LevelShutdown(void)
{
	++m_ShutDownLevel;

	if(m_RefCount == m_ShutDownLevel)
	{
		for(std::map<CActionKey, CAction *>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
		{
			it->second->LevelShutdown();
		}
		m_ShutDownLevel = 0;
	}
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::DrawAction_get(void)
{
	return m_DrawAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::DrawMatteAction_get(void)
{
	return m_DrawMatteAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::NoDrawMatteAction_get(void)
{
	return m_NoDrawMatteAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::NoDrawAction_get(void)
{
	return m_NoDrawAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::DepthAction_get(void)
{
	return m_DepthAction;
}

/*
CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::Depth24Action_get(void)
{
	return m_Depth24Action;
}
*/

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::MaskAction_get(void)
{
	return m_MaskAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::WhiteAction_get(void)
{
	return m_WhiteAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CShared::BlackAction_get(void)
{
	return m_BlackAction;
}

void CAfxBaseFxStream::CShared::CreateStdAction(CAction * & stdTarget, CActionKey const & key, CAction * action)
{
	CreateAction(key, action, true);
	if(action) action->AddRef();
	stdTarget = action;
}

void CAfxBaseFxStream::CShared::CreateAction(CActionKey const & key, CAction * action, bool isStockAction)
{
	if(action)
	{
		action->Key_set(key);
		action->IsStockAction_set(isStockAction);
		action->AddRef();
	}
	CActionKey lowerKey(key, true);
	m_Actions[lowerKey] = action;
}

bool CAfxBaseFxStream::CShared::Console_CheckActionKey(CActionKey & key)
{
	CActionKey lowerKey(key, true);

	if(StringIsEmpty(key.m_Name.c_str()))
	{
		Tier0_Warning("Error: actionName must not be empty!\n");
		return false;
	}

	if(!StringIsAlNum(key.m_Name.c_str()))
	{
		Tier0_Warning("Error: actionName can only contain letters and numbers!\n");
		return false;
	}

	std::map<CActionKey, CAction *>::iterator it = m_Actions.find(lowerKey);

	if(it != m_Actions.end())
	{
		Tier0_Warning("Error: actionName is already in use!\n");
		return false;
	}

	return true;
}

// CAfxBaseFxStream::CActionFilterValue ////////////////////////////////////////

CAfxBaseFxStream::CActionFilterValue * CAfxBaseFxStream::CActionFilterValue::Console_Parse(CAfxStreams * streams, IWrpCommandArgs * args)
{
	if (args->ArgC() <= 1)
	{
		Tier0_Msg(
			"Usage:\n"
			"%s <option> <option> ...\n"
			"\n"
			"<option> can be:\n"
			"\"handle=<handleNumber>\" (of entity, see mirv_listentities)\n"
			"\"className=<wildCardString>\" (of entity)\n"
			"\"modelName=<wildCardString>\" (of entity)\n"
			"\"isPlayer=<0|1>\" (of entity)\n"
			"\"teamNumber=<iValue>\" (of entity)\n"
			"\"name=<wildCardString>\" (of material)\n"
			"\"textureGroup=<wildCardString>\" (of material)\n"
			"\"shader=<wildCardString>\" (of material)\n"
			"\"isErrorMaterial=0|1\" (of material)\n"
			"\"action=<actionName>\"\n"
			"\n"
			"- The action option must be given!\n"
			"- <wildCardString> is a string without quotes, where \\* is the wildcard and \\\\ is \\\n"
			"- Any option except action that is not given will be treated as if it doesn't matter for a match.\n"
			, args->ArgV(0)
		);

		return 0;
	}

	bool useHandle = false;
	SOURCESDK::CSGO::CBaseHandle * handle = 0;
	SOURCESDK::CSGO::CBaseHandle * rootHandle = 0;
	SOURCESDK::CSGO::CBaseHandle * rootMoveHandle = 0;
	std::string className("\\*");
	std::string modelName("\\*");
	bool useIsPlayer = false;
	bool isPlayer = false;
	bool useTeamNumber = false;
	int teamNumber = 0;
	std::string name("\\*");
	std::string textureGroupName("\\*");
	std::string shaderName("\\*");
	TriState isErrorMaterial = TS_DontCare;
	CAction * matchAction = 0;

	for (int i = 1; i < args->ArgC(); ++i)
	{
		std::string sArg(args->ArgV(i));

		size_t posDelimiter = sArg.find_first_of('=');

		std::string sKey = sArg.substr(0, posDelimiter);
		std::string sValue = sArg.substr(posDelimiter + 1);

		char const * key = sKey.c_str();

		if (!_stricmp("handle", key))
		{
			useHandle = true;
			handle = new SOURCESDK::CSGO::CBaseHandle((unsigned long)(atoi(sValue.c_str())));
		}
		else if (!_stricmp("name", key))
		{
			name = sValue;
		}
		else if (!_stricmp("textureGroup", key))
		{
			textureGroupName = sValue;
		}
		else if (!_stricmp("shader", key))
		{
			shaderName = sValue;
		}
		else if (!_stricmp("isErrorMaterial", key))
		{
			bool value = 0 != atoi(sValue.c_str());

			isErrorMaterial = value ? TS_True : TS_False;
		}
		else if (!_stricmp("className", key))
		{
			className = sValue;
		}
		else if (!_stricmp("modelName", key))
		{
			modelName = sValue;
		}
		else if (!_stricmp("isPlayer", key))
		{
			useIsPlayer = true;
			isPlayer = 0 != atoi(sValue.c_str());
		}
		else if (!_stricmp("teamNumber", key))
		{
			useTeamNumber = true;
			teamNumber = atoi(sValue.c_str());
		}
		else if (!_stricmp("action", key))
		{
			CAfxBaseFxStream::CAction * action;
			if (streams->Console_ToAfxAction(sValue.c_str(), action))
			{
				matchAction = action;
			}
			else
				return 0;
		}
		else
		{
			Tier0_Warning("Error: %s is not a valid option!\n", sKey.c_str());
			return 0;
		}
	}

	if (!matchAction)
	{
		Tier0_Warning("Error: Action must be given!\n");
		return 0;
	}

	if (!handle)
	{
		handle = new SOURCESDK::CSGO::CBaseHandle();
	}

	CActionFilterValue * result = new CActionFilterValue(
		useHandle, *handle, className.c_str(), modelName.c_str(), useIsPlayer, isPlayer, useTeamNumber, teamNumber,		
		name.c_str(), textureGroupName.c_str(), shaderName.c_str(), isErrorMaterial, matchAction);

	delete handle;

	return result;
}

bool CAfxBaseFxStream::CActionFilterValue::CalcMatch_Material(const CAfxTrackedMaterialRef& trackedMaterial)
{
	SOURCESDK::IMaterial_csgo * material = trackedMaterial.Get()->GetMaterial();
		
	if (!material)
		return false;

	return
		StringWildCard1Matched(m_Name.c_str(), material->GetName())
		&& StringWildCard1Matched(m_TextureGroupName.c_str(), material->GetTextureGroupName())
		&& StringWildCard1Matched(m_ShaderName.c_str(), material->GetShaderName())
		&& (m_IsErrorMaterial == TS_True ? (material->IsErrorMaterial() == true) : (m_IsErrorMaterial == TS_False ? (material->IsErrorMaterial() == false) : true));
}

bool CAfxBaseFxStream::CActionFilterValue::CalcMatch_Entity(const CEntityMeta & info)
{
	return
		(!m_UseHandle || m_Handle == info.Handle)
		&& (!m_UseClassName || StringWildCard1Matched(m_ClassName.c_str(), info.ClassName.c_str()))
		&& (!m_UseModelName || StringWildCard1Matched(m_ModelName.c_str(), info.ModelName.c_str()))
		&& (!m_UseIsPlayer || m_IsPlayer == info.IsPlayer)
		&& (!m_UseTeamNumber || m_TeamNumber == info.TeamNumber)
	;
}

// CAfxBaseFxStream::CAction /////////////////////////////////////////

void CAfxBaseFxStream::CAction::Console_Edit(IWrpCommandArgs* args)
{
	Tier0_Msg("This action has no editable settings.\n");
}

void CAfxBaseFxStream::CAction::MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{
}

// CAfxBaseFxStream::CActionDebugDepth /////////////////////////////////////////

CAfxBaseFxStream::CActionDebugDepth::CActionDebugDepth(CAction * fallBackAction)
: CAction()
, m_FallBackAction(fallBackAction)
, m_DebugDepthMaterial(0)
, m_TrackedMaterial(nullptr)
{
	if(fallBackAction) fallBackAction->AddRef();
}

CAfxBaseFxStream::CActionDebugDepth::~CActionDebugDepth()
{
	delete m_DebugDepthMaterial;
	if(m_FallBackAction) m_FallBackAction->Release();
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionDebugDepth::ResolveAction(const CAfxTrackedMaterialRef& trackedMaterial)
{
	SOURCESDK::IMaterial_csgo * material = trackedMaterial.Get()->GetMaterial();
	bool splinetype = false;
	bool useinstancing = false;

	if(material)
	{
		int numVars = material->ShaderParamCount();
		SOURCESDK::IMaterialVar_csgo ** orgParams = material->GetShaderParams();
		
		SOURCESDK::IMaterialVar_csgo ** params = orgParams;

		for(int i=0; i<numVars; ++i)
		{
			if(params[0]->IsDefined())
			{
				char const * varName = params[0]->GetName();

				if(!strcmp(varName,"$splinetype"))
				{
					if(params[0]->GetIntValue())
						splinetype = true;
				}
				else
				if(!strcmp(varName,"$useinstancing"))
				{
					if(params[0]->GetIntValue())
						useinstancing = true;
				}
			}

			++params;
		}
	}

	if(splinetype || useinstancing)
		return SafeSubResolveAction(m_FallBackAction, trackedMaterial);

	return this;
}

void CAfxBaseFxStream::CActionDebugDepth::MainThreadInitialize(void)
{
	if(!m_DebugDepthMaterial)
	{
		m_DebugDepthMaterial = new CAfxOwnedMaterial(g_AfxStreams.GetMaterialSystem()->FindMaterial("afx/depth", 0));
	}
}

void CAfxBaseFxStream::CActionDebugDepth::AfxUnbind(CAfxBaseFxStreamContext * ch)
{
	if (m_TrackedMaterial)
	{
		m_TrackedMaterial->SetReplacement(nullptr);
		m_TrackedMaterial->Release();
		m_TrackedMaterial = nullptr;
	}
}

void CAfxBaseFxStream::CActionDebugDepth::MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{


	int scale = ch->DrawingSkyBoxView_get() ? csgo_CSkyBoxView_GetScale() : 1;
	float flDepthFactor = ch->GetStream()->m_DepthVal / scale;
	float flDepthFactorMax = ch->GetStream()->m_DepthValMax / scale;

	if (m_DebugDepthMaterial)
	{
		m_TrackedMaterial = trackedMaterial.Get();
		m_TrackedMaterial->AddRef();
		m_TrackedMaterial->SetReplacement(m_DebugDepthMaterial->GetMaterial());
	}
	else
		m_TrackedMaterial = nullptr;

	m_Static.SetDepthVal(flDepthFactor, flDepthFactorMax);
}

CAfxBaseFxStream::CActionDebugDepth::CStatic CAfxBaseFxStream::CActionDebugDepth::m_Static;

void CAfxBaseFxStream::CActionDebugDepth::CStatic::SetDepthVal(float min, float max)
{
	m_MatDebugDepthValsMutex.lock();

	if (!m_MatDebugDepthVal) m_MatDebugDepthVal = new WrpConVarRef("mat_debugdepthval");
	if (!m_MatDebugDepthValMax) m_MatDebugDepthValMax = new WrpConVarRef("mat_debugdepthvalmax");

	m_MatDebugDepthVal->SetValueFastHack(min);
	m_MatDebugDepthValMax->SetValueFastHack(max);

	m_MatDebugDepthValsMutex.unlock();
}

// CAfxBaseFxStream::CActionReplace ////////////////////////////////////////////

CAfxBaseFxStream::CActionReplace::CActionReplace(
	char const * materialName,
	CAction * fallBackAction)
: CAction()
, m_Material(0)
, m_MaterialName(materialName)
, m_OverrideDepthWrite(false)
, m_TrackedMaterial(nullptr)
{
	if(fallBackAction) fallBackAction->AddRef();
	m_FallBackAction = fallBackAction;
}

CAfxBaseFxStream::CActionReplace::~CActionReplace()
{
	if(m_FallBackAction) m_FallBackAction->Release();
	if(m_Material) delete m_Material;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionReplace::ResolveAction(const CAfxTrackedMaterialRef& trackedMaterial)
{
	SOURCESDK::IMaterial_csgo * material = trackedMaterial.Get()->GetMaterial();

	if (m_Material)
	{
		bool srcSplinetype;
		bool srcUseinstancing;

		bool dstSplinetype;
		bool dstUseinstancing;

		ExamineMaterial(material, srcSplinetype, srcUseinstancing);
		ExamineMaterial(m_Material->GetMaterial(), dstSplinetype, dstUseinstancing);

		if (srcSplinetype == dstSplinetype
			&& srcUseinstancing == dstUseinstancing)
			return this;
	}

	return SafeSubResolveAction(m_FallBackAction, trackedMaterial);
}

void CAfxBaseFxStream::CActionReplace::MainThreadInitialize(void)
{
	if (!m_Material)
	{
		m_Material = new CAfxOwnedMaterial(g_AfxStreams.GetMaterialSystem()->FindMaterial(m_MaterialName.c_str(), 0));
	}
}

void CAfxBaseFxStream::CActionReplace::AfxUnbind(CAfxBaseFxStreamContext * ch)
{
	if (m_LightScaleOverride.m_Override) AfxD3D9OverrideEnd_LightScale();
	if (m_ModulationColorBlendOverride.m_OverrideColor || m_ModulationColorBlendOverride.m_OverrideBlend) AfxD3D9OverrideEnd_ModulationColorBlend();

	if (m_OverrideDepthWrite) AfxD3D9OverrideEnd_D3DRS_ZWRITEENABLE();

	if (m_TrackedMaterial)
	{
		m_TrackedMaterial->SetReplacement(nullptr);
		m_TrackedMaterial->Release();
		m_TrackedMaterial = nullptr;
	}

	AfxD3D9PopOverrideState();

}

void CAfxBaseFxStream::CActionReplace::MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{
	AfxD3D9PushOverrideState(false);

	if (m_Material)
	{
		m_TrackedMaterial = trackedMaterial.Get();
		m_TrackedMaterial->AddRef();
		m_TrackedMaterial->SetReplacement(m_Material->GetMaterial());

	}
	else
		m_TrackedMaterial = nullptr;

	if (m_ModulationColorBlendOverride.m_OverrideColor || m_ModulationColorBlendOverride.m_OverrideBlend) AfxD3D9OverrideBegin_ModulationColorBlend(&m_ModulationColorBlendOverride);
	if (m_LightScaleOverride.m_Override) AfxD3D9OverrideBegin_LightScale(&m_LightScaleOverride);

	if(m_OverrideDepthWrite) AfxD3D9OverrideBegin_D3DRS_ZWRITEENABLE(m_DepthWrite ? TRUE : FALSE);
}

void CAfxBaseFxStream::CActionReplace::ExamineMaterial(SOURCESDK::IMaterial_csgo * material, bool & outSplinetype, bool & outUseinstancing)
{
	bool splinetype = false;
	bool useinstancing = false;

	if(material)
	{
		int numVars = material->ShaderParamCount();
		SOURCESDK::IMaterialVar_csgo ** orgParams = material->GetShaderParams();
		
		SOURCESDK::IMaterialVar_csgo ** params = orgParams;

		for(int i=0; i<numVars; ++i)
		{
			if(params[0]->IsDefined())
			{
				char const * varName = params[0]->GetName();

				if(!strcmp(varName,"$splinetype"))
				{
					if(params[0]->GetIntValue())
						splinetype = true;
				}
				else
				if(!strcmp(varName,"$useinstancing"))
				{
					if(params[0]->GetIntValue())
						useinstancing = true;
				}
			}

			++params;
		}
	}

	outSplinetype = splinetype;
	outUseinstancing = useinstancing;
}

// CAfxBaseFxStream::CActionGlowColorMap ///////////////////////////////////////////

CAfxBaseFxStream::CActionGlowColorMap::~CActionGlowColorMap()
{
	if (nullptr != m_AfxColorLut) delete m_AfxColorLut;
}

void CAfxBaseFxStream::CActionGlowColorMap::Console_Edit(IWrpCommandArgs* args)
{
	int argC = args->ArgC();
	const char* arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char* arg1 = args->ArgV(1);

		if (0 == _stricmp("load", arg1) && 3 == argC)
		{
			std::unique_lock<std::shared_timed_mutex> lock(m_EditMutex);
			if (nullptr != m_AfxColorLut)
			{
				delete m_AfxColorLut;
				m_AfxColorLut = nullptr;
			}
			FILE* file = nullptr;
			if (0 == fopen_s(&file, args->ArgV(2), "rb"))
			{
				m_AfxColorLut = new CAfxColorLut();
				if (!m_AfxColorLut->LoadFromFile(file))
				{
					delete m_AfxColorLut;
					m_AfxColorLut = nullptr;
					Tier0_Warning("Error when processing HLAE Lookup Table Tree file at %i.\n", ftell(file));
				}
				fclose(file);
			}
			else Tier0_Warning("Error opening HLAE Lookup Table Tree file \"%s\".\n", args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("clear", arg1))
		{
			{
				std::unique_lock<std::shared_timed_mutex> lock(m_EditMutex);
				if (nullptr != m_AfxColorLut)
				{
					delete m_AfxColorLut;
					m_AfxColorLut = nullptr;
				}
			}
			return;
		}
		else if (0 == _stricmp("debugColor", arg1))
		{
			if (3 == argC)
			{
				std::unique_lock<std::shared_timed_mutex> lock(m_EditMutex);
				m_DebugColor = atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s debugColor 0|1|2 - Enable debug coloring (1: opaque color, 2: alpha value), combine with doBloomAndToneMapping 0 on a stream.\n"
				"Current value: %i\n"
				, arg0
				,m_DebugColor);
			return;
		}
		else if(0 == _stricmp("normalize", arg1))
		{
			if (3 == argC)
			{
				std::unique_lock<std::shared_timed_mutex> lock(m_EditMutex);
				m_Normalize = 0 != atoi(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s normalize 0|1 - Enable alpha normalization for mapping.\n"
				"Current value: %i\n"
				, arg0
				, m_Normalize ? 1 : 0);
			return;
		}
	}

	Tier0_Msg("%s load <aFilePath> - Load color mapping tree form file.\n", arg0);
	Tier0_Msg("%s clear - Clear color mapping tree.\n", arg0);
	Tier0_Msg("%s normalize [...]\n", arg0);
	Tier0_Msg("%s debugColor [...]\n", arg0);
}

void CAfxBaseFxStream::CActionGlowColorMap::AfxUnbind(CAfxBaseFxStreamContext* ch)
{
	AfxD3D9OverrideEnd_ModulationColorBlend();

	AfxD3D9PopOverrideState();
}

void CAfxBaseFxStream::CActionGlowColorMap::MaterialHook(CAfxBaseFxStreamContext* ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{
	AfxD3D9PushOverrideState(false);

	AfxD3D9OverrideBegin_ModulationColorBlend(this);
}

void CAfxBaseFxStream::CActionGlowColorMap::UnlockMesh(CAfxBaseFxStreamContext* ch, IAfxMesh* am, int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc)
{
	auto mesh = am->GetParent();
	if (mesh->GetVertexFormat() & 0x0004) // VERTEX_COLOR
	{
		for (int i = 0; i < numVerts; ++i)
		{
			unsigned char* Color = desc.m_pColor + i * desc.m_VertexSize_Color;
			CAfxColorLut::CRgba rgba(Color[2] / 255.0f, Color[1] / 255.0f, Color[0] / 255.0f, Color[3] / 255.0f);

			if (m_DebugColor)
			{
				rgba.A = 1;
			}
			else
			{
				RemapColor(rgba.R, rgba.G, rgba.B, rgba.A);
			}

			Color[2] = (unsigned char)std::min(255.0f, rgba.R * 255.0f + 0.5f);
			Color[1] = (unsigned char)std::min(255.0f, rgba.G * 255.0f + 0.5f);
			Color[0] = (unsigned char)std::min(255.0f, rgba.B * 255.0f + 0.5f);
			Color[3] = (unsigned char)std::min(255.0f, rgba.A * 255.0f + 0.5f);
		}
	}
	am->GetParent()->UnlockMesh(numVerts, numIndices, desc);
}

void CAfxBaseFxStream::CActionGlowColorMap::RemapColor(float& r, float& g, float& b, float& a)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_EditMutex);

	if (!m_AfxColorLut) return;
	
	if (24 < m_Queue.size())
	{
		auto it = m_Cache.find(m_Queue.front());
		if (0 == --(it->second.Count)) m_Cache.erase(it);
		m_Queue.pop();
	}

	CAfxColorLut::CRgba iV(r, g, b, a);
	m_Queue.push(iV);

	auto it = m_Cache.find(iV);
	if (it != m_Cache.end())
	{
		++it->second.Count;
		r = it->second.Result.R;
		g = it->second.Result.G;
		b = it->second.Result.B;
		a = it->second.Result.A;
		return;
	}

	CAfxColorLut::CRgba result(iV);

	m_AfxColorLut->Query(r, g, b, a, result.R, result.G, result.B, result.A);
	{
		r = result.R;
		g = result.G;
		b = result.B;
		a = result.A;
	}

	m_Cache.emplace(std::piecewise_construct, std::forward_as_tuple(iV), std::forward_as_tuple(result));
}

/*
float CAfxBaseFxStream::CActionGlowColorMap::DistSquared(const CRgba & x1, const CRgba & x2)
{
	return x1.R * x2.R + x1.G * x2.G + x1.B * x2.B + x1.A * x2.A;
}

void CAfxBaseFxStream::CActionGlowColorMap::MakeNaturalNeighbourLut()
{
	SOURCESDK::Vector *** mapVoronoi = nullptr;
	float ***nearestDistTable = nullptr;
	MakeLut(m_Lut);
	MakeLut(mapVoronoi);

	nearestDistTable = new float** [m_ResX];
	for (int x = 0; x < m_ResX; ++x)
	{
		nearestDistTable[x] = new float* [m_ResY];
		for (int y = 0; y < m_ResY; ++y)
		{
			nearestDistTable[x][y] = new float[m_ResZ];

			for (int z = 0; z < m_ResZ; ++z)
			{
				SOURCESDK::Vector input(x / (float)(m_ResX - 1), y / (float)(m_ResY - 1), z / (float)(m_ResZ - 1));

				SOURCESDK::Vector& lutV = m_Lut[x][y][z];
				lutV.x = 0;
				lutV.y = 0;
				lutV.z = 0;

				SOURCESDK::Vector* nearest = nullptr;
				float nearestDist;

				for (auto it = m_Map.begin(); it != m_Map.end(); ++it)
				{
					float dist = DistSquared(it->first, input.x, input.y, input.z);
					if (nullptr == nearest || dist < nearestDist)
					{
						nearestDist = dist;
						nearest = &it->second;
					}
				}

				SOURCESDK::Vector& mapVoronoi = m_Lut[x][y][z];

				if (nullptr != nearest)
				{
					mapVoronoi.x = nearest->x;
					mapVoronoi.y = nearest->y;
					mapVoronoi.z = nearest->z;
					nearestDistTable[x][y][z] = nearestDist;
				}
				else
				{
					mapVoronoi.x = input.x;
					mapVoronoi.y = input.y;
					mapVoronoi.z = input.z;
					nearestDistTable[x][y][z] = std::numeric_limits<float>::infinity();
				}
			}
		}
	}

	for (int x = 0; x < m_ResX; ++x)
	{
		for (int y = 0; y < m_ResY; ++y)
		{
			for (int z = 0; z < m_ResZ; ++z)
			{
				SOURCESDK::Vector input(x / (float)(m_ResX - 1), y / (float)(m_ResY - 1), z / (float)(m_ResZ - 1));

				SOURCESDK::Vector* nearest = nullptr;
				float nearestDist;

				for (auto it = m_Map.begin(); it != m_Map.end(); ++it)
				{
					float dist = DistSquared(it->first, input.x, input.y, input.z);
					if (nullptr == nearest || dist < nearestDist)
					{
						nearestDist = dist;
						nearest = &it->second;
					}
				}

				if (nearest == nullptr)
				{

				}
			}
		}
	}

	for (int x = 0; x < m_ResX; ++x)
	{
		for (int y = 0; y < m_ResY; ++y)
		{
			for (int z = 0; z < m_ResZ; ++z)
			{
				SOURCESDK::Vector input(x / (float)(m_ResX - 1), y / (float)(m_ResY - 1), z / (float)(m_ResZ - 1));				


			}
		}
	}

	free nearestDist

	FreeLut(mapVoronoi);
}
*/

/*
void CAfxBaseFxStream::CActionGlowColorMap::BowyerWatson(ColorMap_t& map, TetrahedonSet_t& tetras)
{
	std::unique_lock<std::shared_timed_mutex> lock(m_EditMutex);

	m_RecentList.clear();
	m_RecentMap.clear();

	// https://en.wikipedia.org/wiki/Delaunay_triangulation
	// https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm --> pseudocode there is currently faulty, used http://paulbourke.net/papers/triangulate/ instead

	const float epsilon = 1;

	tetras.clear();
	//https://math.stackexchange.com/questions/2670972/coordinates-of-a-tetrahedron-containing-a-cube
	SOURCESDK::Vector v1 = {
		1 + std::sqrtf(3.0f / 8.0f) + 1 / std::sqrtf(3.0f) + epsilon,
		-1 / (2 * std::sqrtf(2)) - epsilon,
		0 - epsilon
	};
	SOURCESDK::Vector v2 = {
		1/2.0f + epsilon,
		1+  (std::sqrtf(2) + std::sqrtf(3)) / 2.0f + epsilon,
		0 - epsilon
	};
	SOURCESDK::Vector v3 = {
		-1/std::sqrtf(3) -std::sqrtf(3/8.0f) - epsilon,
		-1 / (2 * std::sqrtf(2)) - epsilon,
		0 - epsilon
	};
	SOURCESDK::Vector v4 = {
		1/2.0f + epsilon,
		1/3.0f + std::sqrtf(3)/6.0f + epsilon,
		1 + 2* std::sqrtf(2)/3.0f + std::sqrtf(6)/3.0f + epsilon
	};

	map[v1] = v1;
	map[v2] = v2;
	map[v3] = v3;
	map[v4] = v4;
	tetras.emplace(v1, v2, v3, v4, v1, v2, v3, v4);

	for (ColorMap_t::const_iterator itPoint = map.begin(); itPoint != map.end(); ++itPoint)
	{
		std::set<CTriangle> poly;
		for (TetrahedonSet_t::iterator itTetra = tetras.begin(); itTetra != tetras.end(); )
		{
			if (InsideCircumSphere(*itTetra, itPoint->first))
			{
				for (int i = 0; i < 4; ++i)
				{
					poly.emplace(GetTetraTriangle(*itTetra, i));
				}

				TetrahedonSet_t::iterator itT = std::next(itTetra);
				tetras.erase(itTetra);
				itTetra = itT;
			}
			else  ++itTetra;
		}
		for (std::set<CTriangle>::iterator itTri = poly.begin(); itTri != poly.end(); ++itTri)
		{
			for (std::set<CTriangle>::iterator itTri2 = std::next(itTri); itTri2 != poly.end(); )
			{
				if (Equal(*itTri, *itTri2))
				{
					std::set<CTriangle>::iterator itT = std::next(itTri2);
					poly.erase(itTri2);
					itTri2 = itT;
				}
				else ++itTri2;
			}
		}
		for (std::set<CTriangle>::iterator itTri = poly.begin(); itTri != poly.end(); ++itTri)
		{
			tetras.emplace(itTri->s1, itTri->s2, itTri->s3, itPoint->first, itTri->d1, itTri->d2, itTri->d3, itPoint->second);
		}
	}
	for (TetrahedonSet_t::iterator itTetra = tetras.begin(); itTetra != tetras.end(); )
	{
		if (Equal(itTetra->s1, v1) || Equal(itTetra->s2, v1) || Equal(itTetra->s3, v1) || Equal(itTetra->s4, v1)
			|| Equal(itTetra->s1, v2) || Equal(itTetra->s2, v2) || Equal(itTetra->s3, v2) || Equal(itTetra->s4, v2)
			|| Equal(itTetra->s1, v3) || Equal(itTetra->s2, v3) || Equal(itTetra->s3, v3) || Equal(itTetra->s4, v3)
			|| Equal(itTetra->s1, v4) || Equal(itTetra->s2, v4) || Equal(itTetra->s3, v4) || Equal(itTetra->s4, v4)) {
			TetrahedonSet_t::iterator itT = std::next(itTetra);
			tetras.erase(itTetra);
			itTetra = itT;
		}
		else ++itTetra;
	}
	map.erase(v1);
	map.erase(v2);
	map.erase(v3);
	map.erase(v4);
}

bool CAfxBaseFxStream::CActionGlowColorMap::InsideTetra(const CTetrahedron& tetra, const SOURCESDK::Vector& p)
{
	// http://steve.hollasch.net/cgindex/geometry/ptintet.html

	SOURCESDK::Vector v1 = tetra.s1;
	SOURCESDK::Vector v2 = tetra.s2;
	SOURCESDK::Vector v3 = tetra.s3;
	SOURCESDK::Vector v4 = tetra.s4;

	float D0_4_1 = v2.x * (v3.y * v4.z - v3.z * v4.y) - v3.x * (v2.y * v4.z - v2.z * v4.y) + v4.x * (v2.y * v3.z - v2.z * v3.y);
	float D0_4_2 = v1.x * (v3.y * v4.z - v3.z * v4.y) - v3.x * (v1.y * v4.z - v1.z * v4.y) + v4.x * (v1.y * v3.z - v1.z * v3.y);
	float D0_4_3 = v1.x * (v2.y * v4.z - v2.z * v4.y) - v2.x * (v1.y * v4.z - v1.z * v4.y) + v4.x * (v1.y * v2.z - v1.z * v2.y);
	float D0_4_4 = v1.x * (v2.y * v3.z - v2.z * v3.y) - v2.x * (v1.y * v3.z - v1.z * v3.y) + v3.x * (v1.y * v2.z - v1.z * v2.y);
	float D0_4 = -1 * (D0_4_1)+1 * (D0_4_2)-1 * (D0_4_3)+1 * (D0_4_4);

	float D1_4_1 = D0_4_1;
	float D1_4_2 = p.x * (v3.y * v4.z - v3.z * v4.y) - v3.x * (p.y * v4.z - p.z * v4.y) + v4.x * (p.y * v3.z - p.z * v3.y);
	float D1_4_3 = p.x * (v2.y * v4.z - v2.z * v4.y) - v2.x * (p.y * v4.z - p.z * v4.y) + v4.x * (p.y * v2.z - p.z * v2.y);
	float D1_4_4 = p.x * (v2.y * v3.z - v2.z * v3.y) - v2.x * (p.y * v3.z - p.z * v3.y) + v3.x * (p.y * v2.z - p.z * v2.y);
	float D1_4 = -1 * (D1_4_1)+1 * (D1_4_2)-1 * (D1_4_3)+1 * (D1_4_4);

	float D2_4_1 = p.x * (v3.y * v4.z - v3.z * v4.y) - v3.x * (p.y * v4.z - p.z * v4.y) + v4.x * (p.y * v3.z - p.z * v3.y);
	float D2_4_2 = D0_4_2;
	float D2_4_3 = v1.x * (p.y * v4.z - p.z * v4.y) - p.x * (v1.y * v4.z - v1.z * v4.y) + v4.x * (v1.y * p.z - v1.z * p.y);
	float D2_4_4 = v1.x * (p.y * v3.z - p.z * v3.y) - p.x * (v1.y * v3.z - v1.z * v3.y) + v3.x * (v1.y * p.z - v1.z * p.y);
	float D2_4 = -1 * (D2_4_1)+1 * (D2_4_2)-1 * (D2_4_3)+1 * (D2_4_4);

	float D3_4_1 = v2.x * (p.y * v4.z - p.z * v4.y) - p.x * (v2.y * v4.z - v2.z * v4.y) + v4.x * (v2.y * p.z - v2.z * p.y);
	float D3_4_2 = v1.x * (p.y * v4.z - p.z * v4.y) - p.x * (v1.y * v4.z - v1.z * v4.y) + v4.x * (v1.y * p.z - v1.z * p.y);
	float D3_4_3 = D0_4_3;
	float D3_4_4 = v1.x * (v2.y * p.z - v2.z * p.y) - v2.x * (v1.y * p.z - v1.z * p.y) + p.x * (v1.y * v2.z - v1.z * v2.y);
	float D3_4 = -1 * (D3_4_1)+1 * (D3_4_2)-1 * (D3_4_3)+1 * (D3_4_4);

	float D4_4_1 = v2.x * (v3.y * p.z - v3.z * p.y) - v3.x * (v2.y * p.z - v2.z * p.y) + p.x * (v2.y * v3.z - v2.z * v3.y);
	float D4_4_2 = v1.x * (v3.y * p.z - v3.z * p.y) - v3.x * (v1.y * p.z - v1.z * p.y) + p.x * (v1.y * v3.z - v1.z * v3.y);
	float D4_4_3 = v1.x * (v2.y * p.z - v2.z * p.y) - v2.x * (v1.y * p.z - v1.z * p.y) + p.x * (v1.y * v2.z - v1.z * v2.y);
	float D4_4_4 = D0_4_4;
	float D4_4 = -1 * (D4_4_1)+1 * (D4_4_2)-1 * (D4_4_3)+1 * (D4_4_4);

	return std::signbit(D0_4) == std::signbit(D1_4) == std::signbit(D2_4) == std::signbit(D3_4) == std::signbit(D4_4);
}

bool CAfxBaseFxStream::CActionGlowColorMap::InsideCircumSphere(const CTetrahedron& tetra, const SOURCESDK::Vector& p, SOURCESDK::Vector* pCenter)
{
	SOURCESDK::Vector center = {
		(tetra.s1.x + tetra.s2.x + tetra.s3.x + tetra.s4.x) / 4,
		(tetra.s1.y + tetra.s2.y + tetra.s3.y + tetra.s4.y) / 4,
		(tetra.s1.z + tetra.s2.z + tetra.s3.z + tetra.s4.z) / 4,
	};

	float rad = DistSquared(center, tetra.s1.x, tetra.s1.y, tetra.s1.z);
	float dist = DistSquared(center, p.x, p.y, p.z);

	if (pCenter) *pCenter = center;

	return dist <= rad + AFX_MATH_EPS;
}

CAfxBaseFxStream::CActionGlowColorMap::CTriangle CAfxBaseFxStream::CActionGlowColorMap::GetTetraTriangle(const CTetrahedron& tetra, int i)
{
	switch (i)
	{
	default:
	case 0:
		return CTriangle(tetra.s1, tetra.s2, tetra.s3, tetra.d1, tetra.d2, tetra.d3);
	case 1:
		return CTriangle(tetra.s1, tetra.s2, tetra.s4, tetra.d1, tetra.d2, tetra.d4);
	case 2:
		return CTriangle(tetra.s1, tetra.s3, tetra.s4, tetra.d1, tetra.d3, tetra.d4);
	case 3:
		return CTriangle(tetra.s2, tetra.s3, tetra.s4, tetra.d2, tetra.d3, tetra.d4);
	}
}

bool CAfxBaseFxStream::CActionGlowColorMap::Equal(const CTriangle& tri1, const CTriangle& tri2)
{
	return
		Equal(tri1.s1, tri2.s1) && Equal(tri1.s2, tri2.s2) && Equal(tri1.s3, tri2.s3)
		|| Equal(tri1.s1, tri2.s1) && Equal(tri1.s3, tri2.s2) && Equal(tri1.s2, tri2.s3)
		|| Equal(tri1.s2, tri2.s1) && Equal(tri1.s1, tri2.s2) && Equal(tri1.s3, tri2.s3)
		|| Equal(tri1.s2, tri2.s1) && Equal(tri1.s3, tri2.s2) && Equal(tri1.s1, tri2.s3)
		|| Equal(tri1.s3, tri2.s1) && Equal(tri1.s1, tri2.s2) && Equal(tri1.s2, tri2.s3)
		|| Equal(tri1.s3, tri2.s1) && Equal(tri1.s2, tri2.s2) && Equal(tri1.s1, tri2.s3);
}


bool CAfxBaseFxStream::CActionGlowColorMap::Equal(SOURCESDK::Vector a, SOURCESDK::Vector b)
{
	return abs(a.x - b.x) <= AFX_MATH_EPS && (a.y - b.y) <= AFX_MATH_EPS && (a.z - b.z) <= AFX_MATH_EPS;
}
*/

#if AFX_SHADERS_CSGO
// CAfxBaseFxStream::CActionStandardResolve:CShared ////////////////////////////

CAfxBaseFxStream::CActionStandardResolve::CShared::CShared()
: m_RefCount(0)
, m_ShutDownLevel(0)
{
	m_StdDepthAction = new CActionUnlitGenericFallback(
		CActionAfxVertexLitGenericHookKey(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_0, 0.7f)
		, "afx/white"
		);
	m_StdDepthAction->AddRef();
	m_StdDepthAction->Key_set(CActionKey("drawDepth (std)"));

	m_StdDepth24Action = new CActionUnlitGenericFallback(
		CActionAfxVertexLitGenericHookKey(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_1, 0.7f)
		, "afx/white"
		);
	m_StdDepth24Action->AddRef();
	m_StdDepth24Action->Key_set(CActionKey("drawDepth24 (std)"));

	m_StdMatteAction = new CActionUnlitGenericFallback(
		CActionAfxVertexLitGenericHookKey(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_2, 0.7f)
		, "afx/white"
		);
	m_StdMatteAction->AddRef();
	m_StdMatteAction->Key_set(CActionKey("mask (std)"));
		
	m_StdBlackAction = new CActionUnlitGenericFallback(
		CActionAfxVertexLitGenericHookKey(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_3, 0.7f)
		, "afx/white"
		);
	m_StdBlackAction->AddRef();
	m_StdBlackAction->Key_set(CActionKey("white (std)"));

	m_StdWhiteAction = new CActionUnlitGenericFallback(
		CActionAfxVertexLitGenericHookKey(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_4, 0.7f)
		, "afx/white"
		);
	m_StdWhiteAction->AddRef();
	m_StdWhiteAction->Key_set(CActionKey("white (std)"));
}

CAfxBaseFxStream::CActionStandardResolve::CShared::~CShared()
{
	InvalidateSplineRopeHookActions();
	InvalidateSpritecardHookActions();
	InvalidateVertexLitGenericHookActions();

	if(m_StdWhiteAction) m_StdWhiteAction->Release();
	if(m_StdBlackAction) m_StdBlackAction->Release();
	if(m_StdMatteAction) m_StdMatteAction->Release();
	if(m_StdDepth24Action) m_StdDepth24Action->Release();
	if(m_StdDepthAction) m_StdDepthAction->Release();
}

void CAfxBaseFxStream::CActionStandardResolve::CShared::AddRef()
{
	++m_RefCount;
}

void CAfxBaseFxStream::CActionStandardResolve::CShared::Release()
{
	--m_RefCount;
}

void CAfxBaseFxStream::CActionStandardResolve::CShared::LevelShutdown(IAfxStreams4Stream * streams)
{
	++m_ShutDownLevel;

	if(m_RefCount == m_ShutDownLevel)
	{
		for(std::map<CActionAfxSplineRopeHookKey, CActionAfxSplineRopeHook *>::iterator it = m_SplineRopeHookActions.begin(); it != m_SplineRopeHookActions.end(); ++it)
		{
			it->second->LevelShutdown(streams);
		}
		InvalidateSplineRopeHookActions();

		for(std::map<CActionAfxSpritecardHookKey, CActionAfxSpritecardHook *>::iterator it = m_SpritecardHookActions.begin(); it != m_SpritecardHookActions.end(); ++it)
		{
			it->second->LevelShutdown(streams);
		}
		InvalidateSpritecardHookActions();

		for(std::map<CActionAfxVertexLitGenericHookKey, CActionAfxVertexLitGenericHook *>::iterator it = m_VertexLitGenericHookActions.begin(); it != m_VertexLitGenericHookActions.end(); ++it)
		{
			it->second->LevelShutdown(streams);
		}
		InvalidateVertexLitGenericHookActions();

		if(m_StdWhiteAction) m_StdWhiteAction->LevelShutdown(streams);
		if(m_StdBlackAction) m_StdBlackAction->LevelShutdown(streams);
		if(m_StdMatteAction) m_StdMatteAction->LevelShutdown(streams);
		if(m_StdDepth24Action) m_StdDepth24Action->LevelShutdown(streams);
		if(m_StdDepthAction) m_StdDepthAction->LevelShutdown(streams);

		m_ShutDownLevel = 0;
	}
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetStdDepthAction()
{
	return m_StdDepthAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetStdDepth24Action()
{
	return m_StdDepth24Action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetStdMatteAction()
{
	return m_StdMatteAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetStdBlackAction()
{
	return m_StdBlackAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetStdWhiteAction()
{
	return m_StdWhiteAction;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetSplineRopeHookAction(CActionAfxSplineRopeHookKey & key)
{
	std::map<CActionAfxSplineRopeHookKey, CActionAfxSplineRopeHook *>::iterator it = m_SplineRopeHookActions.find(key);

	if(it != m_SplineRopeHookActions.end())
	{
		return it->second;
	}

	CActionAfxSplineRopeHook * action = new CActionAfxSplineRopeHook(key);
	{
		std::ostringstream ossName;

		ossName << "splineRopeHook(";

		switch(key.AFXMODE)
		{
		case ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_0:
			ossName << "drawDepth";
			break;
		case ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_1:
			ossName << "drawDepth24";
			break;
		case ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_2:
			ossName << "mask";
			break;
		case ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_3:
			ossName << "white";
			break;
		case ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_4:
			ossName << "black";
			break;
		default:
			ossName << "(unknown)";
			break;
		}

		ossName << ", " << key.AlphaTestReference << ")";

		action->Key_set(CActionKey(ossName.str().c_str()));
	}
	action->IsStockAction_set(true);
	action->AddRef();
	m_SplineRopeHookActions[key] = action;

	return action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetSpritecardHookAction(CActionAfxSpritecardHookKey & key)
{
	std::map<CActionAfxSpritecardHookKey, CActionAfxSpritecardHook *>::iterator it = m_SpritecardHookActions.find(key);

	if(it != m_SpritecardHookActions.end())
	{
		return it->second;
	}

	CActionAfxSpritecardHook * action = new CActionAfxSpritecardHook(key);
	{
		std::ostringstream ossName;

		ossName << "spriteCardHook(";

		switch(key.AFXMODE)
		{
		case ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_0:
			ossName << "drawDepth";
			break;
		case ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_1:
			ossName << "drawDepth24";
			break;
		case ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_2:
			ossName << "mask";
			break;
		case ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_3:
			ossName << "white";
			break;
		case ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_4:
			ossName << "black";
			break;
		default:
			ossName << "(unknown)";
			break;
		}

		ossName << ", " << key.AlphaTestReference << ")";

		action->Key_set(CActionKey(ossName.str().c_str()));
	}
	action->IsStockAction_set(true);
	action->AddRef();
	m_SpritecardHookActions[key] = action;

	return action;
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::CShared::GetVertexLitGenericHookAction(CActionAfxVertexLitGenericHookKey & key)
{
	std::map<CActionAfxVertexLitGenericHookKey, CActionAfxVertexLitGenericHook *>::iterator it = m_VertexLitGenericHookActions.find(key);

	if(it != m_VertexLitGenericHookActions.end())
	{
		return it->second;
	}

	CActionAfxVertexLitGenericHook * action = new CActionAfxVertexLitGenericHook(key);
	{
		std::ostringstream ossName;

		ossName << "vertexLitGenericHook(";

		switch(key.AFXMODE)
		{
		case ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_0:
			ossName << "drawDepth";
			break;
		case ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_1:
			ossName << "drawDepth24";
			break;
		case ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_2:
			ossName << "mask";
			break;
		case ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_3:
			ossName << "white";
			break;
		case ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_4:
			ossName << "black";
			break;
		default:
			ossName << "(unknown)";
			break;
		}

		ossName << ", " << key.AlphaTestReference << ")";

		action->Key_set(CActionKey(ossName.str().c_str()));
	}
	action->IsStockAction_set(true);
	action->AddRef();
	m_VertexLitGenericHookActions[key] = action;

	return action;
}

void CAfxBaseFxStream::CActionStandardResolve::CShared::InvalidateSplineRopeHookActions()
{
	for(std::map<CActionAfxSplineRopeHookKey, CActionAfxSplineRopeHook *>::iterator it = m_SplineRopeHookActions.begin();
		it != m_SplineRopeHookActions.end();
		++it)
	{
		it->second->Release();
	}
	m_SplineRopeHookActions.clear();
}


void CAfxBaseFxStream::CActionStandardResolve::CShared::InvalidateSpritecardHookActions()
{
	for(std::map<CActionAfxSpritecardHookKey, CActionAfxSpritecardHook *>::iterator it = m_SpritecardHookActions.begin();
		it != m_SpritecardHookActions.end();
		++it)
	{
		it->second->Release();
	}
	m_SpritecardHookActions.clear();
}

void CAfxBaseFxStream::CActionStandardResolve::CShared::InvalidateVertexLitGenericHookActions()
{
	for(std::map<CActionAfxVertexLitGenericHookKey, CActionAfxVertexLitGenericHook *>::iterator it = m_VertexLitGenericHookActions.begin();
		it != m_VertexLitGenericHookActions.end();
		++it)
	{
		it->second->Release();
	}
	m_VertexLitGenericHookActions.clear();
}


// CAfxBaseFxStream::CActionStandardResolve ////////////////////////////////////

CAfxBaseFxStream::CActionStandardResolve::CShared CAfxBaseFxStream::CActionStandardResolve::m_Shared;

CAfxBaseFxStream::CActionStandardResolve::CActionStandardResolve(ResolveFor resolveFor, CAction * fallBackAction)
: CAction()
, m_ResolveFor(resolveFor)
, m_FallBackAction(fallBackAction)
{
	m_Shared.AddRef();

	if(fallBackAction) fallBackAction->AddRef();
}

CAfxBaseFxStream::CActionStandardResolve::~CActionStandardResolve()
{
	if(m_FallBackAction) m_FallBackAction->Release();

	m_Shared.Release();
}

CAfxBaseFxStream::CAction * CAfxBaseFxStream::CActionStandardResolve::ResolveAction(IMaterial_csgo * material)
{
	const char * shaderName = material->GetShaderName();

	int numVars = material->ShaderParamCount();
	IMaterialVar_csgo ** orgParams = material->GetShaderParams();

	int flags = orgParams[FLAGS]->GetIntValue();
	bool isAlphatest = 0 != (flags & MATERIAL_VAR_ALPHATEST);
	bool isTranslucent = 0 != (flags & MATERIAL_VAR_TRANSLUCENT);
	bool isAdditive = 0 != (flags & MATERIAL_VAR_ADDITIVE);

	float alphaTestReference = 0.7f;
	bool alphaTestReferenceDefined = false;
	bool isPhong = false;
	bool isBump = false;
	bool isSpline = false;
	bool isUseInstancing = false;

	{
		IMaterialVar_csgo ** params = orgParams;

		//Tier0_Msg("---- Params ----\n");

		for(int i=0; i<numVars; ++i)
		{
			//Tier0_Msg("Param: %s -> %s (%s,isTexture: %s)\n",params[0]->GetName(), params[0]->GetStringValue(), params[0]->IsDefined() ? "defined" : "UNDEFINED", params[0]->IsTexture() ? "Y" : "N");

			if(params[0]->IsDefined())
			{
				char const * varName = params[0]->GetName();

				if(!strcmp(varName,"$bumpmap"))
				{
					if(params[0]->IsTexture())
						isBump = true;
				}
				else
				if(!strcmp(varName,"$phong"))
				{
					if(params[0]->GetIntValue())
						isPhong = true;
				}
				else
				if(!strcmp(params[0]->GetName(),"$alphatestreference") && 0.0 < params[0]->GetFloatValue())
				{
					alphaTestReferenceDefined = true;
					alphaTestReference = params[0]->GetFloatValue();
					break;
				}
				else
				if(!strcmp(varName,"$splinetype"))
				{
					if(params[0]->GetIntValue())
						isSpline = true;
				}
				else
				if(!strcmp(varName,"$useinstancing"))
				{
					if(params[0]->GetIntValue())
						isUseInstancing = true;
				}
			}

			++params;
		}
	}

	if(!isAdditive && !strcmp(shaderName, "SplineRope"))
	{
		switch(m_ResolveFor)
		{
		case RF_DrawDepth:
			{
				CActionAfxSplineRopeHookKey key(
					ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_0,
					alphaTestReference);

				return SafeSubResolveAction(m_Shared.GetSplineRopeHookAction(key), material);
			}
		case RF_DrawDepth24:
			{
				CActionAfxSplineRopeHookKey key(
					ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_1,
					alphaTestReference);

				return SafeSubResolveAction(m_Shared.GetSplineRopeHookAction(key), material);
			}
		case RF_GreenScreen:
			{
				CActionAfxSplineRopeHookKey key(
					ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_2,
					alphaTestReference);

				return SafeSubResolveAction(m_Shared.GetSplineRopeHookAction(key), material);
			}
		case RF_Black:
			{
				CActionAfxSplineRopeHookKey key(
					ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_3,
					alphaTestReference);

				return SafeSubResolveAction(m_Shared.GetSplineRopeHookAction(key), material);
			}
		case RF_White:
			{
				CActionAfxSplineRopeHookKey key(
					ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_4,
					alphaTestReference);

				return SafeSubResolveAction(m_Shared.GetSplineRopeHookAction(key), material);
			}
		}
	}
	else
	if(!strcmp(shaderName, "Spritecard"))
	{
		switch(m_ResolveFor)
		{
		case RF_DrawDepth:
			{
				CActionAfxSpritecardHookKey key(
					ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_0,
					alphaTestReference
					);

				return SafeSubResolveAction(m_Shared.GetSpritecardHookAction(key), material);
			}
		case RF_DrawDepth24:
			{
				CActionAfxSpritecardHookKey key(
					ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_1,
					alphaTestReference
					);

				return SafeSubResolveAction(m_Shared.GetSpritecardHookAction(key), material);
			}
		case RF_GreenScreen:
			{
				CActionAfxSpritecardHookKey key(
					ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_2,
					alphaTestReference
					);

				return SafeSubResolveAction(m_Shared.GetSpritecardHookAction(key), material);
			}
		case RF_Black:
			{
				CActionAfxSpritecardHookKey key(
					ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_3,
					alphaTestReference
					);

				return SafeSubResolveAction(m_Shared.GetSpritecardHookAction(key), material);
			}
		case RF_White:
			{
				CActionAfxSpritecardHookKey key(
					ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_4,
					alphaTestReference
					);

				return SafeSubResolveAction(m_Shared.GetSpritecardHookAction(key), material);
			}
		}
	}
	else
	if(!isAdditive && (!strcmp(shaderName, "VertexLitGeneric") || !strcmp(shaderName, "UnlitGeneric")))
	{
		if(!isPhong && !isBump)
		{
			switch(m_ResolveFor)
			{
			case RF_DrawDepth:
				{
					CActionAfxVertexLitGenericHookKey key(
						ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_0,
						alphaTestReference);

					return SafeSubResolveAction(m_Shared.GetVertexLitGenericHookAction(key), material);
				}
			case RF_DrawDepth24:
				{
					CActionAfxVertexLitGenericHookKey key(
						ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_1,
						alphaTestReference);

					return SafeSubResolveAction(m_Shared.GetVertexLitGenericHookAction(key), material);
				}
			case RF_GreenScreen:
				{
					CActionAfxVertexLitGenericHookKey key(
						ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_2,
						alphaTestReference);

					return SafeSubResolveAction(m_Shared.GetVertexLitGenericHookAction(key), material);
				}
			case RF_Black:
				{
					CActionAfxVertexLitGenericHookKey key(
						ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_3,
						alphaTestReference);

					return SafeSubResolveAction(m_Shared.GetVertexLitGenericHookAction(key), material);
				}
			case RF_White:
				{
					CActionAfxVertexLitGenericHookKey key(
						ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_4,
						alphaTestReference);

					return SafeSubResolveAction(m_Shared.GetVertexLitGenericHookAction(key), material);
				}
			}
		}
	}
	else
	if(!strcmp(shaderName, "Sprite_DX9"))
	{
		// cant really handle that shader atm, so use fallback:
		return SafeSubResolveAction(m_FallBackAction, material);
	}

	// Std fallback:

	if(!isAdditive && !isSpline && !isUseInstancing)
	{
		switch(m_ResolveFor)
		{
		case RF_DrawDepth:
			return SafeSubResolveAction(m_Shared.GetStdDepthAction(), material);
		case RF_DrawDepth24:
			return SafeSubResolveAction(m_Shared.GetStdDepth24Action(), material);
		case RF_GreenScreen:
			return SafeSubResolveAction(m_Shared.GetStdMatteAction(), material);
		case RF_Black:
			return SafeSubResolveAction(m_Shared.GetStdBlackAction(), material);
		case RF_White:
			return SafeSubResolveAction(m_Shared.GetStdWhiteAction(), material);
		};
	}

	// total fallback:

	return SafeSubResolveAction(m_FallBackAction, material);
}
#endif

// CAfxBaseFxStream::CActionNoDraw /////////////////////////////////////////////

void CAfxBaseFxStream::CActionNoDraw::AfxUnbind(CAfxBaseFxStreamContext * ch)
{
	AfxD3D9OverrideEnd_D3DRS_ZWRITEENABLE();
	AfxD3D9OverrideEnd_D3DRS_COLORWRITEENABLE();
	AfxD3D9OverrideEnd_D3DRS_DESTBLEND();
	AfxD3D9OverrideEnd_D3DRS_SRCBLEND();
	AfxD3D9OverrideEnd_D3DRS_ALPHABLENDENABLE();

	AfxD3D9PopOverrideState();
}

void CAfxBaseFxStream::CActionNoDraw::MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{
	AfxD3D9PushOverrideState(false);

	AfxD3D9OverrideBegin_D3DRS_ALPHABLENDENABLE(TRUE);
	AfxD3D9OverrideBegin_D3DRS_SRCBLEND(D3DBLEND_ZERO);
	AfxD3D9OverrideBegin_D3DRS_DESTBLEND(D3DBLEND_ONE);
	AfxD3D9OverrideBegin_D3DRS_COLORWRITEENABLE(0);
	AfxD3D9OverrideBegin_D3DRS_ZWRITEENABLE(FALSE);
}


// CAfxBaseFxStream::CActionZOnly //////////////////////////////////////////////

void CAfxBaseFxStream::CActionZOnly::AfxUnbind(CAfxBaseFxStreamContext * ch)
{
	AfxD3D9OverrideEnd_D3DRS_COLORWRITEENABLE();
	AfxD3D9OverrideEnd_D3DRS_DESTBLEND();
	AfxD3D9OverrideEnd_D3DRS_SRCBLEND();
	AfxD3D9OverrideEnd_D3DRS_ALPHABLENDENABLE();

	AfxD3D9PopOverrideState();
}

void CAfxBaseFxStream::CActionZOnly::MaterialHook(CAfxBaseFxStreamContext * ch, IAfxMatRenderContext* ctx, const CAfxTrackedMaterialRef& trackedMaterial)
{
	AfxD3D9PushOverrideState(false);

	AfxD3D9OverrideBegin_D3DRS_ALPHABLENDENABLE(TRUE);
	AfxD3D9OverrideBegin_D3DRS_SRCBLEND(D3DBLEND_ZERO);
	AfxD3D9OverrideBegin_D3DRS_DESTBLEND(D3DBLEND_ONE);
	AfxD3D9OverrideBegin_D3DRS_COLORWRITEENABLE(0);
}

// CAfxBaseFxStream::CActionAfxVertexLitGenericHook ////////////////////////////

#if AFX_SHADERS_CSGO
csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20 CAfxBaseFxStream::CActionAfxVertexLitGenericHook::m_Combos_ps20;
csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20b CAfxBaseFxStream::CActionAfxVertexLitGenericHook::m_Combos_ps20b;
csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps30 CAfxBaseFxStream::CActionAfxVertexLitGenericHook::m_Combos_ps30;

CAfxBaseFxStream::CActionAfxVertexLitGenericHook::CActionAfxVertexLitGenericHook(CActionAfxVertexLitGenericHookKey & key)
: CAction()
, m_Key(key)
{
}

void CAfxBaseFxStream::CActionAfxVertexLitGenericHook::AfxUnbind(IAfxMatRenderContext * ctx)
{
	AfxD3D9_OverrideEnd_ps_c5();

	AfxD3D9OverrideEnd_D3DRS_SRGBWRITEENABLE();

	//AfxD3D9OverrideEnd_D3DRS_MULTISAMPLEANTIALIAS();

	AfxD3D9_OverrideEnd_SetPixelShader();
}

IMaterial_csgo * CAfxBaseFxStream::CActionAfxVertexLitGenericHook::MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material)
{
	// depth factors:

	int scale = g_bIn_csgo_CSkyBoxView_Draw ? csgo_CSkyBoxView_GetScale() : 1;
	float flDepthFactor = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthVal / scale;
	float flDepthFactorMax = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthValMax / scale;

	// Foce multisampling off for depth24:
	//if(m_Key.AFXMODE == ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_1)
	//	AfxD3D9OverrideBegin_D3DRS_MULTISAMPLEANTIALIAS(FALSE);

	// Force SRGBWriteEnable to off:
	AfxD3D9OverrideBegin_D3DRS_SRGBWRITEENABLE(FALSE);

	// Fill in g_AfxConstants in shader:

	float mulFac = flDepthFactorMax -flDepthFactor;
	mulFac = !mulFac ? 0.0f : 1.0f / mulFac;

	float overFac[4] = { flDepthFactor, mulFac, m_Key.AlphaTestReference, 0.0f };

	AfxD3D9_OverrideBegin_ps_c5(overFac);

	// Bind normal material:
	return material;
}

void CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader(CAfx_csgo_ShaderState & state)
{
	char const * shaderName = state.Static.SetPixelShader.pFileName.c_str();

	if(!strcmp(shaderName,"vertexlit_and_unlit_generic_ps20"))
	{
		static bool firstPass = true;
		if(firstPass)
		{
			firstPass = false;
			Tier0_Warning("AFXWARNING: You are using an untested code path in CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader for %s.\n", shaderName);
		}

		m_Combos_ps20.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		if(0 < m_Combos_ps20.m_LIGHTNING_PREVIEW)
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: 0 < m_LIGHTNING_PREVIEW not supported for %s.\n", shaderName);
			return;
		}

		int combo = ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::GetCombo(
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::AFXMODE_e)m_Key.AFXMODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::DETAILTEXTURE_e)m_Combos_ps20.m_DETAILTEXTURE,
			!m_Combos_ps20.m_BASEALPHAENVMAPMASK && !m_Combos_ps20.m_SELFILLUM ? ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_1 : ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_0,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::FLASHLIGHT_e)m_Combos_ps20.m_FLASHLIGHT,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::DETAIL_BLEND_MODE_e)m_Combos_ps20.m_DETAIL_BLEND_MODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::DESATURATEWITHBASEALPHA_e)m_Combos_ps20.m_DESATURATEWITHBASEALPHA,
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20::NOT_LIGHTING_PREVIEW_ONLY_0
		);

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_vertexlit_and_unlit_generic_ps20.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
	if(!strcmp(shaderName,"vertexlit_and_unlit_generic_ps20b"))
	{
		static bool firstPass = true;
		if(firstPass)
		{
			firstPass = false;
			Tier0_Warning("AFXWARNING: You are using an untested code path in CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader for %s.\n", shaderName);
		}

		m_Combos_ps20b.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		if(0 < m_Combos_ps20b.m_LIGHTNING_PREVIEW)
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: 0 < m_LIGHTNING_PREVIEW not supported for %s.\n", shaderName);
			return;
		}

		int combo = ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::GetCombo(
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::AFXMODE_e)m_Key.AFXMODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::DETAILTEXTURE_e)m_Combos_ps20b.m_DETAILTEXTURE,
			!m_Combos_ps20b.m_BASEALPHAENVMAPMASK && !m_Combos_ps20b.m_SELFILLUM ? ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_1 : ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_0,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::FLASHLIGHT_e)m_Combos_ps20b.m_FLASHLIGHT,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::DETAIL_BLEND_MODE_e)m_Combos_ps20b.m_DETAIL_BLEND_MODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::DESATURATEWITHBASEALPHA_e)m_Combos_ps20b.m_DESATURATEWITHBASEALPHA,
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps20b::NOT_LIGHTING_PREVIEW_ONLY_0
		);

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_vertexlit_and_unlit_generic_ps20b.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
	if(!strcmp(shaderName,"vertexlit_and_unlit_generic_ps30"))
	{
		m_Combos_ps30.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		if(0 < m_Combos_ps30.m_LIGHTNING_PREVIEW)
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: 0 < m_LIGHTNING_PREVIEW not supported for %s.\n", shaderName);
			return;
		}

		int combo = ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::GetCombo(
			m_Key.AFXMODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DETAILTEXTURE_e)m_Combos_ps30.m_DETAILTEXTURE,
			!m_Combos_ps30.m_BASEALPHAENVMAPMASK && !m_Combos_ps30.m_SELFILLUM ? ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_1 : ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_0,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::FLASHLIGHT_e)m_Combos_ps30.m_FLASHLIGHT,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DETAIL_BLEND_MODE_e)m_Combos_ps30.m_DETAIL_BLEND_MODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DESATURATEWITHBASEALPHA_e)m_Combos_ps30.m_DESATURATEWITHBASEALPHA,
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_LIGHTING_PREVIEW_ONLY_0
		);

		/*
		Tier0_Msg("%s %i %i - %i - %i %i %i %i %i %i -> %i\n",
			shaderName,
			state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex,
			m_Key.AFXMODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DETAILTEXTURE_e)m_Combos_ps30.m_DETAILTEXTURE,
			!m_Combos_ps30.m_BASEALPHAENVMAPMASK && !m_Combos_ps30.m_SELFILLUM ? ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_1 : ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_BASEALPHAENVMAPMASK_AND_NOT_SELFILLUM_0,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::FLASHLIGHT_e)m_Combos_ps30.m_FLASHLIGHT,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DETAIL_BLEND_MODE_e)m_Combos_ps30.m_DETAIL_BLEND_MODE,
			(ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::DESATURATEWITHBASEALPHA_e)m_Combos_ps30.m_DESATURATEWITHBASEALPHA,
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::NOT_LIGHTING_PREVIEW_ONLY_0,
			combo
			);
		*/

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_vertexlit_and_unlit_generic_ps30.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
		Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxVertexLitGenericHook::SetPixelShader: No replacement defined for %s.\n", shaderName);

}

// CAfxBaseFxStream::CActionAfxSpritecardHook ////////////////////////////

csgo_Stdshader_dx9_Combos_splinecard_vs20 CAfxBaseFxStream::CActionAfxSpritecardHook::m_Combos_splinecard_vs20;
csgo_Stdshader_dx9_Combos_spritecard_vs20 CAfxBaseFxStream::CActionAfxSpritecardHook::m_Combos_spritecard_vs20;
csgo_Stdshader_dx9_Combos_spritecard_ps20 CAfxBaseFxStream::CActionAfxSpritecardHook::m_Combos_ps20;
csgo_Stdshader_dx9_Combos_spritecard_ps20b CAfxBaseFxStream::CActionAfxSpritecardHook::m_Combos_ps20b;

CAfxBaseFxStream::CActionAfxSpritecardHook::CActionAfxSpritecardHook( CActionAfxSpritecardHookKey & key)
: CAction()
, m_Key(key)
{
}

void CAfxBaseFxStream::CActionAfxSpritecardHook::AfxUnbind(IAfxMatRenderContext * ctx)
{
	AfxD3D9_OverrideEnd_ps_c31();

	AfxD3D9OverrideEnd_D3DRS_SRGBWRITEENABLE();
	AfxD3D9OverrideEnd_D3DRS_DESTBLEND();
	AfxD3D9OverrideEnd_D3DRS_SRCBLEND();
	
	//AfxD3D9OverrideEnd_D3DRS_MULTISAMPLEANTIALIAS();

	AfxD3D9_OverrideEnd_SetPixelShader();
	AfxD3D9_OverrideEnd_SetVertexShader();
}

IMaterial_csgo * CAfxBaseFxStream::CActionAfxSpritecardHook::MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material)
{
	// depth factors:
	int scale = g_bIn_csgo_CSkyBoxView_Draw ? csgo_CSkyBoxView_GetScale() : 1;
	float flDepthFactor = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthVal / scale;
	float flDepthFactorMax = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthValMax / scale;

	// Force wanted state:

	// Foce multisampling off for depth24:
	//if(m_Key.AFXMODE == ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_1)
	//	AfxD3D9OverrideBegin_D3DRS_MULTISAMPLEANTIALIAS(FALSE);

	AfxD3D9OverrideBegin_D3DRS_SRCBLEND(D3DBLEND_SRCALPHA);
	AfxD3D9OverrideBegin_D3DRS_DESTBLEND(D3DBLEND_INVSRCALPHA);
	AfxD3D9OverrideBegin_D3DRS_SRGBWRITEENABLE(FALSE);

	// Fill in g_AfxConstants in shader:

	float mulFac = flDepthFactorMax -flDepthFactor;
	mulFac = !mulFac ? 0.0f : 1.0f / mulFac;

	float overFac[4] = { flDepthFactor, mulFac, m_Key.AlphaTestReference, 0.0f };

	AfxD3D9_OverrideBegin_ps_c31(overFac);

	// Bind normal material:
	return material;
}

void CAfxBaseFxStream::CActionAfxSpritecardHook::SetVertexShader(CAfx_csgo_ShaderState & state)
{
	char const * shaderName = state.Static.SetVertexShader.pFileName.c_str();

	if(!strcmp(shaderName,"splinecard_vs20"))
	{
		int remainder = m_Combos_splinecard_vs20.CalcCombos(state.Static.SetVertexShader.nStaticVshIndex, state.Dynamic.SetVertexShaderIndex.vshIndex);

		int combo = ShaderCombo_afxHook_splinecard_vs20::GetCombo(
			(ShaderCombo_afxHook_splinecard_vs20::ORIENTATION_e)m_Combos_splinecard_vs20.m_Orientation,
			(ShaderCombo_afxHook_splinecard_vs20::ADDBASETEXTURE2_e)m_Combos_splinecard_vs20.m_ADDBASETEXTURE2,
			(ShaderCombo_afxHook_splinecard_vs20::EXTRACTGREENALPHA_e)m_Combos_splinecard_vs20.m_EXTRACTGREENALPHA,
			(ShaderCombo_afxHook_splinecard_vs20::DUALSEQUENCE_e)m_Combos_splinecard_vs20.m_DUALSEQUENCE,
			(ShaderCombo_afxHook_splinecard_vs20::DEPTHBLEND_e)m_Combos_splinecard_vs20.m_DEPTHBLEND,
			(ShaderCombo_afxHook_splinecard_vs20::PACKED_INTERPOLATOR_e)m_Combos_splinecard_vs20.m_PACKED_INTERPOLATOR,
			(ShaderCombo_afxHook_splinecard_vs20::ANIMBLEND_OR_MAXLUMFRAMEBLEND1_e)m_Combos_splinecard_vs20.m_ANIMBLEND_OR_MAXLUMFRAMEBLEND1		
		);

		IAfxVertexShader * afxVertexShader = g_AfxShaders.GetAcsVertexShader("afxHook_splinecard_vs20.acs", combo);

		if(!afxVertexShader->GetVertexShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetVertexShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetVertexShader(afxVertexShader->GetVertexShader());
		}

		afxVertexShader->Release();

		return;
	}
	else
	if(!strcmp(shaderName,"spritecard_vs20"))
	{
		int remainder = m_Combos_spritecard_vs20.CalcCombos(state.Static.SetVertexShader.nStaticVshIndex, state.Dynamic.SetVertexShaderIndex.vshIndex);

		int combo = ShaderCombo_afxHook_spritecard_vs20::GetCombo(
			(ShaderCombo_afxHook_spritecard_vs20::ORIENTATION_e)m_Combos_spritecard_vs20.m_Orientation,
			(ShaderCombo_afxHook_spritecard_vs20::ZOOM_ANIMATE_SEQ2_e)m_Combos_spritecard_vs20.m_ZOOM_ANIMATE_SEQ2,
			(ShaderCombo_afxHook_spritecard_vs20::DUALSEQUENCE_e)m_Combos_spritecard_vs20.m_DUALSEQUENCE,
			(ShaderCombo_afxHook_spritecard_vs20::ADDBASETEXTURE2_e)m_Combos_spritecard_vs20.m_ADDBASETEXTURE2,
			(ShaderCombo_afxHook_spritecard_vs20::EXTRACTGREENALPHA_e)m_Combos_spritecard_vs20.m_EXTRACTGREENALPHA,
			(ShaderCombo_afxHook_spritecard_vs20::DEPTHBLEND_e)m_Combos_spritecard_vs20.m_DEPTHBLEND,
			(ShaderCombo_afxHook_spritecard_vs20::ANIMBLEND_OR_MAXLUMFRAMEBLEND1_e)m_Combos_spritecard_vs20.m_ANIMBLEND_OR_MAXLUMFRAMEBLEND1,
			(ShaderCombo_afxHook_spritecard_vs20::CROP_e)m_Combos_spritecard_vs20.m_CROP,
			(ShaderCombo_afxHook_spritecard_vs20::PACKED_INTERPOLATOR_e)m_Combos_spritecard_vs20.m_PACKED_INTERPOLATOR,
			(ShaderCombo_afxHook_spritecard_vs20::SPRITECARDVERTEXFOG_e)m_Combos_spritecard_vs20.m_SPRITECARDVERTEXFOG,
			(ShaderCombo_afxHook_spritecard_vs20::HARDWAREFOGBLEND_e)m_Combos_spritecard_vs20.m_HARDWAREFOGBLEND,
			(ShaderCombo_afxHook_spritecard_vs20::PERPARTICLEOUTLINE_e)m_Combos_spritecard_vs20.m_PERPARTICLEOUTLINE
		);

		IAfxVertexShader * afxVertexShader = g_AfxShaders.GetAcsVertexShader("afxHook_spritecard_vs20.acs", combo);

		if(!afxVertexShader->GetVertexShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetVertexShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetVertexShader(afxVertexShader->GetVertexShader());
		}

		afxVertexShader->Release();

		return;
	}
	else
		Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetVertexShader: No replacement defined for %s.\n", shaderName);
}

void CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader(CAfx_csgo_ShaderState & state)
{
	char const * shaderName = state.Static.SetPixelShader.pFileName.c_str();

	if(!strcmp(shaderName,"spritecard_ps20"))
	{
		static bool firstPass = true;
		if(firstPass)
		{
			firstPass = false;
			Tier0_Warning("AFXWARNING: You are using an untested code path in CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader for %s.\n", shaderName);
		}

		ShaderCombo_afxHook_spritecard_ps20::AFXORGBLENDMODE_e afxOrgBlendMode;

		if(state.Static.EnableBlending.bEnable)
		{
			if(SHADER_BLEND_DST_COLOR == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_SRC_COLOR == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20::AFXORGBLENDMODE_0;
			}
			else
			if(SHADER_BLEND_ONE == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE_MINUS_SRC_ALPHA == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20::AFXORGBLENDMODE_1;
			}
			else
			if(SHADER_BLEND_SRC_ALPHA == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20::AFXORGBLENDMODE_2;
			}
			else
			if(SHADER_BLEND_SRC_ALPHA == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE_MINUS_SRC_ALPHA == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20::AFXORGBLENDMODE_3;
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: current blend mode not supported for %s.\n", shaderName);
				return;
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: non-blending mode not supported for %s.\n", shaderName);
			return;
		}

		int remainder = m_Combos_ps20.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		int combo = ShaderCombo_afxHook_spritecard_ps20::GetCombo(
			(ShaderCombo_afxHook_spritecard_ps20::AFXMODE_e)m_Key.AFXMODE,
			afxOrgBlendMode,
			(ShaderCombo_afxHook_spritecard_ps20::DUALSEQUENCE_e)m_Combos_ps20.m_DUALSEQUENCE,
			(ShaderCombo_afxHook_spritecard_ps20::SEQUENCE_BLEND_MODE_e)m_Combos_ps20.m_SEQUENCE_BLEND_MODE,
			(ShaderCombo_afxHook_spritecard_ps20::ADDBASETEXTURE2_e)m_Combos_ps20.m_ADDBASETEXTURE2,
			(ShaderCombo_afxHook_spritecard_ps20::MAXLUMFRAMEBLEND1_e)m_Combos_ps20.m_MAXLUMFRAMEBLEND1,
			(ShaderCombo_afxHook_spritecard_ps20::MAXLUMFRAMEBLEND2_e)m_Combos_ps20.m_MAXLUMFRAMEBLEND2,
			(ShaderCombo_afxHook_spritecard_ps20::EXTRACTGREENALPHA_e)m_Combos_ps20.m_EXTRACTGREENALPHA,
			(ShaderCombo_afxHook_spritecard_ps20::COLORRAMP_e)m_Combos_ps20.m_COLORRAMP,
			(ShaderCombo_afxHook_spritecard_ps20::ANIMBLEND_e)m_Combos_ps20.m_ANIMBLEND,
			(ShaderCombo_afxHook_spritecard_ps20::ADDSELF_e)m_Combos_ps20.m_ADDSELF,
			(ShaderCombo_afxHook_spritecard_ps20::MOD2X_e)m_Combos_ps20.m_MOD2X,
			(ShaderCombo_afxHook_spritecard_ps20::COLOR_LERP_PS_e)m_Combos_ps20.m_COLOR_LERP_PS,
			(ShaderCombo_afxHook_spritecard_ps20::PACKED_INTERPOLATOR_e)m_Combos_ps20.m_PACKED_INTERPOLATOR,
			(ShaderCombo_afxHook_spritecard_ps20::DISTANCEALPHA_e)m_Combos_ps20.m_DISTANCEALPHA,
			(ShaderCombo_afxHook_spritecard_ps20::SOFTEDGES_e)m_Combos_ps20.m_SOFTEDGES,
			(ShaderCombo_afxHook_spritecard_ps20::OUTLINE_e)m_Combos_ps20.m_OUTLINE,
			(ShaderCombo_afxHook_spritecard_ps20::MULOUTPUTBYALPHA_e)m_Combos_ps20.m_MULOUTPUTBYALPHA
		);

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_spritecard_ps20.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();

		return;
	}
	else
	if(!strcmp(shaderName,"spritecard_ps20b"))
	{
		ShaderCombo_afxHook_spritecard_ps20b::AFXORGBLENDMODE_e afxOrgBlendMode;

		if(state.Static.EnableBlending.bEnable)
		{
			if(SHADER_BLEND_DST_COLOR == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_SRC_COLOR == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20b::AFXORGBLENDMODE_0;
			}
			else
			if(SHADER_BLEND_ONE == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE_MINUS_SRC_ALPHA == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20b::AFXORGBLENDMODE_1;
			}
			else
			if(SHADER_BLEND_SRC_ALPHA == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20b::AFXORGBLENDMODE_2;
			}
			else
			if(SHADER_BLEND_SRC_ALPHA == state.Static.BlendFunc.srcFactor
				&& SHADER_BLEND_ONE_MINUS_SRC_ALPHA == state.Static.BlendFunc.dstFactor)
			{
				afxOrgBlendMode = ShaderCombo_afxHook_spritecard_ps20b::AFXORGBLENDMODE_3;
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: current blend mode not supported for %s.\n", shaderName);
				return;
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: non-blending mode not supported for %s.\n", shaderName);
			return;
		}

		int remainder = m_Combos_ps20b.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		int combo = ShaderCombo_afxHook_spritecard_ps20b::GetCombo(
			(ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e)m_Key.AFXMODE,
			afxOrgBlendMode,
			(ShaderCombo_afxHook_spritecard_ps20b::DUALSEQUENCE_e)m_Combos_ps20b.m_DUALSEQUENCE,
			(ShaderCombo_afxHook_spritecard_ps20b::SEQUENCE_BLEND_MODE_e)m_Combos_ps20b.m_SEQUENCE_BLEND_MODE,
			(ShaderCombo_afxHook_spritecard_ps20b::ADDBASETEXTURE2_e)m_Combos_ps20b.m_ADDBASETEXTURE2,
			(ShaderCombo_afxHook_spritecard_ps20b::MAXLUMFRAMEBLEND1_e)m_Combos_ps20b.m_MAXLUMFRAMEBLEND1,
			(ShaderCombo_afxHook_spritecard_ps20b::MAXLUMFRAMEBLEND2_e)m_Combos_ps20b.m_MAXLUMFRAMEBLEND2,
			(ShaderCombo_afxHook_spritecard_ps20b::EXTRACTGREENALPHA_e)m_Combos_ps20b.m_EXTRACTGREENALPHA,
			(ShaderCombo_afxHook_spritecard_ps20b::COLORRAMP_e)m_Combos_ps20b.m_COLORRAMP,
			(ShaderCombo_afxHook_spritecard_ps20b::ANIMBLEND_e)m_Combos_ps20b.m_ANIMBLEND,
			(ShaderCombo_afxHook_spritecard_ps20b::ADDSELF_e)m_Combos_ps20b.m_ADDSELF,
			(ShaderCombo_afxHook_spritecard_ps20b::MOD2X_e)m_Combos_ps20b.m_MOD2X,
			(ShaderCombo_afxHook_spritecard_ps20b::COLOR_LERP_PS_e)m_Combos_ps20b.m_COLOR_LERP_PS,
			(ShaderCombo_afxHook_spritecard_ps20b::PACKED_INTERPOLATOR_e)m_Combos_ps20b.m_PACKED_INTERPOLATOR,
			(ShaderCombo_afxHook_spritecard_ps20b::DISTANCEALPHA_e)m_Combos_ps20b.m_DISTANCEALPHA,
			(ShaderCombo_afxHook_spritecard_ps20b::SOFTEDGES_e)m_Combos_ps20b.m_SOFTEDGES,
			(ShaderCombo_afxHook_spritecard_ps20b::OUTLINE_e)m_Combos_ps20b.m_OUTLINE,
			(ShaderCombo_afxHook_spritecard_ps20b::MULOUTPUTBYALPHA_e)m_Combos_ps20b.m_MULOUTPUTBYALPHA
		);

		/*
		Tier0_Msg(
			"%i -> %i %i | %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i (%i)\n",
			combo,
			(ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e)m_Key.AFXMODE,
			afxOrgBlendMode,
			(ShaderCombo_afxHook_spritecard_ps20b::DUALSEQUENCE_e)m_Combos_ps20b.m_DUALSEQUENCE,
			(ShaderCombo_afxHook_spritecard_ps20b::SEQUENCE_BLEND_MODE_e)m_Combos_ps20b.m_SEQUENCE_BLEND_MODE,
			(ShaderCombo_afxHook_spritecard_ps20b::ADDBASETEXTURE2_e)m_Combos_ps20b.m_ADDBASETEXTURE2,
			(ShaderCombo_afxHook_spritecard_ps20b::MAXLUMFRAMEBLEND1_e)m_Combos_ps20b.m_MAXLUMFRAMEBLEND1,
			(ShaderCombo_afxHook_spritecard_ps20b::MAXLUMFRAMEBLEND2_e)m_Combos_ps20b.m_MAXLUMFRAMEBLEND2,
			(ShaderCombo_afxHook_spritecard_ps20b::EXTRACTGREENALPHA_e)m_Combos_ps20b.m_EXTRACTGREENALPHA,
			(ShaderCombo_afxHook_spritecard_ps20b::COLORRAMP_e)m_Combos_ps20b.m_COLORRAMP,
			(ShaderCombo_afxHook_spritecard_ps20b::ANIMBLEND_e)m_Combos_ps20b.m_ANIMBLEND,
			(ShaderCombo_afxHook_spritecard_ps20b::ADDSELF_e)m_Combos_ps20b.m_ADDSELF,
			(ShaderCombo_afxHook_spritecard_ps20b::MOD2X_e)m_Combos_ps20b.m_MOD2X,
			(ShaderCombo_afxHook_spritecard_ps20b::COLOR_LERP_PS_e)m_Combos_ps20b.m_COLOR_LERP_PS,
			(ShaderCombo_afxHook_spritecard_ps20b::PACKED_INTERPOLATOR_e)m_Combos_ps20b.m_PACKED_INTERPOLATOR,
			(ShaderCombo_afxHook_spritecard_ps20b::DISTANCEALPHA_e)m_Combos_ps20b.m_DISTANCEALPHA,
			(ShaderCombo_afxHook_spritecard_ps20b::SOFTEDGES_e)m_Combos_ps20b.m_SOFTEDGES,
			(ShaderCombo_afxHook_spritecard_ps20b::OUTLINE_e)m_Combos_ps20b.m_OUTLINE,
			(ShaderCombo_afxHook_spritecard_ps20b::MULOUTPUTBYALPHA_e)m_Combos_ps20b.m_MULOUTPUTBYALPHA,
			remainder
			);
		*/

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_spritecard_ps20b.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
		Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSpritecardHook::SetPixelShader: No replacement defined for %s.\n", shaderName);

}

// CAfxBaseFxStream::CActionUnlitGenericFallback ///////////////////////////////

CAfxBaseFxStream::CActionUnlitGenericFallback::CActionUnlitGenericFallback(CActionAfxVertexLitGenericHookKey & key, char const * unlitGenericFallbackMaterialName)
: CActionAfxVertexLitGenericHook(key)
, m_Material(0)
, m_MaterialName(unlitGenericFallbackMaterialName)
{
}

CAfxBaseFxStream::CActionUnlitGenericFallback::~CActionUnlitGenericFallback()
{
	if(m_Material) delete m_Material;
}

IMaterial_csgo * CAfxBaseFxStream::CActionUnlitGenericFallback::MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material)
{
	if(!m_Material) m_Material = new CAfxMaterial(
		CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_Streams->GetFreeMaster(),
		CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_Streams->GetMaterialSystem()->FindMaterial(m_MaterialName.c_str(), 0));

	return CActionAfxVertexLitGenericHook::MaterialHook(ctx, m_Material->GetMaterial());
}

// CAfxBaseFxStream::CActionAfxSplineRopeHook //////////////////////////////////

csgo_Stdshader_dx9_Combos_splinerope_ps20 CAfxBaseFxStream::CActionAfxSplineRopeHook::m_Combos_ps20;
csgo_Stdshader_dx9_Combos_splinerope_ps20b CAfxBaseFxStream::CActionAfxSplineRopeHook::m_Combos_ps20b;

CAfxBaseFxStream::CActionAfxSplineRopeHook::CActionAfxSplineRopeHook(CActionAfxSplineRopeHookKey & key)
: CAction()
, m_Key(key)
{
}

void CAfxBaseFxStream::CActionAfxSplineRopeHook::AfxUnbind(IAfxMatRenderContext * ctx)
{
	AfxD3D9_OverrideEnd_ps_c31();

	AfxD3D9OverrideEnd_D3DRS_SRGBWRITEENABLE();
	
	//AfxD3D9OverrideEnd_D3DRS_MULTISAMPLEANTIALIAS();

	AfxD3D9_OverrideEnd_SetPixelShader();
}

IMaterial_csgo * CAfxBaseFxStream::CActionAfxSplineRopeHook::MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material)
{
	// depth factors:

	int scale = g_bIn_csgo_CSkyBoxView_Draw ? csgo_CSkyBoxView_GetScale() : 1;
	float flDepthFactor = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthVal / scale;
	float flDepthFactorMax = CAfxBaseFxStream::m_Shared.m_ActiveBaseFxStream->m_DepthValMax / scale;

	// Foce multisampling off for depth24:
	//if(m_Key.AFXMODE == ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_1)
	//	AfxD3D9OverrideBegin_D3DRS_MULTISAMPLEANTIALIAS(FALSE);

	// Force SRGBWriteEnable to off:
	AfxD3D9OverrideBegin_D3DRS_SRGBWRITEENABLE(FALSE);

	// Fill in g_AfxConstants in shader:

	float mulFac = flDepthFactorMax -flDepthFactor;
	mulFac = !mulFac ? 0.0f : 1.0f / mulFac;

	float overFac[4] = { flDepthFactor, mulFac, m_Key.AlphaTestReference, 0.0f };

	AfxD3D9_OverrideBegin_ps_c31(overFac);

	// Bind normal material:
	return material;
	//return m_Material.GetMaterial();
}

void CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader(CAfx_csgo_ShaderState & state)
{
	char const * shaderName = state.Static.SetPixelShader.pFileName.c_str();

	if(!strcmp(shaderName,"splinerope_ps20"))
	{
		static bool firstPass = true;
		if(firstPass)
		{
			firstPass = false;
			Tier0_Warning("AFXWARNING: You are using an untested code path in CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader for %s.\n", shaderName);
		}

		m_Combos_ps20.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		if(0 < m_Combos_ps20.m_SHADOWDEPTH)
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader: 0 < m_SHADOWDEPTH not supported for %s.\n", shaderName);
			return;
		}

		int combo = ShaderCombo_afxHook_splinerope_ps20::GetCombo(
			(ShaderCombo_afxHook_splinerope_ps20::AFXMODE_e)m_Key.AFXMODE,
			(ShaderCombo_afxHook_splinerope_ps20::SHADER_SRGB_READ_e)m_Combos_ps20.m_SHADER_SRGB_READ,
			(ShaderCombo_afxHook_splinerope_ps20::ALPHATESTREF_e)m_Combos_ps20.m_ALPHATESTREF
		);

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_splinerope_ps20.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
	if(!strcmp(shaderName,"splinerope_ps20b"))
	{
		m_Combos_ps20b.CalcCombos(state.Static.SetPixelShader.nStaticPshIndex, state.Dynamic.SetPixelShaderIndex.pshIndex);

		/*
		static bool firstPass = true;
		if(firstPass)
		{
			firstPass = false;
			
			Tier0_Warning("Requesting dump of shader %s %i_%i_%i_%i_%i.\n", shaderName,
				m_Combos_ps20b.m_WRITE_DEPTH_TO_DESTALPHA,
				m_Combos_ps20b.m_PIXELFOGTYPE,
				m_Combos_ps20b.m_SHADER_SRGB_READ,
				m_Combos_ps20b.m_SHADOWDEPTH,
				m_Combos_ps20b.m_ALPHATESTREF
				);
			
			g_bD3D9DumpPixelShader = true;
		}

		return;
		*/

		if(0 < m_Combos_ps20b.m_SHADOWDEPTH)
		{
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader: 0 < m_SHADOWDEPTH not supported for %s.\n", shaderName);
			return;
		}

		int combo = ShaderCombo_afxHook_splinerope_ps20b::GetCombo(
			(ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_e)m_Key.AFXMODE,
			(ShaderCombo_afxHook_splinerope_ps20b::SHADER_SRGB_READ_e)m_Combos_ps20b.m_SHADER_SRGB_READ,
			(ShaderCombo_afxHook_splinerope_ps20b::ALPHATESTREF_e)m_Combos_ps20b.m_ALPHATESTREF
		);

		IAfxPixelShader * afxPixelShader = g_AfxShaders.GetAcsPixelShader("afxHook_splinerope_ps20b.acs", combo);

		if(!afxPixelShader->GetPixelShader())
			Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader: Replacement Shader combo %i for %s is null.\n", combo, shaderName);
		else
		{
			// Override shader:
			AfxD3D9_OverrideBegin_SetPixelShader(afxPixelShader->GetPixelShader());
		}

		afxPixelShader->Release();
	}
	else
		Tier0_Warning("AFXERROR: CAfxBaseFxStream::CActionAfxSplineRopeHook::SetPixelShader: No replacement defined for %s.\n", shaderName);

}
#endif

// CAfxStreamsCaptureOutput ////////////////////////////////////////////////////

CAfxStreamsCaptureOutput::CAfxStreamsCaptureOutput(class CAfxRecordStream* target, size_t streamIndex)
	: advancedfx::CRefCountedThreadSafe()
	, m_Target(target)
	, m_StreamIndex(streamIndex) {
	target->AddRef();
}

CAfxStreamsCaptureOutput::~CAfxStreamsCaptureOutput() {
	m_Target->Release();
}

void CAfxStreamsCaptureOutput::OnCapture(class ICapture* capture) {
	m_Target->OnImageBufferCaptured(m_StreamIndex, capture);
}


// CAfxStreams /////////////////////////////////////////////////////////////////

CAfxStreams::CAfxStreams()
: m_RecordName("untitled_rec")
, m_PresentRecordOnScreen(false)
, m_StartMovieWav(true)
, m_RecordVoices(false)
, m_MaterialSystem(0)
, m_AfxBaseClientDll(0)
, m_ShaderShadow(0)
, m_Recording(false)
, m_Frame(0)
, m_FormatBmpAndNotTga(false)
, m_View_Render_ThreadId(0)
//, m_RgbaRenderTarget(nullptr)
//, m_RenderTargetDepthF(nullptr)
, m_CamBvh(false)
, m_GameRecording(false)
{

}

CAfxStreams::~CAfxStreams()
{
	ShutDown();
}

void CAfxStreams::OnClientEntityCreated(SOURCESDK::C_BaseEntity_csgo* ent) {
}

void CAfxStreams::OnClientEntityDeleted(SOURCESDK::C_BaseEntity_csgo* ent) {
	QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxDeleteEntityMetaFunctor(ent->GetIClientUnknown())));
}

bool CAfxStreams::OnEngineThread() {
	return GetCurrentThreadId() == m_View_Render_ThreadId;
}

bool CAfxStreams::IsQueuedThreaded() {
	return nullptr != m_MaterialSystem && (m_MaterialSystem->GetThreadMode() == SOURCESDK::CSGO::MATERIAL_QUEUED_THREADED);
}

bool CAfxStreams::IsSingleThreaded() {
	return nullptr != m_MaterialSystem && (m_MaterialSystem->GetThreadMode() == SOURCESDK::CSGO::MATERIAL_SINGLE_THREADED);
}

void CAfxStreams::OnMaterialSystem(SOURCESDK::IMaterialSystem_csgo * value)
{
	m_MaterialSystem = value;

	CreateRenderTargets(value);
}

void CAfxStreams::Set_View_Render_ThreadId(DWORD id)
{
	InterlockedExchange(&m_View_Render_ThreadId, id);
}

DWORD CAfxStreams::Get_View_Render_ThreadId()
{
	return InterlockedCompareExchange(&m_View_Render_ThreadId, -1, -1);
}


void CAfxStreams::OnAfxBaseClientDll(IAfxBaseClientDll * value)
{
	m_AfxBaseClientDll = value;
	if(m_AfxBaseClientDll)
	{
		m_AfxBaseClientDll->OnView_Render_set(this);
	}
}

void CAfxStreams::OnAfxBaseClientDll_Free(void)
{
	/*
	if(m_RenderTargetDepthF)
	{
		m_RenderTargetDepthF->DecrementReferenceCount();
		m_RenderTargetDepthF = 0;
	}
	*/
	/*
	if(m_RgbaRenderTarget)
	{
		m_RgbaRenderTarget->DecrementReferenceCount();
		m_RgbaRenderTarget = 0;
	}*/

	if(m_AfxBaseClientDll)
	{
		m_AfxBaseClientDll->OnView_Render_set(0);
		m_AfxBaseClientDll = 0;
	}
}

void CAfxStreams::OnShaderShadow(SOURCESDK::IShaderShadow_csgo * value)
{
	m_ShaderShadow = value;
}

#if AFX_SHADERS_CSGO
void CAfxStreams::OnSetVertexShader(CAfx_csgo_ShaderState & state)
{
	IAfxStreamContext * hook = FindStreamContext(GetCurrentContext());

	if (hook)
		hook->SetVertexShader(state);
}

void CAfxStreams::OnSetPixelShader(CAfx_csgo_ShaderState & state)
{
	IAfxStreamContext * hook = FindStreamContext(GetCurrentContext());

	if (hook)
		hook->SetPixelShader(state);
}
#endif

IAfxMatRenderContextOrg * CAfxStreams::CaptureStream(IAfxMatRenderContextOrg * ctxp, CAfxRecordStream * stream, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer)
{
	size_t streamCount = stream->GetStreamCount();

	for(size_t streamIndex=0; streamIndex < streamCount; ++streamIndex)
	{
		CAfxRenderViewStream * renderViewStream = stream->GetStream(streamIndex);

		ctxp = CaptureStreamToBuffer(ctxp, streamIndex, renderViewStream, stream, 0 == streamIndex, streamIndex + 1 >= streamCount, fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, smokeOverlayAlphaFactorMultiplyer);
	}

	return ctxp;
}

IAfxMatRenderContextOrg * CAfxStreams::PreviewStream(IAfxMatRenderContextOrg * ctxp, CAfxRenderViewStream * previewStream, int slot, int cols, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer)
{
	if (m_FirstStreamToBeRendered) {
		m_FirstStreamToBeRendered = false;
	}
	else {
		ctxp = CommitDrawingContext(ctxp, !m_PresentLastStream);
		m_PresentLastStream = false;
	}

	SetMatVarsForStreams(); // keep them set in case someone resets them.

	if (0 < strlen(previewStream->AttachCommands_get()))
		WrpConCommands::ImmediatelyExecuteCommands(previewStream->AttachCommands_get()); // Execute commands before we lock the stream!

	AfxViewportData_t afxViewport = {
		view.m_nUnscaledX,
		view.m_nUnscaledY,
		view.m_nUnscaledWidth,
		view.m_nUnscaledHeight,
		view.zNear,
		view.zFar
	};

	SOURCESDK::VMatrix worldToView;
	SOURCESDK::VMatrix viewToProjection;
	SOURCESDK::VMatrix worldToProjection;
	SOURCESDK::VMatrix worldToPixels;

	SOURCESDK::VMatrix viewToProjectionSky;

	{
		SOURCESDK::CViewSetup_csgo skyView = view;

		int scale = csgo_CSkyBoxView_GetScale();

		skyView.zNear = 2.0f * scale;
		skyView.zFar = (float)SOURCESDK_CSGO_MAX_TRACE_LENGTH * scale;

		g_pVRenderView_csgo->GetMatricesForView(skyView, &worldToView, &viewToProjectionSky, &worldToProjection, &worldToPixels);
	}

	g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

	int myWhatToDraw = whatToDraw;

	switch (previewStream->DrawHud_get())
	{
	case CAfxRenderViewStream::DT_Draw:
		myWhatToDraw |= SOURCESDK::RENDERVIEW_DRAWHUD;
		break;
	case CAfxRenderViewStream::DT_NoDraw:
		//myWhatToDraw &= ~SOURCESDK::RENDERVIEW_DRAWHUD;
		break;
	case CAfxRenderViewStream::DT_NoChange:
	default:
		break;
	}

	switch (previewStream->DrawViewModel_get())
	{
	case CAfxRenderViewStream::DT_Draw:
		myWhatToDraw |= SOURCESDK::RENDERVIEW_DRAWVIEWMODEL;
		break;
	case CAfxRenderViewStream::DT_NoDraw:
		myWhatToDraw &= ~SOURCESDK::RENDERVIEW_DRAWVIEWMODEL;
		break;
	case CAfxRenderViewStream::DT_NoChange:
	default:
		break;
	}

	SOURCESDK::CViewSetup_csgo newView = view;
	SOURCESDK::CViewSetup_csgo newHudView = hudViewSetup;

	int col = slot % cols;
	int row = slot / cols;

	newView.m_nUnscaledWidth /= cols;
	newView.m_nUnscaledHeight /= cols;
	newView.m_nUnscaledX += col * newView.m_nUnscaledWidth;
	newView.m_nUnscaledY += row * newView.m_nUnscaledHeight;
	newView.width /= cols;
	newView.height /= cols;
	newView.x += col * newView.width;
	newView.y += row * newView.height;

	newHudView.m_nUnscaledWidth /= cols;
	newHudView.m_nUnscaledHeight /= cols;
	newHudView.m_nUnscaledX += col * newHudView.m_nUnscaledWidth;
	newHudView.m_nUnscaledY += row * newHudView.m_nUnscaledHeight;
	newHudView.width /= cols;
	newHudView.height /= cols;
	newHudView.x += col * newHudView.width;
	newHudView.y += row * newHudView.height;

	float oldSmokeOverlayAlphaFactor = *smokeOverlayAlphaFactor;
	smokeOverlayAlphaFactorMultiplyer = previewStream->SmokeOverlayAlphaFactor_get();
	if (smokeOverlayAlphaFactorMultiplyer < 1) *smokeOverlayAlphaFactor = 0;

	{
		bool forceBuildingCubeMaps = true;

		float oldFrameTime;
		int oldBuildingCubeMaps;

		if (true)
		{
			forceBuildingCubeMaps = previewStream->ForceBuildingCubemaps_get();
		}
		else
		{
			oldFrameTime = g_Hook_VClient_RenderView.GetGlobals()->frametime_get();
			g_Hook_VClient_RenderView.GetGlobals()->frametime_set(0);
		}

		if (forceBuildingCubeMaps)
		{
			oldBuildingCubeMaps = m_BuildingCubemaps->GetInt();
			m_BuildingCubemaps->SetValue(1.0f);
		}

		ctxp->PushRenderTargetAndViewport( nullptr, nullptr, newView.m_nUnscaledX, newView.m_nUnscaledY, newView.m_nUnscaledWidth, newView.m_nUnscaledHeight);


		bool oldDoBloomAndToneMapping;
		bool overrideDoBloomAndToneMapping;
		{
			bool value;
			if (overrideDoBloomAndToneMapping = previewStream->DoBloomAndToneMapping.Get(value))
			{
				oldDoBloomAndToneMapping = newView.m_bDoBloomAndToneMapping;
				const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDoBloomAndToneMapping = value;
			}
		}
		bool oldDoDepthOfField;
		bool overrideDoDepthOfField;
		{
			bool value;
			if (overrideDoDepthOfField = previewStream->DoDepthOfField.Get(value))
			{
				oldDoDepthOfField = newView.m_bDoDepthOfField;
				const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDoDepthOfField = value;
			}
		}
		bool oldDrawWorldNormal;
		bool overrideDrawWorldNormal;
		{
			bool value;
			if (overrideDrawWorldNormal = previewStream->DrawWorldNormal.Get(value))
			{
				oldDrawWorldNormal = newView.m_bDrawWorldNormal;
				const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDrawWorldNormal = value;
			}
		}
		bool oldCullFrontFaces;
		bool overrideCullFrontFaces;
		{
			bool value;
			if (overrideCullFrontFaces = previewStream->CullFrontFaces.Get(value))
			{
				oldCullFrontFaces = newView.m_bCullFrontFaces;
				const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bCullFrontFaces = value;
			}
		}
		bool oldRenderFlashlightDepthTranslucents;
		bool overrideRenderFlashlightDepthTranslucents;
		{
			bool value;
			if (overrideRenderFlashlightDepthTranslucents = previewStream->RenderFlashlightDepthTranslucents.Get(value))
			{
				oldRenderFlashlightDepthTranslucents = newView.m_bRenderFlashlightDepthTranslucents;
				const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bRenderFlashlightDepthTranslucents = value;
			}
		}

		//if (previewStream->GetDisableFastPath()) DisableFastPath();

		previewStream->OnRenderBegin(nullptr, afxViewport, viewToProjection, viewToProjectionSky);

		DoRenderView(fn, This, Edx, newView, newHudView, nClearFlags, myWhatToDraw);

		previewStream->OnRenderEnd();

		//if (previewStream->GetDisableFastPath()) RestoreFastPath();

		if (overrideRenderFlashlightDepthTranslucents) const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bRenderFlashlightDepthTranslucents = oldRenderFlashlightDepthTranslucents;
		if (overrideCullFrontFaces) const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bCullFrontFaces = oldCullFrontFaces;
		if (overrideDrawWorldNormal) const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDrawWorldNormal = oldDrawWorldNormal;
		if (overrideDoDepthOfField) const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDoDepthOfField = oldDoDepthOfField;
		if (overrideDoBloomAndToneMapping) const_cast<SOURCESDK::CViewSetup_csgo &>(newView).m_bDoBloomAndToneMapping = oldDoBloomAndToneMapping;

		ctxp->PopRenderTargetAndViewport();


		if (forceBuildingCubeMaps)
		{
			m_BuildingCubemaps->SetValue((float)oldBuildingCubeMaps);
		}

		if (true)
		{
		}
		else
		{
			g_Hook_VClient_RenderView.GetGlobals()->frametime_set(oldFrameTime);
		}
	}

	*smokeOverlayAlphaFactor = oldSmokeOverlayAlphaFactor;

	if (0 < strlen(previewStream->DetachCommands_get()))
		WrpConCommands::ImmediatelyExecuteCommands(previewStream->DetachCommands_get()); // Execute commands after we unlocked the stream!

	return ctxp;
}

void CAfxStreams::DoRenderView(CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw)
{
	m_DoRenderViewCount++;
	if(m_ShowRenderViewCount) Tier0_Msg("m_DoRenderViewCount: %i (m_ForceCacheFullSceneState: %i)\n", m_DoRenderViewCount, m_ForceCacheFullSceneState ? 1 : 0);

#ifdef AFX_INTEROP
	if (AfxInterop::Enabled() && AfxInterop::Active())
	{
		AfxInterop::OnRenderView(view, g_InteropFeatures);

		if (g_InteropFeatures.GetDepthRequired())
		{
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxInteropOverrideDepthBegin_Functor()));
		}
	}
#endif

	bool oldCacheFullSceneState;
	if (m_ForceCacheFullSceneState)
	{
		oldCacheFullSceneState = view.m_bCacheFullSceneState;
		const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bCacheFullSceneState = true;
	}

	fn(This, Edx, view, hudViewSetup, nClearFlags, whatToDraw);

	if (m_ForceCacheFullSceneState)
	{
		const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bCacheFullSceneState = oldCacheFullSceneState;
	}

#ifdef AFX_INTEROP
	if (AfxInterop::Enabled() && AfxInterop::Active())
	{
		AfxInterop::OnRenderViewEnd();

		if (g_InteropFeatures.GetDepthRequired())
		{
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CAfxInteropOverrideDepthEnd_Functor()));
		}
	}
#endif
}

void CAfxStreams::CalcMainStream()
{
	switch (m_MainStreamMode)
	{
	case MainStreamMode_None:
		m_MainStream = nullptr;
		break;
	case MainStreamMode_FirstActive:
		m_MainStream = nullptr;
		if (m_Recording)
		{
			for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
			{
				CAfxRecordStream * recordStream = *it;
				if (recordStream->Record_get())
				{
					m_MainStream = recordStream;
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < 16; ++i)
			{
				if (m_PreviewStreams[i])
				{
					if (1 <= m_PreviewStreams[i]->GetStreamCount())
					{
						m_MainStream = m_PreviewStreams[i];
						break;
					}
				}
			}
		}
		break;
	case MainStreamMode_First:
		m_MainStream = nullptr;
		for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); )
		{
			CAfxRecordStream * recordStream = *it;
			m_MainStream = recordStream;
			break;
		}
		break;
	case MainStreamMode_Set:
		break;
	}
}

class CCaptureNodeGpuQueuesExecution
	: public CAfxFunctor
{
public:
	virtual void operator()() {
		CCaptureNode::GpuExecuteLockQueue();
	}
};

void QueueCaptureGpuQueuesExecution() {
	QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new class CCaptureNodeGpuQueuesExecution()));
}

class CCaptureNodeGpuQueuesRelease
	: public CAfxFunctor
{
public:
	virtual void operator()() {
		CCaptureNode::GpuExecuteReleaseQueue();
	}
};

void QueueCaptureGpuQueuesRelease() {
	QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new class CCaptureNodeGpuQueuesRelease()));
}

class CPushRenderTargetFunctor
	: public CAfxFunctor
{
public:
	virtual void operator()() {
		g_AfxStreams.DrawingThread_SetRenderTarget();
	}
};

void QueuePushRenderTarget() {
	QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new class CPushRenderTargetFunctor()));
}

class CPopRenderTargetFunctor
	: public CAfxFunctor
{
public:
	virtual void operator()() {
		g_AfxStreams.DrawingThread_UnsetRenderTarget(false);
	}
};

void QueuePopRenderTarget() {
	QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new class CPopRenderTargetFunctor()));
}


IAfxMatRenderContextOrg* CAfxStreams::CommitDrawingContext(IAfxMatRenderContextOrg* context, bool blockPresent) {
	if (blockPresent)
	{
		if (!m_PresentBlocked) {
			BlockPresent(context, true);
			m_PresentBlocked = true;
		}
	}
	else {
		if (m_PresentBlocked)
		{
			BlockPresent(context, false);
			m_PresentBlocked = false;
		}
	}

	// Work around game running out of memory because of too much shit on the queue
	// aka issue ripieces/advancedfx-prop#22 by using a sub-context:
	if (m_MaterialSystem->GetThreadMode() != SOURCESDK::CSGO::MATERIAL_SINGLE_THREADED) {
		// Only do this when we are in threaded mode, it's only needed then and otherwise we will crash when switching from un-threaded to threaded.
		m_MaterialSystem->EndFrame();
		m_MaterialSystem->SwapBuffers(); // Apparently we have to do this always, otherwise the state is messed up.
		m_MaterialSystem->BeginFrame(0);
		context = GetCurrentContext()->GetOrg(); // We are potentially on a new context now
	}

	return context;
}

void CAfxStreams::OnRenderView(CCSViewRender_RenderView_t fn, void * This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer)
{
	m_ForceCacheFullSceneState = false;

	smokeOverlayAlphaFactorMultiplyer = 1;

	m_CurrentView = &view;

	if(m_FirstRenderAfterLevelInit)
	{
		// HACK: Rendering streams during map load will crash things, so don't do that.

		m_FirstRenderAfterLevelInit = false;

		fn(This, Edx, view, hudViewSetup, nClearFlags, whatToDraw);
		return;
	}

	QueueCaptureGpuQueuesRelease();

	//Hook_csgo_C_BaseEntity_IClientRenderable_DrawModel();
	//Hook_csgo_C_BaseAnimating_IClientRenderable_DrawModel();
	//Hook_csgo_C_BaseCombatWeapon_IClientRenderable_DrawModel();
	//Hook_csgo_CStaticProp_IClientRenderable_DrawModel();

	IAfxMatRenderContextOrg * ctxp = GetCurrentContext()->GetOrg();

	CalcMainStream();

	std::list<CAfxRecordStream*> recordStreams;
	int previewNumSlots = 0;
	std::map<int, CAfxRecordStream*> previewSlotsToStreams;
	CAfxRecordStream* firstPreviewedStream = nullptr;
	CAfxRecordStream* lastStreamRecorded = nullptr;

	if (!CheckCanFeedStreams()) {
		Tier0_Warning("Error: Cannot record / preview streams due to missing dependencies!\n");
	}
	else {
		if (m_Recording)
		{
			for (std::list<CAfxRecordStream*>::iterator it = m_Streams.begin(); it != m_Streams.end(); it++)
			{
				if (!(*it)->Record_get()) continue;

				if ((*it) == m_MainStream) {
					recordStreams.emplace_front(*it);
				}
				else
					recordStreams.emplace_back(*it);
			}
		}

		if (!m_SuspendPreview)
		{
			for (int i = 0; i < 16; ++i)
			{
				if (m_PreviewStreams[i])
				{
					if (1 <= m_PreviewStreams[i]->GetStreamCount())
					{
						previewNumSlots = 1 + i;
						previewSlotsToStreams[i] = m_PreviewStreams[i];
						if (firstPreviewedStream == nullptr)
							firstPreviewedStream = m_PreviewStreams[i];
					}
				}
			}
		}
	}

	bool hasPushedTarget = false;

	bool bNeedsPreRender =
		nullptr != m_MainStream && (
			1 <= recordStreams.size() && recordStreams.front() != m_MainStream
			|| 0 == recordStreams.size() && 1 <= previewNumSlots && previewSlotsToStreams.begin()->second != m_MainStream
		)
		|| nullptr == m_MainStream && (
			1 <= recordStreams.size() && !recordStreams.front()->IsTrulyNormalGameView()
			|| 0 == recordStreams.size() && 1 <= previewNumSlots && !previewSlotsToStreams.begin()->second->IsTrulyNormalGameView()
		)
	;

	bool bPushedRenderTarget = false;

	m_FirstStreamToBeRendered = true;
	m_PresentLastStream = false;
	m_DoRenderViewCount = 0;

	if (bNeedsPreRender) {
		m_ForceCacheFullSceneState = true;
		if (m_MainStream) {
			if (m_MainStream->Record_get()) {
				ctxp = CaptureStream(ctxp, m_MainStream, fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, smokeOverlayAlphaFactorMultiplyer);
				lastStreamRecorded = m_MainStream;
			}
			else {
				CAfxRenderViewStream* previewStream = m_MainStream->GetStream(0);
				ctxp = PreviewStream(ctxp, previewStream, 0, 1, fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, smokeOverlayAlphaFactorMultiplyer);
			}
			if (previewNumSlots == 1 && firstPreviewedStream == m_MainStream)
			{
				QueuePushRenderTarget();
				bPushedRenderTarget = true;
			}
		}
		else {
			// Render normal game view and freeze it
			m_FirstStreamToBeRendered = false;
			DoRenderView(fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw);

			if (previewNumSlots == 1 && firstPreviewedStream->IsTrulyNormalGameView())
			{
				QueuePushRenderTarget();
				bPushedRenderTarget = true;
			}
		}
	}

	for (auto it = recordStreams.begin(); it != recordStreams.end(); it++) {
		if (*it == m_MainStream && bNeedsPreRender) continue;

		auto nextIt = it;
		nextIt++;
		m_ForceCacheFullSceneState = false
			|| nextIt != recordStreams.end()
			|| 1 < previewNumSlots
			|| 1 == previewNumSlots && *it != previewSlotsToStreams.begin()->second && !bPushedRenderTarget
			|| 0 == previewNumSlots && !(*it)->IsTrulyNormalGameView() && !bPushedRenderTarget
			;
		ctxp = CaptureStream(ctxp, *it, fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, smokeOverlayAlphaFactorMultiplyer);
		lastStreamRecorded = *it;
		if (!bPushedRenderTarget && m_ForceCacheFullSceneState && 1 == previewNumSlots && *it == previewSlotsToStreams.begin()->second) {
			QueuePushRenderTarget();
			bPushedRenderTarget = true;
		}
	}

	if (1 <= previewNumSlots) {
		int cols = 1;

		if (4 < previewNumSlots)
			cols = 4;
		else if (1 < previewNumSlots)
			cols = 2;

		int num = 0;

		for (auto it = previewSlotsToStreams.begin(); it != previewSlotsToStreams.end(); it++)
		{
			num++;
			if (bPushedRenderTarget) {
				QueuePopRenderTarget();
				bPushedRenderTarget = false;
				continue;
			}
			else if (1 == previewNumSlots && lastStreamRecorded == it->second) {
				continue;
			}

			int slot = it->first;

			slot = cols * cols - slot - 1; // We draw backwards (bottom,right) -> (top,left) in order to solve some weird problem.

			CAfxRenderViewStream* previewStream = it->second->GetStream(0);

			m_ForceCacheFullSceneState = num != previewSlotsToStreams.size();
			ctxp = PreviewStream(ctxp, previewStream, slot, cols, fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw, smokeOverlayAlphaFactor, smokeOverlayAlphaFactorMultiplyer);
		}
	}
	else if (bPushedRenderTarget) {
		QueuePopRenderTarget();
		bPushedRenderTarget = false;

	}
	else if(m_FirstStreamToBeRendered || m_ForceCacheFullSceneState) {
		if (m_FirstStreamToBeRendered) {
			m_FirstStreamToBeRendered = false;
		}
		else {
			ctxp = CommitDrawingContext(ctxp, true);
		}
		m_ForceCacheFullSceneState = false;
		DoRenderView(fn, This, Edx, view, hudViewSetup, nClearFlags, whatToDraw);
	}

	if (m_Recording)
	{
		QueueCaptureGpuQueuesExecution();

		++m_Frame;
	}

	if (m_PresentBlocked)
	{
		BlockPresent(ctxp, false);
		m_PresentBlocked = false;
	}
}

bool CAfxStreams::OnViewRenderShouldForceNoVis(bool orgValue)
{
	IAfxStreamContext * hook = FindStreamContext(GetCurrentContext());

	if (hook)
		return hook->ViewRenderShouldForceNoVis(orgValue);

	return orgValue;
}

void CAfxStreams::On_DrawTranslucentRenderables(SOURCESDK::CSGO::CRendering3dView * rendering3dView, bool bInSkybox, bool bShadowDepth, bool afterCall)
{
#ifdef AFX_INTEROP
	if (AfxInterop::Enabled())
	{
		bool enabled = false;
		bool beforeDepth = false;
		bool afterDepth = false;

		if (false == bInSkybox)
		{
			if (true == bShadowDepth)
			{
				if (false == afterCall)
				{
					if (g_InteropFeatures.BeforeTranslucentShadow)
					{
						enabled = true;
						beforeDepth = true;
						afterDepth = true;
					}
				}
				else
				{
					if (g_InteropFeatures.AfterTranslucentShadow)
					{
						enabled = true;
						beforeDepth = true;
						afterDepth = true;
					}
				}
			}
			else
			{
				if (false == afterCall)
				{
					if (g_InteropFeatures.BeforeTranslucent)
					{
						enabled = true;
						beforeDepth = true;
						afterDepth = true;
					}
				}
				else
				{
					if (g_InteropFeatures.AfterTranslucentShadow)
					{
						enabled = true;
						beforeDepth = true;
						afterDepth = true;
					}
				}
			}
		}

		if (enabled)
		{
			IAfxMatRenderContext * afxMatRenderContext = GetCurrentContext();
			IAfxMatRenderContextOrg * orgCtx = afxMatRenderContext->GetOrg();

			if (beforeDepth)
			{
				if (g_InteropFeatures.GetDepthRequired())
					QueueOrExecute(orgCtx, new CAfxLeafExecute_Functor(new CAfxInteropDrawDepth_Functor(true, m_CurrentView->zNear, m_CurrentView->zFar, m_CurrentView->zNear, m_CurrentView->zFar)));
			}

			AfxInterop::On_DrawTranslucentRenderables(afxMatRenderContext, rendering3dView, bInSkybox, bShadowDepth, afterCall);
		}
	}
#endif
}

void CAfxStreams::OnDrawingHudBegin(void)
{
	IAfxMatRenderContext * afxMatRenderContext = GetCurrentContext();
	CAfxRenderViewStream* stream = CAfxRenderViewStream::EngineThreadStream_get();

	if (IAfxStreamContext * hook = FindStreamContext(afxMatRenderContext)) hook->DrawingHudBegin();

	if (DrawPhiGrid || DrawRuleOfThirds)
	{
		QueueOrExecute(afxMatRenderContext->GetOrg(), new CAfxLeafExecute_Functor(new AfxDrawGuidesFunctor(DrawPhiGrid, DrawRuleOfThirds)));
	}

#ifdef AFX_INTEROP
	if (AfxInterop::Enabled())
	{
		IAfxMatRenderContextOrg * orgCtx = afxMatRenderContext->GetOrg();

		if (g_InteropFeatures.BeforeHud)
		{
			if (g_InteropFeatures.GetDepthRequired())
				QueueOrExecute(orgCtx, new CAfxLeafExecute_Functor(new CAfxInteropDrawDepth_Functor(true, m_CurrentView->zNear, m_CurrentView->zFar, m_CurrentView->zNear, m_CurrentView->zFar)));

			AfxInterop::OnBeforeHud(afxMatRenderContext);
		}
	}
#endif

	if (stream)
	{
		if (CAfxRenderViewStream::DT_NoDraw == stream->DrawHud_get())
		{
			QueueOrExecute(afxMatRenderContext->GetOrg(), new CAfxLeafExecute_Functor(new CAfxBlockFunctor(true)));
		}
	}
}

void CAfxStreams::OnDrawingHudEnd(void)
{
	IAfxMatRenderContext * afxMatRenderContext = GetCurrentContext();

	if (CAfxRenderViewStream * stream = CAfxRenderViewStream::EngineThreadStream_get())
	{
		if (CAfxRenderViewStream::DT_NoDraw == stream->DrawHud_get())
		{
			QueueOrExecute(afxMatRenderContext->GetOrg(), new CAfxLeafExecute_Functor(new CAfxBlockFunctor(false)));
		}
	}

#ifdef AFX_INTEROP
	if (AfxInterop::Enabled())
	{
		IAfxMatRenderContextOrg * orgCtx = afxMatRenderContext->GetOrg();

		if (g_InteropFeatures.AfterHud)
		{
			AfxInterop::OnAfterHud(afxMatRenderContext);
		}
	}
#endif

	if (IAfxStreamContext * hook = FindStreamContext(afxMatRenderContext)) hook->DrawingHudEnd();
}

void CAfxStreams::OnDrawingSkyBoxViewBegin(void)
{
	IAfxStreamContext * hook = FindStreamContext(GetCurrentContext());

	if (hook)
		hook->DrawingSkyBoxViewBegin();
}

void CAfxStreams::OnDrawingSkyBoxViewEnd(void)
{
	IAfxMatRenderContext * ctx = GetCurrentContext();

	if (IAfxStreamContext * hook = FindStreamContext(ctx))
		hook->DrawingSkyBoxViewEnd();

#ifdef AFX_INTEROP
	if (AfxInterop::Enabled())
	{
		if (g_InteropFeatures.GetDepthRequired())
		{
			int scale = csgo_CSkyBoxView_GetScale();
			QueueOrExecute(ctx->GetOrg(), new CAfxLeafExecute_Functor(new CAfxInteropDrawDepth_Functor(false, m_CurrentView->zNear, m_CurrentView->zFar, 2.0f * scale, (float)SOURCESDK_CSGO_MAX_TRACE_LENGTH * scale)));
		}
	}
#endif

}

void CAfxStreams::Console_RecordName_set(const char * value)
{
	m_RecordName.assign(value);
}

const char * CAfxStreams::Console_RecordName_get()
{
	return m_RecordName.c_str();
}

void CAfxStreams::Console_PresentRecordOnScreen_set(bool value)
{
	m_PresentRecordOnScreen = value;
}

bool CAfxStreams::Console_PresentRecordOnScreen_get()
{
	return m_PresentRecordOnScreen;
}

void CAfxStreams::Console_PreviewSuspend_set(bool value)
{
	m_SuspendPreview = value;
}

bool CAfxStreams::Console_PreviewSuspend_get()
{
	return m_SuspendPreview;
}


void CAfxStreams::Console_StartMovieWav_set(bool value)
{
	m_StartMovieWav = value;
}

bool CAfxStreams::Console_StartMovieWav_get()
{
	return m_StartMovieWav;
}

void CAfxStreams::Console_RecordVoices_set(bool value)
{
	m_RecordVoices = value;
}

bool CAfxStreams::Console_RecordVoices_get()
{
	return m_RecordVoices;
}

void CAfxStreams::Console_MatPostprocessEnable_set(int value)
{
	m_NewMatPostProcessEnable = value;
}

int CAfxStreams::Console_MatPostprocessEnable_get()
{
	return m_NewMatPostProcessEnable;
}

void CAfxStreams::Console_MatDynamicToneMapping_set(int value)
{
	m_NewMatDynamicTonemapping = value;
}

int CAfxStreams::Console_MatDynamicToneMapping_get()
{
	return m_NewMatDynamicTonemapping;
}

void CAfxStreams::Console_MatMotionBlurEnabled_set(int value)
{
	m_NewMatMotionBlurEnabled = value;
}

int CAfxStreams::Console_MatMotionBlurEnabled_get()
{
	return m_NewMatMotionBlurEnabled;
}

void CAfxStreams::Console_MatForceTonemapScale_set(float value)
{
	m_NewMatForceTonemapScale = value;
}

float CAfxStreams::Console_MatForceTonemapScale_get()
{
	return m_NewMatForceTonemapScale;
}

void CAfxStreams::Console_RecordFormat_set(const char * value)
{
	if(!_stricmp(value, "bmp"))
		m_FormatBmpAndNotTga = true;
	else
	if(!_stricmp(value, "tga"))
		m_FormatBmpAndNotTga = false;
	else
		Tier0_Warning("Error: Invalid format %s\n.", value);
}

const char * CAfxStreams::Console_RecordFormat_get()
{
	return m_FormatBmpAndNotTga ? "bmp" : "tga";
}

void CAfxStreams::Console_Record_Start()
{
	Console_Record_End();

	Tier0_Msg("Starting recording ... ");
	
	if(UTF8StringToWideString(m_RecordName.c_str(), m_TakeDir)
		&& (m_TakeDir.append(L"\\take"), SuggestTakePath(m_TakeDir.c_str(), 4, m_TakeDir))
		&& CreatePath(m_TakeDir.c_str(), m_TakeDir)
	)
	{
		m_Recording = true;
		m_Frame = 0;
		m_StartMovieWavUsed = false;

		std::string utf8TakeDir;
		bool utf8TakeDirOk = WideStringToUTF8String(m_TakeDir.c_str(), utf8TakeDir);

		BackUpMatVars();
		SetMatVarsForStreams();

		if (!m_HostFrameRate)
			m_HostFrameRate = new WrpConVarRef("host_framerate");

		m_StartHostFrameRateValue = m_HostFrameRate->GetFloat();

		double frameTime = m_HostFrameRate->GetFloat();
		if (1.0 <= frameTime) frameTime = 1.0 / frameTime;

		for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
		{
			(*it)->RecordStart();
			if((*it)->Record_get()) (*it)->SetActive(true);
		}

		if (m_GameRecording)
		{
			std::wstring fileName(m_TakeDir);
			fileName.append(L"\\afxGameRecord.agr");

			if(CClientTools * instance = CClientTools::Instance()) instance->StartRecording(fileName.c_str());
		}

		if (m_CamBvh)
		{
			std::wstring camFileName(m_TakeDir);
			camFileName.append(L"\\cam_main.bvh");

			g_Hook_VClient_RenderView.ExportBegin(camFileName.c_str(), frameTime);
		}

		for (std::list<CEntityBvhCapture *>::iterator it = m_EntityBvhCaptures.begin(); it != m_EntityBvhCaptures.end(); ++it)
		{
			(*it)->StartCapture(m_TakeDir, frameTime);
		}

		if (m_CamExport)
		{
			std::wstring camFileName(m_TakeDir);
			camFileName.append(L"\\cam_main.cam");

			m_CamExportObj = new CamExport(camFileName.c_str());
		}

		Tier0_Msg("done.\n");

		Tier0_Msg("Recording to \"%s\".\n", utf8TakeDirOk ? utf8TakeDir.c_str() : "?");

		m_StartMovieWavUsed = m_StartMovieWav;

		if (m_StartMovieWavUsed)
		{
			if (!csgo_Audio_StartRecording(m_TakeDir.c_str()))
				Tier0_Warning("Error: Could not start WAV audio recording!\n");
		}

		m_RecordVoicesUsed = m_RecordVoices;

		if (m_RecordVoicesUsed)
		{
			if(!Mirv_Voice_StartRecording(m_TakeDir.c_str()))
				Tier0_Warning("Error: Could not start voice recording!\n");

		}
	}
	else
	{
		Tier0_Msg("FAILED");
		Tier0_Warning("Error: Failed to create directories for \"%s\".\n", m_RecordName.c_str());
	}

	UpdateStreamDeps();
}

void CAfxStreams::Console_Record_End()
{
	if(m_Recording)
	{
		Tier0_Msg("Finishing recording ... ");

		if (m_RecordVoicesUsed)
		{
			Mirv_Voice_EndRecording();
		}

		if (m_StartMovieWavUsed)
		{
			csgo_Audio_EndRecording();
		}

		if (m_CamExportObj)
		{
			delete m_CamExportObj;
			m_CamExportObj = 0;
		}

		if (m_CamBvh)
		{
			g_Hook_VClient_RenderView.ExportEnd();
		}

		for (std::list<CEntityBvhCapture *>::iterator it = m_EntityBvhCaptures.begin(); it != m_EntityBvhCaptures.end(); ++it)
		{
			(*it)->EndCapture();
		}

		if (m_GameRecording)
		{
			if (CClientTools * instance = CClientTools::Instance()) instance->EndRecording();
		}

		for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
		{
			(*it)->RecordEnd();
		}

		RestoreMatVars();

		Tier0_Msg("done.\n");

		//AfxD3D9_Block_Present(false);
	}

	m_Recording = false;
	UpdateStreamDeps();
}

void CAfxStreams::Console_AddStream(const char * streamName)
{
	if(!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxRenderViewStream()));
}

void CAfxStreams::Console_AddBaseFxStream(const char * streamName)
{
	if(!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxBaseFxStream()));
}

void CAfxStreams::Console_AddDepthStream(const char * streamName, bool tryZDepth)
{
	if(!Console_CheckStreamName(streamName))
		return;

	if (tryZDepth && !AfxD3d9_DrawDepthSupported())
	{
		Tier0_Warning("Your graphics card does not support this feature (FOURCC_INTZ) or you are using -afxinterop, falling back to old draw depth method.\n");
		tryZDepth = false;
	}

	AddStream(new CAfxSingleStream(streamName, tryZDepth ? static_cast<CAfxRenderViewStream *>(new CAfxZDepthStream()) : static_cast<CAfxRenderViewStream *>(new CAfxDepthStream())));
}

void CAfxStreams::Console_AddMatteWorldStream(const char * streamName)
{
	if(!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxMatteWorldStream()));
}

void CAfxStreams::Console_AddDepthWorldStream(const char * streamName, bool tryZDepth)
{
	if (!Console_CheckStreamName(streamName))
		return;

	if (tryZDepth && !AfxD3d9_DrawDepthSupported())
	{
		Tier0_Warning("Your graphics card does not support this feature (FOURCC_INTZ) or you are using -afxinterop, falling back to old draw depth method.\n");
		tryZDepth = false;
	}

	AddStream(new CAfxSingleStream(streamName, tryZDepth ? static_cast<CAfxRenderViewStream *>(new CAfxZDepthWorldStream()) : static_cast<CAfxRenderViewStream *>(new CAfxDepthWorldStream())));
}

void CAfxStreams::Console_AddMatteEntityStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxMatteEntityStream()));
}

void CAfxStreams::Console_AddDepthEntityStream(const char * streamName, bool tryZDepth)
{
	if (!Console_CheckStreamName(streamName))
		return;

	if (tryZDepth && !AfxD3d9_DrawDepthSupported())
	{
		Tier0_Warning("Your graphics card does not support this feature (FOURCC_INTZ) or you are using -afxinterop, falling back to old draw depth method.\n");
		tryZDepth = false;
	}

	AddStream(new CAfxSingleStream(streamName, tryZDepth ? static_cast<CAfxRenderViewStream *>(new CAfxZDepthEntityStream()) : static_cast<CAfxRenderViewStream *>(new CAfxDepthEntityStream())));
}

void CAfxStreams::Console_AddAlphaMatteStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	Tier0_Warning("AFXWARNING: alphaMatteStream is deprecated and will be removed!\n");

	AddStream(new CAfxSingleStream(streamName, new CAfxAlphaMatteStream()));
}

void CAfxStreams::Console_AddAlphaEntityStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	Tier0_Warning("AFXWARNING: alphaEntityStream is deprecated and will be removed!\n");

	AddStream(new CAfxSingleStream(streamName, new CAfxAlphaEntityStream()));
}

void CAfxStreams::Console_AddAlphaWorldStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	Tier0_Warning("AFXWARNING: alphaWorldStream is deprecated and will be removed!\n");

	AddStream(new CAfxSingleStream(streamName, new CAfxAlphaWorldStream()));
}

void CAfxStreams::Console_AddAlphaMatteEntityStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	Tier0_Warning("AFXWARNING: alphaMatteEntityStream is deprecated and will be removed!\n");

	AddStream(new CAfxTwinStream(streamName, new CAfxAlphaMatteStream(), new CAfxAlphaEntityStream(), CAfxTwinStream::SCT_ARedAsAlphaBColor));
}

void CAfxStreams::Console_AddMatteStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	auto meStream = new CAfxMatteFxStream();
	AddStream(new CAfxMatteStream(streamName, meStream));
	meStream->SetClearBeforeRender(true);
}

void CAfxStreams::Console_AddHudWhiteStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxHudWhiteStream()));
}

void CAfxStreams::Console_AddHudBlackStream(const char * streamName)
{
	if (!Console_CheckStreamName(streamName))
		return;

	AddStream(new CAfxSingleStream(streamName, new CAfxHudBlackStream()));
}


void CAfxStreams::Console_PrintStreams()
{
	Tier0_Msg("index: name -> recorded?, preview?\n");
	int index = 0;
	for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		int previewSlot = -1;
		
		if (1 == (*it)->GetStreamCount())
		{
			for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
			{
				if (m_PreviewStreams[i] == (*it))
				{
					previewSlot = i;
					break;
				}
			}
		}

		char inPreview[44] = "IN PREVIEW (";

		_itoa(previewSlot, inPreview + 12, 10);

		strcpy(inPreview + strlen(inPreview), ")");

		Tier0_Msg(
			"%i: %s -> %s, %s\n",
			index,
			(*it)->StreamName_get(),
			(*it)->Record_get()
				? "RECORD ON (1)"
				: "record off (0)",
			0 <= previewSlot
				? inPreview
				: "no preview");
		++index;
	}
	Tier0_Msg(
		"=== Total streams: %i ===\n",
		index
	);
}

struct StreamInfoList_s {
	CAfxRecordStream * Stream;
	int Index;
	int PreviewSlot;

	StreamInfoList_s(CAfxRecordStream * stream, int index, int previewSlot)
		: Stream(stream)
		, Index(index)
		, PreviewSlot(previewSlot)
	{

	}
};

void CAfxStreams::Console_PrintStreams2()
{
	Tier0_Msg(
		"============================================================\n"
		"index: name (recording preset)\n"
	);
	std::list<StreamInfoList_s> streams[3];
	int index = 0;
	for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		if (1 == (*it)->GetStreamCount())
		{
			for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
			{
				if (m_PreviewStreams[i] == (*it))
				{
					streams[0].emplace_back(*it,index,i);
				}
			}
		}

		if((*it)->Record_get())
			streams[1].emplace_back(*it,index,-1);
		else
			streams[2].emplace_back(*it,index,-1);
		++index;
	}

	for(int i=0;i<3;i++) {
		if(0 == streams[i].size()) continue;
		Tier0_Msg("\n");
		switch(i) {
		case 0:
			Tier0_Msg("[*] PREVIEWING\n");
			break;
		case 1:
			Tier0_Msg("[R] RECORD ON\n");
			break;
		case 2:
			Tier0_Msg("[-] RECORD OFF\n");
			break;
		}
		for(auto it = streams[i].begin(); it != streams[i].end(); it++) {
			const char * streamName = it->Stream->StreamName_get();
			const char * settingName = "";
			if(CAfxRecordingSettings * setting =  it->Stream->GetSettings()) {
				settingName = setting->GetName();
			}
			if(i == 0 && streams[i].size() > 1 || it->PreviewSlot > 0) {
				Tier0_Msg("%i: %s (%s) @%i\n", it->Index, streamName, settingName, it->PreviewSlot);
			} else {
				Tier0_Msg("%i: %s (%s)\n", it->Index, streamName, settingName);
			}
		}
	}

	Tier0_Msg(
		"\n"
		"==== Total streams: %i ====\n",
		index
	);
}

void CAfxStreams::Console_MoveStream(IWrpCommandArgs * args)
{
	int argC = args->ArgC();

	if (3 <= argC)
	{
		char const * inputName = args->ArgV(1);
		std::string sInputName(inputName);
		std::transform(sInputName.begin(), sInputName.end(), sInputName.begin(), ::tolower);

		int moveBeforeId = atoi(args->ArgV(2));

		CAfxRecordStream * stream;

		if (moveBeforeId < 0 || moveBeforeId >(int)m_Streams.size())
		{
			Tier0_Msg("Error: %i is not in valid range for <targetId>\n");
			return;
		}

		{
			int curId = 0;
			std::list<CAfxRecordStream *>::iterator it = m_Streams.begin();
			while (it != m_Streams.end())
			{
				char const * targetName = (*it)->StreamName_get();
				std::string sTargetName(targetName);
				std::transform(sTargetName.begin(), sTargetName.end(), sTargetName.begin(), ::tolower);

				if (sInputName == sTargetName)
					break;

				++curId;
				++it;
			}

			if (it == m_Streams.end())
			{
				Tier0_Warning("Error: \"%s\" is not a valid stream name!\n", inputName);
				return;
			}

			stream = *it;

			m_Streams.erase(it);
		}

		{
			int curId = 0;
			std::list<CAfxRecordStream *>::iterator it = m_Streams.begin();
			while (curId < moveBeforeId && it != m_Streams.end())
			{
				++curId;
				++it;
			}

			m_Streams.insert(it, stream);
		}

		return;
	}

	char const * arg0 = args->ArgV(0);

	Tier0_Msg(
		"%s <streamName> <moveBeforeId> - Move stream before <moveBeforeId>, get the ID from mirv_streams print (first column).\n"
		, arg0
	);
}

void CAfxStreams::Console_RemoveStream(const char * streamName)
{
	for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		if (!_stricmp(streamName, (*it)->StreamName_get()))
		{
			CAfxRecordStream * cur = *it;

			if (m_Recording) cur->RecordEnd();

			for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
			{
				if (m_PreviewStreams[i] == cur)
				{
					Console_PreviewStream("", i);
				}
			}

			if (m_MainStream == cur)
			{
				m_MainStream = nullptr;
				m_MainStreamMode = MainStreamMode_First;
			}

			m_Streams.erase(it);

			cur->WaitUncritical();
			cur->Release();

			return;
		}
	}
	Tier0_Msg("Error: invalid streamName.\n");
}

void CAfxStreams::Console_MainStream(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("none", arg1))
		{
			m_MainStreamMode = MainStreamMode_None;
			return;
		}
		else if (0 == _stricmp("firstActive", arg1))
		{
			m_MainStreamMode = MainStreamMode_FirstActive;
			return;
		}
		else if (0 == _stricmp("first", arg1))
		{
			m_MainStreamMode = MainStreamMode_First;
			return;
		}
		else if (0 == _stricmp("set", arg1) && 3 <= argC)
		{
			const char * arg2 = args->ArgV(2);

			for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
			{
				if (!_stricmp(arg2, (*it)->StreamName_get()))
				{
					m_MainStreamMode = MainStreamMode_Set;
					m_MainStream = *it;
					return;
				}
			}

			Tier0_Warning("AFXERROR: Stream %s not found.\n", arg2);
			return;
		}
	}

	Tier0_Msg(
		"%s none - Never use a main stream, instead always do a (hidden) render to cache the scene state.\n"
		"%s firstActive - Default: The first stream recorded or previewed is considered the main stream.\n"
		"%s first - The first stream in the list is considered the main stream.\n"
		"%s set <sStreamName> - Set a stream named <sStreamName> to use as main stream.\n"
		"Current value: %s%s\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, m_MainStreamMode == MainStreamMode_None ? "none" : (m_MainStreamMode == MainStreamMode_FirstActive ? "firstActive" : (m_MainStreamMode == MainStreamMode_First ? "first" : (m_MainStreamMode == MainStreamMode_Set ? "set " : "")))
		, m_MainStreamMode == MainStreamMode_Set ? m_MainStream->StreamName_get() : ""
	);
}

void CAfxStreams::UpdateStreamDeps() {
	CalcMainStream();
	if(m_MainStream) m_MainStream->SetActive(true);

	for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		bool inPreview = false;
		for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
		{
			if (m_PreviewStreams[i] == (*it))
			{
				inPreview = true;
				break;
			}
		}

		bool active =
			m_MainStream == *it
			|| inPreview
			|| IsRecording() && (*it)->Record_get();

		(*it)->SetActive(active);
	}			
}

void CAfxStreams::Console_PreviewStream(const char * streamName, int slot)
{
	if (slot >= (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])))
	{
		Tier0_Warning("Error: invalid slot %i.\n", slot);
		return;
	}

	if(StringIsEmpty(streamName))
	{

		if(slot < 0)
		{
			for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
				m_PreviewStreams[i] = nullptr;
		}
		else
		{
			m_PreviewStreams[slot] = nullptr;
		}

		bool allEmpty = true;

		for (int i = 0; i < (int)(sizeof(m_PreviewStreams) / sizeof(m_PreviewStreams[0])); ++i)
		{
			if (m_PreviewStreams[i])
			{
				allEmpty = false;
				break;
			}
		}

		if(allEmpty)
		{
			if(!m_Recording) RestoreMatVars();
		}
		return;
	}

	if (slot < 0)
	{
		Tier0_Warning("Error: invalid slot %i.\n", slot);
		return;
	}

	for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		if(!_stricmp(streamName, (*it)->StreamName_get()))
		{
			if(1 != (*it)->GetStreamCount())
			{
				Tier0_Msg("Error: Only simple (single) streams can be previewed.\n");
				return;
			}

			CAfxRecordStream * cur = *it;
			m_PreviewStreams[slot] = cur;
			if(!m_Recording) BackUpMatVars();
			SetMatVarsForStreams();
			return;
		}
	}

	Tier0_Msg("Error: invalid streamName.\n");
}

void CAfxStreams::Console_ListActions(void)
{
	CAfxBaseFxStream::Console_ListActions();
}

void CAfxStreams::Console_EditStream(const char * streamName, IWrpCommandArgs * args)
{
	bool streamEdited = false;

	std::string sInputName(streamName);
	std::transform(sInputName.begin(), sInputName.end(), sInputName.begin(), ::tolower);

	for (std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		char const * targetName = (*it)->StreamName_get();
		std::string sTargetName(targetName);
		std::transform(sTargetName.begin(), sTargetName.end(), sTargetName.begin(), ::tolower);

		if (StringWildCard1Matched(sInputName.c_str(), sTargetName.c_str()))
		{
			Tier0_Msg("--- Editing stream \"%s\" ----\n", targetName);
			Console_EditStream((*it), args);
			streamEdited = true;
		}
	}

	if(!streamEdited)
		Tier0_Warning("Error: No streamName matches \"%s\".\n", streamName);
}

void CAfxStreams::Console_EditStream(CAfxStream * stream, IWrpCommandArgs * args)
{
	CAfxStream * cur = stream;
	CAfxRecordStream * curRecord = nullptr;

	if(cur)
	{
		cur->WaitUncritical();
		curRecord = cur->AsAfxRecordStream();
	}

	bool headDone = false;

	if (curRecord)
	{
		if (!(headDone = curRecord->Console_Edit_Head(args)))
		{
			curRecord->Console_Edit_Tail(args);
		}
	}

	if(!headDone) Tier0_Msg("== No more properties. ==\n");
}

bool CAfxStreams::Console_EditStream(CAfxRenderViewStream * stream, IWrpCommandArgs * args)
{
	CAfxRenderViewStream * curRenderView = stream;
	CAfxBaseFxStream * curBaseFx = 0;
	
	if(curRenderView)
	{
		curRenderView->WaitUncritical();
		curBaseFx = curRenderView->AsAfxBaseFxStream();
	}

	char const * cmdPrefix = args->ArgV(0);

	int argcOffset = 1;

	int argc = args->ArgC() - argcOffset;

	if(curRenderView)
	{
		if(1 <= argc)
		{
			char const * cmd0 = args->ArgV(argcOffset +0);

			if(!_stricmp(cmd0, "attachCommands"))
			{
				if(2 <= argc)
				{
					std::string value;

					for(int i=argcOffset +1; i < args->ArgC(); ++i)
					{
						if(argcOffset +1 < i)
						{
							value.append(" ");
						}
						value.append(args->ArgV(i));
					}
					
					curRenderView->AttachCommands_set(value.c_str());
					return true;
				}

				Tier0_Msg(
					"%s attachCommands <commandString1> [<commandString2>] ... [<commandStringN>] - Set command strings to be executed when stream is attached.\n"
					"Current value: %s.\n"
					, cmdPrefix
					, curRenderView->AttachCommands_get()
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "detachCommands"))
			{
				if(2 <= argc)
				{
					std::string value;

					for(int i=argcOffset +1; i < args->ArgC(); ++i)
					{
						if(argcOffset +1 < i)
						{
							value.append(" ");
						}
						value.append(args->ArgV(i));
					}
					
					curRenderView->DetachCommands_set(value.c_str());
					return true;
				}

				Tier0_Msg(
					"%s detachCommands <commandString1> [<commandString2>] ... [<commandStringN>] - Set command strings to be executed when stream is detached.\n"
					"Current value: %s.\n"
					, cmdPrefix
					, curRenderView->DetachCommands_get()
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "drawHud"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					int iCmd1 = atoi(cmd1);

					curRenderView->DrawHud_set(iCmd1 < 0 ? CAfxRenderViewStream::DT_NoChange : iCmd1 < 1 ? CAfxRenderViewStream::DT_NoDraw : CAfxRenderViewStream::DT_Draw);

					return true;
				}

				CAfxRenderViewStream::DrawType eVal = curRenderView->DrawHud_get();

				Tier0_Msg(
					"%s drawHud -1|0|1 - Whether to draw HUD for this stream - -1 = no change, 0 = don't draw, 1 = draw.\n"
					"Current value: %i.\n"
					, cmdPrefix
					, (eVal == CAfxRenderViewStream::DT_NoChange ? -1 : eVal == CAfxRenderViewStream::DT_NoDraw ? 0 : 1)
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "drawViewModel"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					int iCmd1 = atoi(cmd1);

					curRenderView->DrawViewModel_set(iCmd1 < 0 ? CAfxRenderViewStream::DT_NoChange : iCmd1 < 1 ? CAfxRenderViewStream::DT_NoDraw : CAfxRenderViewStream::DT_Draw);

					return true;
				}

				CAfxRenderViewStream::DrawType eVal = curRenderView->DrawViewModel_get();

				Tier0_Msg(
					"%s drawViewModel -1|0|1 - Whether to draw view model (in-eye weapon) for this stream - -1 = no change, 0 = don't draw, 1 = draw.\n"
					"Current value: %i.\n"
					, cmdPrefix
					, (eVal == CAfxRenderViewStream::DT_NoChange ? -1 : eVal == CAfxRenderViewStream::DT_NoDraw ? 0 : 1)
				);
				return true;
			}
			else if(!_stricmp(cmd0, "forceBuildingCubeMaps"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					int iCmd1 = atoi(cmd1);

					curRenderView->ForceBuildingCubemaps_set(0 != iCmd1);

					return true;
				}

				Tier0_Msg(
					"%s forceBuildingCubeMaps - 0|1 - Whether to always force building_cubemaps 1 for this stream or let HLAE decide.\n"
					"Current value: %i.\n"
					, cmdPrefix
					, curRenderView->ForceBuildingCubemaps_get() ? 1 : 0
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "captureType"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxRenderViewStream::StreamCaptureType value;

					if(Console_ToStreamCaptureType(cmd1, value))
					{
						if(value == CAfxRenderViewStream::SCT_DepthF || value == CAfxRenderViewStream::SCT_DepthFZIP)
						{
							if(!AfxD3D9_Check_Supports_R32F(false))
							{
								Tier0_Warning("AFXERROR: This capture type is not supported according to your graphics card / driver. Aborting to avoid crashes.\n");
								return true;
							}
						}
						curRenderView->StreamCaptureType_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s captureType " CAFXBASEFXSTREAM_STREAMCAPTURETYPES " - Set new render type.\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromStreamCaptureType(curRenderView->StreamCaptureType_get())
				);
				return true;
			}
			else if (0 == _stricmp("doBloomAndToneMapping", cmd0))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				AfxOverrideable_Console(&subArgs, curRenderView->DoBloomAndToneMapping, NULL);
				return true;
			}
			else if (0 == _stricmp("doDepthOfField", cmd0))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				AfxOverrideable_Console(&subArgs, curRenderView->DoDepthOfField, NULL);
				return true;
			}
			else if (0 == _stricmp("drawWorldNormal", cmd0))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				AfxOverrideable_Console(&subArgs, curRenderView->DrawWorldNormal, NULL);
				return true;
			}
			else if (0 == _stricmp("cullFrontFaces", cmd0))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				AfxOverrideable_Console(&subArgs, curRenderView->CullFrontFaces, NULL);
				return true;
			}
			else if (0 == _stricmp("renderFlashlightDepthTranslucents", cmd0))
			{
				CSubWrpCommandArgs subArgs(args, 2);
				AfxOverrideable_Console(&subArgs, curRenderView->RenderFlashlightDepthTranslucents, NULL);
				return true;
			}
			/*
			else if (0 == _stricmp("disableFastPath", cmd0))
			{
				if (2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset + 1);

					curRenderView->SetDisableFastPath(0 != atoi(cmd1));
					return true;
				}

				Tier0_Msg(
					"%s disableFastPath 0|1 - Whether to disable fast path for this stream (required for some features, e.g. detailed entity handles).\n"
					"Current value: %s.\n"
					, cmdPrefix
					, curRenderView->GetDisableFastPath() ? "1" : "0"
				);
				return true;
			}
			*/
		}
	}

	if(curBaseFx)
	{
		if(1 <= argc)
		{
			char const * cmd0 = args->ArgV(argcOffset +0);

			if (!_stricmp(cmd0, "picker"))
			{
				if (2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset + 1);

					if (!_stricmp(cmd1, "ent") && 3 <= argc)
					{
						//curBaseFx->Console_DisableFastPathRequired();

						bool value = 0 != atoi(args->ArgV(argcOffset + 2));
						curBaseFx->Picker_Pick(true, value);
						return true;
					}
					else
					if (!_stricmp(cmd1, "mat") && 3 <= argc)
					{
						//curBaseFx->Console_DisableFastPathRequired();

						bool value = 0 != atoi(args->ArgV(argcOffset + 2));
						curBaseFx->Picker_Pick(false, value);
						return true;
					}
					else
					if (!_stricmp(cmd1, "print"))
					{
						curBaseFx->Picker_Print();
						return true;
					}
					else
					if (!_stricmp(cmd1, "stop"))
					{
						curBaseFx->Picker_Stop();
						return true;
					}
				}

				Tier0_Msg(
					"%s picker ent 0|1 - Tell picker if entity is visible (1) or not (0). (Or start picking with 1.)\n"
					"%s picker mat 0|1 - Tell picker if material is visible (1) or not (0). (Or start picking with 1.)\n"
					"%s picker print - Prints currently picked result set (entities / materials).\n"
					"%s picker stop - Stop picking.\n"
					"ATTENTION: Stream needs to be in preview of course, otherwise you won't see anything ;)\n"
					"ATTENTION: Do not forget to stop the picker, otherwise you might have some surprises ;)\n"
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
				);

				return true;
			}
			else
			if(!_stricmp(cmd0, "actionFilter"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					if(!_stricmp(cmd1, "add"))
					{
						if(4 <= argc)
						{
							char const * cmd2 = args->ArgV(argcOffset +2);
							char const * cmd3 = args->ArgV(argcOffset +3);

							CAfxBaseFxStream::CAction * value;

							if(Console_ToAfxAction(cmd3, value))
							{
								curBaseFx->Console_ActionFilter_Add(cmd2, value);
								return true;
							}
						}

						Tier0_Msg(
							"%s actionFilter add <materialNameMask> <actionName> - Add a new filter action.\n"
							"\t<materialNameMask> - material name/path to match, where \\* = wildcard and \\\\ = \\\n"
							"\tTo find a material name/path enable debugPrint and use invalidateMap command in stream options!\n"
							"\t<actionName> - name of action (see mirv_streams actions).\n"
							, cmdPrefix
						);
						return true;
					}
					else if (!_stricmp("addEx", cmd1))
					{
						CSubWrpCommandArgs subArgs(args, 3);
						curBaseFx->Console_ActionFilter_AddEx(this, &subArgs);
						return true;
					}
					else
					if(!_stricmp(cmd1, "print"))
					{
						curBaseFx->Console_ActionFilter_Print();
						return true;
					}
					else
					if(!_stricmp(cmd1, "remove"))
					{
						if(3 <= argc)
						{
							char const * cmd2 = args->ArgV(argcOffset +2);

							curBaseFx->Console_ActionFilter_Remove(atoi(cmd2));

							return true;
						}
						
						Tier0_Msg(
							"%s actionFilter remove <actionId> - Removes action with id number <actionId>.\n"
							, cmdPrefix
						);
						return true;
					}
					else
					if(!_stricmp(cmd1, "move"))
					{
						if(4 <= argc)
						{
							char const * cmd2 = args->ArgV(argcOffset +2);
							char const * cmd3 = args->ArgV(argcOffset +3);


							curBaseFx->Console_ActionFilter_Move(atoi(cmd2), atoi(cmd3));

							return true;
						}
						
						Tier0_Msg(
							"%s actionFilter move <actionId> <beforeId> - Moves action with id number <actionId> before action with id <beforeId> (<beforeId> value can be 1 greater than the last id to move at the end).\n"
							, cmdPrefix
						);
						return true;
					}
					else if (!_stricmp(cmd1, "clear"))
					{
						curBaseFx->Console_ActionFilter_Clear();

						return true;
					}
				}

				Tier0_Msg(
					"%s actionFilter add [...] - Add a new filter action.\n"
					"%s actionFilter addEx [...] - Add a new filter action.\n"
					"%s actionFilter print - Print current filter actions.\n"
					"%s actionFilter remove [...] - Remove a filter action.\n"
					"%s actionFilter move [...] - Move filter action (change priority).\n"
					"%s actionFilter clear - Remove all actions from actionfilter.\n"
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "clientEffectTexturesAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->ClientEffectTexturesAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s clientEffectTexturesAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->ClientEffectTexturesAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "worldTexturesAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->WorldTexturesAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s worldTexturesAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->WorldTexturesAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "skyBoxTexturesAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->SkyBoxTexturesAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s skyBoxTexturesAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->SkyBoxTexturesAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "staticPropTexturesAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->StaticPropTexturesAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s staticPropTexturesAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->StaticPropTexturesAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "cableAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->CableAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s cableAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->CableAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "playerModelsAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->PlayerModelsAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s playerModelsAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->PlayerModelsAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "weaponModelsAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->WeaponModelsAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s weaponModelsAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->WeaponModelsAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "statTrakAction")||!_stricmp(cmd0, "stattrackAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->StatTrakAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s statTrakAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->StatTrakAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "shellModelsAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->ShellModelsAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s shellModelsAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->ShellModelsAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "otherModelsAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->OtherModelsAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s otherModelsAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->OtherModelsAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "decalTexturesAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->DecalTexturesAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s decalTexturesAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->DecalTexturesAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "effectsAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->EffectsAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s effectsAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->EffectsAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "shellParticleAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->ShellParticleAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s shellParticleAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->ShellParticleAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "otherParticleAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->OtherParticleAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s otherParticleAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->OtherParticleAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "stickerAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->StickerAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s stickerAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->StickerAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "errorMaterialAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->ErrorMaterialAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s errorMaterialAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->ErrorMaterialAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "otherAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->OtherAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s otherAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->OtherAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "writeZAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					CAfxBaseFxStream::CAction * value;

					if(Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->WriteZAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s writeZAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->WriteZAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "devAction"))
			{
				Tier0_Msg(
					"%s devAction\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->DevAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "otherEngineAction"))
			{
				Tier0_Msg(
					"%s otherEngineAction\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->OtherEngineAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "otherSpecialAction"))
			{
				Tier0_Msg(
					"%s otherSpecialAction\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->OtherSpecialAction_get())
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "vguiAction"))
			{
				if (2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset + 1);
					CAfxBaseFxStream::CAction * value;

					if (Console_ToAfxAction(cmd1, value))
					{
						curBaseFx->VguiAction_set(value);
						return true;
					}
				}

				Tier0_Msg(
					"%s vguiAction" CAFXSTREAMS_ACTIONSUFFIX "\n"
					"Current value: %s.\n"
					, cmdPrefix
					, Console_FromAfxAction(curBaseFx->VguiAction_get())
				);
				return true;
			}
			else if (0 == _stricmp(cmd0, "clear"))
			{
				if (2 <= argc)
				{
					const char * cmd1 = args->ArgV(argcOffset + 1);

					curBaseFx->SetClearBeforeRender(0 != atoi(cmd1));
					return true;
				}

				Tier0_Msg(
					"%s clear 0|1 - Whether to clear (black) before rendering or not.\n"
					"Current value: %i.\n"
					, cmdPrefix
					, 0 != curBaseFx->GetClearBeforeRender() ? 1 : 0
				);
				return true;
			}
			else if (0 == _stricmp(cmd0, "clearBeforeHud"))
			{
				if (2 <= argc)
				{
					const char * cmd1 = args->ArgV(argcOffset + 1);

					if (0 == _stricmp(cmd1, "none"))
					{
						curBaseFx->ClearBeforeHud_set(CAfxBaseFxStream::EClearBeforeHud_No);
						return true;
					}
					else if (0 == _stricmp(cmd1, "black"))
					{
						curBaseFx->ClearBeforeHud_set(CAfxBaseFxStream::EClearBeforeHud_Black);
						return true;
					}
					else if (0 == _stricmp(cmd1, "white"))
					{
						curBaseFx->ClearBeforeHud_set(CAfxBaseFxStream::EClearBeforeHud_White);
						return true;
					}
				}

				CAfxBaseFxStream::EClearBeforeHud value = curBaseFx->ClearBeforeHud_get();
				const char * pszValue = "none";
				switch (value)
				{
				case CAfxBaseFxStream::EClearBeforeHud_Black:
					pszValue = "black";
					break;
				case CAfxBaseFxStream::EClearBeforeHud_White:
					pszValue = "white";
					break;
				}

				Tier0_Msg(
					"%s clearBeforeHud none|black|white - Clear with color before HUD is drawn\n"
					"Current value: %s\n"
					, cmdPrefix
					, pszValue
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "depthVal"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					curBaseFx->DepthVal_set((float)atof(cmd1));
					return true;
				}

				Tier0_Msg(
					"%s depthVal <fValue> - Set new miniumum depth floating point value <fValue>.\n"
					"Current value: %f.\n"
					, cmdPrefix
					, curBaseFx->DepthVal_get()
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "depthValMax"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					curBaseFx->DepthValMax_set((float)atof(cmd1));
					return true;
				}

				Tier0_Msg(
					"%s depthValMax <fValue> - Set new maximum depth floating point value <fValue>.\n"
					"Current value: %f.\n"
					, cmdPrefix
					, curBaseFx->DepthValMax_get()
				);
				return true;
			}
			else if (0 == _stricmp(cmd0, "drawZ"))
			{
				if (!AfxD3d9_DrawDepthSupported())
				{
					Tier0_Warning("Your graphics card does not support this feature (FOURCC_INTZ) or you are using -afxinterop.\n");
				}

				if (2 <= argc)
				{
					const char * cmd1 = args->ArgV(argcOffset + 1);

					if (0 == _stricmp(cmd1, "none"))
					{
						curBaseFx->DrawDepth_set(CAfxBaseFxStream::EDrawDepth_None);
						return true;
					}
					else if (0 == _stricmp(cmd1, "gray"))
					{
						curBaseFx->DrawDepth_set(CAfxBaseFxStream::EDrawDepth_Gray);
						return true;
					}
					else if (0 == _stricmp(cmd1, "rgb"))
					{
						curBaseFx->DrawDepth_set(CAfxBaseFxStream::EDrawDepth_Rgb);
						return true;
					}
					else if (0 == _stricmp(cmd1, "dithered"))
					{
						curBaseFx->DrawDepth_set(CAfxBaseFxStream::EDrawDepth_Dithered);
						return true;
					}
				}

				CAfxBaseFxStream::EDrawDepth value = curBaseFx->DrawDepth_get();
				const char * pszValue = "[unkown]";
				switch (value)
				{
				case CAfxBaseFxStream::EDrawDepth_None:
					pszValue = "none";
					break;
				case CAfxBaseFxStream::EDrawDepth_Gray:
					pszValue = "gray";
					break;
				case CAfxBaseFxStream::EDrawDepth_Rgb:
					pszValue = "rgb";
					break;
				case CAfxBaseFxStream::EDrawDepth_Dithered:
					pszValue = "dithered";
					break;
				}

				Tier0_Msg(
					"%s drawZ none|gray|rgb|dithered - Use special shader to draw the z-(depth) buffer (does not support HUD atm).\n"
					"Current value: %s\n"
					, cmdPrefix
					, pszValue
				);
				return true;
			}
			else if (0 == _stricmp(cmd0, "drawZMode"))
			{
				if (!AfxD3d9_DrawDepthSupported())
				{
					Tier0_Warning("Your graphics card does not support this feature (FOURCC_INTZ) or you are using -afxinterop.\n");
				}

				if (2 <= argc)
				{
					const char * cmd1 = args->ArgV(argcOffset + 1);

					if (0 == _stricmp(cmd1, "inverse"))
					{
						curBaseFx->DrawDepthMode_set(CAfxBaseFxStream::EDrawDepthMode_Inverse);
						return true;
					}
					else if (0 == _stricmp(cmd1, "linear"))
					{
						curBaseFx->DrawDepthMode_set(CAfxBaseFxStream::EDrawDepthMode_Linear);
						return true;
					}
					else if (0 == _stricmp(cmd1, "logE") || 0 == _stricmp(cmd1, "log"))
					{
						curBaseFx->DrawDepthMode_set(CAfxBaseFxStream::EDrawDepthMode_LogE);
						return true;
					}
					else if (0 == _stricmp(cmd1, "pyramidalLinear"))
					{
						curBaseFx->DrawDepthMode_set(CAfxBaseFxStream::EDrawDepthMode_PyramidalLinear);
						return true;
					}
					else if (0 == _stricmp(cmd1, "pyramidalLogE") || 0 == _stricmp(cmd1, "pyramidalLog"))
					{
						curBaseFx->DrawDepthMode_set(CAfxBaseFxStream::EDrawDepthMode_PyramidalLogE);
						return true;
					}
				}

				CAfxBaseFxStream::EDrawDepthMode value = curBaseFx->DrawDepthMode_get();
				const char * pszValue = "[unknown]";
				switch (value)
				{
				case CAfxBaseFxStream::EDrawDepthMode_Inverse:
					pszValue = "inverse";
					break;
				case CAfxBaseFxStream::EDrawDepthMode_Linear:
					pszValue = "linear";
					break;
				case CAfxBaseFxStream::EDrawDepthMode_LogE:
					pszValue = "log";
					break;
				case CAfxBaseFxStream::EDrawDepthMode_PyramidalLinear:
					pszValue = "pyramidalLinear";
					break;
				case CAfxBaseFxStream::EDrawDepthMode_PyramidalLogE:
					pszValue = "pyramidalLog";
					break;
				}

				Tier0_Msg(
					"%s drawZMode inverse|linear|log|pyramidalLinear|pyramidalLog - Mode to use for drawZ.\n"
					"Current value: %s\n"
					, cmdPrefix
					, pszValue
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "smokeOverlayAlphaFactor"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);
					curBaseFx->SmokeOverlayAlphaFactor_set((float)atof(cmd1));
					return true;
				}

				Tier0_Msg(
					"%s smokeOverlayAlphaFactor <fValue> - Set new factor that is multiplied with smoke overlay alpha (i.e. 0 would disable it completely).\n"
					"Current value: %f.\n"
					, cmdPrefix
					, curBaseFx->SmokeOverlayAlphaFactor_get()
				);
				return true;
			}
			else
				if (!_stricmp(cmd0, "shouldForceNoVisOverride"))
				{
					if (!Hook_csgo_CViewRender_ShouldForceNoVis())
					{
						Tier0_Warning("%s smokeOverlayAlphaFactor - ERROR: Required hook not available, will not work.\n", cmdPrefix);
					}

					if (2 <= argc)
					{
						char const * cmd1 = args->ArgV(argcOffset + 1);
						curBaseFx->ShouldForceNoVisOverride_set(0 != atoi(cmd1));
						return true;
					}

					Tier0_Msg(
						"%s shouldForceNoVisOverride 0|1 - Useful for wallhack: If to force ShouldForceNoVisOverride to return true.\n"
						"Current value: %i.\n"
						, cmdPrefix
						, curBaseFx->ShouldForceNoVisOverride_get() ? 1 : 0
					);
					return true;
				}
				else
			if(!_stricmp(cmd0, "debugPrint"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					curBaseFx->DebugPrint_set(0 != atoi(cmd1) ? true : false);
					return true;
				}

				Tier0_Msg(
					"%s debugPrint 0|1 - Disable / enable debug console output.\n"
					"Current value: %s.\n"
					, cmdPrefix
					, curBaseFx->DebugPrint_get() ? "1" : "0"
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "invalidateMap"))
			{
				curBaseFx->InvalidateMap();
				return true;
			}
			else
			if(!_stricmp(cmd0, "testAction"))
			{
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					curBaseFx->TestAction_set(0 != atoi(cmd1) ? true : false);
					return true;
				}

				Tier0_Msg(
					"%s testAction 0|1 - Disable / enable action for developer testing purposes.\n"
					"Current value: %s.\n"
					, cmdPrefix
					, curBaseFx->TestAction_get() ? "1" : "0"
				);
				return true;
			}
			else
			if(!_stricmp(cmd0, "man"))
			{
				/*
				if(2 <= argc)
				{
					char const * cmd1 = args->ArgV(argcOffset +1);

					if(!_stricmp(cmd1, "toDepth"))
					{
						curBaseFx->ConvertStreamDepth(false, false);
						return;
					}
					else
					if(!_stricmp(cmd1, "toDepth24"))
					{
						curBaseFx->ConvertStreamDepth(true, false);
						return;
					}
					else
					if(!_stricmp(cmd1, "toDepth24ZIP"))
					{
						curBaseFx->ConvertStreamDepth(true, true);
						return;
					}
				}

				Tier0_Msg(
					"%s man toDepth - Convert drawDepth24 actions to drawDepth and set captureType normal.\n"
					"%s man toDepth24 - Convert drawDepth actions to drawDepth24 and set captureType depth24.\n"
					"%s man toDepth24ZIP - Convert drawDepth actions to drawDepth24 and set captureType depth24ZIP (might be slower than depth24).\n"
					, cmdPrefix
					, cmdPrefix
					, cmdPrefix
				);*/
				Tier0_Warning("Warning: Due to CS:GO 17th Ferbuary 2016 update this feature is not available.\n");
				return true;
			}
			else if (0 == _stricmp("reshade", cmd0))
			{
				if (!g_ReShadeAdvancedfx.IsConnected()) {
					Tier0_Warning("AFXERROR: ReShade or ReShade_advancedfx.addon not loaded.\n");
					return true;
				}

				if (2 <= argc)
				{
					char const* cmd1 = args->ArgV(argcOffset + 1);

					if (0 == _stricmp("enabled", cmd1)) {
						if (3 <= argc) {
							bool bEnabled = 0 != atoi(args->ArgV(argcOffset + 2));

							if (bEnabled && !AfxD3d9_DrawDepthSupported()) {
								Tier0_Warning("Your graphics card does not support FOURCC_INTZ or you are using -afxinterop, thus ReShade addon will not work correctly.\n");
							}
							if (bEnabled && !AfxD3D9_Check_Supports_R32F(true)) {
								Tier0_Warning("Your graphics card does not support D3DFMT_R32F render targets, thus ReShade addon will not work correctly.\n");
							}

							curBaseFx->ReShadeEnabled_set(bEnabled);
							return true;
						}

						Tier0_Msg(
							"%s reshade enabled 0|1 - Enable / disable reshade addon on this stream (usually you want to do this on the main stream).\n"
							"Current value: %s\n"
							, cmdPrefix
							, curBaseFx->ReShadeEnabled_get() ? "1" : "0"
						);
						return true;
					}
				}

				Tier0_Msg(
					"%s reshade enabled [...].\n"
					, cmdPrefix
				);

				return true;
			}
		}
	}

	if(curBaseFx)
	{
		Tier0_Msg("-- baseFx properties --\n");
		Tier0_Msg("%s picker [...] - Helps picking a visible material / entity.\n", cmdPrefix);
		Tier0_Msg("%s actionFilter [...] - Set actions by material name (not safe maybe).\n", cmdPrefix);
		Tier0_Msg("%s clientEffectTexturesAction [...]\n", cmdPrefix);
		Tier0_Msg("%s worldTexturesAction [...]\n", cmdPrefix);
		Tier0_Msg("%s skyBoxTexturesAction [...]\n", cmdPrefix);
		Tier0_Msg("%s staticPropTexturesAction [...]\n", cmdPrefix);
		Tier0_Msg("%s cableAction [...]\n", cmdPrefix);
		Tier0_Msg("%s playerModelsAction [...]\n", cmdPrefix);
		Tier0_Msg("%s weaponModelsAction [...]\n", cmdPrefix);
		Tier0_Msg("%s statTrakAction [...]\n", cmdPrefix);
		Tier0_Msg("%s shellModelsAction [...]\n", cmdPrefix);
		Tier0_Msg("%s otherModelsAction [...]\n", cmdPrefix);
		Tier0_Msg("%s decalTexturesAction [...]\n", cmdPrefix);
		Tier0_Msg("%s effectsAction [...]\n", cmdPrefix);
		Tier0_Msg("%s shellParticleAction [...]\n", cmdPrefix);
		Tier0_Msg("%s otherParticleAction [...]\n", cmdPrefix);
		Tier0_Msg("%s stickerAction [...]\n", cmdPrefix);
		Tier0_Msg("%s errorMaterialAction [...]\n", cmdPrefix);
		Tier0_Msg("%s otherAction [...]\n", cmdPrefix);
		Tier0_Msg("%s writeZAction [...]\n", cmdPrefix);
		Tier0_Msg("%s devAction [...] - Readonly.\n", cmdPrefix);
		Tier0_Msg("%s otherEngineAction [...] - Readonly.\n", cmdPrefix);
		Tier0_Msg("%s otherSpecialAction [...] - Readonly.\n", cmdPrefix);
		Tier0_Msg("%s clear [...]\n", cmdPrefix);
		Tier0_Msg("%s clearBeforeHud [...]\n", cmdPrefix);
		Tier0_Msg("%s vguiAction [...]\n", cmdPrefix);
		Tier0_Msg("%s depthVal [...]\n", cmdPrefix);
		Tier0_Msg("%s depthValMax [...]\n", cmdPrefix);
		Tier0_Msg("%s drawZ [...]\n", cmdPrefix);
		Tier0_Msg("%s drawZMode [...]\n", cmdPrefix);
		Tier0_Msg("%s smokeOverlayAlphaFactor [...]\n", cmdPrefix);
		Tier0_Msg("%s shouldForceNoVisOverride [...]\n", cmdPrefix);
		Tier0_Msg("%s debugPrint [...]\n", cmdPrefix);
		Tier0_Msg("%s invalidateMap - invaldiates the material map.\n", cmdPrefix);
		//Tier0_Msg("%s man [...] - Manipulate stream more easily (i.e. depth to depth24).\n", cmdPrefix);
		// testAction options is not displayed, because we don't want users to use it.
		// Tier0_Msg("%s testAction [...]\n", cmdPrefix);
		Tier0_Msg("%s reshade [...] - Control the ReShade_advancedfx.addon settings.\n", cmdPrefix);
	}

	if (curRenderView)
	{
		Tier0_Msg("-- renderView properties --\n");
		Tier0_Msg("%s attachCommands [...] - Commands to be executed when stream is attached. WARNING. Use at your own risk, game may crash!\n", cmdPrefix);
		Tier0_Msg("%s detachCommands [...] - Commands to be executed when stream is detached. WARNING. Use at your own risk, game may crash!\n", cmdPrefix);
		Tier0_Msg("%s drawHud [...] - Controls whether or not HUD is drawn for this stream.\n", cmdPrefix);
		Tier0_Msg("%s drawViewModel [...] - Controls whether or not view model (in-eye weapon) is drawn for this stream.\n", cmdPrefix);
		Tier0_Msg("%s forceBuildingCubeMaps [...] - Control if to enable force building_cubemaps to 1. This should be set on all streams that are composited with other streams or should not have any post-processing. For technical reasons only the first stream rendered (recorded or previewed) will obey this option, others always force this.\n", cmdPrefix);
		Tier0_Msg("%s captureType [...] - Stream capture type.\n", cmdPrefix);
		Tier0_Msg("%s doBloomAndToneMapping [...]\n", cmdPrefix);
		Tier0_Msg("%s doDepthOfField [...]\n", cmdPrefix);
		//Tier0_Msg("%s drawWorldNormal [...]\n", cmdPrefix);
		//Tier0_Msg("%s cullFrontFaces [...]\n", cmdPrefix);
		//Tier0_Msg("%s renderFlashlightDepthTranslucents [...]\n", cmdPrefix);
		//Tier0_Msg("%s disableFastPath [...]\n", cmdPrefix);
	}

	return false;
}


void CAfxStreams::Console_Bvh(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * prefix = args->ArgV(0);

	if (m_Recording)
	{
		Tier0_Warning("Error: These settings cannot be accessed during mirv_streams recording!\n");
		return;
	}

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp(cmd1, "cam"))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				m_CamBvh = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"%s cam 0|1 - Enable (1) / Disable (0) main camera export (overrides/uses mirv_camexport actually).\n"
				"Current value: %i.\n"
				, prefix
				, m_CamBvh ? 1 : 0
			);
			return;
		}
		else
		if (!_stricmp(cmd1, "ent"))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (!_stricmp(cmd2, "add"))
				{
					if (6 <= argc)
					{
						char const * sIndex = args->ArgV(3);
						char const * sOrigin = args->ArgV(4);
						char const * sAngles = args->ArgV(5);

						int entityIndex = atoi(sIndex);

						CEntityBvhCapture::Origin_e origin = CEntityBvhCapture::O_View;
						if (!_stricmp(sOrigin, "net"))
						{
							origin = CEntityBvhCapture::O_Net;
						}
						else
						if (!_stricmp(sOrigin, "view"))
						{
							origin = CEntityBvhCapture::O_View;
						}
						else
						{
							Tier0_Warning("Error: invalid <originType>!\n");
							return;
						}

						CEntityBvhCapture::Angles_e angles = CEntityBvhCapture::A_View;
						if (!_stricmp(sAngles, "net"))
						{
							angles = CEntityBvhCapture::A_Net;
						}
						else
						if (!_stricmp(sAngles, "view"))
						{
							angles = CEntityBvhCapture::A_View;
						}
						else
						{
							Tier0_Warning("Error: invalid <anglesType>!\n");
							return;
						}

						CEntityBvhCapture * bvhEntityCapture = new CEntityBvhCapture(entityIndex, origin, angles);

						std::list<CEntityBvhCapture *>::iterator cur = m_EntityBvhCaptures.begin();

						while (cur != m_EntityBvhCaptures.end() && (*cur)->EntityIndex_get() < entityIndex)
							++cur;

						if (cur != m_EntityBvhCaptures.end() && (*cur)->EntityIndex_get() == entityIndex)
						{
							delete (*cur);
							m_EntityBvhCaptures.emplace(cur, bvhEntityCapture);
						}
						else
						{
							m_EntityBvhCaptures.insert(cur, bvhEntityCapture);
						}

						return;
					}

					Tier0_Msg(
						"%s ent add <entityIndex> <originType> <anglesType> - Add an entity to list, <originType> and <anglesType> can be \"net\" or \"view\".\n"
						, prefix
					);
					return;
				}
				else
				if (!_stricmp(cmd2, "del"))
				{
					if (4 <= argc)
					{
						int index = atoi(args->ArgV(3));

						for (std::list<CEntityBvhCapture *>::iterator it = m_EntityBvhCaptures.begin(); it != m_EntityBvhCaptures.end(); ++it)
						{
							if ((*it)->EntityIndex_get() == index)
							{
								delete (*it);

								m_EntityBvhCaptures.erase(it);
								return;
							}
						}

						Tier0_Warning("Error: Index %i not found!\n", index);
						return;
					}

					Tier0_Msg(
						"%s ent del <entityIndex> - Remove given <entityIndex> from list.\n"
						, prefix
					);
					return;
				}
				else
				if (!_stricmp(cmd2, "list"))
				{
					Tier0_Msg("<entityIndex>: <originType> <anglesType>\n");

					for (std::list<CEntityBvhCapture *>::iterator it = m_EntityBvhCaptures.begin(); it != m_EntityBvhCaptures.end(); ++it)
					{
						CEntityBvhCapture * cap = (*it);

						CEntityBvhCapture::Origin_e origin = cap->Origin_get();
						CEntityBvhCapture::Angles_e angles = cap->Angles_get();

						Tier0_Msg(
							"%i: %s %s"
							, cap->EntityIndex_get()
							, origin == CEntityBvhCapture::O_Net ? "net" : (origin == CEntityBvhCapture::O_View ? "view" : "[UNKNOWN]")
							, angles == CEntityBvhCapture::A_Net ? "net" : (angles == CEntityBvhCapture::A_View ? "view" : "[UNKNOWN]")
						);
					}

					Tier0_Msg("---- End Of List ----\n");
					return;
				}
				else
				if (!_stricmp(cmd2, "clear"))
				{
					while (!m_EntityBvhCaptures.empty())
					{
						delete m_EntityBvhCaptures.front();
						m_EntityBvhCaptures.pop_front();
					}

					Tier0_Msg("List cleared.\n");
					return;
				}
			}

			Tier0_Msg(
				"%s ent add [...] - Add an entity to the export list.\n"
				"%s ent del [...] - Remove an entity from the export list.\n"
				"%s ent list - List current entities being exported.\n"
				"%s ent clear - Remove all from list.\n"
				, prefix
				, prefix
				, prefix
				, prefix
			);
			return;
		}
	}

	Tier0_Msg(
		"%s cam [...] - Whether main camera export (overrides/uses mirv_camexport actually).\n"
		"%s ent [...] - Entity BVH export list control.\n"
		, prefix
		, prefix
	);
}

void CAfxStreams::Console_GameRecording(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * prefix = args->ArgV(0);

	if (m_Recording)
	{
		Tier0_Warning("Error: These settings cannot be accessed during mirv_streams recording!\n");
		return;
	}

	CClientTools * clientTools = CClientTools::Instance();

	if (!clientTools)
	{
		Tier0_Warning("Error: Feature not available!\n");
		return;
	}

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp(cmd1, "enabled"))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				m_GameRecording = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"%s enabled 0|1 - Enable (1) / Disable (0) afxGameRecording (game state recording).\n"
				"Current value: %i.\n"
				, prefix
				, m_GameRecording ? 1 : 0
			);
			return;
		}
	}

	if (ClientTools_Console_Cfg(args))
		return;

	Tier0_Msg(
		"%s enabled [...]\n"
		, prefix
	);
}

SOURCESDK::IMaterialSystem_csgo * CAfxStreams::GetMaterialSystem(void)
{
	return m_MaterialSystem;
}

SOURCESDK::IShaderShadow_csgo * CAfxStreams::GetShaderShadow(void)
{
	return m_ShaderShadow;
}

const std::wstring & CAfxStreams::GetTakeDir(void) const
{
	return m_TakeDir;
}

void CAfxStreams :: LevelInitPostEntity(void)
{

}

void CAfxStreams::LevelShutdown()
{
	for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
	{
		(*it)->LevelShutdown();
	}

	m_FirstRenderAfterLevelInit = true;
}

extern bool g_bD3D9DebugPrint;

#ifdef AFX_MIRV_PGL

MirvPgl::CamData GetMirvPglCamData(SOURCESDK::vrect_t_csgo *rect) {
	return MirvPgl::CamData(
		g_MirvTime.GetTime(),
		(float)g_Hook_VClient_RenderView.LastCameraOrigin[0],
		(float)g_Hook_VClient_RenderView.LastCameraOrigin[1],
		(float)g_Hook_VClient_RenderView.LastCameraOrigin[2],
		(float)g_Hook_VClient_RenderView.LastCameraAngles[2],
		(float)g_Hook_VClient_RenderView.LastCameraAngles[0],
		(float)g_Hook_VClient_RenderView.LastCameraAngles[1],
		(float)(AlienSwarm_FovScaling(rect->width, rect->height, g_Hook_VClient_RenderView.LastCameraFov))
	);
}

#endif

void CAfxStreams::View_Render(IAfxBaseClientDll * cl, SOURCESDK::vrect_t_csgo *rect)
{
	Set_View_Render_ThreadId(GetCurrentThreadId());

	//GetCsgoCGlowOverlayFix()->OnMainViewRenderBegin();

#ifdef AFX_MIRV_PGL
	if (MirvPgl::IsDataActive())
	{
		MirvPgl::SupplyCamData(GetMirvPglCamData(rect));
	}
#endif

	cl->GetParent()->View_Render(rect);

	//IAfxMatRenderContextOrg * ctxp = GetCurrentContext()->GetOrg(); // We are on potentially a new context now!

#ifdef AFX_MIRV_PGL
	if (MirvPgl::IsDrawingActive())
	{
		MirvPgl::QueueDrawing(GetMirvPglCamData(rect), rect->width, rect->height);
	}
#endif

	// Capture
	if (m_CamExportObj)
	{
		CamIO::CamData camData;

		camData.Time = g_MirvTime.GetTime();
		camData.XPosition = g_Hook_VClient_RenderView.LastCameraOrigin[0];
		camData.YPosition = g_Hook_VClient_RenderView.LastCameraOrigin[1];
		camData.ZPosition = g_Hook_VClient_RenderView.LastCameraOrigin[2];
		camData.YRotation = g_Hook_VClient_RenderView.LastCameraAngles[0];
		camData.ZRotation = g_Hook_VClient_RenderView.LastCameraAngles[1];
		camData.XRotation = g_Hook_VClient_RenderView.LastCameraAngles[2];
		camData.Fov = g_Hook_VClient_RenderView.LastCameraFov;

		m_CamExportObj->WriteFrame(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, camData);
	}

	// Capture BVHs (except main):
	for (std::list<CEntityBvhCapture *>::iterator it = m_EntityBvhCaptures.begin(); it != m_EntityBvhCaptures.end(); ++it)
	{
		(*it)->CaptureFrame();
	}
}

IAfxMatRenderContextOrg * CAfxStreams::CaptureStreamToBuffer(IAfxMatRenderContextOrg * ctxp, size_t streamIndex, CAfxRenderViewStream * stream, CAfxRecordStream * captureTarget, bool first, bool last, CCSViewRender_RenderView_t fn, void* This, void* Edx, const SOURCESDK::CViewSetup_csgo &view, const SOURCESDK::CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw, float * smokeOverlayAlphaFactor, float & smokeOverlayAlphaFactorMultiplyer)
{
	if (m_FirstStreamToBeRendered) {
		m_FirstStreamToBeRendered = false;
	}
	else {
		ctxp = CommitDrawingContext(ctxp, !m_PresentLastStream);
		m_PresentLastStream = m_PresentRecordOnScreen;
	}

	AfxViewportData_t afxViewport = {
		view.m_nUnscaledX,
		view.m_nUnscaledY,
		view.m_nUnscaledWidth,
		view.m_nUnscaledHeight,
		view.zNear,
		view.zFar
	};

	if (first)
	{
		captureTarget->DoCaptureStart(ctxp, afxViewport);
	}

	CAfxRenderViewStream::StreamCaptureType captureType = stream->StreamCaptureType_get();
	bool isDepthF = captureType == CAfxRenderViewStream::SCT_DepthF || captureType == CAfxRenderViewStream::SCT_DepthFZIP;

	SetMatVarsForStreams(); // keep them set in case someone resets them.

	if (0 < strlen(stream->AttachCommands_get()))
		WrpConCommands::ImmediatelyExecuteCommands(stream->AttachCommands_get()); // Execute commands before we lock the stream!

	SOURCESDK::VMatrix worldToView;
	SOURCESDK::VMatrix viewToProjection;
	SOURCESDK::VMatrix worldToProjection;
	SOURCESDK::VMatrix worldToPixels;

	SOURCESDK::VMatrix viewToProjectionSky;

	{
		SOURCESDK::CViewSetup_csgo skyView = view;

		int scale = csgo_CSkyBoxView_GetScale();

		skyView.zNear = 2.0f * scale;
		skyView.zFar = (float)SOURCESDK_CSGO_MAX_TRACE_LENGTH * scale;

		g_pVRenderView_csgo->GetMatricesForView(skyView, &worldToView, &viewToProjectionSky, &worldToProjection, &worldToPixels);
	}

	g_pVRenderView_csgo->GetMatricesForView(view, &worldToView, &viewToProjection, &worldToProjection, &worldToPixels);

	int myWhatToDraw = whatToDraw;

	switch (stream->DrawHud_get())
	{
	case CAfxRenderViewStream::DT_Draw:
		myWhatToDraw |= SOURCESDK::RENDERVIEW_DRAWHUD;
		break;
	case CAfxRenderViewStream::DT_NoDraw:
		//myWhatToDraw &= ~SOURCESDK::RENDERVIEW_DRAWHUD;
		break;
	case CAfxRenderViewStream::DT_NoChange:
	default:
		break;
	}

	switch (stream->DrawViewModel_get())
	{
	case CAfxRenderViewStream::DT_Draw:
		myWhatToDraw |= SOURCESDK::RENDERVIEW_DRAWVIEWMODEL;
		break;
	case CAfxRenderViewStream::DT_NoDraw:
		myWhatToDraw &= ~SOURCESDK::RENDERVIEW_DRAWVIEWMODEL;
		break;
	case CAfxRenderViewStream::DT_NoChange:
	default:
		break;
	}

	float oldSmokeOverlayAlphaFactor = *smokeOverlayAlphaFactor;
	smokeOverlayAlphaFactorMultiplyer = stream->SmokeOverlayAlphaFactor_get();
	if (smokeOverlayAlphaFactorMultiplyer < 1) *smokeOverlayAlphaFactor = 0;

	{
		bool forceBuildingCubeMaps = true;

		float oldFrameTime;
		int oldBuildingCubeMaps;

		if (true)
		{
			forceBuildingCubeMaps = stream->ForceBuildingCubemaps_get();
		}
		else
		{
			oldFrameTime = g_Hook_VClient_RenderView.GetGlobals()->frametime_get();
			g_Hook_VClient_RenderView.GetGlobals()->frametime_set(0);
		}

		if(forceBuildingCubeMaps)
		{
			oldBuildingCubeMaps = m_BuildingCubemaps->GetInt();
			m_BuildingCubemaps->SetValue(1.0f);
		}
		/*
		if (isDepthF)
		{
			if (m_RenderTargetDepthF)
			{
				ctxp->PushRenderTargetAndViewport(
					m_RenderTargetDepthF,
					0,
					view.m_nUnscaledX,
					view.m_nUnscaledY,
					view.m_nUnscaledWidth,
					view.m_nUnscaledHeight
				);
			}
			else
			{
				Tier0_Warning("AFXERROR: CAfxStreams::CaptureStreamToBuffer: missing render target.\n");
				isDepthF = false;
			}
		}*/

		bool oldDoBloomAndToneMapping;
		bool overrideDoBloomAndToneMapping;
		{
			bool value;
			if (overrideDoBloomAndToneMapping = stream->DoBloomAndToneMapping.Get(value))
			{
				oldDoBloomAndToneMapping = view.m_bDoBloomAndToneMapping;
				const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDoBloomAndToneMapping = value;
			}
		}
		bool oldDoDepthOfField;
		bool overrideDoDepthOfField;
		{
			bool value;
			if (overrideDoDepthOfField = stream->DoDepthOfField.Get(value))
			{
				oldDoDepthOfField = view.m_bDoDepthOfField;
				const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDoDepthOfField = value;
			}
		}
		bool oldDrawWorldNormal;
		bool overrideDrawWorldNormal;
		{
			bool value;
			if (overrideDrawWorldNormal = stream->DrawWorldNormal.Get(value))
			{
				oldDrawWorldNormal = view.m_bDrawWorldNormal;
				const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDrawWorldNormal = value;
			}
		}
		bool oldCullFrontFaces;
		bool overrideCullFrontFaces;
		{
			bool value;
			if (overrideCullFrontFaces = stream->CullFrontFaces.Get(value))
			{
				oldCullFrontFaces = view.m_bCullFrontFaces;
				const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bCullFrontFaces = value;
			}
		}
		bool oldRenderFlashlightDepthTranslucents;
		bool overrideRenderFlashlightDepthTranslucents;
		{
			bool value;
			if (overrideRenderFlashlightDepthTranslucents = stream->RenderFlashlightDepthTranslucents.Get(value))
			{
				oldRenderFlashlightDepthTranslucents = view.m_bRenderFlashlightDepthTranslucents;
				const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bRenderFlashlightDepthTranslucents = value;
			}
		}

		stream->OnRenderBegin(captureTarget->GetBasefxStreamModifier(streamIndex), afxViewport, viewToProjection, viewToProjectionSky);

		DoRenderView(fn, This, Edx, view, hudViewSetup, SOURCESDK::VIEW_CLEAR_STENCIL | SOURCESDK::VIEW_CLEAR_DEPTH, myWhatToDraw);

		captureTarget->QueueCapture(ctxp, streamIndex);

		stream->OnRenderEnd();

		if (overrideRenderFlashlightDepthTranslucents) const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bRenderFlashlightDepthTranslucents = oldRenderFlashlightDepthTranslucents;
		if (overrideCullFrontFaces) const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bCullFrontFaces = oldCullFrontFaces;
		if (overrideDrawWorldNormal) const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDrawWorldNormal = oldDrawWorldNormal;
		if (overrideDoDepthOfField) const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDoDepthOfField = oldDoDepthOfField;
		if (overrideDoBloomAndToneMapping) const_cast<SOURCESDK::CViewSetup_csgo &>(view).m_bDoBloomAndToneMapping = oldDoBloomAndToneMapping;

		/*
		if (isDepthF)
		{
			ctxp->PopRenderTargetAndViewport();
		}
		*/

		if (forceBuildingCubeMaps)
		{
			m_BuildingCubemaps->SetValue((float)oldBuildingCubeMaps);
		}

		if (true)
		{
		}
		else
		{
			g_Hook_VClient_RenderView.GetGlobals()->frametime_set(oldFrameTime);
		}
	}

	*smokeOverlayAlphaFactor = oldSmokeOverlayAlphaFactor;


	if (0 < strlen(stream->DetachCommands_get()))
		WrpConCommands::ImmediatelyExecuteCommands(stream->DetachCommands_get()); // Execute commands after we lock the stream!


	if (last)
	{
		// This is now done implicitely instead // captureTarget->QueueCaptureEnd(ctxp);
	}

	return ctxp;
}

bool CAfxStreams::Console_CheckStreamName(char const * value)
{
	if(StringIsEmpty(value))
	{
		Tier0_Msg("Error: Stream name can not be emty.\n");
		return false;
	}
	if(!StringIsAlNum(value))
	{
		Tier0_Msg("Error: Stream name must be alphanumeric.\n");
		return false;
	}

	// Check if name is unique:
	{
		int index = 0;
		for(std::list<CAfxRecordStream *>::iterator it = m_Streams.begin(); it != m_Streams.end(); ++it)
		{
			if(!_stricmp((*it)->StreamName_get(), value))
			{
				Tier0_Msg("Error: Stream name must be unique, \"%s\" is already in use by stream with index %i.\n", value, index);
				return false;
			}

			++index;
		}
	}

	return true;
}

bool CAfxStreams::Console_ToAfxAction(char const * value, CAfxBaseFxStream::CAction * & action)
{
	CAfxBaseFxStream::CAction * tmpAction = CAfxBaseFxStream::GetAction(CAfxBaseFxStream::CActionKey(value));

	if(tmpAction)
	{
		action = tmpAction;
		return true;
	}

	Tier0_Warning("Invalid action name.\n");
	return false;
}

char const * CAfxStreams::Console_FromAfxAction(CAfxBaseFxStream::CAction * action)
{
	if(action)
	{
		return action->Key_get().m_Name.c_str();
	}

	return "[null]";
}

bool CAfxStreams::Console_ToStreamCombineType(char const * value, CAfxTwinStream::StreamCombineType & streamCombineType)
{
	if(!_stricmp(value, "aRedAsAlphaBColor"))
	{
		streamCombineType = CAfxTwinStream::SCT_ARedAsAlphaBColor;
		return true;
	}
	else if(!_stricmp(value, "aColorBRedAsAlpha"))
	{
		streamCombineType = CAfxTwinStream::SCT_AColorBRedAsAlpha;
		return true;
	}

	return false;
}

char const * CAfxStreams::Console_FromStreamCombineType(CAfxTwinStream::StreamCombineType streamCombineType)
{
	switch (streamCombineType)
	{
	case CAfxTwinStream::SCT_ARedAsAlphaBColor:
		return "aRedAsAlphaBColor";
	case CAfxTwinStream::SCT_AColorBRedAsAlpha:
		return "aColorBRedAsAlpha";
	}

	return "[unkown]";
}

bool CAfxStreams::Console_ToStreamCaptureType(char const * value, CAfxRenderViewStream::StreamCaptureType & StreamCaptureType)
{
	if(!_stricmp(value, "normal"))
	{
		StreamCaptureType = CAfxRenderViewStream::SCT_Normal;
		return true;
	}
	else
	if(!_stricmp(value, "depth24"))
	{
		StreamCaptureType = CAfxRenderViewStream::SCT_Depth24;
		return true;
	}
	else
	if(!_stricmp(value, "depth24ZIP"))
	{
		StreamCaptureType = CAfxRenderViewStream::SCT_Depth24ZIP;
		return true;
	}
	else
	if(!_stricmp(value, "depthF"))
	{
		StreamCaptureType = CAfxRenderViewStream::SCT_DepthF;
		return true;
	}
	else
	if(!_stricmp(value, "depthFZIP"))
	{
		StreamCaptureType = CAfxRenderViewStream::SCT_DepthFZIP;
		return true;
	}

	return false;
}

char const * CAfxStreams::Console_FromStreamCaptureType(CAfxRenderViewStream::StreamCaptureType StreamCaptureType)
{
	switch(StreamCaptureType)
	{
	case CAfxRenderViewStream::SCT_Normal:
		return "normal";
	case CAfxRenderViewStream::SCT_Depth24:
		return "depth24";
	case CAfxRenderViewStream::SCT_Depth24ZIP:
		return "depth24ZIP";
	case CAfxRenderViewStream::SCT_DepthF:
		return "depthF";
	case CAfxRenderViewStream::SCT_DepthFZIP:
		return "depthFZIP";
	}

	return "[unkown]";

}

bool CAfxStreams::CheckCanFeedStreams(void)
{
	return 0 != GetView_csgo()
		&& 0 != m_MaterialSystem
		&& 0 != m_AfxBaseClientDll
		&& 0 != m_ShaderShadow
	;
}

void CAfxStreams::BackUpMatVars()
{
	EnsureMatVars();

	m_OldMatPostProcessEnable = m_MatPostProcessEnableRef->GetInt();
	m_OldMatDynamicTonemapping = m_MatDynamicTonemappingRef->GetInt();
	m_OldMatMotionBlurEnabled = m_MatMotionBlurEnabledRef->GetInt();
	m_OldMatForceTonemapScale = m_MatForceTonemapScale->GetFloat();
	m_OldSndMuteLosefocus = m_SndMuteLosefocus->GetInt();
	m_OldPanoramaDisableLayerCache = m_PanoramaDisableLayerCache->GetInt();
}

void CAfxStreams::SetMatVarsForStreams()
{
	EnsureMatVars();

	if (0 <= m_NewMatPostProcessEnable) m_MatPostProcessEnableRef->SetValue((float)m_NewMatPostProcessEnable);
	if (0 <= m_NewMatDynamicTonemapping) m_MatDynamicTonemappingRef->SetValue((float)m_NewMatDynamicTonemapping);
	if (0 <= m_NewMatMotionBlurEnabled) m_MatMotionBlurEnabledRef->SetValue((float)m_NewMatMotionBlurEnabled);
	if (0 <= m_NewMatForceTonemapScale) m_MatForceTonemapScale->SetValue(m_NewMatForceTonemapScale);

	m_SndMuteLosefocus->SetValue(0.0f);
	m_PanoramaDisableLayerCache->SetValue(1.0f);
}

void CAfxStreams::RestoreMatVars()
{
	EnsureMatVars();

	m_MatPostProcessEnableRef->SetValue((float)m_OldMatPostProcessEnable);
	m_MatDynamicTonemappingRef->SetValue((float)m_OldMatDynamicTonemapping);
	m_MatMotionBlurEnabledRef->SetValue((float)m_OldMatMotionBlurEnabled);
	m_MatForceTonemapScale->SetValue(m_OldMatForceTonemapScale);
	m_SndMuteLosefocus->SetValue((float)m_OldSndMuteLosefocus);
	m_PanoramaDisableLayerCache->SetValue((float)m_OldPanoramaDisableLayerCache);
}

void CAfxStreams::EnsureMatVars()
{
	if(!m_MatPostProcessEnableRef) m_MatPostProcessEnableRef = new WrpConVarRef("mat_postprocess_enable");
	if(!m_MatDynamicTonemappingRef) m_MatDynamicTonemappingRef = new WrpConVarRef("mat_dynamic_tonemapping");
	if(!m_MatMotionBlurEnabledRef) m_MatMotionBlurEnabledRef = new WrpConVarRef("mat_motion_blur_enabled");
	if(!m_MatForceTonemapScale) m_MatForceTonemapScale = new WrpConVarRef("mat_force_tonemap_scale");
	if (!m_SndMuteLosefocus) m_SndMuteLosefocus = new WrpConVarRef("snd_mute_losefocus");
	if (!m_PanoramaDisableLayerCache) m_PanoramaDisableLayerCache = new WrpConVarRef("@panorama_disable_layer_cache");
	if (!m_BuildingCubemaps) m_BuildingCubemaps = new WrpConVarRef("building_cubemaps");
	//if (!m_cl_modelfastpath) m_cl_modelfastpath = new WrpConVarRef("cl_modelfastpath");
	//if (!m_cl_tlucfastpath) m_cl_tlucfastpath = new WrpConVarRef("cl_tlucfastpath");
	//if (!m_cl_brushfastpath) m_cl_brushfastpath = new WrpConVarRef("cl_brushfastpath");
	//if (!m_r_drawstaticprops) m_r_drawstaticprops = new WrpConVarRef("r_drawstaticprops");
}

/*
void CAfxStreams::DisableFastPath()
{
	EnsureMatVars();

	m_Old_cl_modelfastpath = m_cl_modelfastpath->GetInt();
	m_Old_cl_tlucfastpath = m_cl_tlucfastpath->GetInt();
	m_Old_cl_brushfastpath = m_cl_brushfastpath->GetInt();
	m_Old_r_drawstaticprops = m_r_drawstaticprops->GetInt();

	if(m_Old_cl_modelfastpath) m_cl_modelfastpath->SetValue(0);
	if(m_Old_cl_tlucfastpath) m_cl_tlucfastpath->SetValue(0);
	if(m_Old_cl_brushfastpath) m_cl_brushfastpath->SetValue(0);
	if(1 == m_Old_r_drawstaticprops) m_r_drawstaticprops->SetValue(4); // any other value than 1 disables fast path, 0,2 are special too.
}

void CAfxStreams::RestoreFastPath()
{
	m_r_drawstaticprops->SetValue((float)m_Old_r_drawstaticprops);
	m_cl_brushfastpath->SetValue((float)m_Old_cl_brushfastpath);
	m_cl_tlucfastpath->SetValue((float)m_Old_cl_tlucfastpath);
	m_cl_modelfastpath->SetValue((float)m_Old_cl_modelfastpath);
}
*/

void CAfxStreams::AddStream(CAfxRecordStream * stream)
{
	stream->AddRef();
	m_Streams.push_back(stream);

	if(m_Recording) stream->RecordStart();
}

void CAfxStreams::CreateRenderTargets(SOURCESDK::IMaterialSystem_csgo * materialSystem)
{
	//materialSystem->BeginRenderTargetAllocation();
/*
	m_RgbaRenderTarget = materialSystem->CreateRenderTargetTexture(0, 0, SOURCESDK::RT_SIZE_FULL_FRAME_BUFFER, SOURCESDK::IMAGE_FORMAT_RGBA8888);
	if(m_RgbaRenderTarget)
	{
		m_RgbaRenderTarget->IncrementReferenceCount();
	}
	else
	{
		Tier0_Warning("AFXERROR: CAfxStreams::CreateRenderTargets no m_RgbaRenderTarget!\n");
	}
	m_RenderTargetDepthF = materialSystem->CreateRenderTargetTexture(0,0, SOURCESDK::RT_SIZE_FULL_FRAME_BUFFER, SOURCESDK::IMAGE_FORMAT_R32F, SOURCESDK::MATERIAL_RT_DEPTH_SHARED);
	if(m_RenderTargetDepthF)
	{
		m_RenderTargetDepthF->IncrementReferenceCount();
	}
	else
	{
		Tier0_Warning("AFXWARNING: CAfxStreams::CreateRenderTargets no m_RenderTargetDepthF (affects high precision depthF captures)!\n");
	}
*/
	//materialSystem->EndRenderTargetAllocation();

}

IAfxStreamContext * CAfxStreams::FindStreamContext(IAfxMatRenderContext * ctx)
{
	if (!ctx)
		return 0;

	return ctx->Hook_get();
}

// CAfxStreams::CEntityBvhCapture //////////////////////////////////////////////

CAfxStreams::CEntityBvhCapture::CEntityBvhCapture(int entityIndex, Origin_e origin, Angles_e angles)
: m_BvhExport(0)
, m_EntityIndex(entityIndex)
, m_Origin(origin)
, m_Angles(angles)
{
}

CAfxStreams::CEntityBvhCapture::~CEntityBvhCapture()
{
	EndCapture();
}

void CAfxStreams::CEntityBvhCapture::StartCapture(std::wstring const & takePath, double frameTime)
{
	EndCapture();

	std::wostringstream os;
	os << takePath << L"\\cam_ent_" << m_EntityIndex << L".bvh";

	m_BvhExport = new BvhExport(
		os.str().c_str(),
		"MdtCam",
		frameTime
	);
}

void CAfxStreams::CEntityBvhCapture::EndCapture(void)
{
	if (!m_BvhExport)
		return;

	delete m_BvhExport;
	m_BvhExport = 0;
}

void CAfxStreams::CEntityBvhCapture::CaptureFrame(void)
{
	if (!m_BvhExport)
		return;

	SOURCESDK::Vector o;
	SOURCESDK::QAngle a;

	SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(m_EntityIndex);
	SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

	if (ce)
	{
		o = be && O_View == m_Origin ? be->EyePosition() : ce->GetAbsOrigin();
		a = be && A_View == m_Angles ? be->EyeAngles() : ce->GetAbsAngles();
	}
	else
	{
		o.x = 0;
		o.y = 0;
		o.z = 0;

		a.x = 0;
		a.y = 0;
		a.z = 0;
	}

	m_BvhExport->WriteFrame(
		-o.y, +o.z, -o.x,
		-a.z, -a.x, +a.y
	);
}

void CAfxStreams::BlockPresent(IAfxMatRenderContextOrg * ctx, bool value)
{
	QueueOrExecute(ctx, new CAfxLeafExecute_Functor(new AfxD3D9BlockPresent_Functor(value)));
}

void CAfxStreams::AfxStreamsInit(void)
{
	if(!csgo_CModelRenderSystem_SetupBones_Install()) Tier0_Warning("AFXERROR: csgo_CModelRenderSystem_SetupBones_Install() failed.");

	CAfxBaseFxStream::AfxStreamsInit();
}

void CAfxStreams::ShutDown(void)
{
	if (!m_ShutDown)
	{
		m_ShutDown = true;

		DrawingThread_DeviceLost();
		
		CAfxBaseFxStream::AfxStreamsShutdown();

		delete m_CamExportObj;

		while (!m_EntityBvhCaptures.empty())
		{
			delete m_EntityBvhCaptures.front();
			m_EntityBvhCaptures.pop_front();
		}

		while (!m_Streams.empty())
		{
			m_Streams.front()->Release();
			m_Streams.pop_front();
		}

		delete m_MatPostProcessEnableRef;
		delete m_HostFrameRate;
	}
}

// CAfxRecordingSettings ///////////////////////////////////////////////////////

CAfxRecordingSettings::CShared CAfxRecordingSettings::m_Shared;

CAfxClassicRecordingSettings::CShared::CShared()
{
	CAfxRecordingSettings * classicSettings = new CAfxClassicRecordingSettings();
	m_NamedSettings.emplace(classicSettings->GetName(), classicSettings);

	m_DefaultSettings = new CAfxDefaultRecordingSettings(classicSettings);
	m_DefaultSettings->AddRef();
	m_NamedSettings.emplace(m_DefaultSettings->GetName(), m_DefaultSettings);

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpeg", true, "-c:v libx264 -preset slow -crf 22 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpegYuv420p", true, "-c:v libx264 -pix_fmt yuv420p -preset slow -crf 22 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpegLosslessFast", true, "-c:v libx264rgb -preset ultrafast -crf 0 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpegLosslessBest", true, "-c:v libx264rgb -preset veryslow -crf 0 {QUOTE}{AFX_STREAM_PATH}\\\\video.mp4{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpegRaw", true, "-c:v rawvideo {QUOTE}{AFX_STREAM_PATH}\\\\video.avi{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings("afxFfmpegHuffyuv", true, "-c:v huffyuv {QUOTE}{AFX_STREAM_PATH}\\\\video.avi{QUOTE}");
		m_NamedSettings.emplace(settings->GetName(), settings);
	}

	{
		CAfxRecordingSettings * settings = new CAfxSamplingRecordingSettings("afxSampler30", true, m_DefaultSettings, EasySamplerSettings::ESM_Trapezoid, 30.0f, 1.0f, 1.0f);
		m_NamedSettings.emplace(settings->GetName(), settings);
	}
}

CAfxClassicRecordingSettings::CShared::~CShared()
{
	m_NamedSettings.clear();
	m_DefaultSettings->Release();
}

void CAfxRecordingSettings::Console(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("print", arg1))
		{
			for (auto it = m_Shared.m_NamedSettings.begin(); it != m_Shared.m_NamedSettings.end(); ++it)
			{
				Tier0_Msg("%s%s\n", it->second.Settings->GetName(), it->second.Settings->GetProtected() ? " (protected)" : "");
			}
			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argC)
		{
			const char * arg2 = args->ArgV(2);

			if (CAfxRecordingSettings * setting = CAfxRecordingSettings::GetByName(arg2))
			{
				CSubWrpCommandArgs subArgs(args, 3);
				setting->Console_Edit(&subArgs);
			}
			else
			{
				Tier0_Warning("AFXERROR: There is no recording setting named %s.\n", arg2);
			}
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argC)
		{
			const char * arg2 = args->ArgV(2);

			auto it = m_Shared.m_NamedSettings.find(arg2);

			if (it != m_Shared.m_NamedSettings.end())
			{
				if (it->second.Settings->GetProtected())
				{
					Tier0_Warning("AFXERROR: Setting %s is protected and thus can not be deleted.\n", arg2);
				}
				else if (!m_Shared.DeleteIfUnrefrenced(it))
				{
					Tier0_Warning("AFXERROR: Could not delete %s, because it has further references.\n", arg2);
				}
			}
			else
			{
				Tier0_Warning("AFXERROR: There is no recording setting named %s.\n", arg2);
			}
			return;
		}
		else if (0 == _strcmpi("add", arg1))
		{
			if (5 == argC && 0 == _stricmp("ffmpeg", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					Tier0_Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					Tier0_Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CAfxRecordingSettings * settings = new CAfxFfmpegRecordingSettings(arg3, false, args->ArgV(4));
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (5 == argC && 0 == _stricmp("ffmpegEx", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					Tier0_Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					Tier0_Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CAfxRecordingSettings * settings = new CAfxFfmpegExRecordingSettings(arg3, false, args->ArgV(4));
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (4 == argC && 0 == _stricmp("sampler", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					Tier0_Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					Tier0_Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CAfxRecordingSettings * settings = new CAfxSamplingRecordingSettings(arg3, false, m_Shared.m_DefaultSettings, EasySamplerSettings::ESM_Trapezoid, 30.0f, 1.0f, 1.0f);
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}
			else if (4 == argC && 0 == _stricmp("multi", args->ArgV(2)))
			{
				const char * arg3 = args->ArgV(3);

				if (StringIBeginsWith(arg3, "afx"))
				{
					Tier0_Warning("AFXERROR: Custom presets must not begin with \"afx\".\n");
				}
				else if (nullptr != GetByName(arg3))
				{
					Tier0_Warning("AFXERROR: There is already a setting named %s\n", arg3);
				}
				else
				{
					CAfxRecordingSettings * settings = new CAfxMultiRecordingSettings(arg3, false);
					m_Shared.m_NamedSettings.emplace(settings->GetName(), settings);
				}
				return;
			}

			Tier0_Msg(
				"%s add ffmpeg <name> \"<yourOptionsHere>\" - Adds an FFMPEG setting, <yourOptionsHere> are output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. For an example see one of the afxFfmpeg* templates (edit them).\n"
				"%s add ffmpegEx <name> \"<yourOptionsHere>\" - Adds an extended FFMPEG setting, <yourOptionsHere> are output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. Further variables: {FFMPEG_PATH} {PIXEL_FORMAT} {FRAMERATE} {WIDTH} {HEIGHT} - For an example see one of the afxFfmpeg* templates (edit them).\n"
				"%s add sampler <name> - Adds a sampler with 30 fps and default settings, edit it afterwards to change them.\n"
				"%s add multi <name> - Adds multi settings, edit it afterwards to add settings to it.\n"
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
	}

	Tier0_Msg(
		"%s print - List currently registered settings\n"
		"%s edit <name> - Edit setting.\n"
		"%s remove <name> - Remove setting.\n"
		"%s add [...] - Add a setting.\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

// CAfxClassicRecordingSettings ////////////////////////////////////////////////

void CAfxClassicRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	Tier0_Msg("%s (type classic) recording setting options:\n", m_Name.c_str());
	Tier0_Warning("The classic settings are controlled through mirv_streams settings and can not be edited.\n");
}

advancedfx::COutVideoStream * CAfxClassicRecordingSettings::CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float frameRate, const char * pathSuffix) const
{
	std::wstring wideStreamName;
	std::wstring widePathSuffix;
	if (UTF8StringToWideString(stream.StreamName_get(), wideStreamName) && UTF8StringToWideString(pathSuffix, widePathSuffix))
	{
		std::wstring capturePath(streams.GetTakeDir());
		capturePath.append(L"\\");
		capturePath.append(wideStreamName);
		capturePath.append(widePathSuffix);

		CAfxRenderViewStream::StreamCaptureType captureType = stream.GetCaptureType();

		return new advancedfx::COutImageStream(imageFormat, capturePath, (captureType == CAfxRenderViewStream::SCT_Depth24ZIP || captureType == CAfxRenderViewStream::SCT_DepthFZIP), streams.m_FormatBmpAndNotTga);
	}
	else
	{
		Tier0_Warning("AFXERROR: Could not convert \"%s\" and \"%s\" from UTF8 to wide string.\n", stream.StreamName_get(), pathSuffix);
	}

	return nullptr;
}

// CAfxFfmpegRecordingSettings ////////////////////////////////////////////////

void CAfxFfmpegRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("options", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FfmpegOptions = args->ArgV(2);
				return;
			}

			Tier0_Msg(
				"%s options \"<yourOptionsHere>\" - Set output options, use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_FfmpegOptions.c_str()
			);
			return;
		}
	}

	Tier0_Msg("%s (type ffmpeg) recording setting options:\n", m_Name.c_str());
	Tier0_Msg(
		"%s options [...] - FFMPEG options.\n"
		, arg0
	);
}

advancedfx::COutVideoStream * CAfxFfmpegRecordingSettings::CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float frameRate, const char * pathSuffix) const
{
	std::wstring widePathSuffix;
	if (UTF8StringToWideString(pathSuffix, widePathSuffix))
	{
		std::wstring wideOptions;
		if (UTF8StringToWideString(m_FfmpegOptions.c_str(), wideOptions))
		{
			std::wstring wideStreamName;
			if (UTF8StringToWideString(stream.StreamName_get(), wideStreamName))
			{
				std::wstring capturePath(streams.GetTakeDir());
				capturePath.append(L"\\");
				capturePath.append(wideStreamName);
				capturePath.append(widePathSuffix);

				CAfxRenderViewStream::StreamCaptureType captureType = stream.GetCaptureType();

				return new advancedfx::COutFFMPEGVideoStream(imageFormat, capturePath, std::wstring(L"{QUOTE}{FFMPEG_PATH}{QUOTE} -f rawvideo -pixel_format {PIXEL_FORMAT} -loglevel repeat+level+warning -framerate {FRAMERATE} -video_size {WIDTH}x{HEIGHT} -i pipe:0 -vf setsar=sar=1/1 ").append(wideOptions), frameRate);
			}
			else
			{
				Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", stream.StreamName_get());
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", m_FfmpegOptions.c_str());
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", pathSuffix);
	}

	return nullptr;
}



// CAfxFfmpegRecordingSettings ////////////////////////////////////////////////

void CAfxFfmpegExRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("options", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FfmpegOptions = args->ArgV(2);
				return;
			}

			Tier0_Msg(
				"%s options \"<yourOptionsHere>\" - Set output options use {QUOTE} for \", {AFX_STREAM_PATH} for the folder path of the stream, \\{ for {, \\} for }. Further variables: {FFMPEG_PATH} {PIXEL_FORMAT} {FRAMERATE} {WIDTH} {HEIGHT}\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_FfmpegOptions.c_str()
			);
			return;
		}
	}

	Tier0_Msg("%s (type ffmpegEx) recording setting options:\n", m_Name.c_str());
	Tier0_Msg(
		"%s options [...] - FFMPEG options.\n"
		, arg0
	);
}

advancedfx::COutVideoStream * CAfxFfmpegExRecordingSettings::CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float frameRate, const char * pathSuffix) const
{
	std::wstring widePathSuffix;
	if (UTF8StringToWideString(pathSuffix, widePathSuffix))
	{
		std::wstring wideOptions;
		if (UTF8StringToWideString(m_FfmpegOptions.c_str(), wideOptions))
		{
			std::wstring wideStreamName;
			if (UTF8StringToWideString(stream.StreamName_get(), wideStreamName))
			{
				std::wstring capturePath(streams.GetTakeDir());
				capturePath.append(L"\\");
				capturePath.append(wideStreamName);
				capturePath.append(widePathSuffix);

				CAfxRenderViewStream::StreamCaptureType captureType = stream.GetCaptureType();

				return new advancedfx::COutFFMPEGVideoStream(imageFormat, capturePath, wideOptions, frameRate);
			}
			else
			{
				Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", stream.StreamName_get());
			}
		}
		else
		{
			Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", m_FfmpegOptions.c_str());
		}
	}
	else
	{
		Tier0_Warning("AFXERROR: Could not convert \"%s\" from UTF8 to wide string.\n", pathSuffix);
	}

	return nullptr;
}


// CAfxDefaultRecordingSettings ////////////////////////////////////////////////

void CAfxDefaultRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("settings", arg1))
		{
			if (3 == argC)
			{
				CAfxRecordingSettings * settings = CAfxRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					Tier0_Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if(settings->InheritsFrom(this))
				{
					Tier0_Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (m_DefaultSettings) m_DefaultSettings->Release();
					m_DefaultSettings = settings;
					if (m_DefaultSettings) m_DefaultSettings->AddRef();
				}

				return;
			}

			Tier0_Msg(
				"%s settings <settingsName> - Use settings with name <settingsName> as default settings.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_DefaultSettings ? m_DefaultSettings->GetName() : "[null]"
			);
			return;
		}
	}

	Tier0_Msg("%s (type default) recording setting options:\n", m_Name.c_str());
	Tier0_Msg(
		"%s settings [...] - Set default settings.\n"
		, arg0
	);
}

// CAfxMultiRecordingSettings //////////////////////////////////////////////////


void CAfxMultiRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 == argC)
			{
				CAfxRecordingSettings * settings = CAfxRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					Tier0_Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if (settings->InheritsFrom(this))
				{
					Tier0_Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (settings) settings->AddRef();
					m_Settings.push_back(settings);
				}

				return;
			}

			Tier0_Msg(
				"%s add <settingsName> - Add settings with name <settingsName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1))
		{
			if (3 == argC)
			{
				CAfxRecordingSettings * settings = CAfxRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					Tier0_Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else
				{
					for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
					{
						CAfxRecordingSettings * itSettings = *it;
						if (itSettings == settings)
						{
							it = m_Settings.erase(it);
							itSettings->Release();
						}
					}
				}

				return;
			}

			Tier0_Msg(
				"%s remove <settingsName> - Remove settings with name <settingsName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			int idx = 0;
			for (auto it = m_Settings.begin(); it != m_Settings.end(); ++it)
			{
				CAfxRecordingSettings * itSettings = *it;
				Tier0_Msg("%i: %s\n", idx, itSettings ? itSettings->GetName() : "[null]");
				++idx;
			}
			if (0 == idx) Tier0_Msg("[empty]\n");
			return;
		}
	}

	Tier0_Msg("%s (type multi) recording setting options:\n", m_Name.c_str());
	Tier0_Msg(
		"%s add <settingsName> - Add settings.\n"
		"%s remove <settingsName> - Remove settings.\n"
		"%s print <settingsName> - List settings.\n"
		, arg0
		, arg0
		, arg0
	);
}

// CAfxSamplingRecordingSettings ///////////////////////////////////////////////

advancedfx::COutVideoStream * CAfxSamplingRecordingSettings::CreateOutVideoStream(const CAfxStreams & streams, const CAfxRecordStream & stream, const advancedfx::CImageFormat & imageFormat, float frameRate, const char * pathSuffix) const
{
	if (m_OutputSettings)
	{
		if (advancedfx::COutVideoStream * outVideoStream = m_OutputSettings->CreateOutVideoStream(streams, stream, imageFormat, m_OutFps, pathSuffix))
		{
			return new advancedfx::COutSamplingStream(imageFormat, outVideoStream, frameRate, m_Method, m_OutFps ? 1.0 / m_OutFps : 0.0, m_Exposure, m_FrameStrength, &g_AfxStreams.ImageBufferPoolThreadSafe);
		}
	}

	return nullptr;
}

void CAfxSamplingRecordingSettings::Console_Edit(IWrpCommandArgs * args)
{
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("settings", arg1))
		{
			if (3 == argC)
			{
				CAfxRecordingSettings * settings = CAfxRecordingSettings::GetByName(args->ArgV(2));

				if (nullptr == settings)
				{
					Tier0_Warning("AFXERROR: There is no setting named %s.\n", args->ArgV(2));
				}
				else if (settings->InheritsFrom(this))
				{
					Tier0_Warning("AFXERROR: Can not assign a setting that depends on this setting.\n");
				}
				else
				{
					if (m_OutputSettings) m_OutputSettings->Release();
					m_OutputSettings = settings;
					if (m_OutputSettings) m_OutputSettings->AddRef();
				}

				return;
			}

			Tier0_Msg(
				"%s settings <settingsName> - Use settings with name <settingsName> as output settings.\n"
				"Current value: \"%s\"\n"
				, arg0
				, m_OutputSettings ? m_OutputSettings->GetName() : "[null]"
			);
			return;
		}
		else if (0 == _stricmp("fps", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_OutFps = (float)atof(args->ArgV(2));
			}

			Tier0_Msg(
				"%s fps <fValue> - Output FPS, has to be greater than 0.\n"
				"Current value: %f\n"
				, arg0
				, m_OutFps
			);
			return;
		}
		else if (0 == _stricmp("method", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp(arg2, "rectangle"))
				{
					m_Method = EasySamplerSettings::ESM_Rectangle;
				}
				else if (0 == _stricmp(arg2, "trapezoid"))
				{
					m_Method = EasySamplerSettings::ESM_Trapezoid;
				}
				else
				{
					Tier0_Warning("AFXERROR: Invalid value.\n");
				}

				return;
			}

			const char * curMethod = "[n/a]";

			switch (m_Method)
			{
			case EasySamplerSettings::ESM_Rectangle:
				curMethod = "rectangle";
				break;
			case EasySamplerSettings::ESM_Trapezoid:
				curMethod = "trapezoid";
				break;
			};

			Tier0_Msg(
				"%s method rectangle|trapezoid.\n"
				"Current value: %s\n"
				, arg0
				, curMethod
			);
			return;
		}
		else if (0 == _stricmp("exposure", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_Exposure = atof(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s exposure <fValue>.\n"
				"Current value: %f\n"
				, arg0
				, m_Exposure
			);
			return;
		}
		else if (0 == _stricmp("strength", arg1))
		{
			if (3 == argC)
			{
				if (m_Protected)
				{
					Tier0_Warning("This setting is protected and can not be changed.\n");
					return;
				}

				m_FrameStrength = (float)atof(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"%s strength <fValue>.\n"
				"Current value: %f\n"
				, arg0
				, m_FrameStrength
			);
			return;
		}
	}

	Tier0_Msg("%s (type sampling) recording setting options:\n", m_Name.c_str());
	Tier0_Msg(
		"%s settings [...] - Output settings.\n"
		"%s fps [...] - Output fps.\n"
		"%s method [...] - Sampling method (default: trapezoid).\n"
		"%s exposure [...] - Frame exposure (0.0 (0\xc2\xb0 shutter angle) - 1.0 (360\xc2\xb0 shutter angle), default: 1.0).\n"
		"%s strength [...] - Frame strength (0.0 (max cross-frame blur) - 1.0 (no cross-frame blur), default: 1.0).\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

SOURCESDK::C_BaseEntity_csgo * GetMoveParent(SOURCESDK::C_BaseEntity_csgo * value)
{
	if (value)
	{
		if (SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(value->AfxGetMoveParentHandle()))
			return ce->GetBaseEntity();

	}

	return nullptr;
}


void CAfxStreams::DrawingThread_DeviceLost() {
	if (m_IntZTextureSurface) {
		m_IntZTextureSurface->Release();
		m_IntZTextureSurface = nullptr;
	}

	if (m_RenderTargetSurfaceNoMsaa) {
		m_RenderTargetSurfaceNoMsaa->Release();
		m_RenderTargetSurfaceNoMsaa = nullptr;
	}

	if (m_RenderTargetSurface) {
		m_RenderTargetSurface->Release();
		m_RenderTargetSurface = nullptr;
	}
}

void CAfxStreams::DrawingThread_DeviceRestored() {

}

IDirect3DSurface9* CAfxStreams::DrawingThread_GetOrCreateRenderTargetSurface() {
	if (nullptr == m_RenderTargetSurface) {
		m_RenderTargetSurface = AfxCreateCompatibleRenderTarget(AfxGetRenderTargetSurface(), true);
	}

	return m_RenderTargetSurface;
}

bool CAfxStreams::DrawingThread_HasRenderTargetMsaa() {
	return AfxD3d9HashRenderTargetMsaa();
}

IDirect3DSurface9* CAfxStreams::DrawingThread_GetOrCreateRenderTargetSurfaceNoMssaa() {
	if (nullptr == m_RenderTargetSurfaceNoMsaa) {
		m_RenderTargetSurfaceNoMsaa = AfxCreateCompatibleRenderTarget(AfxGetRenderTargetSurface(), false);
	}

	return m_RenderTargetSurfaceNoMsaa;
}

void CAfxStreams::DrawingThread_SetRenderTarget() {
	if (m_SetRenderTarget) {
		m_SetRenderTarget++;
		return;
	}

	if (IDirect3DSurface9* surface = DrawingThread_GetOrCreateRenderTargetSurface()) {
		AfxD3d9PushRenderTargetEx(surface);
	}

	m_SetRenderTarget++;
}

void CAfxStreams::DrawingThread_UnsetRenderTarget(bool strechtRect) {
	if (m_SetRenderTarget) {
		AfxD3d9PopRenderTarget(strechtRect);
		m_SetRenderTarget--;
	}
}

void CAfxStreams::DrawingThread_SetRenderTargetNoMsaa() {
	if (m_SetRenderTargetNoMsaa) {
		m_SetRenderTargetNoMsaa++;
		return;
	}

	if (IDirect3DSurface9* surface = DrawingThread_GetOrCreateRenderTargetSurfaceNoMssaa()) {
		AfxD3d9PushRenderTargetEx(surface);
	}

	m_SetRenderTargetNoMsaa++;
}

void CAfxStreams::DrawingThread_UnsetRenderTargetNoMsaa(bool strechtRect) {
	if (m_SetRenderTargetNoMsaa) {
		AfxD3d9PopRenderTarget(strechtRect);
		m_SetRenderTargetNoMsaa--;
	}
}

IDirect3DSurface9* CAfxStreams::DrawingThread_GetOrCreateIntZTextureSurface() {
	if (nullptr == m_IntZTextureSurface) {
		m_IntZTextureSurface = AfxCreateCompatibleDepthStencilINTZTextureSurface(AfxGetDepthStencilSurface());
	}

	return m_IntZTextureSurface;
}

void CAfxStreams::DrawingThread_SetIntZTextureSurface() {
	if (m_SetIntZTextureSurface)
		return;

	if (IDirect3DSurface9* surface = DrawingThread_GetOrCreateIntZTextureSurface()) {
		AfxD3d9PushDepthStencilEx(surface);
		m_SetIntZTextureSurface = true;
	}
}

void CAfxStreams::DrawingThread_UnsetIntZTextureSurface() {
	if (m_SetIntZTextureSurface) {
		AfxD3d9PopDepthStencil();
		m_SetIntZTextureSurface = false;
	}
}
