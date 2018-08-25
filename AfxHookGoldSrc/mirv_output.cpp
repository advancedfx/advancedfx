//#include "stdafx.h"

// Copyright (c) by advancedfx.org
//
// Last changes:
// 2010-01-11 dominik.matrixstorm.com
//
// First changes
// 2010-01-11 dominik.matrixstorm.com

// BEGIN HLSDK includes
#pragma push_macro("HSPRITE")
#define HSPRITE MDTHACKED_HSPRITE
//
#include <hlsdk/multiplayer/cl_dll/wrect.h>
#include <hlsdk/multiplayer/cl_dll/cl_dll.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
// END HLSDK includes

// Own includes:
#include "cmdregister.h"

extern cl_enginefuncs_s* pEngfuncs;


void ListCodecs() {
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	IMoniker *pMoniker = NULL;

	pEngfuncs->Con_Printf("Codecs:\n");

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
		(void**)&pSysDevEnum);
	if (FAILED(hr))
	{
		return;
	}    

	hr = pSysDevEnum->CreateClassEnumerator(
			 CLSID_VideoCompressorCategory, &pEnum, 0);
	if (hr == S_OK)  // S_FALSE means nothing in this category.
	{
		while (S_OK == pEnum->Next(1, &pMoniker, NULL))
		{
			IPropertyBag *pPropBag = NULL;
			pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			VARIANT var;
			VariantInit(&var);
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
			if (SUCCEEDED(hr))
			{
				int ilen=WideCharToMultiByte(
					CP_ACP,
					0,
					var.bstrVal,
					-1,
					NULL,
					0,
					NULL,
					NULL
				);

				LPSTR str = (LPSTR)malloc(ilen);

				if(0 != WideCharToMultiByte(
					CP_ACP,
					0,
					var.bstrVal,
					-1,
					str,
					ilen,
					NULL,
					NULL
				))

				pEngfuncs->Con_Printf("%s\n", str);

				free(str);
			}   
			VariantClear(&var); 
			pPropBag->Release();
			pMoniker->Release();
		}
	}

	pSysDevEnum->Release();
	pEnum->Release();
}


REGISTER_DEBUGCMD_FUNC(output) {
	ListCodecs();
}