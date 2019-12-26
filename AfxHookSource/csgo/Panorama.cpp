#include "stdafx.h"

#include "Panorama.h"

#include "../addresses.h"
#include "../hlaeFolder.h"
#include "AfxCommandLine.h"

#include <shared/AfxDetours.h>
#include <Windows.h>
#include <shared/Detours/src/detours.h>

#include <string>

typedef void(__fastcall * Panorama_CZip_UnkLoadFiles_t)(void * This, void * edx, void * pData, unsigned int countBytes);

Panorama_CZip_UnkLoadFiles_t TruePanorama_CZip_UnkLoadFiles;

void __fastcall MyPanorama_CZip_UnkLoadFiles(void * This, void * edx, void * pData, unsigned int countBytes)
{
	std::wstring filePath;

	filePath = GetHlaeFolderW();
	filePath.append(L"panorama.org.zip");

	FILE * file = _wfopen(filePath.c_str(), L"wb");
	if (file)
	{
		fwrite(pData, sizeof(BYTE), countBytes, file);
		fclose(file);
	}

	filePath = GetHlaeFolderW();
	filePath.append(L"panorama.my.zip");

	file = _wfopen(filePath.c_str(), L"rb");
	if (file)
	{
		long file_size_signed;
		void * pMemory = 0;
		if (fseek(file, 0, SEEK_END) || (file_size_signed = ftell(file)) == -1 || fseek(file, 0, SEEK_SET) || 0 == (pMemory = malloc(file_size_signed)) || file_size_signed != fread(pMemory, sizeof(BYTE), file_size_signed, file))
		{
			fclose(file);
			free(pMemory);
		}
		else
		{
			TruePanorama_CZip_UnkLoadFiles(This, edx, pMemory, (unsigned int)file_size_signed);
			fclose(file);
			free(pMemory);
			return;
		}
	}

	return TruePanorama_CZip_UnkLoadFiles(This, edx, pData, countBytes);
}

bool PanoramaHooks_Install()
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (
		0 == g_CommandLine->FindParam(L"-afxDetourPanorama")
	) {
		firstResult = true;
		return firstResult;
	}

	if (
		AFXADDR_GET(csgo_panorama_CZip_UnkLoadFiles)
	)
	{
		LONG error = NO_ERROR;

		TruePanorama_CZip_UnkLoadFiles = (Panorama_CZip_UnkLoadFiles_t)AFXADDR_GET(csgo_panorama_CZip_UnkLoadFiles);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TruePanorama_CZip_UnkLoadFiles, MyPanorama_CZip_UnkLoadFiles);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}
