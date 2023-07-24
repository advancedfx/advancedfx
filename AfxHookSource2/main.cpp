#include "stdafx.h"

#include "WrpConsole.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/AfxHookSource/SourceInterfaces.h"
#include "../deps/release/prop/cs2/Source2Client.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/convar.h"
#include "../deps/release/prop/cs2/sdk_src/public/cdll_int.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"

#include "../shared/AfxCommandLine.h"
#include "../shared/AfxConsole.h"
#include "../shared/AfxDetours.h"
#include "../shared/StringTools.h"
#include "../shared/binutils.h"
#include "../shared/MirvCampath.h"

#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"

advancedfx::CCommandLine  * g_CommandLine = nullptr;

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookCS2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}


int g_nIgnoreNextDisconnects = 0;

typedef void (*Unknown_ExecuteClientCommandFromNetChan_t)(void * Ecx, void * Edx, void *R8);
Unknown_ExecuteClientCommandFromNetChan_t g_Old_Unknown_ExecuteClientCommandFromNetChan = nullptr;
void New_Unknown_ExecuteClientCommandFromNetChan(void * Ecx, void * Edx, SOURCESDK::CS2::CCommand *r8Command) {
	if(0 < g_nIgnoreNextDisconnects && 0 < r8Command->ArgC()) {
		if(0 == stricmp("disconnect",r8Command->ArgV(0))) {
			if(0 < g_nIgnoreNextDisconnects) g_nIgnoreNextDisconnects--;
			return;
		}
	}
	g_Old_Unknown_ExecuteClientCommandFromNetChan(Ecx, Edx, r8Command);
}


void HookEngineDll(HMODULE engineDll) {

	static bool bFirstCall = true;
	if(!bFirstCall) return;
	bFirstCall = false;
	
	// Unknown_ExecuteClientCommandFromNetChan: // Last checked 2023-07-19
	/*
		The function we hook is called in the function referencing the string
		"Client %s(%d) tried to execute command \"%s\" before being fully connected.\n"
		or the other function referencing "SV: Cheat command '%s' ignored.\n"
		as follows:

		loc_1801842F0:
		mov     r8, rdi
		lea     rdx, [rsp+0D68h+var_D38]
		lea     rcx, [rsp+0D68h+arg_18]
		call    sub_180329DD0 <---
		lea     rcx, [rsp+0D68h+var_D30]
		call    sub_180183A60	
	*/
	{
		Afx::BinUtils::ImageSectionsReader sections((HMODULE)engineDll);
		Afx::BinUtils::MemRange textRange = sections.GetMemRange();
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "4C 8B D1 48 8B 0D ?? ?? ?? ?? 48 85 C9 74 13 48 8B 01 4D 8B C8 4C 8B C2 49 8B 12 48 FF A0 90 00 00 00 C3");
		if (!result.IsEmpty()) {
			g_Old_Unknown_ExecuteClientCommandFromNetChan = (Unknown_ExecuteClientCommandFromNetChan_t)result.Start;	
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_Old_Unknown_ExecuteClientCommandFromNetChan, New_Unknown_ExecuteClientCommandFromNetChan);
			if(NO_ERROR != DetourTransactionCommit())
				ErrorBox("Failed to detour Unknown_ExecuteClientCommandFromNetChan.");
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

}

SOURCESDK::CS2::ISource2EngineToClient * g_pEngineToClient = nullptr;

////////////////////////////////////////////////////////////////////////////////

//TODO: Some bellow here might be not accurate yet.

typedef void * Cs2Gloabls_t;
Cs2Gloabls_t g_pGlobals = nullptr;

float curtime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals + 0*4) : 0;
}

int framecount_get(void)
{
	return g_pGlobals ? *(int *)((unsigned char *)g_pGlobals + 1*4) : 0;
}

float frametime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +2*4) : 0;
}

float absoluteframetime_get(void)
{
	return g_pGlobals ? *(float *)((unsigned char *)g_pGlobals +3*4) : 0;
}

////////////////////////////////////////////////////////////////////////////////

CamPath g_CamPath;

struct CameraData_s {
	float Origin[3];
	float Angles[3];
	float Fov;

	CameraData_s(){

	}

	CameraData_s(float origin[3], float angles[3], float fov)
	{
		Origin[0] = origin[0];
		Origin[1] = origin[1];
		Origin[2] = origin[2];

		Angles[0] = angles[0];
		Angles[1] = angles[1];
		Angles[2] = angles[2];

		Fov = fov;
	}

	CameraData_s(const CameraData_s & other) {
		*this = other;
	}

	CameraData_s & operator=(const CameraData_s & other) {
		Origin[0] = other.Origin[0];
		Origin[1] = other.Origin[1];
		Origin[2] = other.Origin[2];

		Angles[0] = other.Angles[0];
		Angles[1] = other.Angles[1];
		Angles[2] = other.Angles[2];

		Fov = other.Fov;

		return * this;
	}
	

} g_LastCameraData;

class CMirvCampath_Time : public IMirvCampath_Time
{
public:
	virtual double GetTime() {
		// Can be paused time, we don't support that currently.

		return curtime_get();
	}
	virtual double GetCurTime() {
		return curtime_get();
	}
	virtual bool GetCurrentDemoTick(int& outTick) {
		return false;
	}
	virtual bool GetCurrentDemoTime(double& outDemoTime) {
		return false;
	}
	virtual bool GetDemoTickFromDemoTime(double curTime, double time, int& outTick) {
		return false;
	}
	virtual bool GetDemoTimeFromClientTime(double curTime, double time, double& outDemoTime) {
		return false;
	}
} g_MirvCampath_Time;

class CMirvCampath_Camera : public IMirvCampath_Camera
{
public:
	virtual SMirvCameraValue GetCamera() {
		return SMirvCameraValue(
			g_LastCameraData.Origin[0],
			g_LastCameraData.Origin[1],
			g_LastCameraData.Origin[2],
			g_LastCameraData.Angles[0],
			g_LastCameraData.Angles[1],
			g_LastCameraData.Angles[2],
			g_LastCameraData.Fov
		);
	}
} g_MirvCampath_Camera;


CON_COMMAND(mirv_campath, "camera paths")
{
	if (nullptr == g_pGlobals)
	{
		advancedfx::Warning("Error: Hooks not installed.\n");
		return;
	}

	MirvCampath_ConCommand(args, advancedfx::Message, advancedfx::Warning, &g_CamPath, &g_MirvCampath_Time, &g_MirvCampath_Camera, nullptr);
}

bool CS2_Client_CSetupView_Trampoline_IsPlayingDemo(void *ThisCViewSetup) {
	if(!g_pEngineToClient) return false;

	bool originOrAnglesOverriden = false;

	float curTime = curtime_get(); //TODO: + m_PausedTime

	float *pFov = (float*)((unsigned __int64)ThisCViewSetup + 0x4d8);
	float *pViewOrigin = (float*)((unsigned __int64)ThisCViewSetup + 0x4e0);
	float *pViewAngles = (float*)((unsigned __int64)ThisCViewSetup + 0x4f8);

	//advancedfx::Message("%f: (%f,%f,%f) (%f,%f,%f) [%f]\n",curTime,pViewOrigin[0],pViewOrigin[1],pViewOrigin[2],pViewAngles[0],pViewAngles[1],pViewAngles[2],*pFov);

	CameraData_s cameraData(pViewOrigin, pViewAngles, *pFov);

	if (g_CamPath.Enabled_get() && g_CamPath.CanEval())
	{
		double campathCurTime = curTime - g_CamPath.GetOffset();

		// no extrapolation:
		if (g_CamPath.GetLowerBound() <= campathCurTime && campathCurTime <= g_CamPath.GetUpperBound())
		{
			CamPathValue val = g_CamPath.Eval(campathCurTime);
			QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

			//Tier0_Msg("================",curTime);
			//Tier0_Msg("currenTime = %f",curTime);
			//Tier0_Msg("vCp = %f %f %f\n", val.X, val.Y, val.Z);

			originOrAnglesOverriden = true;

			cameraData.Origin[0] = (float)val.X;
			cameraData.Origin[1] = (float)val.Y;
			cameraData.Origin[2] = (float)val.Z;

			cameraData.Angles[0] = (float)ang.Pitch;
			cameraData.Angles[1] = (float)ang.Yaw;
			cameraData.Angles[2] = (float)ang.Roll;

			cameraData.Fov = (float)val.Fov;
		}
	}

	if(originOrAnglesOverriden) {
		pViewOrigin[0] = cameraData.Origin[0];
		pViewOrigin[1] = cameraData.Origin[1];
		pViewOrigin[2] = cameraData.Origin[2];

		pViewAngles[0] = cameraData.Angles[0];
		pViewAngles[1] = cameraData.Angles[1];
		pViewAngles[2] = cameraData.Angles[2];

		*pFov = cameraData.Fov;
	}

	g_LastCameraData = cameraData;

	return g_pEngineToClient->IsPlayingDemo();
}

void HookClientDll(HMODULE clientDll) {
	static bool bFirstCall = true;
	if(!bFirstCall) return;
	bFirstCall = false;

	// 
	/*
		This is where it checks for engine->IsPlayingDemo() (and afterwards for cl_demoviewoverriode (float))
		before under these conditions it is calling CalcDemoViewOverride, so this is in CViewRender::SetUpView:

.text:0000000180768B22                 mov     rcx, cs:qword_18163E0F0
.text:0000000180768B29                 mov     rax, [rcx]
.text:0000000180768B2C                 call    qword ptr [rax+108h]
.text:0000000180768B32                 mov     rbp, [rsp+978h+var_28]
.text:0000000180768B3A                 xorps   xmm6, xmm6
.text:0000000180768B3D                 test    al, al
.text:0000000180768B3F                 jz      short loc_180768BB8
.text:0000000180768B41
.text:0000000180768B41 loc_180768B41:                          ; DATA XREF: .pdata:000000018184283C↓o
.text:0000000180768B41                                         ; .pdata:0000000181842848↓o
.text:0000000180768B41                 mov     edx, 0FFFFFFFFh
	*/
	{
		Afx::BinUtils::ImageSectionsReader sections((HMODULE)clientDll);
		Afx::BinUtils::MemRange textRange = sections.GetMemRange();
		Afx::BinUtils::MemRange result = FindPatternString(textRange, "48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 90 08 01 00 00 48 8B AC 24 50 09 00 00 0F 57 F6 84 C0 74 77 BA FF FF FF FF");
		if (!result.IsEmpty()) {
			/*
				These are the top 16 bytes we change to:

00007fff`95518b22 4889f1               mov     rcx, rsi
00007fff`95518b25 48b8???????????????? mov     rax, ???????????????? <-- here we load our hook's address
00007fff`95518b2f ff10                 call    qword ptr [rax]
00007fff`95518b31 90                   nop
			*/
			unsigned char asmCode[16]={
				0x48, 0x89, 0xf1,
				0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
				0xff, 0x10,
				0x90
			};
			static LPVOID ptr = CS2_Client_CSetupView_Trampoline_IsPlayingDemo;
			LPVOID ptrPtr = &ptr;
			memcpy(&asmCode[5], &ptrPtr, sizeof(LPVOID));

			MdtMemBlockInfos mbis;
			MdtMemAccessBegin((LPVOID)result.Start, 16, &mbis);
			memcpy((LPVOID)result.Start, asmCode, 16);
			MdtMemAccessEnd(&mbis);
		}
		else
			ErrorBox(MkErrStr(__FILE__, __LINE__));
	}	
}

SOURCESDK::CreateInterfaceFn g_AppSystemFactory = 0;
SOURCESDK::CS2::IMemAlloc *SOURCESDK::CS2::g_pMemAlloc = 0;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::cvar = 0;
SOURCESDK::CS2::ICvar * SOURCESDK::CS2::g_pCVar = 0;

typedef int(* CCS2_Client_Connect_t)(void* This, SOURCESDK::CreateInterfaceFn appSystemFactor);
CCS2_Client_Connect_t old_CCS2_Client_Connect;
int new_CCS2_Client_Connect(void* This, SOURCESDK::CreateInterfaceFn appSystemFactory) {
	static bool bFirstCall = true;

	if (bFirstCall) {
		bFirstCall = false;

		void * iface = NULL;

		if (SOURCESDK::CS2::g_pCVar = SOURCESDK::CS2::cvar = (SOURCESDK::CS2::ICvar*)appSystemFactory(SOURCESDK_CS2_CVAR_INTERFACE_VERSION, NULL))
		{
			//
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));

		if (g_pEngineToClient = (SOURCESDK::CS2::ISource2EngineToClient*)appSystemFactory(SOURCESDK_CS2_ENGINE_TO_CLIENT_INTERFACE_VERSION, NULL)) {
		}
		else ErrorBox(MkErrStr(__FILE__, __LINE__));
	}

	return old_CCS2_Client_Connect(This, appSystemFactory);
}

CON_COMMAND(mirv_suppress_disconnects, "Suppresses given number disconnect commands. Can help to test demo system.") {
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);
	if(2 <= argC) {
			g_nIgnoreNextDisconnects = atoi(args->ArgV(1));
			return;
	}
	advancedfx::Message(
		"mirv_suppress_disconnects <iSuppressTimes> - Use -1 to always suppress, or a positive number to suppress a certain count.\n"
		"Eample: \"mirv_suppress_disconnects 1; playdemo test.dem\" - Please don't report bugs for this to Valve, the system is not meant to be used yet!\n"
		"Current value: %i\n",
		g_nIgnoreNextDisconnects
	);
}

CON_COMMAND(mirv_cvar_unhide_all, "Unlocks cmds and cvars.") {
	int total = 0;
	int nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::CCmd * cmd = SOURCESDK::CS2::g_pCVar->GetCmd(i);
		if(nullptr == cmd) break;
		int nFlags = cmd->GetFlags();
		if(nFlags == 0x400) break;
		total++;
		if(nFlags & (FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
			cmd->SetFlags(nFlags &= ~(int)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN));
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cmd->m_nFlags, cmd->m_pszName, cmd->m_pszHelpString);
		}
	}
	advancedfx::Message("==== Cmds total: %i (Cmds unhidden: %i) ====\n",total,nUnhidden);

	total = 0;
	nUnhidden = 0;
	for(size_t i = 0; i < 65536; i++ )
	{
		SOURCESDK::CS2::Cvar_s * cvar = SOURCESDK::CS2::g_pCVar->GetCvar(i);
		if(nullptr == cvar) break;
		total++;
		if(cvar->m_nFlags & (FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN)) {
//			fprintf(f1,"[+] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
			cvar->m_nFlags &= ~(int)(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
			nUnhidden++;
		} else {
//			fprintf(f1,"[ ] %lli: 0x%08x: %s : %s\n", i, cvar->m_nFlags, cvar->m_pszName, cvar->m_pszHelpString);
		}
	}
	
	advancedfx::Message("==== Cvars total: %i (Cvars unhidden: %i) ====\n",total,nUnhidden);
}

typedef int(* CCS2_Client_Init_t)(void* This);
CCS2_Client_Init_t old_CCS2_Client_Init;
int new_CCS2_Client_Init(void* This) {
	int result = old_CCS2_Client_Init(This);

	WrpRegisterCommands();

	return result;
}

typedef void * (* CS2_Client_SetGlobals_t)(void* This, void * pGlobals);
CS2_Client_SetGlobals_t old_CS2_Client_SetGlobals;
void *  new_CS2_Client_SetGlobals(void* This, void * pGlobals) {

	g_pGlobals = (Cs2Gloabls_t)pGlobals;

	return old_CS2_Client_SetGlobals(This, pGlobals);
}


void CS2_HookClientDllInterface(void * iface)
{
	void ** vtable = *(void***)iface;

	AfxDetourPtr((PVOID *)&(vtable[0]), new_CCS2_Client_Connect, (PVOID*)&old_CCS2_Client_Connect);
	AfxDetourPtr((PVOID *)&(vtable[3]), new_CCS2_Client_Init, (PVOID*)&old_CCS2_Client_Init);
	AfxDetourPtr((PVOID *)&(vtable[11]), new_CS2_Client_SetGlobals, (PVOID*)&old_CS2_Client_SetGlobals);
}

SOURCESDK::CreateInterfaceFn old_Client_CreateInterface = 0;

void* new_Client_CreateInterface(const char *pName, int *pReturnCode)
{
	static bool bFirstCall = true;

	void * pRet = old_Client_CreateInterface(pName, pReturnCode);

	if(bFirstCall)
	{
		bFirstCall = false;

		void * iface = NULL;
		
		if (iface = old_Client_CreateInterface(SOURCESDK_CS2_Source2Client_VERSION, NULL)) {
			CS2_HookClientDllInterface(iface);
		}
		else
		{
			ErrorBox("Could not get a supported VClient interface.");
		}
	}

	return pRet;
}


HMODULE g_H_ClientDll = 0;

FARPROC WINAPI new_tier0_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC nResult;
	nResult = GetProcAddress(hModule, lpProcName);

	if (!nResult)
		return nResult;

	if (HIWORD(lpProcName))
	{
		if (!lstrcmp(lpProcName, "GetProcAddress"))
			return (FARPROC) &new_tier0_GetProcAddress;

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

HMODULE WINAPI new_LoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI new_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

HMODULE g_h_engine2Dll;


CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD)> g_Import_tier0_KERNEL32_LoadLibraryExW("LoadLibraryExW", &new_LoadLibraryExW);
CAfxImportFuncHook<FARPROC(WINAPI*)(HMODULE, LPCSTR)> g_Import_tier0_KERNEL32_GetProcAddress("GetProcAddress", &new_tier0_GetProcAddress);

CAfxImportDllHook g_Import_tier0_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_tier0_KERNEL32_LoadLibraryExA
	, &g_Import_tier0_KERNEL32_LoadLibraryExW
	, &g_Import_tier0_KERNEL32_GetProcAddress }));

CAfxImportsHook g_Import_tier0(CAfxImportsHooks({
	&g_Import_tier0_KERNEL32 }));

void CommonHooks()
{
	static bool bFirstRun = true;
	static bool bFirstTier0 = true;

	// do not use messageboxes here, there is some friggin hooking going on in between by the
	// Source engine.

	if (bFirstRun)
	{
		bFirstRun = false;
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

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_engine2_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_engine2_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_engine2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_engine2_KERNEL32_LoadLibraryA
	, &g_Import_engine2_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_engine2(CAfxImportsHooks({
	&g_Import_engine2_KERNEL32 }));

CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR)> g_Import_materialsystem2_KERNEL32_LoadLibraryA("LoadLibraryA", &new_LoadLibraryA);
CAfxImportFuncHook<HMODULE(WINAPI*)(LPCSTR, HANDLE, DWORD)> g_Import_materialsystem2_KERNEL32_LoadLibraryExA("LoadLibraryExA", &new_LoadLibraryExA);

CAfxImportDllHook g_Import_materialsystem2_KERNEL32("KERNEL32.dll", CAfxImportDllHooks({
	&g_Import_materialsystem2_KERNEL32_LoadLibraryA
	, &g_Import_materialsystem2_KERNEL32_LoadLibraryExA }));

CAfxImportsHook g_Import_materialsystem2(CAfxImportsHooks({
	&g_Import_materialsystem2_KERNEL32 }));

void LibraryHooksA(HMODULE hModule, LPCSTR lpLibFileName)
{
	CommonHooks();

	if(!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1=NULL;

	if( !f1 ) f1=fopen("hlae_log_LibraryHooksA.txt","wb");
	fprintf(f1,"%s\n", lpLibFileName);
	fflush(f1);
#endif
}

void LibraryHooksW(HMODULE hModule, LPCWSTR lpLibFileName)
{
	static bool bFirstTier0 = true;
	static bool bFirstClient = true;
	static bool bFirstEngine2 = true;
	static bool bFirstfilesystem_stdio = true;
	static bool bFirstMaterialsystem2 = true;
	
	CommonHooks();

	if (!hModule || !lpLibFileName)
		return;

#if 0
	static FILE *f1 = NULL;

	if (!f1) f1 = fopen("hlae_log_LibraryHooksW.txt", "wb");
	fwprintf(f1, L"%s\n", lpLibFileName);
	fflush(f1);
#endif


	if(bFirstTier0 && StringEndsWithW( lpLibFileName, L"tier0.dll"))
	{
		bFirstTier0 = false;
		
		g_Import_tier0.Apply(hModule);

		SOURCESDK::CS2::g_pMemAlloc = *(SOURCESDK::CS2::IMemAlloc **)GetProcAddress(hModule, "g_pMemAlloc");

		advancedfx::Message = (Tier0MsgFn)GetProcAddress(hModule, "Msg");
		advancedfx::Warning = (Tier0MsgFn)GetProcAddress(hModule, "Warning");
		advancedfx::DevMessage = (Tier0DevMsgFn)GetProcAddress(hModule, "DevMsg");
		advancedfx::DevWarning = (Tier0DevMsgFn)GetProcAddress(hModule, "DevWarning");		
	}
	/*else if(bFirstfilesystem_stdio && StringEndsWithW( lpLibFileName, L"filesystem_stdio.dll"))
	{
		bFirstfilesystem_stdio = false;
		
		g_Import_filesystem_stdio.Apply(hModule);
	}*/
	else if(bFirstEngine2 && StringEndsWithW( lpLibFileName, L"engine2.dll"))
	{
		bFirstEngine2 = false;

		g_h_engine2Dll = hModule;

		HookEngineDll(hModule);

		//g_Import_engine2.Apply(hModule);
	}
	/*else if(bFirstMaterialsystem2 && StringEndsWithW( lpLibFileName, L"materialsystem2.dll"))
	{
		bFirstMaterialsystem2 = false;

		g_Import_materialsystem2.Apply(hModule);
	}*/
	else if(bFirstClient && StringEndsWithW(lpLibFileName, L"csgo\\bin\\win64\\client.dll"))
	{
		bFirstClient = false;

		g_H_ClientDll = hModule;

		HookClientDll(hModule);
	}
}

HMODULE WINAPI new_LoadLibraryA( LPCSTR lpLibFileName ) {
	HMODULE hRet = LoadLibraryA(lpLibFileName);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}

HMODULE WINAPI new_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE hRet = LoadLibraryExA(lpLibFileName, hFile, dwFlags);

	LibraryHooksA(hRet, lpLibFileName);

	return hRet;
}

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
			g_CommandLine = new advancedfx::CCommandLine();

			if(!g_CommandLine->FindParam(L"-insecure"))
			{
				ErrorBox("Please add -insecure to launch options, AfxHookCS2 will refuse to work without it!");

				HANDLE hproc = OpenProcess(PROCESS_TERMINATE, true, GetCurrentProcessId());
				TerminateProcess(hproc, 0);
				CloseHandle(hproc);
				
				do MessageBoxA(NULL, "Please terminate the game manually in the taskmanager!", "Cannot terminate, please help:", MB_OK | MB_ICONERROR);
				while (true);
			}

#if _DEBUG
			MessageBox(0,"DLL_PROCESS_ATTACH","MDT_DEBUG",MB_OK);
#endif
			g_Import_PROCESS.Apply(GetModuleHandle(NULL));

			if (!(g_Import_PROCESS_KERNEL32_LoadLibraryA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExA.TrueFunc || g_Import_PROCESS_KERNEL32_LoadLibraryExW.TrueFunc))
				ErrorBox();

			//
			// Remember we are not on the main program thread here,
			// instead we are on our own thread, so don't run
			// things here that would have problems with that.
			//

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			// actually this gets called now.

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
