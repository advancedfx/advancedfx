#include "stdafx.h"

#include "shaderapidx9.h"

#include "../../addresses.h"
#include "../../AfxInterop.h"

#include <Windows.h>
#include <shared/Detours/src/detours.h>


typedef void *(__fastcall * CShaderAPIDx8_CreateTexture_t)(void * This, void * edx, void * p0, void * p1, void * p2, void * p3, void * p4, void * p5, void * p6, void * p7, void * p8, const char * texture_name, const char * texture_group);

CShaderAPIDx8_CreateTexture_t True_CShaderAPIDx8_CreateTexture;

CShaderAPIDx8_TextureInfo * g_CShaderAPIDx8_CreateTextureInfo = nullptr;

void * __fastcall My_CShaderAPIDx8_CreateTexture(void * This, void * edx, void * p0, void * p1, void * p2, void * p3, void * p4, void * p5, void * p6, void * p7, void * p8, const char * texture_name, const char * texture_group)
{
	static CShaderAPIDx8_TextureInfo info;

	info.TextureName = texture_name;
	info.TextureGroup = texture_group;

	g_CShaderAPIDx8_CreateTextureInfo = &info;

	void * result = True_CShaderAPIDx8_CreateTexture(This, edx, p0, p1, p2, p3, p4, p5, p6, p7, p8, texture_name, texture_group);

	g_CShaderAPIDx8_CreateTextureInfo = nullptr;

	return result;
}

CShaderAPIDx8_TextureInfo * CShaderAPIDx8_GetCreateTextureInfo() {
	return g_CShaderAPIDx8_CreateTextureInfo;
}

bool Hook_CShaderAPIDx8() {
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CShaderAPIDx8_UnkCreateTexture))
	{
		LONG error = NO_ERROR;

		True_CShaderAPIDx8_CreateTexture = (CShaderAPIDx8_CreateTexture_t)AFXADDR_GET(csgo_CShaderAPIDx8_UnkCreateTexture);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_CShaderAPIDx8_CreateTexture, My_CShaderAPIDx8_CreateTexture);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
