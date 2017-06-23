#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrect.h>
#include <cl_dll.h>
#include <cdll_int.h>
#include <r_efx.h>
#include <com_model.h>
#include <r_studioint.h>
#include <pm_defs.h>
#include <cvardef.h>
#include <entity_types.h>

#include "cmdregister.h"
#include "detours.h"

extern cl_enginefuncs_s *pEngfuncs;

namespace DemoPlayer
{

#define DEMOPLAYER_NAME					"demoplayer"
#define DEMOPLAYER_VERSION				"001"
#define DEMOPLAYER_DLL_NAME				DEMOPLAYER_NAME ".dll"
#define DEMOPLAYER_INTERFACE_NAME		DEMOPLAYER_NAME DEMOPLAYER_VERSION


// some parts from source sdk interface.h:

// interface return status
enum 
{
	IFACE_OK = 0,
	IFACE_FAILED
};

#define CREATEINTERFACE_PROCNAME	"CreateInterface"
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

typedef void UTYPE;

class IDemoPlayer
// 4*91 = 0x16c bytes

// Commands we know they exist (thx to Google.com):
//
// m_DemoPlayer->RemoveCommand( m_DrcCmd.m_Index );        // remove old command first
// m_DemoPlayer->AddCommand( &m_DrcCmd );
// IObjectContainer * cmdList = m_DemoPlayer->GetCommands();
// m_DemoPlayer->SetPaused( true );
// m_DemoPlayer->SetWorldTime( m_CurrentCmd->m_Time, false );
// m_DemoPlayer->ExecuteDirectorCmd( m_CurrentCmd );
// m_DemoPlayer->SetPaused( true );
// m_CurrentCmd = m_DemoPlayer->GetLastCommand();
// m_DemoPlayer->IsEditMode()
// m_DemoPlayer->RemoveListener( this );
// if ( !m_DemoPlayer->IsActive() )
// if ( m_DemoPlayer->IsLoading() )
// sprintf( title, "Loading %s ...", m_DemoPlayer->GetFileName() );
// double (/ float) worldTime = m_DemoPlayer->GetWorldTime();
// float   timeScale = m_DemoPlayer->GetTimeScale();
// m_DemoPlayer->SetWorldTime( sliderTime, false );
// if ( m_DemoPlayer->IsPaused() )
// m_DemoPlayer->RegisterListener( this );
// m_World = m_DemoPlayer->GetWorld();
// m_DemoPlayer->SetTimeScale( 1.0f );
// m_DemoPlayer->SetPaused( true );
// m_DemoPlayer->SaveGame( "demoedit.dem" );       // TODO, allow other name
// m_DemoPlayer->GetSerial();
// m_DemoPlayer->Stop();
// m_DemoPlayer->SetMasterMode( params->GetInt("state") );
// m_DemoPlayer->Stop();
//
// Source SDK demosmothersamplesource.cpp (probably not compatible)
// if ( !demoplayer->IsPlayingBack() )
// !demoplayer->IsPlaybackPaused()
{
	UTYPE unknownFn_0000(UTYPE);
	UTYPE unknownFn_0004(UTYPE);
	UTYPE unknownFn_0008(UTYPE);
	UTYPE unknownFn_000C(UTYPE);

	UTYPE unknownFn_0010(UTYPE);
	UTYPE unknownFn_0014(UTYPE);
	UTYPE unknownFn_0018(UTYPE);
	UTYPE unknownFn_001C(UTYPE);

	UTYPE unknownFn_0020(UTYPE);
	UTYPE unknownFn_0024(UTYPE);
	UTYPE unknownFn_0028(UTYPE);
	UTYPE unknownFn_002C(UTYPE);

	UTYPE unknownFn_0030(UTYPE); // double GetDemoTime(void) = 0;
	UTYPE unknownFn_0034(UTYPE);
	UTYPE unknownFn_0038(UTYPE);
	UTYPE unknownFn_003C(UTYPE);

	UTYPE unknownFn_0040(UTYPE);
	UTYPE unknownFn_0044(UTYPE);
	UTYPE unknownFn_0048(UTYPE);
	UTYPE unknownFn_004C(UTYPE);

	UTYPE unknownFn_0050(UTYPE);
	UTYPE unknownFn_0054(UTYPE);
	UTYPE unknownFn_0058(UTYPE);
	UTYPE unknownFn_005C(UTYPE);

	UTYPE unknownFn_0060(UTYPE);
	UTYPE unknownFn_0064(UTYPE);
	UTYPE unknownFn_0068(UTYPE);
	UTYPE unknownFn_006C(UTYPE);

	UTYPE unknownFn_0070(UTYPE);
	UTYPE unknownFn_0074(UTYPE);
	UTYPE unknownFn_0078(UTYPE);
	UTYPE unknownFn_007C(UTYPE);

	UTYPE unknownFn_0080(UTYPE);
	UTYPE unknownFn_0084(UTYPE);
	UTYPE unknownFn_0088(UTYPE);
	UTYPE unknownFn_008C(UTYPE);

	UTYPE unknownFn_0090(UTYPE);
	UTYPE unknownFn_0094(UTYPE);
	UTYPE unknownFn_0098(UTYPE);
	UTYPE unknownFn_009C(UTYPE);

	UTYPE unknownFn_0100(UTYPE);
	UTYPE unknownFn_0104(UTYPE);
	UTYPE unknownFn_0108(UTYPE);
	UTYPE unknownFn_010C(UTYPE);

	UTYPE unknownFn_0110(UTYPE);
	UTYPE unknownFn_0114(UTYPE);
	UTYPE unknownFn_0118(UTYPE);
	UTYPE unknownFn_011C(UTYPE);

	UTYPE unknownFn_0120(UTYPE);
	UTYPE unknownFn_0124(UTYPE);
	UTYPE unknownFn_0128(UTYPE);
	UTYPE unknownFn_012C(UTYPE);

	UTYPE unknownFn_0130(UTYPE);
	UTYPE unknownFn_0134(UTYPE);
	UTYPE unknownFn_0138(UTYPE);
	UTYPE unknownFn_013C(UTYPE);

	UTYPE unknownFn_0140(UTYPE);
	UTYPE unknownFn_0144(UTYPE);
	UTYPE unknownFn_0148(UTYPE);
	UTYPE unknownFn_014C(UTYPE);

	UTYPE unknownFn_0150(UTYPE);
	UTYPE unknownFn_0154(UTYPE);
	UTYPE unknownFn_0158(UTYPE);
	UTYPE unknownFn_015C(UTYPE);

	UTYPE unknownFn_0160(UTYPE);
	UTYPE unknownFn_0164(UTYPE);
	UTYPE unknownFn_0168(UTYPE);
	UTYPE unknownFn_016C(UTYPE);
};

IDemoPlayer *g_DemoPlayer_Interface=NULL;

REGISTER_DEBUGCMD_FUNC(test_demoplayer_iface)
{
	HMODULE hm=GetModuleHandle(DEMOPLAYER_DLL_NAME);
	FARPROC fp=GetProcAddress(hm,CREATEINTERFACE_PROCNAME);

	if(fp)
	{
		int iRetCode;
		void *pRet;
		CreateInterfaceFn CreateFn;
		
		CreateFn=(CreateInterfaceFn)fp;

		pRet=CreateFn(DEMOPLAYER_INTERFACE_NAME,&iRetCode);

		pEngfuncs->Con_Printf("demoplayer.dll::%s(%s,&iRetCode):\nreturn 0x%08x\n,iRetCode %i",CREATEINTERFACE_PROCNAME,DEMOPLAYER_INTERFACE_NAME,pRet,iRetCode);

		if (iRetCode==IFACE_OK)
		{
			g_DemoPlayer_Interface=(IDemoPlayer *)pRet;

		}

	}
	
}

}