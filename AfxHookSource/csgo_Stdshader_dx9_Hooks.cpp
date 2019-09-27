#include "stdafx.h"

#if AFX_SHADERS_CSGO

// Please note we handle the following problems and issues here:
//
// - Calls of touring_csgo_*_CShader_OnDrawElements can happen on different threads
// - The CBasePerMaterialContextData used in touring_csgo_VertexLitGeneric_CShader_OnDrawElements
//   will not last till the final Set*ShaderIndex calls, because it get's
//   deleted between SHADOW_STATE and DYNAMIC SHADE.
//
// The following issues are not handled here:
// - Calls to g_AfxStreams.OnSet*Shader can happen from different
//   threads, however we ensure that it's only one thread at a time.
//   When the streams system is live, then the calls should happen
//   from the current View_Render thread only, because we are in
//   mat_queue_mode 0 then, however, it's up to the g_AfxStreams
//   class to check and ensure that.

#include "csgo_Stdshader_dx9_Hooks.h"

#include "SourceInterfaces.h"
#include "addresses.h"
#include <shared/AfxDetours.h>
#include "asmClassTools.h"
#include "AfxStreams.h"
#include <shared/StringTools.h>

#include <string>
#include <map>

typedef void * CAfxStdShaderHookKey;

class CAfxStdShaderHook
{
public:
	CAfxStdShaderHook()
	: m_Lock(0)
	{
	}

	//
	// OnStatic_*
	//

	void OnStatic_EnableDepthWrites(CAfxStdShaderHookKey key, bool bEnable)
	{
		Lock();

		CAfx_csgo_ShaderState & state = FindOrCreate(key);

		state.Static.EnableDepthWrites.bEnable = bEnable;

		Unlock();
	}

	void OnStatic_EnableBlending(CAfxStdShaderHookKey key, bool bEnable)
	{
		Lock();

		CAfx_csgo_ShaderState & state = FindOrCreate(key);

		state.Static.EnableBlending.bEnable = bEnable;

		Unlock();
	}

	void OnStatic_BlendFunc(CAfxStdShaderHookKey key, SOURCESDK::ShaderBlendFactor_t_csgo srcFactor, SOURCESDK::ShaderBlendFactor_t_csgo dstFactor)
	{
		Lock();

		CAfx_csgo_ShaderState & state = FindOrCreate(key);

		state.Static.BlendFunc.srcFactor = srcFactor;
		state.Static.BlendFunc.dstFactor = dstFactor;

		Unlock();
	}

	void OnStatic_SetVertexShader(CAfxStdShaderHookKey key, const char* pFileName, int nStaticVshIndex )
	{
		Lock();

		//Tier0_Msg("OnStatic_SetVertexShader %s %i\n", pFileName, nStaticVshIndex);

		CAfx_csgo_ShaderState & state = FindOrCreate(key);

		state.Static.SetVertexShader.pFileName.assign(pFileName);
		state.Static.SetVertexShader.nStaticVshIndex = nStaticVshIndex;

		Unlock();
	}

	void OnStatic_SetPixelShader(CAfxStdShaderHookKey key, const char* pFileName, int nStaticPshIndex)
	{
		Lock();

		//Tier0_Msg("OnStatic_SetPixelShader %s %i\n", pFileName, nStaticPshIndex);

		CAfx_csgo_ShaderState & state = FindOrCreate(key);

		state.Static.SetPixelShader.pFileName.assign(pFileName);
		state.Static.SetPixelShader.nStaticPshIndex = nStaticPshIndex;

		Unlock();
	}

	//
	// OnDynamic_*
	//
	// It can happen that the case is hit, that there is no mapping
	// in the map for the material:
	// The following materials go through here: Pretty much every
	// material on LevelShutDown is going throug here for no apparent
	// reason.
	// Materials that last longer than the level will go through here
	// too, because we kick them out of the list
	// on LevelShutDown too, but we are not interested in those anyway.
	//

	void OnDynamic_SetVertexShaderIndex(CAfxStdShaderHookKey key, int vshIndex)
	{
		Lock();
		
		if(CAfx_csgo_ShaderState * pState = Find(key))
		{
			CAfx_csgo_ShaderState & state = *pState;

			state.Dynamic.SetVertexShaderIndex.vshIndex = vshIndex;

			g_AfxStreams.OnSetVertexShader(state);
		}

		Unlock();
	}

	void OnDynamic_SetPixelShaderIndex(CAfxStdShaderHookKey key, int pshIndex)
	{
		Lock();
		
		if(CAfx_csgo_ShaderState * pState = Find(key))
		{
			CAfx_csgo_ShaderState & state = *pState;

			state.Dynamic.SetPixelShaderIndex.pshIndex = pshIndex;

			g_AfxStreams.OnSetPixelShader(state);
		}

		Unlock();
	}


	void OnLevelShutDown()
	{
		Lock();

		//for(std::map<CAfxStdShaderHookKey, CAfx_csgo_ShaderState>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
		//{
		//	it->first->DecrementReferenceCount();
		//}
		m_Map.clear();

		Unlock();
	}

private:
	LONG m_Lock;
	std::map<CAfxStdShaderHookKey, CAfx_csgo_ShaderState> m_Map;

	void Lock()
	{
		while(0 != InterlockedExchange(&m_Lock, 1))
		{
		}
	}

	void Unlock()
	{
		InterlockedExchange(&m_Lock, 0);
	}

	CAfx_csgo_ShaderState & FindOrCreate(CAfxStdShaderHookKey key)
	{
		std::map<CAfxStdShaderHookKey, CAfx_csgo_ShaderState>::iterator it = m_Map.find(key);

		if(it != m_Map.end())
		{
			return it->second;
		}

		//key->IncrementReferenceCount();
		return m_Map[key];
	}

	CAfx_csgo_ShaderState * Find(CAfxStdShaderHookKey key)
	{
		std::map<CAfxStdShaderHookKey, CAfx_csgo_ShaderState>::iterator it = m_Map.find(key);

		if(it != m_Map.end())
		{
			return &(it->second);
		}

		return 0;
	}

} g_AfxStdShaderHook;

class CAfxBasePerMaterialContextDataPiggyBack_csgo
: public SOURCESDK::CBasePerMaterialContextData_csgo
, public SOURCESDK::IShaderShadow_csgo
, public SOURCESDK::IShaderDynamicAPI_csgo
{
public:
	CAfxBasePerMaterialContextDataPiggyBack_csgo( void )
	: m_PiggyBack(0)
	, m_Key(0)
	, m_ParentShaderShadow(0)
	, m_ParentShaderDynamicAPI(0)
	{
		m_bMaterialVarsChanged = true;
		m_nVarChangeID = 0xffffffff;
	}

	// virtual destructor so that derived classes can have their own data to be cleaned up on
	// delete of material
	virtual ~CAfxBasePerMaterialContextDataPiggyBack_csgo( void )
	{
		delete m_PiggyBack;
	}

	CBasePerMaterialContextData_csgo * PreUpdatePiggy()
	{
		if(m_PiggyBack)
		{
			m_PiggyBack->m_nVarChangeID = m_nVarChangeID;
			m_PiggyBack->m_bMaterialVarsChanged = m_bMaterialVarsChanged;
		}

		return m_PiggyBack;
	}

	void PostUpdatePiggy(CBasePerMaterialContextData_csgo * piggy)
	{
		m_PiggyBack = piggy;

		if(m_PiggyBack)
		{
			m_nVarChangeID = m_PiggyBack->m_nVarChangeID;
			m_bMaterialVarsChanged = m_PiggyBack->m_bMaterialVarsChanged;
		}
	}

	void SetParentApis(	IShaderShadow_csgo * parentShaderShadow, IShaderDynamicAPI_csgo * parentShaderDynamicAPI)
	{
		m_ParentShaderShadow = parentShaderShadow;
		m_ParentShaderDynamicAPI = parentShaderDynamicAPI;
	}

	void SetKey(CAfxStdShaderHookKey key)
	{
		m_Key = key;
	}

#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'ebp' modified by inline assembly code

	//
	// IShaderShadow_csgo:

	virtual void IShaderShadow_csgo::_UNKOWN_000(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 0) }

	virtual void IShaderShadow_csgo::_UNKOWN_001(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 1) }

	virtual void IShaderShadow_csgo::EnableDepthWrites( bool bEnable )
	{
		//JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 2)

		g_AfxStdShaderHook.OnStatic_EnableDepthWrites(m_Key, bEnable);

		m_ParentShaderShadow->EnableDepthWrites(bEnable);
	}

	virtual void IShaderShadow_csgo::_UNKOWN_003(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 3) }

	virtual void IShaderShadow_csgo::_UNKOWN_004(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 4) }

	virtual void IShaderShadow_csgo::EnableColorWrites( bool bEnable )
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 5) }

	virtual void IShaderShadow_csgo::EnableAlphaWrites( bool bEnable )
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 6) }

	virtual void IShaderShadow_csgo::EnableBlending( bool bEnable )
	{
		//JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 7)
	
		g_AfxStdShaderHook.OnStatic_EnableBlending(m_Key, bEnable);

		m_ParentShaderShadow->EnableBlending(bEnable);
	}

	virtual void IShaderShadow_csgo::_UNKOWN_008(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 8) }

	virtual void IShaderShadow_csgo::BlendFunc(SOURCESDK::ShaderBlendFactor_t_csgo srcFactor, SOURCESDK::ShaderBlendFactor_t_csgo dstFactor )
	{
		//JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 9)
	
		g_AfxStdShaderHook.OnStatic_BlendFunc(m_Key, srcFactor, dstFactor);

		m_ParentShaderShadow->BlendFunc(srcFactor, dstFactor);
	}

	virtual void IShaderShadow_csgo::_UNKOWN_010(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 10) }

	virtual void IShaderShadow_csgo::_UNKOWN_011(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 11) }

	virtual void IShaderShadow_csgo::_UNKOWN_012(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 12) }

	virtual void IShaderShadow_csgo::_UNKOWN_013(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 13) }

	virtual void IShaderShadow_csgo::_UNKOWN_014(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 14) }

	virtual void IShaderShadow_csgo::_UNKOWN_015(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 15) }

	virtual void IShaderShadow_csgo::_UNKOWN_016(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 16) }

	virtual void IShaderShadow_csgo::SetVertexShader( const char* pFileName, int nStaticVshIndex )
	{
		//JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 17)

		g_AfxStdShaderHook.OnStatic_SetVertexShader(m_Key, pFileName, nStaticVshIndex);

		m_ParentShaderShadow->SetVertexShader(pFileName, nStaticVshIndex);
	}

	virtual	void IShaderShadow_csgo::SetPixelShader( const char* pFileName, int nStaticPshIndex = 0 )
	{
		// JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 18)

		g_AfxStdShaderHook.OnStatic_SetPixelShader(m_Key, pFileName, nStaticPshIndex);

		m_ParentShaderShadow->SetPixelShader(pFileName, nStaticPshIndex);
	}

	virtual void IShaderShadow_csgo::EnableSRGBWrite( bool bEnable )
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 19) }

	virtual void IShaderShadow_csgo::_UNKOWN_020(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 20) }

	virtual void IShaderShadow_csgo::_UNKOWN_021(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 21) }

	virtual void IShaderShadow_csgo::_UNKOWN_022(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 22) }

	virtual void IShaderShadow_csgo::_UNKOWN_023(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 23) }

	virtual void IShaderShadow_csgo::_UNKOWN_024(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 24) }

	virtual void IShaderShadow_csgo::_UNKOWN_025(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 25) }

	virtual void IShaderShadow_csgo::_UNKOWN_026(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 26) }

	virtual void IShaderShadow_csgo::_UNKOWN_027(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 27) }

	virtual void IShaderShadow_csgo::_UNKOWN_028(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 28) }

	virtual void IShaderShadow_csgo::_UNKOWN_029(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 29) }

	virtual void IShaderShadow_csgo::_UNKOWN_030(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 30) }

	virtual void IShaderShadow_csgo::_UNKOWN_031(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 31) }

	virtual void IShaderShadow_csgo::_UNKOWN_032(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 32) }

	virtual void IShaderShadow_csgo::_UNKOWN_033(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderShadow, 0xC, 0x0, 33) }

	//
	// IShaderDynamicAPI_csgo:

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_000(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 0) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_001(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 1) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_002(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 2) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_003(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 3) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_004(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 4) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_005(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 5) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_006(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 6) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_007(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 7) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_008(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 8) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_009(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 9) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_010(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 10) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_011(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 11) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_012(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 12) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_013(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 13) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_014(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 14) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_015(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 15) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_016(void) 
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 16) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_017(void) 
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 17) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_018(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 18) }

	virtual void IShaderDynamicAPI_csgo::SetVertexShaderIndex( int vshIndex = -1 )
	{
		// JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 19)
	
		DoSetVertexShaderIndex(vshIndex);

		m_ParentShaderDynamicAPI->SetVertexShaderIndex(vshIndex);
	}

	virtual void IShaderDynamicAPI_csgo::SetPixelShaderIndex( int pshIndex = 0 )
	{
		// JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 20)
		//Tier0_Msg("CAfxShaderDynamicAPI::SetPixelShaderIndex(%i)\n", pshIndex);

		DoSetPixelShaderIndex(pshIndex);

		m_ParentShaderDynamicAPI->SetPixelShaderIndex(pshIndex);
	}

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_021(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 21) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_022(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 22) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_023(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 23) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_024(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 24) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_025(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 25) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_026(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 26) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_027(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 27) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_028(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 28) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_029(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 29) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_030(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 30) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_031(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 31) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_032(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 32) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_033(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 33) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_034(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 34) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_035(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 35) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_036(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 36) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_037(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 37) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_038(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 38) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_039(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 39) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_040(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 40) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_041(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 41) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_042(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 42) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_043(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 43) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_044(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 44) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_045(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 45) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_046(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 46) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_047(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 47) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_048(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 48) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_049(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 49) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_050(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 50) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_051(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 51) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_052(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 52) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_053(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 53) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_054(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 54) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_055(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 55) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_056(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 56) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_057(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 57) }

	virtual void IShaderDynamicAPI_csgo::ExecuteCommandBuffer(SOURCESDK::uint8 *pCmdBuffer )
	{
		// JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 58)

		DoExecuteCommandBuffer(pCmdBuffer);

		m_ParentShaderDynamicAPI->ExecuteCommandBuffer(pCmdBuffer);
	}

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_059(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 59) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_060(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 60) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_061(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 61) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_062(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 62) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_063(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 63) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_064(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 64) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_065(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 65) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_066(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 66) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_067(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 67) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_068(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 68) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_069(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 69) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_070(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 70) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_071(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 71) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_072(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 72) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_073(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 73) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_074(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 74) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_075(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 75) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_076(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 76) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_077(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 77) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_078(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 78) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_079(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 79) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_080(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 80) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_081(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 81) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_082(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 82) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_083(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 83) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_084(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 84) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_085(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 85) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_086(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 86) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_087(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 87) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_088(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 88) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_089(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 89) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_090(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 90) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_091(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 91) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_092(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 92) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_093(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 93) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_094(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 94) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_095(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 95) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_096(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 96) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_097(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 97) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_098(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 98) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_099(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 99) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_100(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 100) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_101(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 101) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_102(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 102) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_103(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 103) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_104(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 104) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_105(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 105) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_106(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 106) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_107(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 107) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_108(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 108) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_109(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 109) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_110(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 110) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_111(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 111) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_112(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 112) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_113(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 113) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_114(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 114) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_115(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 115) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_116(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 116) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_117(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 117) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_118(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 118) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_119(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 119) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_120(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 120) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_121(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 121) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_122(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 122) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_123(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 123) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_124(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 124) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_125(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 125) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_126(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 126) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_127(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 127) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_128(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 128) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_129(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 129) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_130(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 130) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_131(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 131) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_132(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 132) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_133(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 133) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_134(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 134) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_135(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 135) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_136(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 136) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_137(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 137) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_138(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 138) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_139(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 139) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_140(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 140) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_141(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 141) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_142(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 142) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_143(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 143) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_144(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 144) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_145(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 145) }

	virtual void IShaderDynamicAPI_csgo::_UNKOWN_146(void)
	{ JMP_CLASSMEMBERIFACE_OFSEX_FN(CAfxBasePerMaterialContextDataPiggyBack_csgo, m_ParentShaderDynamicAPI, 0x10, 0x0, 146) }

#pragma warning(pop)

private:
	SOURCESDK::CBasePerMaterialContextData_csgo * m_PiggyBack;
	CAfxStdShaderHookKey m_Key;
	SOURCESDK::IShaderShadow_csgo * m_ParentShaderShadow;
	SOURCESDK::IShaderDynamicAPI_csgo * m_ParentShaderDynamicAPI;
	CAfx_csgo_ShaderState m_State;

	void DoSetVertexShaderIndex(int vshIndex = -1 )
	{
		g_AfxStdShaderHook.OnDynamic_SetVertexShaderIndex(m_Key, vshIndex);
	}

	void DoSetPixelShaderIndex(int pshIndex = 0 )
	{
		g_AfxStdShaderHook.OnDynamic_SetPixelShaderIndex(m_Key, pshIndex);
	}

	void DoExecuteCommandBuffer(SOURCESDK::uint8 *pCmdBuffer)
	{
		if(!pCmdBuffer)
			// In that case we can't do anything useful anyway.
			return;

		// implemented according to Valve's commandbuilder.h:

		while(true)
		{
			switch(*(int *)pCmdBuffer)
			{
			case SOURCESDK::CBCMD_END:
				{
					pCmdBuffer += sizeof(int);
				}
				return;
			case SOURCESDK::CBCMD_JUMP:
				{
					pCmdBuffer += sizeof(int);
					SOURCESDK::uint8 * ptr = *(SOURCESDK::uint8 **)pCmdBuffer;
					pCmdBuffer += sizeof(ptr);
					DoExecuteCommandBuffer(ptr);
				}
				return;
			case SOURCESDK::CBCMD_JSR:
				{
					pCmdBuffer += sizeof(int);
					SOURCESDK::uint8 * ptr = *(SOURCESDK::uint8 **)pCmdBuffer;
					pCmdBuffer += sizeof(ptr);
					DoExecuteCommandBuffer(ptr);
				}
				break;
			case SOURCESDK::CBCMD_SET_PIXEL_SHADER_FLOAT_CONST:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					int nregs = *(int *)pCmdBuffer;
					pCmdBuffer += sizeof(int);
					pCmdBuffer += nregs * 4 * sizeof(float);
				}
				break;
			case SOURCESDK::CBCMD_SET_VERTEX_SHADER_FLOAT_CONST:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					int nregs = *(int *)pCmdBuffer;
					pCmdBuffer += sizeof(int);
					pCmdBuffer += nregs * 4 * sizeof(float);
				}
				break;
			case SOURCESDK::CBCMD_SETPIXELSHADERFOGPARAMS:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_STORE_EYE_POS_IN_PSCONST:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(float);
				}
				break;
			case SOURCESDK::CBCMD_SET_DEPTH_FEATHERING_CONST:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(float);
				}
				break;
			case SOURCESDK::CBCMD_BIND_STANDARD_TEXTURE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_PSHINDEX:
				{
					pCmdBuffer += sizeof(int);
					int nIndex = *(int *)pCmdBuffer;
					DoSetPixelShaderIndex(nIndex);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_VSHINDEX:
				{
					pCmdBuffer += sizeof(int);
					int nIndex = *(int *)pCmdBuffer;
					DoSetVertexShaderIndex(nIndex);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_VERTEX_SHADER_FLASHLIGHT_STATE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_PIXEL_SHADER_FLASHLIGHT_STATE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_PIXEL_SHADER_UBERLIGHT_STATE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			case SOURCESDK::CBCMD_SET_VERTEX_SHADER_NEARZFARZ_STATE:
				{
					pCmdBuffer += sizeof(int);
					pCmdBuffer += sizeof(int);
				}
				break;
			default:
				Tier0_Warning("AFXERROR: CStdshader_dx9_OnDrawElements_Hook::DoExecuteCommandBuffer: Unknown command #%i.\n", *(int *)pCmdBuffer);
				return;
			}
		}
	}
};

typedef void (__stdcall *csgo_stdshader_dx9_CBaseShader_DrawElements_t)(
	DWORD *this_ptr,
	SOURCESDK::IMaterialVar_csgo **ppParams, int nModulationFlags,
	SOURCESDK::IShaderShadow_csgo* pShaderShadow, SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	SOURCESDK::VertexCompressionType_t_csgo vertexCompression,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContextDataPtr, SOURCESDK::CBasePerInstanceContextData_csgo** pInstanceDataPtr);

typedef void (__stdcall *csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t)(
	DWORD *this_ptr,
	unsigned __int32 * unkDataPtr1,
	unsigned __int32 * unkDataPtr2,
	void * unkClass1, // related to VertexBuffer
	SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	unsigned __int32 unkData2,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContext,
	unsigned __int32 unkData3
	);

csgo_stdshader_dx9_CBaseShader_DrawElements_t detoured_csgo_SplineRope_CShader_DrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_SplineRope_CShader_DrawElements(
	DWORD *this_ptr,
	SOURCESDK::IMaterialVar_csgo **ppParams, int nModulationFlags,
	SOURCESDK::IShaderShadow_csgo* pShaderShadow, SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	SOURCESDK::VertexCompressionType_t_csgo vertexCompression,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContextDataPtr, SOURCESDK::CBasePerInstanceContextData_csgo** pInstanceDataPtr)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_SplineRope_CShader_OnDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContextDataPtr );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(pShaderShadow, pShaderAPI);
	pContextData->SetKey(pContextDataPtr);

	detoured_csgo_SplineRope_CShader_DrawElements(
		this_ptr,
		ppParams,
		nModulationFlags,
		pShaderShadow ? pContextData : 0,
		pShaderAPI ? pContextData : 0,
		vertexCompression,
		&payLoad,
		pInstanceDataPtr
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContextDataPtr = pContextData;
}

csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t detoured_csgo_SplineRope_CShader_SomewhatDrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_SplineRope_CShader_SomewhatDrawElements(
	DWORD *this_ptr,
	unsigned __int32 * unkDataPtr1,
	unsigned __int32 * unkDataPtr2,
	void * unkClass1, // related to VertexBuffer
	SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	unsigned __int32 unkData2,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContext,
	unsigned __int32 unkData3
	)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_SplineRope_CShader_OnSomewhatDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	if(!pContext)
	{
		// Apparently this can happen i.e. in the SplineRope shaders, not much we can do for now,
		// better safe and sorry than violated.

		detoured_csgo_SplineRope_CShader_SomewhatDrawElements(
			this_ptr,
			unkDataPtr1,
			unkDataPtr2,
			unkClass1,
			pShaderAPI,
			unkData2,
			pContext,
			unkData3);
		return;
	}

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContext );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(0, pShaderAPI);
	pContextData->SetKey(pContext);

	detoured_csgo_SplineRope_CShader_SomewhatDrawElements(
		this_ptr,
		unkDataPtr1,
		unkDataPtr2,
		unkClass1,
		pShaderAPI ? pContextData : 0,
		unkData2,
		&payLoad,
		unkData3
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContext = pContextData;
}

csgo_stdshader_dx9_CBaseShader_DrawElements_t detoured_csgo_Spritecard_CShader_DrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_Spritecard_CShader_DrawElements(
	DWORD *this_ptr,
	SOURCESDK::IMaterialVar_csgo **ppParams, int nModulationFlags,
	SOURCESDK::IShaderShadow_csgo* pShaderShadow, SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	SOURCESDK::VertexCompressionType_t_csgo vertexCompression,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContextDataPtr, SOURCESDK::CBasePerInstanceContextData_csgo** pInstanceDataPtr)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContextDataPtr );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(pShaderShadow, pShaderAPI);
	pContextData->SetKey(pContextDataPtr);

	detoured_csgo_Spritecard_CShader_DrawElements(
		this_ptr,
		ppParams,
		nModulationFlags,
		pShaderShadow ? pContextData : 0,
		pShaderAPI ? pContextData : 0,
		vertexCompression,
		&payLoad,
		pInstanceDataPtr
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContextDataPtr = pContextData;
}

csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t detoured_csgo_Spritecard_CShader_SomewhatDrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_Spritecard_CShader_SomewhatDrawElements(
	DWORD *this_ptr,
	unsigned __int32 * unkDataPtr1,
	unsigned __int32 * unkDataPtr2,
	void * unkClass1, // related to VertexBuffer
	SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	unsigned __int32 unkData2,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContext,
	unsigned __int32 unkData3
	)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnSomewhatDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	if(!pContext)
	{
		// Apparently this can happen i.e. in the SplineRope shaders, not much we can do for now,
		// better safe and sorry than violated.

		detoured_csgo_Spritecard_CShader_SomewhatDrawElements(
			this_ptr,
			unkDataPtr1,
			unkDataPtr2,
			unkClass1,
			pShaderAPI,
			unkData2,
			pContext,
			unkData3);
		return;
	}

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContext );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(0, pShaderAPI);
	pContextData->SetKey(pContext);

	detoured_csgo_Spritecard_CShader_SomewhatDrawElements(
		this_ptr,
		unkDataPtr1,
		unkDataPtr2,
		unkClass1,
		pShaderAPI ? pContextData : 0,
		unkData2,
		&payLoad,
		unkData3
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContext = pContextData;
}

csgo_stdshader_dx9_CBaseShader_DrawElements_t detoured_csgo_VertexLitGeneric_CShader_DrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_VertexLitGeneric_CShader_DrawElements(
	DWORD *this_ptr,
	SOURCESDK::IMaterialVar_csgo **ppParams, int nModulationFlags,
	SOURCESDK::IShaderShadow_csgo* pShaderShadow, SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	SOURCESDK::VertexCompressionType_t_csgo vertexCompression,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContextDataPtr, SOURCESDK::CBasePerInstanceContextData_csgo** pInstanceDataPtr)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContextDataPtr );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(pShaderShadow, pShaderAPI);
	pContextData->SetKey(pContextDataPtr);

	detoured_csgo_VertexLitGeneric_CShader_DrawElements(
		this_ptr,
		ppParams,
		nModulationFlags,
		pShaderShadow ? pContextData : 0,
		pShaderAPI ? pContextData : 0,
		vertexCompression,
		&payLoad,
		pInstanceDataPtr
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContextDataPtr = pContextData;
}

csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t detoured_csgo_VertexLitGeneric_CShader_SomewhatDrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_VertexLitGeneric_CShader_SomewhatDrawElements(
	DWORD *this_ptr,
	unsigned __int32 * unkDataPtr1,
	unsigned __int32 * unkDataPtr2,
	void * unkClass1, // related to VertexBuffer
	SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	unsigned __int32 unkData2,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContext,
	unsigned __int32 unkData3
	)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnSomewhatDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	if(!pContext)
	{
		// Apparently this can happen i.e. in the SplineRope shaders, not much we can do for now,
		// better safe and sorry than violated.

		detoured_csgo_VertexLitGeneric_CShader_SomewhatDrawElements(
			this_ptr,
			unkDataPtr1,
			unkDataPtr2,
			unkClass1,
			pShaderAPI,
			unkData2,
			pContext,
			unkData3);
		return;
	}

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContext );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(0, pShaderAPI);
	pContextData->SetKey(pContext);

	detoured_csgo_VertexLitGeneric_CShader_SomewhatDrawElements(
		this_ptr,
		unkDataPtr1,
		unkDataPtr2,
		unkClass1,
		pShaderAPI ? pContextData : 0,
		unkData2,
		&payLoad,
		unkData3
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContext = pContextData;
}

csgo_stdshader_dx9_CBaseShader_DrawElements_t detoured_csgo_UnlitGeneric_CShader_DrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_UnlitGeneric_CShader_DrawElements(
	DWORD *this_ptr,
	SOURCESDK::IMaterialVar_csgo **ppParams, int nModulationFlags,
	SOURCESDK::IShaderShadow_csgo* pShaderShadow, SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	SOURCESDK::VertexCompressionType_t_csgo vertexCompression,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContextDataPtr, SOURCESDK::CBasePerInstanceContextData_csgo** pInstanceDataPtr)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContextDataPtr );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(pShaderShadow, pShaderAPI);
	pContextData->SetKey(pContextDataPtr);

	detoured_csgo_UnlitGeneric_CShader_DrawElements(
		this_ptr,
		ppParams,
		nModulationFlags,
		pShaderShadow ? pContextData : 0,
		pShaderAPI ? pContextData : 0,
		vertexCompression,
		&payLoad,
		pInstanceDataPtr
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContextDataPtr = pContextData;
}

csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t detoured_csgo_UnlitGeneric_CShader_SomewhatDrawElements;

// Keep in mind this function can run on multiple threads simultaneously!
//
void __stdcall touring_csgo_UnlitGeneric_CShader_SomewhatDrawElements(
	DWORD *this_ptr,
	unsigned __int32 * unkDataPtr1,
	unsigned __int32 * unkDataPtr2,
	void * unkClass1, // related to VertexBuffer
	SOURCESDK::IShaderDynamicAPI_csgo* pShaderAPI,
	unsigned __int32 unkData2,
	SOURCESDK::CBasePerMaterialContextData_csgo **pContext,
	unsigned __int32 unkData3
	)
{
	//if(pShaderShadow) Tier0_Msg("touring_csgo_Spritecard_CShader_OnSomewhatDrawElements(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x (* = 0x%08x)):%s (0x%08x) threadId=%i\n",this_ptr, ppParams, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr, *pContextDataPtr, ppParams[0]->GetOwningMaterial()->GetName(), ppParams[0]->GetOwningMaterial(), GetCurrentThreadId());

	if(!pContext)
	{
		// Apparently this can happen i.e. in the SplineRope shaders, not much we can do for now,
		// better safe and sorry than violated.

		detoured_csgo_UnlitGeneric_CShader_SomewhatDrawElements(
			this_ptr,
			unkDataPtr1,
			unkDataPtr2,
			unkClass1,
			pShaderAPI,
			unkData2,
			pContext,
			unkData3);
		return;
	}

	CAfxBasePerMaterialContextDataPiggyBack_csgo * pContextData = reinterpret_cast< CAfxBasePerMaterialContextDataPiggyBack_csgo *> ( *pContext );

	if(!pContextData)
	{
		pContextData = new CAfxBasePerMaterialContextDataPiggyBack_csgo();
	}

	SOURCESDK::CBasePerMaterialContextData_csgo * payLoad = pContextData->PreUpdatePiggy();

	pContextData->SetParentApis(0, pShaderAPI);
	pContextData->SetKey(pContext);

	detoured_csgo_UnlitGeneric_CShader_SomewhatDrawElements(
		this_ptr,
		unkDataPtr1,
		unkDataPtr2,
		unkClass1,
		pShaderAPI ? pContextData : 0,
		unkData2,
		&payLoad,
		unkData3
	);

	pContextData->PostUpdatePiggy(payLoad);

	*pContext = pContextData;
}

bool csgo_Stdshader_dx9_Hooks_Init(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = true;

	// Currently not supported, since we need to update a whole lot of shit due to latest CS:GO update:
	return false;

	if(AFXADDR_GET(csgo_SplineRope_CShader_vtable))
	{
		detoured_csgo_SplineRope_CShader_DrawElements = (csgo_stdshader_dx9_CBaseShader_DrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_SplineRope_CShader_vtable)+4*6), touring_csgo_SplineRope_CShader_DrawElements);
		detoured_csgo_SplineRope_CShader_SomewhatDrawElements = (csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_SplineRope_CShader_vtable)+4*7), touring_csgo_SplineRope_CShader_SomewhatDrawElements);
	}
	else
		firstResult = false;

	if(AFXADDR_GET(csgo_Spritecard_CShader_vtable))
	{
		detoured_csgo_Spritecard_CShader_DrawElements = (csgo_stdshader_dx9_CBaseShader_DrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_Spritecard_CShader_vtable)+4*6), touring_csgo_Spritecard_CShader_DrawElements);
		detoured_csgo_Spritecard_CShader_SomewhatDrawElements = (csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_Spritecard_CShader_vtable)+4*7), touring_csgo_Spritecard_CShader_SomewhatDrawElements);
	}
	else
		firstResult = false;

	if(AFXADDR_GET(csgo_VertexLitGeneric_CShader_vtable))
	{
		detoured_csgo_VertexLitGeneric_CShader_DrawElements = (csgo_stdshader_dx9_CBaseShader_DrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_VertexLitGeneric_CShader_vtable)+4*6), touring_csgo_VertexLitGeneric_CShader_DrawElements);
		detoured_csgo_VertexLitGeneric_CShader_SomewhatDrawElements = (csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_VertexLitGeneric_CShader_vtable)+4*7), touring_csgo_VertexLitGeneric_CShader_SomewhatDrawElements);
	}
	else
		firstResult = false;

	if(AFXADDR_GET(csgo_UnlitGeneric_CShader_vtable))
	{
		detoured_csgo_UnlitGeneric_CShader_DrawElements = (csgo_stdshader_dx9_CBaseShader_DrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_UnlitGeneric_CShader_vtable)+4*6), touring_csgo_UnlitGeneric_CShader_DrawElements);
		detoured_csgo_UnlitGeneric_CShader_SomewhatDrawElements = (csgo_stdshader_dx9_CBaseShader_SomewhatDrawElements_t)DetourIfacePtr((DWORD *)(AFXADDR_GET(csgo_UnlitGeneric_CShader_vtable)+4*7), touring_csgo_UnlitGeneric_CShader_SomewhatDrawElements);
	}
	else
		firstResult = false;


	return firstResult;
}

void csgo_Stdshader_dx9_Hooks_OnLevelShutdown(void)
{
	g_AfxStdShaderHook.OnLevelShutDown();
}

#endif