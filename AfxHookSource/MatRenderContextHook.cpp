#include "stdafx.h"

#include "MatRenderContextHook.h"

#include "AfxStreams.h"
#include "asmClassTools.h"
#include <shared/detours.h>

#include <map>
#include <stack>
#include <set>
#include <shared_mutex>
#include <mutex>


class CAfxMesh
	: public SOURCESDK::IMeshEx_csgo
	, public IAfxMesh
{
public:
	CAfxMesh(SOURCESDK::IMeshEx_csgo * parent)
		: m_Parent(parent)
	{
	}

	void AfxMatRenderContext_set(IAfxMatRenderContext * value)
	{
		m_AfxMatRenderContext = value;
	}

private:
	void Debug(int index, int ofs)
	{

	}

public:

	//
	// IAfxMesh:

	virtual SOURCESDK::IMeshEx_csgo * GetParent(void)
	{
		return m_Parent;
	}

	virtual IAfxMatRenderContext * GetContext(void)
	{
		return m_AfxMatRenderContext;
	}

	//
	// IVertexBuffer_csgo:

	virtual int VertexCount() const;
	virtual SOURCESDK::VertexFormat_t_csgo GetVertexFormat() const;
	virtual bool IsDynamic() const;
	virtual void BeginCastBuffer(SOURCESDK::VertexFormat_t_csgo format);
	virtual void EndCastBuffer();
	virtual int GetRoomRemaining() const;
	virtual bool Lock(int nVertexCount, bool bAppend, SOURCESDK::VertexDesc_t_csgo &desc);
	virtual void Unlock(int nVertexCount, SOURCESDK::VertexDesc_t_csgo &desc);
	virtual void Spew(int nVertexCount, const SOURCESDK::VertexDesc_t_csgo &desc);
	virtual void ValidateData(int nVertexCount, const SOURCESDK::VertexDesc_t_csgo & desc);
	virtual void _Unknown_10_IVertexBuffer_csgo(void);

	//
	// IMesh_csgo:

	virtual void Draw(int firstIndex = -1, int numIndices = 0);
	virtual void SetColorMesh(IMesh_csgo *pColorMesh, int nVertexOffset);
	virtual void Draw(SOURCESDK::CPrimList_csgo *pLists, int nLists);
	virtual void CopyToMeshBuilder(int iStartVert, int nVerts, int iStartIndex, int nIndices, int indexOffset, SOURCESDK::CMeshBuilder_csgo &builder);
	virtual void Spew(int numVerts, int numIndices, const SOURCESDK::MeshDesc_t_csgo &desc);
	virtual void ValidateData(int numVerts, int numIndices, const SOURCESDK::MeshDesc_t_csgo &desc);
	virtual void LockMesh(int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc, SOURCESDK::MeshBuffersAllocationSettings_t_csgo *pSettings);
	virtual void ModifyBegin(int firstVertex, int numVerts, int firstIndex, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc);
	virtual void ModifyEnd(SOURCESDK::MeshDesc_t_csgo& desc);
	virtual void UnlockMesh(int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc);
	virtual void ModifyBeginEx(bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc);
	virtual void SetFlexMesh(IMesh_csgo *pMesh, int nVertexOffset);
	virtual void DisableFlexMesh();
	virtual void MarkAsDrawn();
	virtual void DrawModulated(const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0);
	virtual unsigned int ComputeMemoryUsed();
	virtual void *AccessRawHardwareDataStream(SOURCESDK::uint8 nRawStreamIndex, SOURCESDK::uint32 numBytes, SOURCESDK::uint32 uiFlags, void *pvContext);
	virtual SOURCESDK::ICachedPerFrameMeshData_csgo *GetCachedPerFrameMeshData();
	virtual void ReconstructFromCachedPerFrameMeshData(SOURCESDK::ICachedPerFrameMeshData_csgo *pData);

	//
	// IMeshEx_csgo:

	virtual void _UNKNOWN_030(void);
	virtual void _UNKNOWN_031(void);
	virtual void _UNKNOWN_032(void);
	virtual void _UNKNOWN_033(void);
	virtual void _UNKNOWN_034(void);
	virtual void _UNKNOWN_035(void);
	virtual void _UNKNOWN_036(void);
	virtual void _UNKNOWN_037(void);
	virtual void _UNKNOWN_038(void);
	virtual void _UNKNOWN_039(void);
	virtual void _UNKNOWN_040(void);
	virtual void _UNKNOWN_041(void);
	virtual void _UNKNOWN_042(void);
	virtual void _UNKNOWN_043(void);
	virtual void _UNKNOWN_044(void);
	virtual void _UNKNOWN_045(void);
	virtual void _UNKNOWN_046(void);
	virtual void _UNKNOWN_047(void);
	virtual void _UNKNOWN_048(void);

	//
	// IIndexBuffer_csgo:

	virtual int IndexCount() const;
	virtual SOURCESDK::MaterialIndexFormat_t_csgo IndexFormat() const;
	//virtual bool IsDynamic() const;
	virtual void BeginCastBuffer(SOURCESDK::MaterialIndexFormat_t_csgo format);
	//virtual void EndCastBuffer();
	//virtual int GetRoomRemaining() const;
	virtual bool Lock(int nMaxIndexCount, bool bAppend, SOURCESDK::IndexDesc_t_csgo &desc);
	virtual void Unlock(int nWrittenIndexCount, SOURCESDK::IndexDesc_t_csgo &desc);
	virtual void ModifyBegin(bool bReadOnly, int nFirstIndex, int nIndexCount, SOURCESDK::IndexDesc_t_csgo& desc);
	virtual void ModifyEnd(SOURCESDK::IndexDesc_t_csgo& desc);
	virtual void Spew(int nIndexCount, const SOURCESDK::IndexDesc_t_csgo &desc);
	virtual void ValidateData(int nIndexCount, const SOURCESDK::IndexDesc_t_csgo &desc);
	virtual SOURCESDK::IMesh_csgo* GetMesh();
	virtual void _Unknown_13_IIndexBuffer_csgo(void * arg0);
	virtual bool _Unknown_14_IIndexBuffer_csgo(void);

private:
	IMeshEx_csgo * m_Parent;
	IAfxMatRenderContext * m_AfxMatRenderContext;
};

__declspec(naked) int CAfxMesh::VertexCount() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 0) }

__declspec(naked) SOURCESDK::VertexFormat_t_csgo CAfxMesh::GetVertexFormat() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 1) }

__declspec(naked) bool CAfxMesh::IsDynamic() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 2) }

__declspec(naked) void CAfxMesh::BeginCastBuffer(SOURCESDK::VertexFormat_t_csgo format)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 3) }

__declspec(naked) void CAfxMesh::EndCastBuffer()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 4) }

__declspec(naked) int CAfxMesh::GetRoomRemaining() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 5) }

__declspec(naked) bool CAfxMesh::Lock(int nVertexCount, bool bAppend, SOURCESDK::VertexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 6) }

__declspec(naked) void CAfxMesh::Unlock(int nVertexCount, SOURCESDK::VertexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 7) }

__declspec(naked) void CAfxMesh::Spew(int nVertexCount, const SOURCESDK::VertexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 8) }

__declspec(naked) void CAfxMesh::ValidateData(int nVertexCount, const SOURCESDK::VertexDesc_t_csgo & desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 9) }

__declspec(naked) void CAfxMesh::_Unknown_10_IVertexBuffer_csgo(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 10) }


//
// IMesh_csgo:

//__declspec(naked) 
void CAfxMesh::Draw(int firstIndex, int numIndices)
{ //NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,0,12) /* !ofs different due to overloaded method! */

	Debug(0, 12);

	IAfxStreamContext * stream = m_AfxMatRenderContext->Hook_get();

	if (stream)
		stream->Draw(this, firstIndex, numIndices);
	else
		m_Parent->Draw(firstIndex, numIndices);
}

__declspec(naked) void CAfxMesh::SetColorMesh(IMesh_csgo *pColorMesh, int nVertexOffset)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 13) /* !ofs different due to overloaded method! */ }

//__declspec(naked) 
void CAfxMesh::Draw(SOURCESDK::CPrimList_csgo *pLists, int nLists)
{ // NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,0,11) /* !ofs different due to overloaded method! */

	Debug(0, 11);

	IAfxStreamContext * stream = m_AfxMatRenderContext->Hook_get();

	if (stream)
		stream->Draw_2(this, pLists, nLists);
	else
		m_Parent->Draw(pLists, nLists);
}

__declspec(naked) void CAfxMesh::CopyToMeshBuilder(int iStartVert, int nVerts, int iStartIndex, int nIndices, int indexOffset, SOURCESDK::CMeshBuilder_csgo &builder)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 14) }

__declspec(naked) void CAfxMesh::Spew(int numVerts, int numIndices, const SOURCESDK::MeshDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 15) }

__declspec(naked) void CAfxMesh::ValidateData(int numVerts, int numIndices, const SOURCESDK::MeshDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 16) }

__declspec(naked) void CAfxMesh::LockMesh(int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc, SOURCESDK::MeshBuffersAllocationSettings_t_csgo *pSettings)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 17) }

__declspec(naked) void CAfxMesh::ModifyBegin(int firstVertex, int numVerts, int firstIndex, int numIndices, SOURCESDK::MeshDesc_t_csgo& desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 18) }

__declspec(naked) void CAfxMesh::ModifyEnd(SOURCESDK::MeshDesc_t_csgo& desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 19) }

__declspec(naked) void CAfxMesh::UnlockMesh(int numVerts, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 20) }

__declspec(naked) void CAfxMesh::ModifyBeginEx(bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, SOURCESDK::MeshDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 21) }

__declspec(naked) void CAfxMesh::SetFlexMesh(IMesh_csgo *pMesh, int nVertexOffset)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 22) }

__declspec(naked) void CAfxMesh::DisableFlexMesh()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 23) }

__declspec(naked) void CAfxMesh::MarkAsDrawn()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 24) }

//__declspec(naked) 
void CAfxMesh::DrawModulated(const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex, int numIndices)
{ // NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,0,25)

	IAfxStreamContext * stream = m_AfxMatRenderContext->Hook_get();

	if (stream)
		stream->DrawModulated(this, vecDiffuseModulation, firstIndex, numIndices);
	else
		m_Parent->DrawModulated(vecDiffuseModulation, firstIndex, numIndices);
}

__declspec(naked) unsigned int CAfxMesh::ComputeMemoryUsed()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 26) }

__declspec(naked) void *CAfxMesh::AccessRawHardwareDataStream(SOURCESDK::uint8 nRawStreamIndex, SOURCESDK::uint32 numBytes, SOURCESDK::uint32 uiFlags, void *pvContext)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 27) }

__declspec(naked) SOURCESDK::ICachedPerFrameMeshData_csgo *CAfxMesh::GetCachedPerFrameMeshData()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 28) }

__declspec(naked) void CAfxMesh::ReconstructFromCachedPerFrameMeshData(SOURCESDK::ICachedPerFrameMeshData_csgo *pData)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 29) }

//
// IMeshEx_csgo:

__declspec(naked) void CAfxMesh::_UNKNOWN_030(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 30) }

__declspec(naked) void CAfxMesh::_UNKNOWN_031(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 31) }

__declspec(naked) void CAfxMesh::_UNKNOWN_032(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 32) }

__declspec(naked) void CAfxMesh::_UNKNOWN_033(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 33) }

__declspec(naked) void CAfxMesh::_UNKNOWN_034(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 34) }

__declspec(naked) void CAfxMesh::_UNKNOWN_035(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 35) }

__declspec(naked) void CAfxMesh::_UNKNOWN_036(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 36) }

__declspec(naked) void CAfxMesh::_UNKNOWN_037(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 37) }

__declspec(naked) void CAfxMesh::_UNKNOWN_038(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 38) }

__declspec(naked) void CAfxMesh::_UNKNOWN_039(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 39) }

__declspec(naked) void CAfxMesh::_UNKNOWN_040(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 40) }

__declspec(naked) void CAfxMesh::_UNKNOWN_041(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 41) }

__declspec(naked) void CAfxMesh::_UNKNOWN_042(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 42) }

__declspec(naked) void CAfxMesh::_UNKNOWN_043(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 43) }

__declspec(naked) void CAfxMesh::_UNKNOWN_044(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 44) }

__declspec(naked) void CAfxMesh::_UNKNOWN_045(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 45) }

__declspec(naked) void CAfxMesh::_UNKNOWN_046(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 46) }

__declspec(naked) void CAfxMesh::_UNKNOWN_047(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 47) }

__declspec(naked) void CAfxMesh::_UNKNOWN_048(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 0, 48) }

//
// IIndexBuffer_csgo:

__declspec(naked) int CAfxMesh::IndexCount() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 0) }

__declspec(naked) SOURCESDK::MaterialIndexFormat_t_csgo CAfxMesh::IndexFormat() const
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 1) }

//__declspec(naked) bool IsDynamic() const
//{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,1,2) }

__declspec(naked) void CAfxMesh::BeginCastBuffer(SOURCESDK::MaterialIndexFormat_t_csgo format)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 3) }

//__declspec(naked) void CAfxMesh::EndCastBuffer()
//{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,1,4) }

//__declspec(naked) int CAfxMesh::GetRoomRemaining() const
//{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh,m_Parent,1,5) }

__declspec(naked) bool CAfxMesh::Lock(int nMaxIndexCount, bool bAppend, SOURCESDK::IndexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 6) }

__declspec(naked) void CAfxMesh::Unlock(int nWrittenIndexCount, SOURCESDK::IndexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 7) }

__declspec(naked) void CAfxMesh::ModifyBegin(bool bReadOnly, int nFirstIndex, int nIndexCount, SOURCESDK::IndexDesc_t_csgo& desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 8) }

__declspec(naked) void CAfxMesh::ModifyEnd(SOURCESDK::IndexDesc_t_csgo& desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 9) }

__declspec(naked) void CAfxMesh::Spew(int nIndexCount, const SOURCESDK::IndexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 10) }

__declspec(naked) void CAfxMesh::ValidateData(int nIndexCount, const SOURCESDK::IndexDesc_t_csgo &desc)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 11) }

__declspec(naked) SOURCESDK::IMesh_csgo* CAfxMesh::GetMesh()
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 12) }

__declspec(naked) void CAfxMesh::_Unknown_13_IIndexBuffer_csgo(void * arg0)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 13) }

__declspec(naked) bool CAfxMesh::_Unknown_14_IIndexBuffer_csgo(void)
{ NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG(CAfxMesh, m_Parent, 1, 14) }


std::map<SOURCESDK::IMeshEx_csgo *, CAfxMesh *> g_MeshMap_csgo;
std::stack<CAfxMesh *> g_MeshHooks_csgo;
std::shared_timed_mutex g_MeshMap_csgo_Mutex;


class CAfxCallQueue
	: public SOURCESDK::CSGO::ICallQueue
	, public IAfxCallQueue
{
public:
	CAfxCallQueue(SOURCESDK::CSGO::ICallQueue * parent)
		: m_Parent(parent)
	{
	}

	void AfxMatRenderContext_set(IAfxMatRenderContext * value)
	{
		m_AfxMatRenderContext = value;
	}

public:

	//
	// IAfxCallQueue:

	virtual SOURCESDK::CSGO::ICallQueue * GetParent(void)
	{
		return m_Parent;
	}


protected:
	virtual void QueueFunctorInternal(SOURCESDK::CSGO::CFunctor *pFunctor)
	{
		IAfxStreamContext * stream = m_AfxMatRenderContext->Hook_get();

		if (stream)
			stream->QueueFunctorInternal(this, pFunctor);
		else
			m_Parent->QueueFunctorInternal(pFunctor);
	}

private:
	SOURCESDK::CSGO::ICallQueue * m_Parent;
	IAfxMatRenderContext * m_AfxMatRenderContext;
};

std::map<SOURCESDK::CSGO::ICallQueue *, CAfxCallQueue *> g_CallQueueMap_csgo;
std::stack<CAfxCallQueue *> g_CallQueueHooks_csgo;
std::shared_timed_mutex g_CallQueueMap_csgo_Mutex;

//:009
typedef void (__stdcall * MatRenderContextHook_Bind_t)(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo * material,
	void *proxyData);

/*
//:040
typedef void(__stdcall * MatRenderContextHook_Viewport_t)(
	DWORD *this_ptr,
	int x, int y, int width, int height);
*/

//:062
typedef SOURCESDK::IMeshEx_csgo* (_stdcall * MatRenderContextHook_GetDynamicMesh_t)(
	DWORD *this_ptr,
	bool buffered,
	SOURCESDK::IMesh_csgo* pVertexOverride,
	SOURCESDK::IMesh_csgo* pIndexOverride,
	SOURCESDK::IMaterial_csgo *pAutoBind);

//:081
typedef void (_stdcall * MatRenderContextHook_DrawScreenSpaceQuad_t)(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo * pMaterial);

//:113
typedef void (_stdcall * MatRenderContextHook_DrawScreenSpaceRectangle_t)(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo *pMaterial,
	int destx, int desty,
	int width, int height,
	float src_texture_x0, float src_texture_y0,
	float src_texture_x1, float src_texture_y1,
	int src_texture_width, int src_texture_height,
	void *pClientRenderable,
	int nXDice,
	int nYDice);

//:150
typedef SOURCESDK::CSGO::ICallQueue * (_stdcall * MatRenderContextHook_GetCallQueue_t)(
	DWORD *this_ptr);

//:167
typedef SOURCESDK::IMeshEx_csgo* (_stdcall * MatRenderContextHook_GetDynamicMeshEx_t)(
	DWORD *this_ptr,
	SOURCESDK::VertexFormat_t_csgo vertexFormat,
	bool buffered,
	SOURCESDK::IMesh_csgo* pVertexOverride,
	SOURCESDK::IMesh_csgo* pIndexOverride,
	SOURCESDK::IMaterial_csgo *pAutoBind);

//:192
typedef void (_stdcall * MatRenderContextHook_DrawInstances_t)(
	DWORD *this_ptr,
	int nInstanceCount,
	const SOURCESDK::MeshInstanceData_t_csgo *pInstance);

struct CMatRenderContextDetours
{
	//:009
	MatRenderContextHook_Bind_t Bind;

	/*
	// :040
	MatRenderContextHook_Viewport_t Viewport;
	*/

	//:062
	MatRenderContextHook_GetDynamicMesh_t GetDynamicMesh;

	//:081
	MatRenderContextHook_DrawScreenSpaceQuad_t DrawScreenSpaceQuad;

	//:113
	MatRenderContextHook_DrawScreenSpaceRectangle_t DrawScreenSpaceRectangle;

	//:150
	MatRenderContextHook_GetCallQueue_t GetCallQueue;

	//:167
	MatRenderContextHook_GetDynamicMeshEx_t GetDynamicMeshEx;

	//:192
	MatRenderContextHook_DrawInstances_t DrawInstances;
};


class CMatRenderContextHook
	: public IAfxMatRenderContext
	, public IAfxMatRenderContextOrg
{
public:
	static std::map<SOURCESDK::IMatRenderContext_csgo *, CMatRenderContextHook *> m_Map;
	static std::shared_timed_mutex m_MapMutex;

	/// <remarks>
	/// This can be called from yet unknown contexts outside of the current material system context,
	/// due to the vtable being hooked!
	/// <remarks>
	static CMatRenderContextHook * GetMatRenderContextHook(SOURCESDK::IMatRenderContext_csgo * ctx)
	{
		CMatRenderContextHook * result;

		m_MapMutex.lock_shared();

		std::map<SOURCESDK::IMatRenderContext_csgo *, CMatRenderContextHook *>::iterator it = m_Map.find(ctx);

		if (it != m_Map.end())
		{
			result = it->second;
			m_MapMutex.unlock_shared();
		}
		else
		{
			m_MapMutex.unlock_shared();
			m_MapMutex.lock();

			it = m_Map.find(ctx);

			if (it != m_Map.end())
			{
				result = it->second;
			}
			else
			{
				result = m_Map[ctx] = new CMatRenderContextHook(ctx);
			}

			m_MapMutex.unlock();
		}


		return result;
	}

	/// <remarks>When calling it has to be made sure that no other ctx with the same vtbale is being hooked at the same time. (Use a mutex i.e.).</remarks>
	CMatRenderContextHook(SOURCESDK::IMatRenderContext_csgo * orgCtx)
		: m_Ctx(orgCtx)
		, m_Hook(0)
	{
		HooKVtable(orgCtx);
	}

	//
	// IAfxMatRenderContext:

	virtual IAfxMatRenderContextOrg * GetOrg(void)
	{
		return this;
	}

	virtual IAfxStreamContext * Hook_get(void)
	{
		return m_Hook;
	}

	virtual void Hook_set(IAfxStreamContext * value)
	{
		m_Hook = value;

	}

	//
	////

	virtual void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil = false)
	{
		// This is unhooked, so pass through:

		m_Ctx->ClearBuffers(bClearColor, bClearDepth, bClearStencil);
	}

	virtual void ReadPixels(int x, int y, int width, int height, unsigned char *data, SOURCESDK::ImageFormat_csgo dstFormat, unsigned __int32 _unknown7 = 0)
	{
		// This is unhooked, so pass through:

		m_Ctx->ReadPixels(x, y, width, height, data, dstFormat, _unknown7);
	}

	/*
	virtual void Viewport(int x, int y, int width, int height)
	{
		// This is hooked, so use detour:

		m_Detours->Viewport((DWORD *)m_Ctx, x, y, width, height);
	}
	*/

	virtual void ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		// This is unhooked, so pass through:

		m_Ctx->ClearColor4ub(r, g, b, a);
	}

	virtual void PushRenderTargetAndViewport(SOURCESDK::ITexture_csgo *pTexture, SOURCESDK::ITexture_csgo *pDepthTexture, int nViewX, int nViewY, int nViewW, int nViewH)
	{
		// This is unhooked, so pass through:

		m_Ctx->PushRenderTargetAndViewport(pTexture, pDepthTexture, nViewX, nViewY, nViewW, nViewH);
	}

	virtual void PopRenderTargetAndViewport(void)
	{
		// This is unhooked, so pass through:

		m_Ctx->PopRenderTargetAndViewport();
	}

	virtual SOURCESDK::CSGO::ICallQueue *GetCallQueue()
	{
		// This is hooked, so use detour:

		return m_Detours->GetCallQueue((DWORD *)m_Ctx);

	}

	virtual void DrawInstances(int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance)
	{
		// This is hooked, so use detour:

		m_Detours->DrawInstances((DWORD *)m_Ctx,
			nInstanceCount,
			pInstance);
	}

	//
	////

	void Hook_Bind(
		SOURCESDK::IMaterial_csgo * material,
		void *proxyData)
	{
		m_Detours->Bind((DWORD *)m_Ctx, DoOnMaterialHook(material, proxyData), proxyData);
	}

	/*
	void Hook_Viewport(
		int x, int y, int width, int height)
	{
		IAfxStreamContext * afxStream = Hook_get();

		if (afxStream)
		{

			afxStream->Viewport(x, y, width, height);
		}
		else
		{
			m_Detours->Viewport((DWORD *)m_Ctx,
				x, y, width, height);
		}
	}
	*/

	SOURCESDK::IMeshEx_csgo* Hook_GetDynamicMesh(
		bool buffered,
		SOURCESDK::IMesh_csgo* pVertexOverride,
		SOURCESDK::IMesh_csgo* pIndexOverride,
		SOURCESDK::IMaterial_csgo *pAutoBind)
	{
		SOURCESDK::IMeshEx_csgo * iMesh = m_Detours->GetDynamicMesh((DWORD *)m_Ctx,
			buffered,
			pVertexOverride,
			pIndexOverride,
			pAutoBind);

		return AfxWrapMesh(iMesh);
	}

	void Hook_DrawScreenSpaceQuad(
		SOURCESDK::IMaterial_csgo * pMaterial)
	{
		m_Detours->DrawScreenSpaceQuad((DWORD *)m_Ctx,
			pMaterial
		);
	}

	void Hook_DrawScreenSpaceRectangle(
		SOURCESDK::IMaterial_csgo *pMaterial,
		int destx, int desty,
		int width, int height,
		float src_texture_x0, float src_texture_y0,
		float src_texture_x1, float src_texture_y1,
		int src_texture_width, int src_texture_height,
		void *pClientRenderable,
		int nXDice,
		int nYDice)
	{
		m_Detours->DrawScreenSpaceRectangle((DWORD *)m_Ctx,
			pMaterial,
			destx, desty,
			width, height,
			src_texture_x0, src_texture_y0,
			src_texture_x1, src_texture_y1,
			src_texture_width, src_texture_height,
			pClientRenderable,
			nXDice,
			nYDice);
	}

	SOURCESDK::CSGO::ICallQueue * Hook_GetCallQueue()
	{
		SOURCESDK::CSGO::ICallQueue * callQueue = m_Detours->GetCallQueue((DWORD *)m_Ctx);

		return AfxWrapCallQueue(callQueue);
	}

	SOURCESDK::IMeshEx_csgo* Hook_GetDynamicMeshEx(
		SOURCESDK::VertexFormat_t_csgo vertexFormat,
		bool buffered,
		SOURCESDK::IMesh_csgo* pVertexOverride,
		SOURCESDK::IMesh_csgo* pIndexOverride,
		SOURCESDK::IMaterial_csgo *pAutoBind)
	{
		SOURCESDK::IMeshEx_csgo * iMesh = m_Detours->GetDynamicMeshEx((DWORD *)m_Ctx,
			vertexFormat,
			buffered,
			pVertexOverride,
			pIndexOverride,
			pAutoBind);

		return AfxWrapMesh(iMesh);
	}

	void Hook_DrawInstances(
		int nInstanceCount,
		const SOURCESDK::MeshInstanceData_t_csgo *pInstance)
	{
		IAfxStreamContext * afxStream = Hook_get();

		if (afxStream)
		{

			afxStream->DrawInstances(nInstanceCount, pInstance);
		}
		else
		{
			m_Detours->DrawInstances((DWORD *)m_Ctx,
				nInstanceCount,
				pInstance);
		}
	}

private:
	static std::map<int *, CMatRenderContextDetours> m_VtableMap;

	SOURCESDK::IMatRenderContext_csgo * m_Ctx;
	CMatRenderContextDetours * m_Detours;
	IAfxStreamContext * m_Hook;

	void HooKVtable(SOURCESDK::IMatRenderContext_csgo * orgCtx);

	SOURCESDK::IMeshEx_csgo * AfxWrapMesh(SOURCESDK::IMeshEx_csgo * mesh)
	{
		if (!mesh)
			return 0;

		CAfxMesh * afxMesh;

		g_MeshMap_csgo_Mutex.lock_shared();

		std::map<SOURCESDK::IMeshEx_csgo *, CAfxMesh *>::iterator it = g_MeshMap_csgo.find(mesh);


		if (it != g_MeshMap_csgo.end())
		{
			//Tier0_Msg("Found known IMesh 0x%08x.\n", (DWORD)iMesh);
			afxMesh = it->second; // re-use
			g_MeshMap_csgo_Mutex.unlock_shared();
		}
		else
		{
			g_MeshMap_csgo_Mutex.unlock_shared();
			g_MeshMap_csgo_Mutex.lock();

			it = g_MeshMap_csgo.find(mesh);

			if (it != g_MeshMap_csgo.end())
			{
				afxMesh = it->second;
			}
			else
			{
				//Tier0_Msg("New IMesh 0x%08x.\n", (DWORD)iMesh);
				afxMesh = new CAfxMesh(mesh);

				g_MeshMap_csgo[mesh] = afxMesh; // track hooked mesh
				g_MeshMap_csgo[afxMesh] = afxMesh; // make sure we won't wrap ourself!
				g_MeshHooks_csgo.push(afxMesh);
			}

			g_MeshMap_csgo_Mutex.unlock();
		}

		afxMesh->AfxMatRenderContext_set(this); // tell it about us :-)

		return afxMesh;
	}

	SOURCESDK::CSGO::ICallQueue * AfxWrapCallQueue(SOURCESDK::CSGO::ICallQueue * callQueue)
	{
		if (!callQueue)
			return 0;

		CAfxCallQueue * afxCallQueue;

		g_CallQueueMap_csgo_Mutex.lock_shared();

		std::map<SOURCESDK::CSGO::ICallQueue *, CAfxCallQueue *>::iterator it = g_CallQueueMap_csgo.find(callQueue);


		if (it != g_CallQueueMap_csgo.end())
		{
			afxCallQueue = it->second; // re-use
			g_CallQueueMap_csgo_Mutex.unlock_shared();
		}
		else
		{
			g_CallQueueMap_csgo_Mutex.unlock_shared();
			g_CallQueueMap_csgo_Mutex.lock();

			it = g_CallQueueMap_csgo.find(callQueue);

			if (it != g_CallQueueMap_csgo.end())
			{
				afxCallQueue = it->second; // re-use
			}
			else
			{
				afxCallQueue = new CAfxCallQueue(callQueue);

				g_CallQueueMap_csgo[callQueue] = afxCallQueue; // track new queue
				g_CallQueueMap_csgo[afxCallQueue] = afxCallQueue; // make sure we won't wrap ourself!
				g_CallQueueHooks_csgo.push(afxCallQueue);
			}

			g_CallQueueMap_csgo_Mutex.unlock();
		}


		afxCallQueue->AfxMatRenderContext_set(this); // tell it about us :-)

		return afxCallQueue;
	}

	SOURCESDK::IMaterial_csgo * DoOnMaterialHook(SOURCESDK::IMaterial_csgo * value, void * proxyData)
	{
		IAfxStreamContext * afxStream = Hook_get();
		if (afxStream && value)
			return afxStream->MaterialHook(value, proxyData);

		return value;
	}
};


std::map<SOURCESDK::IMatRenderContext_csgo *, CMatRenderContextHook *> CMatRenderContextHook::m_Map;
std::shared_timed_mutex CMatRenderContextHook::m_MapMutex;
std::map<int *, CMatRenderContextDetours> CMatRenderContextHook::m_VtableMap;


void __stdcall MatRenderContextHook_Bind(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo * material,
	void *proxyData)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_Bind(material, proxyData);
}

/*
void __stdcall MatRenderContextHook_Viewport(
	DWORD *this_ptr,
	int x, int y, int width, int height)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_Viewport(x, y, width, height);
}
*/

SOURCESDK::IMeshEx_csgo* _stdcall MatRenderContextHook_GetDynamicMesh(
	DWORD *this_ptr,
	bool buffered,
	SOURCESDK::IMesh_csgo* pVertexOverride,
	SOURCESDK::IMesh_csgo* pIndexOverride,
	SOURCESDK::IMaterial_csgo *pAutoBind)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_GetDynamicMesh(buffered, pVertexOverride, pIndexOverride, pAutoBind);
}

void _stdcall MatRenderContextHook_DrawScreenSpaceQuad(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo * pMaterial)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_DrawScreenSpaceQuad(pMaterial);
}

void _stdcall MatRenderContextHook_DrawScreenSpaceRectangle(
	DWORD *this_ptr,
	SOURCESDK::IMaterial_csgo *pMaterial,
	int destx, int desty,
	int width, int height,
	float src_texture_x0, float src_texture_y0,
	float src_texture_x1, float src_texture_y1,
	int src_texture_width, int src_texture_height,
	void *pClientRenderable,
	int nXDice,
	int nYDice)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_DrawScreenSpaceRectangle(
		pMaterial,
		destx, desty,
		width, height,
		src_texture_x0, src_texture_y0,
		src_texture_x1, src_texture_y1,
		src_texture_width, src_texture_height,
		pClientRenderable,
		nXDice,
		nYDice);
}

SOURCESDK::CSGO::ICallQueue * _stdcall MatRenderContextHook_GetCallQueue(
	DWORD *this_ptr)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_GetCallQueue();
}

SOURCESDK::IMeshEx_csgo* _stdcall MatRenderContextHook_GetDynamicMeshEx(
	DWORD *this_ptr,
	SOURCESDK::VertexFormat_t_csgo vertexFormat,
	bool buffered,
	SOURCESDK::IMesh_csgo* pVertexOverride,
	SOURCESDK::IMesh_csgo* pIndexOverride,
	SOURCESDK::IMaterial_csgo *pAutoBind)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_GetDynamicMeshEx(
		vertexFormat,
		buffered,
		pVertexOverride,
		pIndexOverride,
		pAutoBind);
}

void _stdcall MatRenderContextHook_DrawInstances(
	DWORD *this_ptr,
	int nInstanceCount,
	const SOURCESDK::MeshInstanceData_t_csgo *pInstance)
{
	CMatRenderContextHook * ctxh = CMatRenderContextHook::GetMatRenderContextHook((SOURCESDK::IMatRenderContext_csgo *)this_ptr);
	return ctxh->Hook_DrawInstances(
		nInstanceCount,
		pInstance);
}

void CMatRenderContextHook::HooKVtable(SOURCESDK::IMatRenderContext_csgo * orgCtx)
{
	int * vtable = *(int**)orgCtx;

	std::map<int *, CMatRenderContextDetours>::iterator it = m_VtableMap.find(vtable);

	if (it != m_VtableMap.end())
	{
		m_Detours = &(it->second);
		return;
	}

	m_Detours = &(m_VtableMap[vtable]);

	//OutputDebugString("HooKVtable DETOUR BEGIN\n");
	DetourIfacePtr((DWORD *)&(vtable[9]), MatRenderContextHook_Bind, (DetourIfacePtr_fn &)m_Detours->Bind);
	//DetourIfacePtr((DWORD *)&(vtable[40]), MatRenderContextHook_Viewport, (DetourIfacePtr_fn &)m_Detours->Viewport);
	DetourIfacePtr((DWORD *)&(vtable[62]), MatRenderContextHook_GetDynamicMesh, (DetourIfacePtr_fn &)m_Detours->GetDynamicMesh);
	DetourIfacePtr((DWORD *)&(vtable[81]), MatRenderContextHook_DrawScreenSpaceQuad, (DetourIfacePtr_fn &)m_Detours->DrawScreenSpaceQuad);
	DetourIfacePtr((DWORD *)&(vtable[113]), MatRenderContextHook_DrawScreenSpaceRectangle, (DetourIfacePtr_fn &)m_Detours->DrawScreenSpaceRectangle);
	DetourIfacePtr((DWORD *)&(vtable[150]), MatRenderContextHook_GetCallQueue, (DetourIfacePtr_fn &)m_Detours->GetCallQueue);
	DetourIfacePtr((DWORD *)&(vtable[168]), MatRenderContextHook_GetDynamicMeshEx, (DetourIfacePtr_fn &)m_Detours->GetDynamicMeshEx);
	DetourIfacePtr((DWORD *)&(vtable[193]), MatRenderContextHook_DrawInstances, (DetourIfacePtr_fn &)m_Detours->DrawInstances);
	//OutputDebugString("HooKVtable DETOUR END\n");
}

IAfxMatRenderContext * MatRenderContextHook(SOURCESDK::IMatRenderContext_csgo *  ctx)
{
	return CMatRenderContextHook::GetMatRenderContextHook(ctx);
}

IAfxMatRenderContext * MatRenderContextHook(SOURCESDK::IMaterialSystem_csgo * materialSystem)
{
	SOURCESDK::IMatRenderContext_csgo * ctx = materialSystem->GetRenderContext();
	ctx->Release(); // GetRenderContext calls AddRef on Returned.

	return MatRenderContextHook(ctx);
}

void MatRenderContextHook_Shutdown(void)
{
	while (!g_MeshHooks_csgo.empty())
	{
		delete g_MeshHooks_csgo.top();
		g_MeshHooks_csgo.pop();
	}

	while (!g_CallQueueHooks_csgo.empty())
	{
		delete g_CallQueueHooks_csgo.top();
		g_CallQueueHooks_csgo.pop();
	}

	for (std::map<SOURCESDK::IMatRenderContext_csgo *, CMatRenderContextHook *>::iterator it = CMatRenderContextHook::m_Map.begin(); it != CMatRenderContextHook::m_Map.end(); ++it)
	{
		delete it->second;
	}
}
