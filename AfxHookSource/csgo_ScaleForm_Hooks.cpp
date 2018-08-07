#include "stdafx.h"

#include "csgo_ScaleForm_Hooks.h"

#include "addresses.h"

#include <shared/detours.h>
#include <shared/StringTools.h>

#include "SourceInterfaces.h"
#include "hlaeFolder.h"

#include <string>

/*
typedef bool (__stdcall *csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf_t)(DWORD *this_ptr,char const * path, void * pData, DWORD fileLength);

csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf_t detoured_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf;

bool __stdcall touring_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf(DWORD *this_ptr,char const * path, void * pData, DWORD fileLength)
{
	//Tier0_Msg("touring_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf: %s\n", path);
	return true;

	//bool result = detoured_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf(this_ptr, path, pData, fileLength);
	//Tier0_Msg("detoured_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf(\"%s\",[data],%u) --> %s\n", path, fileLength, result ? "TRUE" : "FALSE");
	//return result;
}

bool csgo_ScaleForm_ClientHooks_Init(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf))
	{
		detoured_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf = (csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf), (BYTE *)touring_csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf, (int)AFXADDR_GET(csgo_CScaleformSlotInitControllerClientImpl_UnkCheckSwf_DSZ));

		firstResult = true;
	}

	return firstResult;
}
*/

typedef void * (__stdcall *csgo_Scaleformui_CUnkown_Loader_t)(DWORD *this_ptr,char const * path, DWORD unkArg1);

csgo_Scaleformui_CUnkown_Loader_t detoured_csgo_Scaleformui_CUnkown_Loader;

void * __stdcall touring_csgo_Scaleformui_CUnkown_Loader(DWORD *this_ptr,char const * path, DWORD unkArg1)
{
	if(StringBeginsWith(path, "resource/flash/"))
	{
		char const * subPath = path + strlen("resource/flash/");

		std::string newPath("resource/afxFlash/");
		newPath.append(subPath);

		std::string checkPath(GetHlaeFolder());
		checkPath.append("resources\\AfxHookSource\\assets\\csgo\\");
		checkPath.append(newPath);

		//Tier0_Msg("touring_csgo_Scaleformui_CUnkown_Loader: checking: %s\n", checkPath.c_str());

		bool fileExists = false;
		FILE * file = fopen(checkPath.c_str(),"rb");
		if(file)
		{
			fileExists = true;
			fclose(file);
		}

		if(fileExists)
		{
			//Tier0_Msg("touring_csgo_Scaleformui_CUnkown_Loader: %s -> %s\n", path, newPath.c_str());
			return detoured_csgo_Scaleformui_CUnkown_Loader(this_ptr, newPath.c_str(), unkArg1);
		}
	}
	
	return detoured_csgo_Scaleformui_CUnkown_Loader(this_ptr, path, unkArg1);
}


bool csgo_ScaleFormDll_Hooks_Init(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(csgo_Scaleformui_CUnkown_Loader))
	{
		detoured_csgo_Scaleformui_CUnkown_Loader = (csgo_Scaleformui_CUnkown_Loader_t)DetourClassFunc((BYTE *)AFXADDR_GET(csgo_Scaleformui_CUnkown_Loader), (BYTE *)touring_csgo_Scaleformui_CUnkown_Loader, (int)AFXADDR_GET(csgo_Scaleformui_CUnkown_Loader_DSZ));

		firstResult = true;
	}

	return firstResult;
}
