#include "stdafx.h"

// TODO:
// - Memory for Wrp* is never freed atm

#include <windows.h>

#include <shared/StringTools.h>

#include <shared/AfxDetours.h>

#include "AfxCommandLine.h"
#include "addresses.h"
#include "RenderView.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "WrpGlobals.h"
#include "d3d9Hooks.h"
#include "csgo_SndMixTimeScalePatch.h"
#include "csgo_CSkyBoxView.h"
#include "AfxHookSourceInput.h"
#include "AfxClasses.h"
#include "AfxStreams.h"
#include "hlaeFolder.h"
#include "CampathDrawer.h"
#include "asmClassTools.h"
#include "csgo_Stdshader_dx9_Hooks.h"
#include "AfxShaders.h"
#include "csgo_CViewRender.h"
#include "CommandSystem.h"
#include "ClientTools.h"
#include "csgo/ClientToolsCsgo.h"
#include "tf2/ClientToolsTf2.h"
#include "momentum/ClientToolsMom.h"
#include "css/ClientToolsCss.h"
#include "cssV34/ClientToolsCssV34.h"
#include "MatRenderContextHook.h"
//#include "csgo_IPrediction.h"
#include "csgo_MemAlloc.h"
#include "csgo_c_baseanimatingoverlay.h"
#include "csgo_CHudDeathNotice.h"
#include "MirvPgl.h"
#include "AfxInterop.h"
#include "csgo_Audio.h"
#include "mirv_voice.h"
#include "Gui.h"
#include <csgo/sdk_src/public/tier0/memalloc.h>
#include <csgo/sdk_src/public/tier1/convar.h>
#include <swarm/sdk_src/public/tier0/memalloc.h>
#include <swarm/sdk_src/public/tier1/convar.h>
#include <l4d2/sdk_src/public/tier0/memalloc.h>
#include <l4d2/sdk_src/public/tier1/convar.h>
#include <bm/sdk_src/public/tier0/memalloc.h>
#include <bm/sdk_src/public/tier1/convar.h>
#include <bm/sdk_src/public/cdll_int.h>
#include <csgo/Panorama.h>
//#include <csgo/hooks/studiorender.h>
#include <insurgency2/public/cdll_int.h>
#include "MirvTime.h"
#include "csgo_CRendering3dView.h"
//#include "csgo_CDemoFile.h"
#include "csgo_net_chan.h"
#include "csgo/hooks/engine/cmd.h"
#include "ReShadeAdvancedfx.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <shared/binutils.h>

#include <set>
#include <map>
#include <string>

WrpVEngineClient * g_VEngineClient = 0;
SOURCESDK::ICvar_003 * g_Cvar = 0;

SOURCESDK::CSGO::IMemAlloc *SOURCESDK::CSGO::g_pMemAlloc = 0;
SOURCESDK::CSGO::ICvar * SOURCESDK::CSGO::cvar = 0;
SOURCESDK::CSGO::ICvar * SOURCESDK::CSGO::g_pCVar = 0;

SOURCESDK::SWARM::IMemAlloc *SOURCESDK::SWARM::g_pMemAlloc = 0;
SOURCESDK::SWARM::ICvar * SOURCESDK::SWARM::cvar = 0;
SOURCESDK::SWARM::ICvar * SOURCESDK::SWARM::g_pCVar = 0;

SOURCESDK::L4D2::IMemAlloc *SOURCESDK::L4D2::g_pMemAlloc = 0;
SOURCESDK::L4D2::ICvar * SOURCESDK::L4D2::cvar = 0;
SOURCESDK::L4D2::ICvar * SOURCESDK::L4D2::g_pCVar = 0;

SOURCESDK::BM::IMemAlloc *SOURCESDK::BM::g_pMemAlloc = 0;
SOURCESDK::BM::ICvar * SOURCESDK::BM::cvar = 0;
SOURCESDK::BM::ICvar * SOURCESDK::BM::g_pCVar = 0;


void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookSource", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}

char const * g_Info_VClient = "NULL";
char const * g_Info_VEngineClient = "NULL";
char const * g_Info_VEngineCvar = "NULL";

void PrintInfo() {
	Tier0_Msg(
		"|" "\n"
		"| AfxHookSource (" __DATE__ " " __TIME__ ")" "\n"
		"| http://advancedfx.org/" "\n"
		"|" "\n"
	);

	Tier0_Msg("| VClient: %s\n", g_Info_VClient);
	Tier0_Msg("| VEngineClient: %s\n", g_Info_VEngineClient);
	Tier0_Msg("| VEngineCvar: %s\n", g_Info_VEngineCvar);
	Tier0_Msg("| GameDirectory: %s\n", g_VEngineClient ? g_VEngineClient->GetGameDirectory() : "n/a");

	Tier0_Msg("|" "\n");
}

SOURCESDK::IClientEngineTools_001 * g_Engine_ClientEngineTools;

class ClientEngineTools : public SOURCESDK::IClientEngineTools_001
{
public:
	// TODO: we should call the destructor of g_Engine_ClientEngineTools on destuction?

	virtual void LevelInitPreEntityAllTools()
	{
		g_CommandSystem.OnLevelInitPreEntityAllTools();

		g_Engine_ClientEngineTools->LevelInitPreEntityAllTools();
	}
	virtual void LevelInitPostEntityAllTools() { g_Engine_ClientEngineTools->LevelInitPostEntityAllTools(); }
	virtual void LevelShutdownPreEntityAllTools() { g_Engine_ClientEngineTools->LevelShutdownPreEntityAllTools(); }
	virtual void LevelShutdownPostEntityAllTools() { g_Engine_ClientEngineTools->LevelShutdownPostEntityAllTools(); }
	virtual void PreRenderAllTools()
	{
		//Tier0_Msg("ClientEngineTools::PreRenderAllTools\n");

		g_CommandSystem.Do_Commands();

		g_Engine_ClientEngineTools->PreRenderAllTools();
	}
	
	virtual void PostRenderAllTools()
	{
		// Warning: This can be called multiple times during a frame (i.e. for skybox view and normal world view)!

		//Tier0_Msg("ClientEngineTools::PostRenderAllTools\n");

		g_CampathDrawer.OnPostRenderAllTools();

		g_Engine_ClientEngineTools->PostRenderAllTools();
	}

	virtual void PostToolMessage(SOURCESDK::HTOOLHANDLE hEntity, SOURCESDK::KeyValues_something *msg )
	{
		if(CClientTools * instance = CClientTools::Instance()) instance->OnPostToolMessage(hEntity, msg);

		g_Engine_ClientEngineTools->PostToolMessage(hEntity, msg);

		if (g_SourceSdkVer == SourceSdkVer::SourceSdkVer_CSGO)
		{
			if(msg)			
			{
				char const* msgName = ((SOURCESDK::CSGO::KeyValues * )msg)->GetName();
				if (0 == strcmp(msgName, "created") && CClientToolsCsgo::Instance())
				{
					if (auto clientTools = CClientToolsCsgo::Instance()->GetClientToolsInterface())
					{
						SOURCESDK::CSGO::EntitySearchResult ent = clientTools->GetEntity(reinterpret_cast<SOURCESDK::CSGO::HTOOLHANDLE>(hEntity));
						g_AfxStreams.OnClientEntityCreated(reinterpret_cast<SOURCESDK::C_BaseEntity_csgo*>(ent));
					}
				}
				else if (0 == strcmp(msgName, "deleted") && CClientToolsCsgo::Instance())
				{
					if (auto clientTools = CClientToolsCsgo::Instance()->GetClientToolsInterface())
					{
						SOURCESDK::CSGO::EntitySearchResult ent = clientTools->GetEntity(reinterpret_cast<SOURCESDK::CSGO::HTOOLHANDLE>(hEntity));
						g_AfxStreams.OnClientEntityDeleted(reinterpret_cast<SOURCESDK::C_BaseEntity_csgo*>(ent));
					}
				}
			}
		}
	}
	virtual void AdjustEngineViewport( int& x, int& y, int& width, int& height )
	{
		g_Engine_ClientEngineTools->AdjustEngineViewport(x, y, width, height);

		g_Hook_VClient_RenderView.OnAdjustEngineViewport(x, y, width, height);
	}
	
	virtual bool SetupEngineView(SOURCESDK::Vector &origin, SOURCESDK::QAngle &angles, float &fov )
	{
		bool bRet = false;

		//Tier0_Msg("ClientEngineTools::SetupEngineView\n");
		if (g_Engine_ClientEngineTools->SetupEngineView(origin, angles, fov)) bRet = true;

		g_Hook_VClient_RenderView.OnViewOverride(
			origin.x, origin.y, origin.z,
			angles.x, angles.y, angles.z,
			fov
		);

		if (CClientTools::Instance()) CClientTools::Instance()->OnAfterSetupEngineView();

		return bRet;
	}
	
	virtual bool SetupAudioState(SOURCESDK::AudioState_t &audioState ) { return g_Engine_ClientEngineTools->SetupAudioState(audioState); }
	
	virtual void VGui_PreRenderAllTools( int paintMode )
	{
		//Tier0_Msg("ClientEngineTools::VGui_PreRenderAllTools\n");
		g_Engine_ClientEngineTools->VGui_PreRenderAllTools(paintMode);
	}
	
	virtual void VGui_PostRenderAllTools( int paintMode )
	{
		//Tier0_Msg("ClientEngineTools::VGui_PostRenderAllTools\n");
		g_Engine_ClientEngineTools->VGui_PostRenderAllTools(paintMode);
	}

	virtual bool IsThirdPersonCamera() { return g_Engine_ClientEngineTools->IsThirdPersonCamera(); }

	virtual bool InToolMode()
	{
		if (CClientTools * instance = CClientTools::Instance())
		{
			return true;
		}

		return g_Engine_ClientEngineTools->InToolMode();
	}

} g_ClientEngineTools;

SOURCESDK::CreateInterfaceFn g_AppSystemFactory = 0;

SOURCESDK::IMaterialSystem_csgo * g_MaterialSystem_csgo = 0;

SOURCESDK::IFileSystem_csgo * g_FileSystem_csgo = 0;

SOURCESDK::CSGO::vgui::IPanel * g_pVGuiPanel_csgo = 0;
SOURCESDK::CSGO::vgui::ISurface *g_pVGuiSurface_csgo = 0;

SOURCESDK::IVRenderView_csgo * g_pVRenderView_csgo = 0;

SOURCESDK::CSGO::IEngineTrace * g_pClientEngineTrace = 0;

SOURCESDK::CSGO::CGameUIFuncs * g_pGameUIFuncs = nullptr;
SOURCESDK::CSGO::panorama::CPanoramaUIEngine * g_pPanoramaUIEngine = nullptr;
SOURCESDK::CSGO::CPanoramaUIClient * g_pPanoramaUIClient = nullptr;

SOURCESDK::CSGO::IStudioRender * g_pStudioRender = nullptr;

SOURCESDK::CSGO::IVModelInfoClient* g_pModelInfo = nullptr;	

void MySetup(SOURCESDK::CreateInterfaceFn appSystemFactory, WrpGlobals *pGlobals)
{
	static bool bFirstRun = true;

	if(bFirstRun)
	{
		bFirstRun = false;

		void *iface , *iface2;

		g_AppSystemFactory = appSystemFactory;

		// VEngineClient:

		switch (g_SourceSdkVer)
		{
		case SourceSdkVer_CSGO:
			if (iface = appSystemFactory(VENGINE_CLIENT_INTERFACE_VERSION_014_CSGO, NULL))
			{
				g_Info_VEngineClient = VENGINE_CLIENT_INTERFACE_VERSION_014_CSGO " (CS:GO)";
				g_VEngineClient = new WrpVEngineClient_014_csgo((SOURCESDK::IVEngineClient_014_csgo*)iface);
			}
			break;
		case SourceSdkVer_BM:
			if (iface = appSystemFactory(SOURCESDK_BM_VENGINE_CLIENT_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineClient = SOURCESDK_BM_VENGINE_CLIENT_INTERFACE_VERSION " (Black Mesa)";
				g_VEngineClient = new WrpVEngineClient_bm((SOURCESDK::BM::IVEngineClient*)iface);
			}
			break;
		case SourceSdkVer_Insurgency2:
			if (iface = appSystemFactory(SOURCESDK_INSURGENCY2_VENGINE_CLIENT_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineClient = SOURCESDK_INSURGENCY2_VENGINE_CLIENT_INTERFACE_VERSION " (Insurgency 2)";
				g_VEngineClient = new WrpVEngineClient_Insurgency2((SOURCESDK::INSURGENCY2::IVEngineClient*)iface);
			}
			break;
		default:
			if (iface = appSystemFactory(VENGINE_CLIENT_INTERFACE_VERSION_015, NULL))
			{
				// This is not really 100% backward compatible, there is a problem with the CVAR interface or s.th..
				// But the guy that tested it wasn't available for further debugging, so I'll just leave it as
				// it is now. Will crash as soon as i.e. ExecuteCliendCmd is used, due to some crash
				// related to CVAR system.

				g_Info_VEngineClient = VENGINE_CLIENT_INTERFACE_VERSION_015;
				g_VEngineClient = new WrpVEngineClient_013((SOURCESDK::IVEngineClient_013*)iface);
			}
			else if (iface = appSystemFactory(VENGINE_CLIENT_INTERFACE_VERSION_013, NULL))
			{
				g_Info_VEngineClient = VENGINE_CLIENT_INTERFACE_VERSION_013;
				g_VEngineClient = new WrpVEngineClient_013((SOURCESDK::IVEngineClient_013*)iface);
			}
			else if (iface = appSystemFactory(VENGINE_CLIENT_INTERFACE_VERSION_012, NULL)) {
				g_Info_VEngineClient = VENGINE_CLIENT_INTERFACE_VERSION_012;
				g_VEngineClient = new WrpVEngineClient_012((SOURCESDK::IVEngineClient_012*)iface);
			}
			break;
		}
		if (nullptr == iface) ErrorBox("Could not get a supported VEngineClient interface.");

		// VEngineCvar:

		switch (g_SourceSdkVer)
		{
		case SourceSdkVer_CSGO:
		case SourceSdkVer_Insurgency2:
			if (iface = appSystemFactory(SOURCESDK_CSGO_CVAR_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineCvar = SOURCESDK_CSGO_CVAR_INTERFACE_VERSION " (CS:GO)";
				SOURCESDK::CSGO::g_pCVar = SOURCESDK::CSGO::cvar = (SOURCESDK::CSGO::ICvar*)iface;

				WrpConCommands::RegisterCommands(SOURCESDK::CSGO::g_pCVar);
			}
			break;
		case SourceSdkVer_SWARM:
			if (iface = appSystemFactory(SOURCESDK_SWARM_CVAR_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineCvar = SOURCESDK_SWARM_CVAR_INTERFACE_VERSION " (Alien Swarm)";
				SOURCESDK::SWARM::g_pCVar = SOURCESDK::SWARM::cvar = (SOURCESDK::SWARM::ICvar*)iface;

				WrpConCommands::RegisterCommands(SOURCESDK::SWARM::g_pCVar);
			}
			break;
		case SourceSdkVer_BM:
			if (iface = appSystemFactory(SOURCESDK_BM_CVAR_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineCvar = SOURCESDK_BM_CVAR_INTERFACE_VERSION " (Black Mesa)";
				SOURCESDK::BM::g_pCVar = SOURCESDK::BM::cvar = (SOURCESDK::BM::ICvar*)iface;

				WrpConCommands::RegisterCommands(SOURCESDK::BM::g_pCVar);
			}
			break;
		default:
			if (iface = appSystemFactory(SOURCESDK_L4D2_CVAR_INTERFACE_VERSION, NULL))
			{
				g_Info_VEngineCvar = SOURCESDK_L4D2_CVAR_INTERFACE_VERSION " (Left 4 Dead 2)";
				SOURCESDK::L4D2::g_pCVar = SOURCESDK::L4D2::cvar = (SOURCESDK::L4D2::ICvar*)iface;

				WrpConCommands::RegisterCommands(SOURCESDK::L4D2::g_pCVar);
			}
			else if (iface = appSystemFactory(VENGINE_CVAR_INTERFACE_VERSION_004, NULL))
			{
				g_Info_VEngineCvar = VENGINE_CVAR_INTERFACE_VERSION_004;
				WrpConCommands::RegisterCommands((SOURCESDK::ICvar_004*)iface);
			}
			else if (
				(iface = appSystemFactory(VENGINE_CVAR_INTERFACE_VERSION_003, NULL))
				&& (iface2 = appSystemFactory(VENGINE_CLIENT_INTERFACE_VERSION_012, NULL)))
			{
				g_Info_VEngineCvar = VENGINE_CVAR_INTERFACE_VERSION_003 " & " VENGINE_CLIENT_INTERFACE_VERSION_012;
				WrpConCommands::RegisterCommands((SOURCESDK::ICvar_003*)iface, (SOURCESDK::IVEngineClient_012*)iface2);
			}
			break;
		}
		if (nullptr == iface) ErrorBox("Could not get a supported VEngineCvar interface.");

		// VClientEnginteTools

		if(iface = appSystemFactory(VCLIENTENGINETOOLS_INTERFACE_VERSION_001, NULL))
		{
			g_Engine_ClientEngineTools = (SOURCESDK::IClientEngineTools_001 *)iface;
		}
		else {
			ErrorBox("Could not get a supported VClientEngineTools interface.");
		}

		// Other

		if(SourceSdkVer_CSGO == g_SourceSdkVer)
		{
			if(iface = appSystemFactory(MATERIAL_SYSTEM_INTERFACE_VERSION_CSGO_80, NULL))
			{
				g_MaterialSystem_csgo = (SOURCESDK::IMaterialSystem_csgo *)iface;
				g_AfxStreams.OnMaterialSystem(g_MaterialSystem_csgo);
			}
			else {
				ErrorBox("Could not get a supported VMaterialSystem interface.");
			}

			if(iface = appSystemFactory(FILESYSTEM_INTERFACE_VERSION_CSGO_017, NULL))
			{
				g_FileSystem_csgo = (SOURCESDK::IFileSystem_csgo *)iface;
			}
			else {
				ErrorBox("Could not get a supported VFileSystem interface.");
			}

			if (iface = appSystemFactory(SOURCESDK_CSGO_VGUI_PANEL_INTERFACE_VERSION, NULL))
			{
				g_pVGuiPanel_csgo = (SOURCESDK::CSGO::vgui::IPanel *)iface;
			}
			else {
				ErrorBox("Could not get a supported VGUI_Panel interface.");
			}

			if (iface = appSystemFactory(SOURCESDK_CSGO_VGUI_VGUI_SURFACE_INTERFACE_VERSION, NULL))
			{
				g_pVGuiSurface_csgo = (SOURCESDK::CSGO::vgui::ISurface *)iface;
			}
			else {
				ErrorBox("Could not get a supported VGUI_Surface interface.");
			}

			if(iface = appSystemFactory(SHADERSHADOW_INTERFACE_VERSION_CSGO, NULL))
			{
				g_AfxStreams.OnShaderShadow((SOURCESDK::IShaderShadow_csgo *)iface);
			}
			else {
				ErrorBox("Could not get a supported ShaderShadow interface.");
			}

			if (iface = appSystemFactory(SOURCESDK_CSGO_INTERFACEVERSION_ENGINETRACE_CLIENT, NULL))
			{
				g_pClientEngineTrace = (SOURCESDK::CSGO::IEngineTrace *)iface;
			}
			else {
				ErrorBox("Could not get a supported client engine trace interface.");
			}

			if (iface = appSystemFactory(VENGINE_RENDERVIEW_INTERFACE_VERSION_CSGO, NULL))
			{
				g_pVRenderView_csgo = (SOURCESDK::IVRenderView_csgo *)iface;
			}
			else {
				ErrorBox("Could not get " VENGINE_RENDERVIEW_INTERFACE_VERSION_CSGO ".");
			}

			if (iface = appSystemFactory(SOURCESDK_CSGO_STUDIO_RENDER_INTERFACE_VERSION, NULL))
			{
				g_pStudioRender = (SOURCESDK::CSGO::IStudioRender *)iface;
				//StudioHooks_Install(g_pStudioRender);
			}
			else {
				ErrorBox("Could not get " SOURCESDK_CSGO_STUDIO_RENDER_INTERFACE_VERSION ".");
			}

			if(iface = appSystemFactory(SOURCESDK_CSGO_VMODELINFO_CLIENT_INTERFACE_VERSION, NULL)) {
				g_pModelInfo = (SOURCESDK::CSGO::IVModelInfoClient *)iface;
			} else {
				ErrorBox("Could not get " SOURCESDK_CSGO_VMODELINFO_CLIENT_INTERFACE_VERSION ".");
			}

			/*
			if (iface = appSystemFactory(SORUCESDK_CSGO_VENGINE_GAMEUIFUNCS_VERSION, NULL))
			{
				g_pGameUIFuncs = (SOURCESDK::CSGO::CGameUIFuncs *)iface;
			}
			else {
				ErrorBox("Could not get " SORUCESDK_CSGO_VENGINE_GAMEUIFUNCS_VERSION " interface.");
			}
			if (iface = appSystemFactory(SOURCECSDK_CSGO_PANORAMAUIENGINE, NULL))
			{
				g_pPanoramaUIEngine = (SOURCESDK::CSGO::panorama::CPanoramaUIEngine *)iface;
			}
			else {
				ErrorBox("Could not get " SOURCECSDK_CSGO_PANORAMAUIENGINE " interface.");
			}
			if (iface = appSystemFactory(SOURCESDK_CSGO_PANORAMAUICLIENT_VERSION, NULL))
			{
				g_pPanoramaUIClient = (SOURCESDK::CSGO::CPanoramaUIClient *)iface;
			}
			else {
				ErrorBox("Could not get " SOURCESDK_CSGO_PANORAMAUICLIENT_VERSION " interface.");
			}
			*/
		}
		
		g_Hook_VClient_RenderView.Install(pGlobals);

		//AfxV34HookWindow();

		PrintInfo();
	}
}

void* AppSystemFactory_ForClient(const char *pName, int *pReturnCode)
{
	if(!strcmp(VCLIENTENGINETOOLS_INTERFACE_VERSION_001, pName))
	{
		return &g_ClientEngineTools;
	}
	//else
	//if(isCsgo && !strcmp(VENGINE_RENDERVIEW_INTERFACE_VERSION_CSGO, pName) && g_AfxVRenderView)
	//{
	//	return g_AfxVRenderView;
	//}
	return g_AppSystemFactory(pName, pReturnCode);
}

typedef int(__fastcall * CVClient_Init_Unknown_t)(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CreateInterfaceFn physicsFactory, SOURCESDK::CGlobalVarsBase *pGlobals);

CVClient_Init_Unknown_t old_CVClient_Init_Unkown;

int __fastcall new_CVClient_Init_Unknown(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CreateInterfaceFn physicsFactory, SOURCESDK::CGlobalVarsBase *pGlobals)
{
	static bool bFirstCall = true;

	if (bFirstCall) {
		bFirstCall = false;

		MySetup(appSystemFactory, new WrpGlobalsOther(pGlobals));
	}

	return old_CVClient_Init_Unkown(This, Edx, AppSystemFactory_ForClient, physicsFactory, pGlobals);
}

typedef int(__fastcall* CVClient_Init_Swarm_t)(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals);

CVClient_Init_Swarm_t old_CVClient_Init_Swarm;

int __fastcall new_CVClient_Init_Swarm(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals)
{
	static bool bFirstCall = true;

	if (bFirstCall) {
		bFirstCall = false;

		MySetup(appSystemFactory, new WrpGlobalsOther(pGlobals));
	}

	return old_CVClient_Init_Swarm(This, Edx, AppSystemFactory_ForClient, pGlobals);
}

typedef int(__fastcall* CVClient_Init_BM_t)(void *This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals);

CVClient_Init_BM_t old_CVClient_Init_BM;

int __fastcall new_CVClient_Init_BM(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals)
{
	static bool bFirstCall = true;

	if( bFirstCall )
	{
		bFirstCall = false;

		MySetup(appSystemFactory, new WrpGlobalsOther(pGlobals));
	}

	return old_CVClient_Init_BM(This, Edx, AppSystemFactory_ForClient, pGlobals);
}


typedef int(__fastcall* CVClient_Init_Garrysmod_t)(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CreateInterfaceFn physicsFactory, SOURCESDK::CGlobalVarsBase* pGlobals, void * unknown3);

CVClient_Init_Garrysmod_t old_CVClient_Init_Garrysmod;

int __fastcall new_CVClient_Init_Garrysmod(void* This, void* Edx, SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CreateInterfaceFn physicsFactory, SOURCESDK::CGlobalVarsBase* pGlobals, void* unknown3)
{
	static bool bFirstCall = true;

	if (bFirstCall) {
		bFirstCall = false;

		MySetup(appSystemFactory, new WrpGlobalsOther(pGlobals));
	}

	return old_CVClient_Init_Garrysmod(This, Edx, AppSystemFactory_ForClient, physicsFactory, pGlobals, unknown3);
}


void Shared_BeforeFrameRenderStart(void)
{
	g_MirvTime.OnFrameRenderStart();

	if (CClientTools * instance = CClientTools::Instance()) instance->OnBeforeFrameRenderStart();
}

void Shared_AfterFrameRenderEnd(void)
{
	if (CClientTools * instance = CClientTools::Instance()) instance->OnAfterFrameRenderEnd();
	Mirv_Voice_OnAfterFrameRenderEnd();
	AfxHookSource::Gui::OnGameFrameRenderEnd();
}

void Shared_Shutdown(void)
{
	if (CClientTools * instance = CClientTools::Instance()) delete instance;
}

typedef void(__fastcall * CVClient_Shutdown_TF2_t)(void* This, void* Edx);

CVClient_Shutdown_TF2_t old_CVClient_Shutdown_TF2;

void __fastcall new_CVClient_Shutdown_TF2(void* This, void* Edx)
{
	Shared_Shutdown();

	old_CVClient_Shutdown_TF2(This, Edx);
}

typedef void (__fastcall* CVClient_FrameStageNotify_TF2_t)(void* This, void* Edx, SOURCESDK::TF2::ClientFrameStage_t curStage);

CVClient_FrameStageNotify_TF2_t old_CVClient_FrameStageNotify_TF2;

// Notification that we're moving into another stage during the frame.
void __fastcall new_CVClient_FrameStageNotify_TF2(void* This, void* Edx, SOURCESDK::TF2::ClientFrameStage_t curStage)
{
	switch (curStage)
	{
	case SOURCESDK::TF2::FRAME_RENDER_START:
		Shared_BeforeFrameRenderStart();
		break;
	}

	old_CVClient_FrameStageNotify_TF2(This, Edx, curStage);

	switch (curStage)
	{
	case SOURCESDK::TF2::FRAME_RENDER_END:
		Shared_AfterFrameRenderEnd();
		break;
	}
}


typedef void(__fastcall* CVClient_Shutdown_CSS_t)(void* This, void* Edx);

CVClient_Shutdown_CSS_t old_CVClient_Shutdown_CSS;

void __fastcall new_CVClient_Shutdown_CSS(void* This, void* Edx)
{
	Shared_Shutdown();

	old_CVClient_Shutdown_CSS(This, Edx);
}

typedef void(__fastcall* CVClient_FrameStageNotify_CSS_t)(void* This, void* Edxr, SOURCESDK::CSS::ClientFrameStage_t curStage);

CVClient_FrameStageNotify_CSS_t old_CVClient_FrameStageNotify_CSS;

// Notification that we're moving into another stage during the frame.
void __fastcall new_CVClient_FrameStageNotify_CSS(void* This, void* Edx, SOURCESDK::CSS::ClientFrameStage_t curStage)
{
	switch (curStage)
	{
	case SOURCESDK::CSS::FRAME_RENDER_START:
		Shared_BeforeFrameRenderStart();
		break;
	}

	old_CVClient_FrameStageNotify_CSS(This, Edx, curStage);

	switch (curStage)
	{
	case SOURCESDK::CSS::FRAME_RENDER_END:
		Shared_AfterFrameRenderEnd();
		break;
	}
}

typedef void(__fastcall* CVClient_Shutdown_CSSV34_t)(void* This, void* Edx);

CVClient_Shutdown_CSSV34_t old_CVClient_Shutdown_CSSV34;

void __fastcall new_CVClient_Shutdown_CSSV34(void* This, void* Edx)
{
	Shared_Shutdown();

	old_CVClient_Shutdown_CSSV34(This,Edx);
}

typedef void (__fastcall* CVClient_FrameStageNotify_CSSV34_t)(void* This, void* Edxr, SOURCESDK::CSSV34::ClientFrameStage_t curStage);

CVClient_FrameStageNotify_CSSV34_t old_CVClient_FrameStageNotify_CSSV34;

// Notification that we're moving into another stage during the frame.
void __fastcall new_CVClient_FrameStageNotify_CSSV34(void* This, void* Edx, SOURCESDK::CSSV34::ClientFrameStage_t curStage)
{
	switch (curStage)
	{
	case SOURCESDK::CSSV34::FRAME_RENDER_START:
		Shared_BeforeFrameRenderStart();
		break;
	}

	old_CVClient_FrameStageNotify_CSSV34(This, Edx, curStage);

	switch (curStage)
	{
	case SOURCESDK::CSSV34::FRAME_RENDER_END:
		Shared_AfterFrameRenderEnd();
		break;
	}
}

bool g_DebugEnabled = false;

bool g_csgo_FirstFrameAfterNetUpdateEnd = false;

extern int g_Mirv_Pov_PingAdjustMent;
int MirvGetPing(int playerIndex);

class CAfxBaseClientDll
: public SOURCESDK::IBaseClientDLL_csgo
, public IAfxBaseClientDll
{
public:
	CAfxBaseClientDll(IBaseClientDLL_csgo * parent)
	: m_Parent(parent)
	, m_OnView_Render(0)
	{
	}

	~CAfxBaseClientDll()
	{
	}

	//
	// IAfxBaseClientDll:

	virtual IBaseClientDLL_csgo * GetParent()
	{
		return m_Parent;
	}

	virtual void OnView_Render_set(IAfxBaseClientDllView_Render * value)
	{
		m_OnView_Render = value;
	}

	//
	// IBaseClientDll_csgo:

	virtual int Connect(SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals);
	virtual void Disconnect();
	virtual int Init(SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals);
	virtual void PostInit();
	virtual void Shutdown(void);
	virtual void LevelInitPreEntity(char const* pMapName);
	virtual void LevelInitPostEntity();
	virtual void LevelShutdown(void);
	virtual void _UNKOWN_008(void);
	virtual void _UNKOWN_009(void);
	virtual void _UNKOWN_010(void);
	virtual void _UNKOWN_011(void);
	virtual void _UNKOWN_012(void);
	virtual void _UNKOWN_013(void);
	virtual void _UNKOWN_014(void);
	virtual void _UNKOWN_015(void);

	virtual void IN_ActivateMouse(void);
	virtual void IN_DeactivateMouse(void);

	virtual void _UNKOWN_018(void);
	virtual void _UNKOWN_019(void);
	virtual void _UNKOWN_020(void);
	virtual void _UNKOWN_021(void);
	virtual void _UNKOWN_022(void);
	virtual void _UNKOWN_023(void);
	virtual void _UNKOWN_024(void);
	virtual void _UNKOWN_025(void);
	virtual void _UNKOWN_026(void);
	virtual void View_Render(SOURCESDK::vrect_t_csgo *rect);
	virtual void RenderView(const SOURCESDK::CViewSetup_csgo &view, int nClearFlags, int whatToDraw);
	virtual void _UNKOWN_029(void);
	virtual void _UNKOWN_030(void);
	virtual void _UNKOWN_031(void);
	virtual void _UNKOWN_032(void);
	virtual void _UNKOWN_033(void);
	virtual void _UNKOWN_034(void);
	virtual void _UNKOWN_035(void);
	virtual void _UNKOWN_036(void);
	virtual void FrameStageNotify(SOURCESDK::CSGO::ClientFrameStage_t curStage);
	virtual void _UNKOWN_038(void);
	virtual void _UNKOWN_039(void);
	virtual void _UNKOWN_040(void);
	virtual void _UNKOWN_041(void);
	virtual void _UNKOWN_042(void);
	virtual void _UNKOWN_043(void);
	virtual void _UNKOWN_044(void);
	virtual void _UNKOWN_045(void);
	virtual void _UNKOWN_046(void);
	virtual void _UNKOWN_047(void);
	virtual void _UNKOWN_048(void);
	virtual void _UNKOWN_049(void);
	virtual void _UNKOWN_050(void);
	virtual void _UNKOWN_051(void);
	virtual void _UNKOWN_052(void);
	virtual void _UNKOWN_053(void);
	virtual void _UNKOWN_054(void);

	virtual void OnDemoPlaybackStart(char const* pDemoBaseName);

	virtual void _UNKOWN_056(void);

	virtual void OnDemoPlaybackStop();
	
	virtual void _UNKOWN_058(void);
	virtual void _UNKOWN_059(void);
	virtual void _UNKOWN_060(void);
	virtual void _UNKOWN_061(void);
	virtual void _UNKOWN_062(void);
	virtual void _UNKOWN_063(void);
	virtual void _UNKOWN_064(void);
	virtual void _UNKOWN_065(void);
	virtual void _UNKOWN_066(void);
	virtual void _UNKOWN_067(void);
	virtual void _UNKOWN_068(void);
	virtual void _UNKOWN_069(void);
	virtual void _UNKOWN_070(void);
	virtual void _UNKOWN_071(void);
	virtual void _UNKOWN_072(void);
	virtual void _UNKOWN_073(void);
	virtual void _UNKOWN_074(void);
	virtual void _UNKOWN_075(void);
	virtual void _UNKOWN_076(void);
	virtual void _UNKOWN_077(void);
	virtual void _UNKOWN_078(void);
	virtual void _UNKOWN_079(void);
	virtual void _UNKOWN_080(void);
	virtual void _UNKOWN_081(void);
	virtual void _UNKOWN_082(void);
	virtual void _UNKOWN_083(void);
	virtual void _UNKOWN_084(void);
	virtual void _UNKOWN_085(void);
	virtual void _UNKOWN_086(void);
	virtual void _UNKOWN_087(void);
	virtual void _UNKOWN_088(void);
	virtual void _UNKOWN_089(void);
	virtual void _UNKOWN_090(void);
	virtual void _UNKOWN_091(void);
	virtual void _UNKOWN_092(void);
	virtual void _UNKOWN_093(void);
	virtual void _UNKOWN_094(void);
	virtual void _UNKOWN_095(void);
	virtual void _UNKOWN_096(void);
	virtual void _UNKOWN_097(void);
	virtual void _UNKOWN_098(void);
	virtual void _UNKOWN_099(void);
	virtual void _UNKOWN_100(void);
	virtual void _UNKOWN_101(void);
	virtual void _UNKOWN_102(void);
	virtual void _UNKOWN_103(void);
	virtual void _UNKOWN_104(void);
	virtual void _UNKOWN_105(void);
	virtual void _UNKOWN_106(void);
	virtual void _UNKOWN_107(void);
	virtual void _UNKOWN_108(void);
	virtual void _UNKOWN_109(void);
	virtual void _UNKOWN_110(void);
	virtual void _UNKOWN_111(void);
	virtual void _UNKOWN_112(void);
	virtual void _UNKOWN_113(void);
	virtual void _UNKOWN_114(void);
	virtual void _UNKOWN_115(void);
	virtual void _UNKOWN_116(void);
	virtual void _UNKOWN_117(void);
	virtual void _UNKOWN_118(void);
	virtual void _UNKOWN_119(void);
	virtual void _UNKOWN_120(void);
	virtual void _UNKOWN_121(void);
	virtual void _UNKOWN_122(void);
	virtual void _UNKOWN_123(void);
	virtual void _UNKOWN_124(void);
	virtual void _UNKOWN_125(void);
	virtual void _UNKOWN_126(void);
	virtual void _UNKOWN_127(void);
	virtual void _UNKOWN_128(void);
	virtual void _UNKOWN_129(void);
	virtual void _UNKOWN_130(void);
	virtual void _UNKOWN_131(void);
	virtual void _UNKOWN_132(void);
	virtual void _UNKOWN_133(void);
	virtual void _UNKOWN_134(void);
	virtual void _UNKOWN_135(void);
	virtual void _UNKOWN_136(void);
	virtual void _UNKOWN_137(void);
	virtual void _UNKOWN_138(void);
	virtual void _UNKOWN_139(void);

private:
	IBaseClientDLL_csgo * m_Parent;
	IAfxBaseClientDllView_Render * m_OnView_Render;
	bool m_IN_MouseActive = false;
};

CAfxBaseClientDll * g_AfxBaseClientDll = 0;

__declspec(naked) int CAfxBaseClientDll::Connect(SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 0) }

__declspec(naked) void CAfxBaseClientDll::Disconnect()
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 1) }




typedef int(__fastcall* CsgoFileSystemGetPureMode_t)(void* This, void* Edx);
CsgoFileSystemGetPureMode_t TrueCsgoFileSystemGetPureMode = nullptr;

bool __cdecl IsFileSystemDllReturnAddress(void* returnAddress)
{
	static bool initialized = false;
	static void* start;
	static void* end;

	if (!initialized)
	{
		if (HMODULE hFileSystemStdio = GetModuleHandleA("filesystem_stdio"))
		{
			Afx::BinUtils::ImageSectionsReader sections((HMODULE)hFileSystemStdio);
			if (!sections.Eof())
			{
				Afx::BinUtils::MemRange range = sections.GetMemRange();
				start = (void*)range.Start;
				end = (void*)range.End;

				initialized = true;
			}
		}
	}

	return start <= returnAddress && returnAddress < end;
}

__declspec(naked) int __fastcall MyCsgoFileSystemGetPureMode(void* This, void* Edx)
{
	__asm mov eax, [esp]
	__asm push ecx
	__asm push edx
	__asm push eax
	__asm call IsFileSystemDllReturnAddress
	__asm add esp, 4
	__asm pop edx
	__asm pop ecx
	__asm test al, al
	__asm jz __you_wanted_lies_we_tell_you_lies
	__asm mov eax, TrueCsgoFileSystemGetPureMode
	__asm call eax
	__asm ret
	__asm __you_wanted_lies_we_tell_you_lies:
	__asm mov eax, 0
	__asm ret
}


typedef void(__fastcall*CsgoFileSystemSetPureMode_t)(void * This, void * Edx, int pureMode);
CsgoFileSystemSetPureMode_t TrueCsgoFileSystemSetPureMode = nullptr;

void __fastcall MyCsgoFileSystemSetPureMode(void* This, void* Edx, int pureModex)
{
	TrueCsgoFileSystemSetPureMode(This, Edx, 1); // nope @CSGO we're not gonna let you switch that back!
}

//__declspec(naked) 
int CAfxBaseClientDll::Init(SOURCESDK::CreateInterfaceFn appSystemFactory, SOURCESDK::CGlobalVarsBase *pGlobals)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 2)

	static bool bFirstCall = true;

	if (bFirstCall)
	{
		bFirstCall = false;

		MySetup(appSystemFactory, new WrpGlobalsCsGo(pGlobals));

		//
		// Install early hooks:

		csgo_CHudDeathNotice_Install();

		// Connect to reshade addon if present:
		//

		g_ReShadeAdvancedfx.Connect();
	}

	int result = m_Parent->Init(AppSystemFactory_ForClient, pGlobals);

	if (g_FileSystem_csgo)
	{
		if (nullptr == TrueCsgoFileSystemSetPureMode)
		{
			TrueCsgoFileSystemGetPureMode = (CsgoFileSystemGetPureMode_t)(*(DWORD*)((*(char**)g_FileSystem_csgo) + 0x200));
			TrueCsgoFileSystemSetPureMode = (CsgoFileSystemSetPureMode_t)(*(DWORD*)((*(char**)g_FileSystem_csgo) + 0x204));

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)TrueCsgoFileSystemGetPureMode, MyCsgoFileSystemGetPureMode);
			DetourAttach(&(PVOID&)TrueCsgoFileSystemSetPureMode, MyCsgoFileSystemSetPureMode);
			if (NO_ERROR == DetourTransactionCommit())
			{
				// Enable non-pure mode, now that @CSGO can't change it back:
				TrueCsgoFileSystemSetPureMode(g_FileSystem_csgo, 0, 1);
			}
			else
			{
				MessageBoxA(0, "Failed to detour CS:GO filesystem_stdio interface", "ERROR", MB_OK | MB_ICONERROR);
			}
		}

		// Add file system search path for our assets:
		std::string path(GetHlaeFolder());
		path.append("resources\\AfxHookSource\\assets\\csgo");
		g_FileSystem_csgo->AddSearchPath(path.c_str(), "GAME", SOURCESDK::PATH_ADD_TO_TAIL);
	}

#ifdef AFX_MIRV_PGL
	MirvPgl::Init();
#endif	

	CAfxStreams::AfxStreamsInit();

	return result;
}

__declspec(naked) void CAfxBaseClientDll::PostInit()
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 3) }

 //__declspec(naked) 
void CAfxBaseClientDll::Shutdown(void)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 4)

	g_AfxStreams.ShutDown();

#ifdef AFX_MIRV_PGL
	MirvPgl::Shutdown();
#endif

#ifdef AFX_INTEROP
	AfxInterop::Shutdown();
#endif

	Shared_Shutdown();

	m_Parent->Shutdown();
}

void MirvCalcs_LevelInitPreEntity();

//__declspec(naked) 
void CAfxBaseClientDll::LevelInitPreEntity(char const* pMapName)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 5)

	m_Parent->LevelInitPreEntity(pMapName);

	g_MirvTime.OnLevelInitPreEntity();

#ifdef AFX_INTEROP
	AfxInterop::LevelInitPreEntity(pMapName);
#endif

#ifdef AFX_MIRV_PGL
	MirvPgl::SupplyLevelInit(pMapName);
#endif

	MirvCalcs_LevelInitPreEntity();
}

void CAfxBaseClientDll::LevelInitPostEntity()
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 6)

	m_Parent->LevelInitPostEntity();

	g_AfxStreams.LevelInitPostEntity();
}

//__declspec(naked) 
void CAfxBaseClientDll::LevelShutdown(void)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 7)
	g_AfxStreams.LevelShutdown();

#if AFX_SHADERS_CSGO
	csgo_Stdshader_dx9_Hooks_OnLevelShutdown();
#endif

	g_AfxShaders.ReleaseUnusedShaders();

#ifdef AFX_MIRV_PGL
	MirvPgl::SupplyLevelShutdown();
#endif

#ifdef AFX_INTEROP
	AfxInterop::LevelShutdown();
#endif

	m_Parent->LevelShutdown();
}

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_008(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 8) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_009(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 9) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_010(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 10) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_011(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 11) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_012(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 12) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_013(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 13) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_014(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 14) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_015(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 15) }

void CAfxBaseClientDll::IN_ActivateMouse(void)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 16)

	m_IN_MouseActive = true;

	m_Parent->IN_ActivateMouse();
}

void CAfxBaseClientDll::IN_DeactivateMouse(void)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 17)

	m_Parent->IN_DeactivateMouse();

	m_IN_MouseActive = false;	
}

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_018(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 18) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_019(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 19) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_020(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 20) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_021(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 21) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_022(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 22) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_023(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 23) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_024(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 24) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_025(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 25) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_026(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 26) }

//__declspec(naked) 
void CAfxBaseClientDll::View_Render(SOURCESDK::vrect_t_csgo *rect)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 27)

	//Tier0_Msg("---- View_Render ----\n");

	bool rectNull = nullptr == rect || rect->width == 0 || rect->height == 0;

	if (g_MaterialSystem_csgo && !rectNull)
	{

		if (m_OnView_Render)
		{
			m_OnView_Render->View_Render(this, rect);
		}
		else
		{
			m_Parent->View_Render(rect);
		}
	}
	else
	{
		m_Parent->View_Render(rect);
	}
}

__declspec(naked) void CAfxBaseClientDll::RenderView(const SOURCESDK::CViewSetup_csgo &view, int nClearFlags, int whatToDraw)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 28) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_029(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 29) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_030(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 30) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_031(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 31) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_032(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 32) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_033(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 33) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_034(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 34) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_035(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 35) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_036(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 36) }

int g_iForcePostDataUpdateChanged = -1;

typedef void(__fastcall* csgo_C_BasePlayer_SetAsLocalPlayer_t)(void* Ecx, void* Edx);

//__declspec(naked)
void CAfxBaseClientDll::FrameStageNotify(SOURCESDK::CSGO::ClientFrameStage_t curStage)
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 37)

	static bool firstFrameAfterNetUpdateEnd = false;
	static int oldMirvPov = 0;

	switch (curStage)
	{
	case SOURCESDK::CSGO::FRAME_START:
		//Tier0_Msg("FRAME_START\n");

#ifdef AFX_INTEROP
		AfxInterop::BeforeFrameStart();
#endif

#ifdef AFX_MIRV_PGL
		MirvPgl::CheckStartedAndRestoreIfDown();
		MirvPgl::ExecuteQueuedCommands();
#endif

		g_AfxStreams.BeforeFrameStart();
		break;
	case SOURCESDK::CSGO::FRAME_NET_UPDATE_END:
		firstFrameAfterNetUpdateEnd = true;
		break;

	case SOURCESDK::CSGO::FRAME_RENDER_START:

#ifdef AFX_INTEROP
		AfxInterop::BeforeFrameRenderStart();
#endif

		Shared_BeforeFrameRenderStart();

		g_csgo_FirstFrameAfterNetUpdateEnd = firstFrameAfterNetUpdateEnd;
		firstFrameAfterNetUpdateEnd = false;

#ifdef AFX_MIRV_PGL
		MirvPgl::QueueThreadDataForDrawingThread();
#endif
		CAfxStreams::MainThreadInitialize();

		break;

	case SOURCESDK::CSGO::FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		if (-1 != g_iForcePostDataUpdateChanged)
		{
			if (SOURCESDK::IClientEntity_csgo* ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_iForcePostDataUpdateChanged))
				ce->PostDataUpdate(SOURCESDK::CSGO::DATA_UPDATE_CREATED);
		}
		break;

	case SOURCESDK::CSGO::FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		{
			int newMirvPov = 0;
			// Switch back to target pov player before updates are applied:
			if (0 != g_i_MirvPov && g_Org_svc_ServerInfo_PlayerSlot + 1 != g_i_MirvPov)
			{
				if (SOURCESDK::IClientEntity_csgo* ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_i_MirvPov))
				{
					if (SOURCESDK::C_BaseEntity_csgo* be = ce->GetBaseEntity())
					{
						if (be->IsPlayer() && !be->IsDormant())
						{
							newMirvPov = g_i_MirvPov;

							if (g_i_MirvPov != g_VEngineClient->GetLocalPlayer())
							{
								// E.g. earlier disconnected and had to switch back to player 1, but now we can switch back to target again.

								static csgo_C_BasePlayer_SetAsLocalPlayer_t setAsLocalPlayer = (csgo_C_BasePlayer_SetAsLocalPlayer_t)AFXADDR_GET(csgo_C_BasePlayer_SetAsLocalPlayer);
								setAsLocalPlayer(be, 0);
							}

							bool* pOsLocalPlayer = (bool*)((char*)be + AFXADDR_GET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer));
							*pOsLocalPlayer = true;

							// make old POV non-local (if there is any):
							if(newMirvPov != oldMirvPov && oldMirvPov != 0) {
								if (SOURCESDK::IClientEntity_csgo* ce1 = SOURCESDK::g_Entitylist_csgo->GetClientEntity(oldMirvPov))
								{
									if (SOURCESDK::C_BaseEntity_csgo* be1 = ce1->GetBaseEntity())
									{
										if (be1->IsPlayer())
										{
											bool* pOsLocalPlayer = (bool*)((char*)be1 + AFXADDR_GET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer));
											*pOsLocalPlayer = false;
										}
									}
								}
							}

							// Make sure real local player stays overriden:
							if (SOURCESDK::IClientEntity_csgo* ce1 = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_Org_svc_ServerInfo_PlayerSlot + 1))
							{
								if (SOURCESDK::C_BaseEntity_csgo* be1 = ce1->GetBaseEntity())
								{
									if (be1->IsPlayer())
									{
										bool* pOsLocalPlayer = (bool*)((char*)be1 + AFXADDR_GET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer));
										*pOsLocalPlayer = false;
									}
								}
							}
						}
					}
				}
			}
			static WrpConVarRef cvar_cl_interp_npcs;
			cvar_cl_interp_npcs.RetryIfNull("cl_interp_npcs"); // GOTV would have this on 0, so force it too.
			bool pingUpdated = 0;
			if(oldMirvPov != newMirvPov) {
				if(newMirvPov == 0) {
					// Lost player, e.g. due to disconnect.
					// Switch back to original local player:
					if (SOURCESDK::IClientEntity_csgo* ce1 = SOURCESDK::g_Entitylist_csgo->GetClientEntity(g_Org_svc_ServerInfo_PlayerSlot + 1))
					{
						if (SOURCESDK::C_BaseEntity_csgo* be1 = ce1->GetBaseEntity())
						{
							if (be1->IsPlayer())
							{
								bool* pOsLocalPlayer = (bool*)((char*)be1 + AFXADDR_GET(csgo_C_BasePlayer_ofs_m_bIsLocalPlayer));
								*pOsLocalPlayer = true;
							}
						}
					}

					g_Mirv_Pov_PingAdjustMent = 0;
					pingUpdated = true;
				}
				oldMirvPov = newMirvPov;
			}
			if(newMirvPov) {
				// We want to adjust interpolation for the local player ping, so we get more accurate view in time:
				g_Mirv_Pov_PingAdjustMent = MirvGetPing(newMirvPov);
				if(g_Mirv_Pov_PingAdjustMent <= 5) g_Mirv_Pov_PingAdjustMent = 0; // 5 is minium and we can not tell.
				pingUpdated = true;
			}
			if(pingUpdated) {
				// Make it update the interpolation, since we need to adjust it for player ping.
				cvar_cl_interp_npcs.SetDirectHack(1.0f - cvar_cl_interp_npcs.GetFloat());
			} else if(0 == newMirvPov && cvar_cl_interp_npcs.GetFloat()) {
				// Restore default value if not using mirv_pov.
				cvar_cl_interp_npcs.SetDirectHack(0.0f);
			}
		}
		break;

	}

	m_Parent->FrameStageNotify(curStage);

	switch (curStage)
	{

	case SOURCESDK::CSGO::FRAME_RENDER_START:
#ifdef AFX_INTEROP
		AfxInterop::AfterFrameRenderStart();
#endif
		break;

	case SOURCESDK::CSGO::FRAME_RENDER_END:
		csgo_Audio_FRAME_RENDEREND();
		Shared_AfterFrameRenderEnd();
		break;
	}
}

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_038(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 38) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_039(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 39) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_040(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 40) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_041(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 41) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_042(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 42) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_043(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 43) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_044(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 44) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_045(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 45) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_046(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 46) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_047(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 47) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_048(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 48) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_049(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 49) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_050(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 50) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_051(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 51) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_052(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 52) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_053(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 53) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_054(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 54) }

//__declspec(naked)
void CAfxBaseClientDll::OnDemoPlaybackStart(char const* pDemoBaseName)
{ //NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 55)
	
	m_Parent->OnDemoPlaybackStart(pDemoBaseName);
}

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_056(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 56) }

//__declspec(naked)
void CAfxBaseClientDll::OnDemoPlaybackStop()
{ // NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 57)

	m_Parent->OnDemoPlaybackStop();
}

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_058(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 58) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_059(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 59) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_060(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 60) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_061(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 61) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_062(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 62) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_063(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 63) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_064(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 64) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_065(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 65) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_066(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 66) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_067(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 67) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_068(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 68) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_069(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 69) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_070(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 70) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_071(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 71) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_072(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 72) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_073(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 73) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_074(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 74) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_075(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 75) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_076(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 76) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_077(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 77) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_078(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 78) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_079(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 79) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_080(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 80) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_081(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 81) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_082(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 82) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_083(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 83) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_084(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 84) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_085(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 85) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_086(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 86) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_087(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 87) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_088(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 88) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_089(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 89) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_090(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 90) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_091(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 91) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_092(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 92) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_093(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 93) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_094(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 94) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_095(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 95) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_096(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 96) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_097(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 97) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_098(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 98) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_099(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 99) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_100(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 100) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_101(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 101) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_102(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 102) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_103(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 103) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_104(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 104) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_105(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 105) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_106(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 106) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_107(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 107) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_108(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 108) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_109(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 109) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_110(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 110) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_111(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 111) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_112(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 112) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_113(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 113) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_114(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 114) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_115(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 115) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_116(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 116) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_117(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 117) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_118(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 118) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_119(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 119) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_120(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 120) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_121(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 121) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_122(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 122) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_123(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 123) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_124(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 124) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_125(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 125) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_126(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 126) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_127(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 127) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_128(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 128) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_129(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 129) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_130(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 130) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_131(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 131) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_132(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 132) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_133(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 133) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_134(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 134) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_135(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 135) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_136(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 136) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_137(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 137) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_138(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 138) }

__declspec(naked) void CAfxBaseClientDll::_UNKOWN_139(void)
{ NAKED_JMP_CLASSMEMBERIFACE_FN(CAfxBaseClientDll, m_Parent, 139) }

void HookClientDllInterface_011_Init(void * iface)
{
	int * vtable = *(int**)iface;

	AfxDetourPtr((PVOID *)&(vtable[0]), new_CVClient_Init_Unknown, (PVOID*)&old_CVClient_Init_Unkown);
}

void HookClientDllInterface_Swarm_Init(void * iface)
{
	int * vtable = *(int**)iface;

	AfxDetourPtr((PVOID*) & (vtable[1]), new_CVClient_Init_Swarm, (PVOID*)&old_CVClient_Init_Swarm);
}

void HookClientDllInterface_BM_Init(void * iface)
{
	int * vtable = *(int**) iface;

	AfxDetourPtr((PVOID*)&(vtable[2]), new_CVClient_Init_BM, (PVOID*)&old_CVClient_Init_BM);
}

void HookClientDllInterface_Insurgency2_Init(void * iface)
{
	int * vtable = *(int**)iface;

	AfxDetourPtr((PVOID*) & (vtable[2]), new_CVClient_Init_Swarm, (PVOID*)&old_CVClient_Init_Swarm);
}

void HookClientDllInterface_Garrysmod_Init(void* iface)
{
	int* vtable = *(int**)iface;

	AfxDetourPtr((PVOID*) & (vtable[0]), new_CVClient_Init_Garrysmod, (PVOID*)&old_CVClient_Init_Garrysmod);
}

SOURCESDK::IClientEntityList_csgo * SOURCESDK::g_Entitylist_csgo = 0;

SOURCESDK::CreateInterfaceFn old_Client_CreateInterface = 0;

class CAfxCsgoPrediction : public SOURCESDK::CSGO::IPrediction
{
public:
	CAfxCsgoPrediction(SOURCESDK::CSGO::IPrediction * prediction) : m_Prediction(prediction) {}

	virtual			~CAfxCsgoPrediction(void) {
		delete m_Prediction;
	};

	virtual void	Init(void) {
		m_Prediction->Init();
	}
	virtual void	Shutdown(void) {
		m_Prediction->Shutdown();
	}

	virtual void	Update (int startframe, bool validframe, int incoming_acknowledged,	 int outgoing_command) {
		if (g_i_MirvPov) return;

		m_Prediction->Update(startframe, validframe, incoming_acknowledged, outgoing_command);
	}

	virtual void	PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) {
		m_Prediction->PreEntityPacketReceived(commands_acknowledged, current_world_update_packet);
	}

	virtual void	PostEntityPacketReceived(void) {
		m_Prediction->PostEntityPacketReceived();
	}
	virtual void	PostNetworkDataReceived(int commands_acknowledged) {
		m_Prediction->PostNetworkDataReceived(commands_acknowledged);
	}

	virtual void	OnReceivedUncompressedPacket(void) {
		m_Prediction->OnReceivedUncompressedPacket();
	}

	virtual void	GetViewOrigin(SOURCESDK::Vector& org) {
		m_Prediction->GetViewOrigin(org);
	}
	virtual void	SetViewOrigin(SOURCESDK::Vector& org) {
		if (g_i_MirvPov) return;

		m_Prediction->SetViewOrigin(org);
	}
	virtual void	GetViewAngles(SOURCESDK::QAngle& ang) {
		m_Prediction->GetViewAngles(ang);
	}
	virtual void	SetViewAngles(SOURCESDK::QAngle& ang) {
		if (g_i_MirvPov) return;

		m_Prediction->SetViewAngles(ang);
	}
	virtual void	GetLocalViewAngles(SOURCESDK::QAngle& ang) {
		m_Prediction->GetLocalViewAngles(ang);
	}
	virtual void	SetLocalViewAngles(SOURCESDK::QAngle& ang) {
		if (g_i_MirvPov) return;

		m_Prediction->SetLocalViewAngles(ang);
	}

private:
	SOURCESDK::CSGO::IPrediction* m_Prediction;

};

CAfxCsgoPrediction* g_AfxCsgoPrediction = nullptr;

void* new_Client_CreateInterface(const char *pName, int *pReturnCode)
{
	static bool bFirstCall = true;

	void * pRet = old_Client_CreateInterface(pName, pReturnCode);

	if(bFirstCall)
	{
		bFirstCall = false;

		void * iface = NULL;
		
		if(SourceSdkVer_CSGO != g_SourceSdkVer)
		{
			if (iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_018, NULL)) {
				if( SourceSdkVer_BM == g_SourceSdkVer )
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_018 " (Black Mesa)";
					HookClientDllInterface_BM_Init(iface);
				}
				else
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_018;
					HookClientDllInterface_011_Init(iface);
				}
			}
			else
			if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_017, NULL)) {
				if (SourceSdkVer_Garrysmod == g_SourceSdkVer)
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_017 " (Garrysmod)";
					HookClientDllInterface_Garrysmod_Init(iface);
				}
				else
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_017;
					HookClientDllInterface_011_Init(iface);
				}
			}
			else if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_016, NULL)) {
				if (SourceSdkVer_Insurgency2 == g_SourceSdkVer)
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_016 " (Insurgency2)";
					HookClientDllInterface_Insurgency2_Init(iface);
				}
				else if (SourceSdkVer_SWARM == g_SourceSdkVer || SourceSdkVer_L4D2 == g_SourceSdkVer )
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_016 " (Alien Swarm / Left 4 Dead 2)";
					HookClientDllInterface_Swarm_Init(iface);
				}
				else
				{
					g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_016;
					HookClientDllInterface_011_Init(iface);
				}
			}
			else if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_015, NULL)) {
				g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_015;
				HookClientDllInterface_011_Init(iface);
			}
			else if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_013, NULL)) {
				g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_013;
				HookClientDllInterface_011_Init(iface);
			}
			else if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_012, NULL)) {
				g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_012;
				HookClientDllInterface_011_Init(iface);
			}
			else if(iface = old_Client_CreateInterface(CLIENT_DLL_INTERFACE_VERSION_011, NULL)) {
				g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_011;
				HookClientDllInterface_011_Init(iface);
			}
			else
			{
				ErrorBox("Could not get a supported VClient interface.");
			}

			if (iface
				&& (SourceSdkVer_TF2 == g_SourceSdkVer
					|| SourceSdkVer_Momentum == g_SourceSdkVer
				)
			) {
				int * vtable = *(int**)iface;

				AfxDetourPtr((PVOID*) & (vtable[2]), new_CVClient_Shutdown_TF2, (PVOID*)&old_CVClient_Shutdown_TF2);
				AfxDetourPtr((PVOID*) & (vtable[35]), new_CVClient_FrameStageNotify_TF2, (PVOID*)&old_CVClient_FrameStageNotify_TF2);
			}

			if (iface && SourceSdkVer_CSS == g_SourceSdkVer)
			{
				int* vtable = *(int**)iface;

				AfxDetourPtr((PVOID*)&(vtable[2]), new_CVClient_Shutdown_CSS, (PVOID*)&old_CVClient_Shutdown_CSS);
				AfxDetourPtr((PVOID*)&(vtable[35]), new_CVClient_FrameStageNotify_CSS, (PVOID*)&old_CVClient_FrameStageNotify_CSS);
			}

			if (iface && SourceSdkVer_CSSV34 == g_SourceSdkVer)
			{
				int * vtable = *(int**)iface;

				AfxDetourPtr((PVOID*) & (vtable[1]), new_CVClient_Shutdown_CSSV34, (PVOID*)&old_CVClient_Shutdown_CSSV34);
				AfxDetourPtr((PVOID*) & (vtable[32]), new_CVClient_FrameStageNotify_CSSV34, (PVOID*)&old_CVClient_FrameStageNotify_CSSV34);
			}
		}
		if(SourceSdkVer_CSGO == g_SourceSdkVer)
		{
			// isCsgo.

			SOURCESDK::g_Entitylist_csgo = (SOURCESDK::IClientEntityList_csgo *)old_Client_CreateInterface(VCLIENTENTITYLIST_INTERFACE_VERSION_CSGO, NULL);

			if (SOURCESDK::CSGO::IClientTools * iface = (SOURCESDK::CSGO::IClientTools *)old_Client_CreateInterface(SOURCESDK_CSGO_VCLIENTTOOLS_INTERFACE_VERSION, NULL))
				new CClientToolsCsgo(iface);
		}
		if (SourceSdkVer_TF2 == g_SourceSdkVer) {

			if (SOURCESDK::TF2::IClientTools * iface = (SOURCESDK::TF2::IClientTools *)old_Client_CreateInterface(SOURCESDK_TF2_VCLIENTTOOLS_INTERFACE_VERSION, NULL))
				new CClientToolsTf2(iface);
		}
		if (SourceSdkVer_Momentum == g_SourceSdkVer) {

			if (SOURCESDK::TF2::IClientTools* iface = (SOURCESDK::TF2::IClientTools*)old_Client_CreateInterface(SOURCESDK_TF2_VCLIENTTOOLS_INTERFACE_VERSION, NULL))
				new CClientToolsMom(iface);
		}
		if (SourceSdkVer_CSS == g_SourceSdkVer)
		{

			if (SOURCESDK::CSS::IClientTools* iface = (SOURCESDK::CSS::IClientTools*)old_Client_CreateInterface(SOURCESDK_CSS_VCLIENTTOOLS_INTERFACE_VERSION, NULL))
				new CClientToolsCss(iface);
		}
		if(SourceSdkVer_CSSV34 == g_SourceSdkVer)
		{

			if (SOURCESDK::CSSV34::IClientTools * iface = (SOURCESDK::CSSV34::IClientTools *)old_Client_CreateInterface(SOURCESDK_CSSV34_VCLIENTTOOLS_INTERFACE_VERSION, NULL))
				new CClientToolsCssV34(iface);
		}
	}

	if(SourceSdkVer_CSGO == g_SourceSdkVer)
	{
		if(!g_AfxBaseClientDll && !strcmp(pName, CLIENT_DLL_INTERFACE_VERSION_CSGO_018))
		{
			g_Info_VClient = CLIENT_DLL_INTERFACE_VERSION_CSGO_018 " (CS:GO)";
			g_AfxBaseClientDll = new CAfxBaseClientDll((SOURCESDK::IBaseClientDLL_csgo *)pRet);
			g_AfxStreams.OnAfxBaseClientDll(g_AfxBaseClientDll);

			pRet = g_AfxBaseClientDll;
		}
		/*
		else if (!g_AfxCsgoPrediction && !strcmp(SOURCESDK_CSGO_VCLIENT_PREDICTION_INTERFACE_VERSION, pName))
		{
			g_AfxCsgoPrediction = new CAfxCsgoPrediction((SOURCESDK::CSGO::IPrediction *)pRet);

			pRet = g_AfxCsgoPrediction;
		}
		*/
	}

	return pRet;
}


HMODULE g_H_ClientDll = 0;

FARPROC WINAPI new_Engine_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

	if (!nResult)
		return nResult;

	if (HIWORD(lpProcName))
	{
		if (!lstrcmp(lpProcName, "GetProcAddress"))
			return (FARPROC) &new_Engine_GetProcAddress;

		if (
			hModule == g_H_ClientDll
			&& !lstrcmp(lpProcName, "CreateInterface")
		) {
			old_Client_CreateInterface = (SOURCESDK::CreateInterfaceFn)nResult;
			return (FARPROC) &new_Client_CreateInterface;
		}

	}

	return nResult;
}

WNDPROC g_NextWindProc;
static bool g_afxWindowProcSet = false;

LRESULT CALLBACK new_Afx_WindowProc(
	__in HWND hwnd,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
)
{
	if (AfxHookSource::Gui::WndProcHandler(hwnd, uMsg, wParam, lParam))
		return 0;

	switch(uMsg)
	{
	case WM_ACTIVATE:
		g_AfxHookSourceInput.Supply_Focus(LOWORD(wParam) != 0);
		break;
	case WM_CHAR:
		if(g_AfxHookSourceInput.Supply_CharEvent(wParam, lParam))
			return 0;
		break;
	case WM_KEYDOWN:
		if(g_AfxHookSourceInput.Supply_KeyEvent(AfxHookSourceInput::KS_DOWN, wParam, lParam))
			return 0;
		break;
	case WM_KEYUP:
		if(g_AfxHookSourceInput.Supply_KeyEvent(AfxHookSourceInput::KS_UP,wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
		if (g_AfxHookSourceInput.Supply_MouseEvent(uMsg, wParam, lParam))
			return 0;
		break;
	case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			UINT getRawInputResult = GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			if(-1 != getRawInputResult && inp.header.dwType == RIM_TYPEMOUSE)
			{
				RAWMOUSE * rawmouse = &inp.data.mouse;
				LONG dX, dY;

				if((rawmouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
				{
					dX = rawmouse->lLastX;
					dY = rawmouse->lLastY;
				}
				else
				{
					static bool initial = true;
					static LONG lastX = 0;
					static LONG lastY = 0;

					if(initial)
					{
						initial = false;
						lastX = rawmouse->lLastX;
						lastY = rawmouse->lLastY;
					}

					dX = rawmouse->lLastX -lastX;
					dY = rawmouse->lLastY -lastY;

					lastX = rawmouse->lLastX;
					lastY = rawmouse->lLastY;
				}

				if (g_AfxHookSourceInput.Supply_RawMouseMotion(dX, dY))
					return DefWindowProcW(hwnd, uMsg, wParam, lParam);
			}
		}
		break;
	}
	return CallWindowProcW(g_NextWindProc, hwnd, uMsg, wParam, lParam);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG WINAPI new_GetWindowLongW(
	__in HWND hWnd,
	__in int nIndex
)
{
	if(nIndex == GWL_WNDPROC)
	{
		if(g_afxWindowProcSet)
		{
			return (LONG)g_NextWindProc;
		}
	}

	return GetWindowLongW(hWnd, nIndex);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG WINAPI new_SetWindowLongW(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong
)
{
	if(nIndex == GWL_WNDPROC)
	{
		LONG lResult = SetWindowLongW(hWnd, nIndex, (LONG)new_Afx_WindowProc);

		if(!g_afxWindowProcSet)
		{
			g_afxWindowProcSet = true;
		}
		else
		{
			lResult = (LONG)g_NextWindProc;
		}

		g_NextWindProc = (WNDPROC)dwNewLong;

		return lResult;
	}

	return SetWindowLongW(hWnd, nIndex, dwNewLong);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG WINAPI new_GetWindowLongA(
	__in HWND hWnd,
	__in int nIndex
)
{
	if (nIndex == GWL_WNDPROC)
	{
		if (g_afxWindowProcSet)
		{
			return (LONG)g_NextWindProc;
		}
	}

	return GetWindowLongA(hWnd, nIndex);
}

// TODO: this is risky, actually we should track the hWnd maybe.
LONG WINAPI new_SetWindowLongA(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong
)
{
	if (nIndex == GWL_WNDPROC)
	{
		LONG lResult = SetWindowLongA(hWnd, nIndex, (LONG)new_Afx_WindowProc);

		if (!g_afxWindowProcSet)
		{
			g_afxWindowProcSet = true;
		}
		else
		{
			lResult = (LONG)g_NextWindProc;
		}

		g_NextWindProc = (WNDPROC)dwNewLong;

		return lResult;
	}

	return SetWindowLongA(hWnd, nIndex, dwNewLong);
}

BOOL WINAPI new_GetCursorPos(
	__out LPPOINT lpPoint
)
{
	BOOL result = GetCursorPos(lpPoint);

	if (AfxHookSource::Gui::OnGetCursorPos(lpPoint))
		return TRUE;

	g_AfxHookSourceInput.Supply_GetCursorPos(lpPoint);

	return result;
}

BOOL WINAPI new_SetCursorPos(
	__in int X,
	__in int Y
)
{
	if (AfxHookSource::Gui::OnSetCursorPos(X, Y))
		return TRUE;

	g_AfxHookSourceInput.Supply_SetCursorPos(X,Y);

	return SetCursorPos(X,Y);
}

HCURSOR WINAPI new_SetCursor(__in_opt HCURSOR hCursor)
{
	HCURSOR result;

	if (AfxHookSource::Gui::OnSetCursor(hCursor, result))
		return result;

	return SetCursor(hCursor);
}

HWND WINAPI new_SetCapture(__in HWND hWnd)
{
	HWND result;

	if (AfxHookSource::Gui::OnSetCapture(hWnd, result))
		return result;

	return SetCapture(hWnd);
}

BOOL WINAPI new_ReleaseCapture()
{
	if (AfxHookSource::Gui::OnReleaseCapture())
		return TRUE;

	return ReleaseCapture();
}

FARPROC WINAPI new_shaderapidx9_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

	if (!nResult)
		return nResult; // This can happen on Windows XP for Direct3DCreateEx9

	if (HIWORD(lpProcName))
	{
		if (
			!lstrcmp(lpProcName, "Direct3DCreate9Ex")
		) {
			old_Direct3DCreate9Ex = (Direct3DCreate9Ex_t)nResult;
			return (FARPROC) &new_Direct3DCreate9Ex;
		}
	}

	return nResult;
}


bool g_b_Suppress_csgo_engine_Do_CCLCMsg_FileCRCCheck = true;
typedef void (__fastcall * csgo_engine_Do_CCLCMsg_FileCRCCheck_t)(void * This, void * Edx);
csgo_engine_Do_CCLCMsg_FileCRCCheck_t g_Org_csgo_engine_Do_CCLCMsg_FileCRCCheck = nullptr;

void __fastcall My_csgo_engine_Do_CCLCMsg_FileCRCCheck(void * This, void * Edx) {
	if(g_b_Suppress_csgo_engine_Do_CCLCMsg_FileCRCCheck) return;

	g_Org_csgo_engine_Do_CCLCMsg_FileCRCCheck(This, Edx);
}

bool Install_csgo_engine_Do_CCLCMsg_FileCRCCheck() {
	static bool firstRun = true;
	static bool firstResult = false;

	if (firstRun) {
		firstRun = false;
		if(0 != AFXADDR_GET(csgo_engine_Do_CCLCMsg_FileCRCCheck)) {
			g_Org_csgo_engine_Do_CCLCMsg_FileCRCCheck = (csgo_engine_Do_CCLCMsg_FileCRCCheck_t)AFXADDR_GET(csgo_engine_Do_CCLCMsg_FileCRCCheck);
		
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_Org_csgo_engine_Do_CCLCMsg_FileCRCCheck, My_csgo_engine_Do_CCLCMsg_FileCRCCheck);

			firstResult = NO_ERROR == DetourTransactionCommit();

		}
	}

	return firstResult;
}


HMODULE WINAPI new_LoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

extern HMODULE g_H_EngineDll;

typedef void(__fastcall * csgo_ICommandLine_RemoveParm_t)(void * This, void * edx, const char *parm);

csgo_ICommandLine_RemoveParm_t True_csgo_ICommandLine_RemoveParam;

void __fastcall My_csgo_ICommandLine_RemoveParam(void * This, void * edx, const char *parm)
{
	// This hook is not active atm. because it's purpose is gone.

	//if (0 == _stricmp("-scaleform", parm))
	//	return;

	True_csgo_ICommandLine_RemoveParam(This, edx, parm);
}

typedef void csgo_ICommandLine_t;

typedef csgo_ICommandLine_t * (*csgo_CommandLine_t)();

CAfxImportFuncHook<BOOL (WINAPI *)(LPPOINT)> g_Import_Tier0_USER32_GetCursorPos("GetCursorPos", new_GetCursorPos);
CAfxImportFuncHook<BOOL (WINAPI *)(int, int)> g_Import_Tier0_USER32_SetCursorPos("SetCursorPos", new_SetCursorPos);
CAfxImportDllHook g_Import_Tier0_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_Tier0_USER32_GetCursorPos
	, & g_Import_Tier0_USER32_SetCursorPos}));
CAfxImportsHook g_Import_Tier0(CAfxImportsHooks({
	&g_Import_Tier0_USER32}));


void CommonHooks()
{
	static bool bFirstRun = true;
	static bool bFirstTier0 = true;

	// do not use messageboxes here, there is some friggin hooking going on in between by the
	// Source engine.

	if (bFirstRun)
	{
		bFirstRun = false;

		// detect if we are csgo:

		char filePath[MAX_PATH] = { 0 };
		GetModuleFileName(0, filePath, MAX_PATH);

		if (int gameIdx = g_CommandLine->FindParam(L"-afxGame"))
		{
			++gameIdx;
			if (gameIdx < g_CommandLine->GetArgC())
			{
				const wchar_t* game = g_CommandLine->GetArgV(gameIdx);
				if (0 == _wcsicmp(L"tf", game))
					g_SourceSdkVer = SourceSdkVer_TF2;
				else if (0 == _wcsicmp(L"csgo", game))
					g_SourceSdkVer = SourceSdkVer_CSGO;
				else if (0 == _wcsicmp(L"css", game))
					g_SourceSdkVer = SourceSdkVer_CSS;
				else if (0 == _wcsicmp(L"css_v34", game))
					g_SourceSdkVer = SourceSdkVer_CSSV34;
				else if (0 == _wcsicmp(L"garrysmod", game))
					g_SourceSdkVer = SourceSdkVer_Garrysmod;
				else if (0 == _wcsicmp(L"swarm", game))
					g_SourceSdkVer = SourceSdkVer_SWARM;
				else if (0 == _wcsicmp(L"l4d2", game))
					g_SourceSdkVer = SourceSdkVer_L4D2;
				else if (0 == _wcsicmp(L"bm", game))
					g_SourceSdkVer = SourceSdkVer_BM;
				else if (0 == _wcsicmp(L"insurgency", game))
					g_SourceSdkVer = SourceSdkVer_Insurgency2;
				else if (0 == _wcsicmp(L"momentum", game))
					g_SourceSdkVer = SourceSdkVer_Momentum;
			}
		}
		else if (g_CommandLine->FindParam(L"-afxV34"))
		{
			g_SourceSdkVer = SourceSdkVer_CSSV34;
		}
		else if (StringIEndsWith(filePath, "csgo.exe"))
		{
			g_SourceSdkVer = SourceSdkVer_CSGO;
		}
		else if (StringIEndsWith(filePath, "swarm.exe"))
		{
			g_SourceSdkVer = SourceSdkVer_SWARM;
		}
		else if (StringIEndsWith(filePath, "left4dead2.exe"))
		{
			g_SourceSdkVer = SourceSdkVer_L4D2;
		}
		else if (StringIEndsWith(filePath, "bms.exe"))
		{
			g_SourceSdkVer = SourceSdkVer_BM;
		}
		else if (StringIEndsWith(filePath, "insurgency.exe"))
		{
			g_SourceSdkVer = SourceSdkVer_Insurgency2;
		}
		else if (int gameIdx = g_CommandLine->FindParam(L"-game"))
		{
			++gameIdx;
			if (gameIdx < g_CommandLine->GetArgC())
			{
				const wchar_t* game = g_CommandLine->GetArgV(gameIdx);
				if(0 == _wcsicmp(L"tf", game))
					g_SourceSdkVer = SourceSdkVer_TF2;
				else if (0 == _wcsicmp(L"cstrike", game))
					g_SourceSdkVer = SourceSdkVer_CSS;
				else if (0 == _wcsicmp(L"garrysmod", game))
					g_SourceSdkVer = SourceSdkVer_Garrysmod;
				else if (0 == _wcsicmp(L"momentum", game))
					g_SourceSdkVer = SourceSdkVer_Momentum;
			}
		}


		//ScriptEngine_StartUp();
	}

	if (bFirstTier0)
	{
		HMODULE hTier0;
		if (hTier0 = GetModuleHandleA("tier0.dll"))
		{
			bFirstTier0 = false;

			advancedfx::Message = Tier0_Msg = (Tier0MsgFn)GetProcAddress(hTier0, "Msg");
			advancedfx::Warning = Tier0_Warning = (Tier0MsgFn)GetProcAddress(hTier0, "Warning");
			Tier0_Error = (Tier0MsgFn)GetProcAddress(hTier0, "Error");

			advancedfx::DevMessage = Tier0_DevMsg = (Tier0DevMsgFn)GetProcAddress(hTier0, "DevMsg");
			advancedfx::DevWarning = Tier0_DevWarning = (Tier0DevMsgFn)GetProcAddress(hTier0, "DevWarning");

			if (SourceSdkVer_CSSV34 == g_SourceSdkVer)
			{
				g_Import_Tier0.Apply(hTier0);
			}

			if (SourceSdkVer_CSGO == g_SourceSdkVer)
			{
				SOURCESDK::CSGO::g_pMemAlloc = *(SOURCESDK::CSGO::IMemAlloc **)GetProcAddress(hTier0, "g_pMemAlloc");

				/*
				// Scaleform hook is pointless, it just crashes now.
				if (csgo_CommandLine_t commandLine = (csgo_CommandLine_t)GetProcAddress(hTier0, "CommandLine")) {

					csgo_ICommandLine_t * iCommandLine = commandLine();

					LONG error = NO_ERROR;

					True_csgo_ICommandLine_RemoveParam = (csgo_ICommandLine_RemoveParm_t)*(DWORD *)(*(DWORD *)iCommandLine +0x14);

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());
					DetourAttach(&(PVOID&)True_csgo_ICommandLine_RemoveParam, My_csgo_ICommandLine_RemoveParam);
					error = DetourTransactionCommit();

					if (NO_ERROR != error)
						ErrorBox("Could not detour tier0!Commandline.");

				}
				else ErrorBox("Could not find tier0!Commandline.");
				*/
			}

			if (SourceSdkVer_SWARM == g_SourceSdkVer)
			{
				SOURCESDK::SWARM::g_pMemAlloc = *(SOURCESDK::SWARM::IMemAlloc **)GetProcAddress(hTier0, "g_pMemAlloc");
			}
			else if( SourceSdkVer_BM == g_SourceSdkVer )
			{
				if( SOURCESDK::BM::IMemAlloc ** ppMemalloc = (SOURCESDK::BM::IMemAlloc **)GetProcAddress(hTier0, "g_pMemAlloc") )
				{
					SOURCESDK::BM::g_pMemAlloc = *ppMemalloc;
				}
			}
			else // default fallback // if (SourceSdkVer_L4D2 == g_SourceSdkVer)
			{
				if (SOURCESDK::L4D2::IMemAlloc** ppMemalloc = (SOURCESDK::L4D2::IMemAlloc**)GetProcAddress(hTier0, "g_pMemAlloc"))
				{
					SOURCESDK::L4D2::g_pMemAlloc = *ppMemalloc;
				}
			}

		}
	}
}

CAfxImportFuncHook<HMODULE (WINAPI *)(LPCSTR)> g_Import_launcher_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE (WINAPI *)(LPCSTR, HANDLE, DWORD)> g_Import_launcher_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_launcher_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_launcher_KERNEL32_LoadLibraryA
	, &g_Import_launcher_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_launcher(CAfxImportsHooks({
	&g_Import_launcher_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_filesystem_steam_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_filesystem_steam_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_filesystem_steam_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_filesystem_steam_KERNEL32_LoadLibraryA
	, &g_Import_filesystem_steam_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_filesystem_steam(CAfxImportsHooks({
	&g_Import_filesystem_steam_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_filesystem_stdio_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_filesystem_stdio_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_filesystem_stdio_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_filesystem_stdio_KERNEL32_LoadLibraryA
	, &g_Import_filesystem_stdio_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_filesystem_stdio(CAfxImportsHooks({
	&g_Import_filesystem_stdio_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_engine_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_engine_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_engine_KERNEL32_GetProcAddress("GetProcAddress", &new_Engine_GetProcAddress);

CAfxImportDllHook g_Import_engine_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_engine_KERNEL32_LoadLibraryA
	, &g_Import_engine_KERNEL32_LoadLibraryExA
	, &g_Import_engine_KERNEL32_GetProcAddress }));

// actually this is not required, since engine.dll calls first and thus is lower in the chain:
CAfxImportFuncHook<LONG(WINAPI*)(HWND, int)> g_Import_engine_USER32_GetWindowLongW("GetWindowLongW", &new_GetWindowLongW);
CAfxImportFuncHook<LONG(WINAPI*)(HWND, int, LONG)> g_Import_engine_USER32_SetWindowLongW("SetWindowLongW", &new_SetWindowLongW);

CAfxImportFuncHook<HCURSOR(WINAPI*)(HCURSOR)> g_Import_engine_USER32_SetCursor("SetCursor", &new_SetCursor);
CAfxImportFuncHook<HWND(WINAPI*)(HWND)> g_Import_engine_USER32_SetCapture("SetCapture", &new_SetCapture);
CAfxImportFuncHook<BOOL(WINAPI*)()> g_Import_engine_USER32_ReleaseCapture("ReleaseCapture", &new_ReleaseCapture);

// CSSV34 only:
CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_engine_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos); // not there, but heh.
CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_engine_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);

CAfxImportDllHook g_Import_engine_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_engine_USER32_GetWindowLongW,
	&g_Import_engine_USER32_SetWindowLongW,
	&g_Import_engine_USER32_SetCursor,
	&g_Import_engine_USER32_SetCapture,
	&g_Import_engine_USER32_ReleaseCapture }));


CAfxImportsHook g_Import_engine(CAfxImportsHooks({
	&g_Import_engine_KERNEL32,
	&g_Import_engine_USER32 }));


CAfxImportFuncHook<LONG(WINAPI*)(HWND, int)> g_Import_inputsystem_USER32_GetWindowLongW("GetWindowLongW", &new_GetWindowLongW);
CAfxImportFuncHook<LONG(WINAPI*)(HWND, int, LONG)> g_Import_inputsystem_USER32_SetWindowLongW("SetWindowLongW", &new_SetWindowLongW);

// CSSV34 only:
CAfxImportFuncHook<LONG(WINAPI*)(HWND, int)> g_Import_inputsystem_USER32_GetWindowLongA("GetWindowLongA", &new_GetWindowLongA);
CAfxImportFuncHook<LONG(WINAPI*)(HWND, int, LONG)> g_Import_inputsystem_USER32_SetWindowLongA("SetWindowLongA", &new_SetWindowLongA);

CAfxImportFuncHook<HCURSOR(WINAPI*)(HCURSOR)> g_Import_inputsystem_USER32_SetCursor("SetCursor", &new_SetCursor);
CAfxImportFuncHook<HWND(WINAPI*)(HWND)> g_Import_inputsystem_USER32_SetCapture("SetCapture", &new_SetCapture);
CAfxImportFuncHook<BOOL(WINAPI*)()> g_Import_inputsystem_USER32_ReleaseCapture("ReleaseCapture", &new_ReleaseCapture);
CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_inputsystem_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos); // not there, but heh.
CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_inputsystem_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);

CAfxImportDllHook g_Import_inputsystem_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_inputsystem_USER32_GetWindowLongW,
	&g_Import_inputsystem_USER32_SetWindowLongW,
	&g_Import_inputsystem_USER32_SetCursor,
	&g_Import_inputsystem_USER32_SetCapture,
	&g_Import_inputsystem_USER32_ReleaseCapture,
	&g_Import_inputsystem_USER32_GetCursorPos,
	&g_Import_inputsystem_USER32_SetCursorPos }));

CAfxImportsHook g_Import_inputsystem(CAfxImportsHooks({
	& g_Import_inputsystem_USER32 }));


CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_materialsystem_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_materialsystem_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_materialsystem_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_materialsystem_KERNEL32_LoadLibraryA
	, &g_Import_materialsystem_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_materialsystem(CAfxImportsHooks({
	&g_Import_materialsystem_KERNEL32 }));


CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_shaderapidx9_KERNEL32_GetProcAddress("GetProcAddress", &new_shaderapidx9_GetProcAddress);

CAfxImportDllHook g_Import_shaderapidx9_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_shaderapidx9_KERNEL32_GetProcAddress }));

CAfxImportFuncHook<IDirect3D9 * (WINAPI*)(UINT)> g_Import_shaderapidx9_d3d9_Direct3DCreate9("Direct3DCreate9", &new_Direct3DCreate9);

CAfxImportDllHook g_Import_shaderapidx9_d3d9("d3d9.dll", CAfxImportDllHooks({
	&g_Import_shaderapidx9_d3d9_Direct3DCreate9 }));

CAfxImportsHook g_Import_shaderapidx9(CAfxImportsHooks({
	&g_Import_shaderapidx9_KERNEL32,
	&g_Import_shaderapidx9_d3d9}));


CAfxImportFuncHook<HCURSOR(WINAPI*)(HCURSOR)> g_Import_vgui2_USER32_SetCursor("SetCursor", &new_SetCursor);
CAfxImportFuncHook<HWND(WINAPI*)(HWND)> g_Import_vgui2_USER32_SetCapture("SetCapture", &new_SetCapture);
CAfxImportFuncHook<BOOL(WINAPI*)()> g_Import_vgui2_USER32_ReleaseCapture("ReleaseCapture", &new_ReleaseCapture);
CAfxImportFuncHook<BOOL(WINAPI*)(LPPOINT)> g_Import_vgui2_USER32_GetCursorPos("GetCursorPos", &new_GetCursorPos); // not there, but heh.
CAfxImportFuncHook<BOOL(WINAPI*)(int, int)> g_Import_vgui2_USER32_SetCursorPos("SetCursorPos", &new_SetCursorPos);

CAfxImportDllHook g_Import_vgui2_USER32("USER32.dll", CAfxImportDllHooks({
	&g_Import_vgui2_USER32_SetCursor,
	&g_Import_vgui2_USER32_SetCapture,
	&g_Import_vgui2_USER32_ReleaseCapture,
	&g_Import_vgui2_USER32_GetCursorPos,
	&g_Import_vgui2_USER32_SetCursorPos }));

CAfxImportsHook g_Import_vgui2(CAfxImportsHooks({
	&g_Import_vgui2_USER32 }));

void LibraryHooksA(HMODULE hModule, LPCSTR lpLibFileName)
{
	static bool bFirstLauncher = true;
	static bool bFirstClient = true;
	static bool bFirstEngine = true;
	static bool bFirstInputsystem = true;
	//static bool bFirstGameOverlayRenderer = true;
	static bool bFirstFileSystemSteam = true;
	static bool bFirstfilesystem_stdio = true;
	static bool bFirstShaderapidx9 = true;
	static bool bFirstMaterialsystem = true;
	static bool bFirstPanorama = true;
	static bool bFirstStdshader_dx9 = true;
	static bool bFirstVgui2 = true;

	CommonHooks();

	if(!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1=NULL;

	if( !f1 ) f1=fopen("hlae_log_LibraryHooksA.txt","wb");
	fprintf(f1,"%s\n", lpLibFileName);
	fflush(f1);
#endif

	if (bFirstLauncher && StringEndsWith(lpLibFileName, "launcher.dll"))
	{
		bFirstLauncher = false;

		g_Import_launcher.Apply(hModule);
	}
	else if(bFirstFileSystemSteam && StringEndsWith( lpLibFileName, "filesystem_steam.dll")) // v34
	{
		bFirstFileSystemSteam = false;

		g_Import_filesystem_steam.Apply(hModule);
	}
	else if(bFirstfilesystem_stdio && StringEndsWith( lpLibFileName, "filesystem_stdio.dll"))
	{
		bFirstfilesystem_stdio = false;
		
		g_Import_filesystem_stdio.Apply(hModule);
	}
	else if(bFirstEngine && StringEndsWith( lpLibFileName, "engine.dll"))
	{
		bFirstEngine = false;

		g_H_EngineDll = hModule;

		Addresses_InitEngineDll((AfxAddr)hModule, g_SourceSdkVer);

		if (SourceSdkVer_CSSV34 == g_SourceSdkVer)
		{
			g_Import_engine_USER32.Add(CAfxImportDllHooks({
				&g_Import_engine_USER32_GetCursorPos,
				&g_Import_engine_USER32_SetCursorPos }));
		}

		g_Import_engine.Apply(hModule);

		// Init the hook early, so we don't run into issues with threading:
		Hook_csgo_SndMixTimeScalePatch();
		csgo_Audio_Install();
		//Hook_csgo_DemoFile();

		if(SourceSdkVer_CSGO == g_SourceSdkVer) {
			Install_csgo_engine_Do_CCLCMsg_FileCRCCheck();
			Install_csgo_Cmd_ExecuteCommand();
		}
	}
	else if(bFirstInputsystem && StringEndsWith( lpLibFileName, "inputsystem.dll"))
	{
		bFirstInputsystem = false;

		if (SourceSdkVer_CSSV34 == g_SourceSdkVer)
		{
			g_Import_inputsystem_USER32.Add(CAfxImportDllHooks({
				&g_Import_inputsystem_USER32_GetWindowLongA,
				&g_Import_inputsystem_USER32_SetWindowLongA }));
		}

		g_Import_inputsystem.Apply(hModule);
	}
	else if(bFirstMaterialsystem && StringEndsWith( lpLibFileName, "materialsystem.dll"))
	{
		bFirstMaterialsystem = false;

		Addresses_InitMaterialsystemDll((AfxAddr)hModule, g_SourceSdkVer);

		g_Import_materialsystem.Apply(hModule);
	}
	else if(bFirstShaderapidx9 && StringEndsWith( lpLibFileName, "shaderapidx9.dll"))
	{
		bFirstShaderapidx9 = false;

		g_Import_shaderapidx9.Apply(hModule);

		old_Direct3DCreate9 = (Direct3DCreate9_t)g_Import_shaderapidx9_d3d9_Direct3DCreate9.TrueFunc;
	}
	else if(bFirstClient && (StringEndsWith( lpLibFileName, "client_panorama.dll") || SourceSdkVer_CSGO != g_SourceSdkVer && StringEndsWith(lpLibFileName, "client.dll") || StringEndsWith(lpLibFileName, "csgo\\bin\\client.dll")))
	{
		bFirstClient = false;

		g_H_ClientDll = hModule;

		Addresses_InitClientDll((AfxAddr)g_H_ClientDll, g_SourceSdkVer);

		//
		// Install early hooks:

		csgo_CSkyBoxView_Draw_Install();
		csgo_CViewRender_Install();
		Hook_csgo_PlayerAnimStateFix();
		csgo_CRendering3dView_Install();
	}
	else if(bFirstPanorama && StringEndsWith( lpLibFileName, "panorama.dll"))
	{
		bFirstPanorama = false;

		Addresses_InitPanoramaDll((AfxAddr)hModule, g_SourceSdkVer);

		//
		// Install hooks:

		PanoramaHooks_Install();
	}
	else if(bFirstStdshader_dx9 && StringEndsWith( lpLibFileName, "stdshader_dx9.dll"))
	{
		bFirstStdshader_dx9 = false;

		//Addresses_InitStdshader_dx9Dll((AfxAddr)hModule, isCsgo);

		//
		// Install early hooks:

		//csgo_Stdshader_dx9_Hooks_Init();
	}
	else if(bFirstVgui2 && StringEndsWith( lpLibFileName, "vgui2.dll"))
	{
		bFirstVgui2 = false;

		g_Import_vgui2.Apply(hModule);
	}
}

void LibraryHooksW(HMODULE hModule, LPCWSTR lpLibFileName)
{
	static bool bFirstLauncher = true;

	CommonHooks();

	if (!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1 = NULL;

	if (!f1) f1 = fopen("hlae_log_LibraryHooksW.txt", "wb");
	fwprintf(f1, L"%s\n", lpLibFileName);
	fflush(f1);
#endif

	if (bFirstLauncher && StringEndsWithW(lpLibFileName, L"launcher.dll"))
	{
		bFirstLauncher = false;

		g_Import_launcher.Apply(hModule);
	}
}


// i.e. called by Counter-Strike Source
HMODULE WINAPI new_LoadLibraryA( LPCSTR lpLibFileName ) {
	HMODULE hRet = LoadLibraryA(lpLibFileName);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}


// i.e. called by Portal First Slice
HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE hRet = LoadLibraryExA(lpLibFileName, hFile, dwFlags);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}

// i.e. called by CS:GO since 5/4/2017:
HMODULE WINAPI new_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE hRet = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	LibraryHooksW(hRet, lpLibFileName);

	return hRet;
}

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_PROCESS_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_PROCESS_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD)> g_Import_PROCESS_KERNEL32_LoadLibraryExW("LoadLibraryExW", &new_LoadLibraryExW);

CAfxImportDllHook g_Import_PROCESS_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_PROCESS_KERNEL32_LoadLibraryA
	, &g_Import_PROCESS_KERNEL32_LoadLibraryExA
	, &g_Import_PROCESS_KERNEL32_LoadLibraryExW }));

CAfxImportsHook g_Import_PROCESS(CAfxImportsHooks({
	&g_Import_PROCESS_KERNEL32 }));

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason) 
	{ 
		case DLL_PROCESS_ATTACH:
		{
			g_CommandLine = new CAfxCommandLine();

			if(!g_CommandLine->FindParam(L"-insecure"))
			{
				ErrorBox("Please add -insecure to launch options, AfxHookSource will refuse to work without it!");

				HANDLE hproc = OpenProcess(PROCESS_TERMINATE, true, GetCurrentProcessId());
				TerminateProcess(hproc, 0);
				CloseHandle(hproc);
				
				do MessageBoxA(NULL, "Please terminate the game manually in the taskmanager!", "Cannot terminate, please help:", MB_OK | MB_ICONERROR);
				while (true);
			}

#ifdef _DEBUG
			MessageBox(0,"DLL_PROCESS_ATTACH","MDT_DEBUG",MB_OK);
#endif
			//break;

			g_Import_PROCESS.Apply(GetModuleHandle(NULL));

			if (!(g_Import_PROCESS_KERNEL32_LoadLibraryA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExW.TrueFunc))
				ErrorBox();

			//
			// Remember we are not on the main program thread here,
			// instead we are on our own thread, so don't run
			// things here that would have problems with that.
			//

#ifdef AFX_INTEROP
			AfxInterop::DllProcessAttach();
#endif

			AfxHookSource::Gui::DllProcessAttach();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			// actually this gets called now.

			MatRenderContextHook_Shutdown();

			if(g_AfxBaseClientDll) { delete g_AfxBaseClientDll; g_AfxBaseClientDll = 0; }

			AfxHookSource::Gui::DllProcessDetach();

			delete g_CommandLine;

#ifdef _DEBUG
			_CrtDumpMemoryLeaks();
#endif

			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	return TRUE;
}
