/*
File        : temporaray_dominik.cpp
Last Change : n/a, we use a code versioning system
Started     : 2007-06-29 22:27:00
Authors     : Gavin Bramhill, Dominik Tugend
Project     : Mirv Demo Tool
Description : for testing purposes in specific the gui
*/

// 	g_nViewports: 0=Game Only 1=GameUI 2=VGUI2 Overlays 3=unused/unkown 4=VGUI 1 (not enabled in CS 1.6)
/* from srfcsdk / ienginevgui.h:
enum VGUIPANEL
{
        PANEL_ROOT = 0,
        PANEL_GAMEUIDLL,
        PANEL_CLIENTDLL,
        PANEL_TOOLS,
        PANEL_INGAMESCREENS
};
*/

#include "windows.h" // we need access to virtualprotect etc.

#include "wrect.h"
#include "cl_dll.h"
#include "cdll_int.h"
#include "r_efx.h"
#include "com_model.h"
#include "r_studioint.h"
#include "pm_defs.h"
#include "cvardef.h"
#include "entity_types.h"
#include "cmdregister.h"

//#include "vgui_TeamFortressViewport.h"
// #include "mdtgui.h"

extern cl_enginefuncs_s *pEngfuncs;
extern engine_studio_api_s *pEngStudio;
extern playermove_s* ppmove;

//extern TeamFortressViewport *gViewPort;
// MDTgui g_MDTgui; // global MDT gui object

//using namespace vgui;
//#include <VGUI_Panel.h>
//#include <VGUI_SurfaceBase.h>
//#include <VGUI_Surface.h>
//#include <VGUI_Desktop.h>
//#include <VGUI_MessageBox.h>
//#include <VGUI_App.h>

//>> Interfacing VGUI2

// srcsdk/public/tier0/platform.h
#ifndef _XBOX
#define abstract_class class
#else
#define abstract_class class NO_VTABLE
#endif

// srcsdk/public/vgui/vgui.h:
// forward declarations:
namespace vgui {
	typedef unsigned int VPANEL;
}

// srcsdk/public/ienginevgui.h:
namespace vgui
{
	class Panel;
};

enum VGuiPanel_t
{
	PANEL_ROOT = 0,
	PANEL_GAMEUIDLL,
	PANEL_CLIENTDLL,
	PANEL_TOOLS,
	PANEL_INGAMESCREENS,
	PANEL_GAMEDLL,
	PANEL_CLIENTDLL_TOOLS
};

// In-game panels are cropped to the current engine viewport size
enum PaintMode_t
{
	PAINT_UIPANELS		= (1<<0),
	PAINT_INGAMEPANELS  = (1<<1),
};

abstract_class IEngineVGui
{
public:
	virtual					~IEngineVGui( void ) { }

	virtual vgui::VPANEL	GetPanel( VGuiPanel_t type ) = 0;

	virtual bool			IsGameUIVisible() = 0;
};

#define VENGINE_VGUI_VERSION	"VEngineVGui001"

IEngineVGui* enginevgui;


#include "interface.h"

//>>IGameUI.h
struct cl_enginefuncs_s;
class IBaseSystem;      

namespace vgui2
{
class Panel;
}

//-----------------------------------------------------------------------------
// Purpose: contains all the functions that the GameUI dll exports
//                      GameUI_GetInterface() is exported via dll export table to get this table
//-----------------------------------------------------------------------------
class IGameUI : public IBaseInterface
{
public:
        virtual void Initialize( CreateInterfaceFn *factories, int count ) = 0;
        virtual void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion, IBaseSystem *system) = 0;
        virtual void Shutdown() = 0;

        virtual int     ActivateGameUI() = 0;   // activates the menus, returns 0 if it doesn't want to handle it
        virtual int     ActivateDemoUI() = 0;   // activates the demo player, returns 0 if it doesn't want to handle it

        virtual int     HasExclusiveInput() = 0;

        virtual void RunFrame() = 0;

        virtual void ConnectToServer(const char *game, int IP, int port) = 0;
        virtual void DisconnectFromServer() = 0;
        virtual void HideGameUI() = 0;

        virtual bool IsGameUIActive() = 0;
        
        virtual void LoadingStarted(const char *resourceType, const char *resourceName) = 0;
        virtual void LoadingFinished(const char *resourceType, const char *resourceName) = 0;

        virtual void StartProgressBar(const char *progressType, int numProgressPoints) = 0;
        virtual int      ContinueProgressBar(int progressPoint, float progressFraction) = 0;
        virtual void StopProgressBar(bool bError, const char *failureReasonIfAny, const char *extendedReason) = 0;
        virtual int  SetProgressBarStatusText(const char *statusText) = 0;
 
        virtual void GetSteamPassword( const char *szAccountName, const char *szUserName ) = 0;

        virtual void ValidateCDKey(bool force, bool inConnect) = 0;
};

// the interface version is the number to call GameUI_GetInterface(int interfaceNumber) with
#define GAMEUI_INTERFACE_VERSION "GameUI006"
#define GAMEUI_INTERFACE_VERSION_HACK "GameUI007" // this is risky
//<<IGameUI.h

//<< Interfacing VGUI2

void* gADDR_HL                    = (void*)0x01400000;
void* gADDR_HL_CreateInterface    = (void*)0x014013ef;

void* ADDR_GAMEUI_CreateInterface = (void*)0x0ee43b10; // can be filled in with getmodulhandle since it's an usual win32 module (no valve sh*t)

#ifdef _DEBUG
  // prints will be later moved to debug
#endif

//#include "temporary_gui.h"

//MDTbaseGUI g_MDTbaseGUI;


REGISTER_DEBUGCMD_FUNC(test_dominik)
{	
	extern cl_enginefuncs_s *pEngfuncs;
	extern engine_studio_api_s *pEngStudio;
	extern playermove_s* ppmove;
	float vorigin[3]; float vup[3]; float vright[3]; float vforward[3];

	pEngfuncs->Con_Printf("ppmove->origin (x,y,z) == (%f,%f,%f)\n",ppmove->origin.x,ppmove->origin.y,ppmove->origin.z);
	pEngStudio->GetViewInfo(vorigin,vup, vright, vforward );
	pEngfuncs->Con_Printf("pEngStudio->GetViewInfo: orign (x,y,z) == (%f,%f,%f)\n",vorigin[0],vorigin[1],vorigin[2]);
	pEngfuncs->Con_Printf("pEngStudio->GetViewInfo: right (x,y,z) == (%f,%f,%f)\n",vright[0],vright[1],vright[2]);
	pEngfuncs->Con_Printf("pEngStudio->GetViewInfo: up (x,y,z) == (%f,%f,%f)\n",vup[0],vup[1],vup[2]);
	pEngfuncs->Con_Printf("pEngStudio->GetViewInfo: forward (x,y,z) == (%f,%f,%f)\n",vforward[0],vforward[1],vforward[2]);

	return;

    // old test stuff:
	
	CreateInterfaceFn myHlIface = (CreateInterfaceFn)ADDR_GAMEUI_CreateInterface;
    	
	int myret=-1;
	IGameUI* myGameUI = (IGameUI*)myHlIface(GAMEUI_INTERFACE_VERSION_HACK,&myret);
	pEngfuncs->Con_Printf("myret: 0x%08x\n",myret);
	pEngfuncs->Con_Printf("myGameUI->IsGameUIActive(): %d\n",myGameUI->IsGameUIActive());
	
	pEngfuncs->Con_Printf("class address: 0x%08x\n",myGameUI);
	pEngfuncs->Con_Printf("factory address: 0x%08x\n",myHlIface);

	// see srcmain/GameUI/IGameUI_interface.h for full Info about class
	// we want to access m_FactoryList.
	// if class sits at            0x0EF8D9B0
	// then m_FactorList sits at - 0x0ef8dac0 (before that is num of factories (4 Byte))
	//                           = 0x110

	DWORD p_m_desired=((DWORD)((LPVOID)myGameUI))+0x110-0x04;
	//p_m_desired+=67; /* moves the pointer@ myGameUI+4*(0x110-0x04)/4*/
	pEngfuncs->Con_Printf("desired address: 0x%08x\n",p_m_desired);

	int m_iNumFactories;
	CreateInterfaceFn m_FactoryList[5];

//	DWORD dwOldProt;

//	VirtualProtect(&p_m_desired, 0x110+0x4, PAGE_READWRITE, &dwOldProt);
	m_iNumFactories = *((int *)((LPVOID)p_m_desired));
	pEngfuncs->Con_Printf("numfacories: %d\n",m_iNumFactories);

	for (int i=0;i<5;i++)
	{
		p_m_desired+=0x4;
		m_FactoryList[i]=*(CreateInterfaceFn *)(LPVOID)p_m_desired;
		pEngfuncs->Con_Printf("GameUI Factory %d: 0x%08x\n",i,m_FactoryList[i]);
	}
//	VirtualProtect(&p_m_desired, 0x110+0x4, dwOldProt, NULL);

	// m_FactoryList[ 0 ] == gameUIFactory (self) // works
	// m_FactoryList[ 1 ] == engineFactory //?
	// m_FactoryList[ 2 ] == vguiFactory //?
	// m_FactoryList[ 3 ] == fileSystemFactory
	// m_FactoryList[ 4 ] == clientFactory // works
	// caling factory 1 or 2 fails for an yet unknown reason, maybe it's not mapped into our
	// space or the factoryfunctions are slightly different.
	// you need to unprotect them before you can do s.th. warning can VAC u :o)

	myret=-1;
//	m_FactoryList[4](/*"GameClientExports001"*/VENGINE_VGUI_VERSION,&myret);

	DWORD dwOldProt,dwOldProt2;
	VirtualProtect(m_FactoryList[1],sizeof(CreateInterfaceFn), PAGE_READWRITE, &dwOldProt);
	//VirtualProtect(&myret,sizeof(int), PAGE_READWRITE, &dwOldProt);
	IEngineVGui* mygui=(IEngineVGui*)m_FactoryList[1](VENGINE_VGUI_VERSION,&myret);
	//VirtualProtect(&myret,sizeof(int), dwOldProt2, NULL);
	VirtualProtect(m_FactoryList[1],sizeof(CreateInterfaceFn), dwOldProt, NULL);
	pEngfuncs->Con_Printf("myret: %d\n",myret);

	vgui::VPANEL myClientPanel=mygui->GetPanel(PANEL_CLIENTDLL);

	pEngfuncs->Con_Printf("IEngineVGui->GetPanel(): 0x%08x\n",myClientPanel);

//	g_MDTbaseGUI.HookOnClient(myClientPanel);


//	vgui::VPANEL tstpanel = enginevgui->GetPanel(PANEL_INGAMESCREENS);

/*	vgui::Panel* guiroot = (vgui::Panel*)pEngfuncs->VGui_GetPanel();

	pEngfuncs->Con_Printf("pEngfuncs->VGui_GetPanel(): %d\n",guiroot);
//	pEngfuncs->Con_Printf("enginevgui->GetPanel(PANEL_INGAMESCREENS): %d\n",tstpanel);

	pEngfuncs->Con_Printf("guiroot->getChildCount(): %d\n",guiroot->getChildCount());
	pEngfuncs->Con_Printf("guiroot->isVisible(): %d\n",guiroot->isVisible());
	pEngfuncs->Con_Printf("guiroot->isVisibleUp(): %d\n",guiroot->isVisibleUp());
	pEngfuncs->Con_Printf("guiroot->isEnabled(): %d\n",guiroot->isEnabled());
	int x,y,w,t=0;
	guiroot->getBounds(x,y,w,t);
	pEngfuncs->Con_Printf("guiroot->getBounds(&x,&y,&w,&t): %d,%d,%d,%d\n",x,y,w,t);
	guiroot->setBgColor(0,0,100,200);

	vgui::Desktop* mydesk=new vgui::Desktop(0,0,800,600);
	vgui::SurfaceBase* surfbase = guiroot->getSurfaceBase();
	pEngfuncs->Con_Printf("guiroot: %d / surfbase->getPanel(): %d\n",guiroot,surfbase->getPanel());
	pEngfuncs->Con_Printf("surfbase->hasFocus(): %d\n",surfbase->hasFocus());

	
	mydesk->setParent(guiroot);
	mydesk->setVisible(true);

	vgui::MessageBox* mymsg = new vgui::MessageBox("MIRV Demo Tool","Hello World!",40,40);
	mymsg->setParent(mydesk);
	mymsg->setVisible (true);

	surfbase->setFullscreenMode(800,600,32);*/
}

/*using namespace vgui;

void MDTgui::Show()
{	
	Panel* guiroot = (Panel*)pEngfuncs->VGui_GetPanel();

	pEngfuncs->Con_Printf("guiroot->getChildCount(): %d\n",guiroot->getChildCount());

	int x,y;
	guiroot->getPos(x,y);

	//guiroot=guiroot->getParent();

	if (!mymsg)
	{
		mymsg= new vgui::MessageBox("MIRV Demo Tool","Greetings Earthling",x+40,y+40);
		mymsg->setParent(guiroot);
		mymsg->setVisible(true);
		mymsg->requestFocus();
	}

	mymsg->getPos(x,y);
	pEngfuncs->Con_Printf("Info: %d,%d %d\n",x,y,mymsg->isVisible);


	pEngfuncs->Con_Printf("guiroot->getChildCount(): %d\n",guiroot->getChildCount());
}

MDTgui::MDTgui()
{
//	mymsg=NULL;
}
MDTgui::~MDTgui()
{
//	if (mymsg) delete mymsg;
}*/
