#include "Globals.h"

#include "SceneSystem.h"
#include "ClientEntitySystem.h"

#include "../shared/StringTools.h"
#include "../deps/release/Detours/src/detours.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/bufferstring.h"

CResourceSystem* g_pCResourceSystem = nullptr;

struct CBufferStringWrapper {
	SOURCESDK::CS2::CBufferString buf;
	u_char pad[0xE0 - sizeof(SOURCESDK::CS2::CBufferString)];
};

typedef void* (__fastcall * ForceUpdateSkybox_t)(void* This);
ForceUpdateSkybox_t org_ForceUpdateSkybox = nullptr;

struct CustomSkyState {
	std::string currentSkyName;
	bool enabled = false;
} g_CustomSky;

// 64-bit hash for 32-bit platforms
// Credit
// https://www.ncbi.nlm.nih.gov/IEB/ToolBox/CPP_DOC/lxr/source/src/util/checksum/murmurhash/MurmurHash2.cxx#0140
uint64_t MurmurHash64B ( const void * key, int len, uint64_t seed )
{
  const uint32_t m = 0x5bd1e995;
  const int r = 24;

  uint32_t h1 = uint32_t(seed) ^ len;
  uint32_t h2 = uint32_t(seed >> 32);

  const uint32_t * data = (const uint32_t *)key;

  while(len >= 8)
  {
    uint32_t k1 = *data++;
    k1 *= m; k1 ^= k1 >> r; k1 *= m;
    h1 *= m; h1 ^= k1;
    len -= 4;

    uint32_t k2 = *data++;
    k2 *= m; k2 ^= k2 >> r; k2 *= m;
    h2 *= m; h2 ^= k2;
    len -= 4;
  }

  if(len >= 4)
  {
    uint32_t k1 = *data++;
    k1 *= m; k1 ^= k1 >> r; k1 *= m;
    h1 *= m; h1 ^= k1;
    len -= 4;
  }

  switch(len)
  {
  case 3: h2 ^= ((unsigned char*)data)[2] << 16;
  case 2: h2 ^= ((unsigned char*)data)[1] << 8;
  case 1: h2 ^= ((unsigned char*)data)[0];
      h2 *= m;
  };

  h1 ^= h2 >> 18; h1 *= m;
  h2 ^= h1 >> 22; h2 *= m;
  h1 ^= h2 >> 17; h1 *= m;
  h2 ^= h1 >> 19; h2 *= m;

  uint64_t h = h1;

  h = (h << 32) | h2;

  return h;
}

CMaterial2** CResourceSystem::PreCache (const char* name) {
	// has "Resource \"%s\" was not precached but was loaded by a just in time blocking load.\n"
	// There are 2 similar ones, this is second one
	typedef CMaterial2** (__fastcall * PreCacheFn_t)(void* This, CBufferStringWrapper* name, const char* unk);
	void** vtable = *(void***)(this);
	auto org_PreCache = (PreCacheFn_t)vtable[46];

	// client dll
	// see where func is called with "FixupResourceName: Illegal full path passed in (\"%s\")!\n" 
	CBufferStringWrapper wrapper { name };
	wrapper.buf.FixupPathName(0x5C); // back slash
	wrapper.buf.ToLowerFast(0);
	wrapper.buf.FixSlashes(0x2F); // forward slash

	SOURCESDK::CS2::CBufferString extension;
	extension.ExtractFileExtension(wrapper.buf.Get());
	// in case its ever updated see where above mentioned is called, there is function that calculates it
	auto hash = MurmurHash64B(wrapper.buf.Get(), wrapper.buf.Length(), 0xEDABCDEF);

	auto pWrapper = (u_char*)&wrapper;
	memcpy(pWrapper + 0xD0, &hash, 8);
	memcpy(pWrapper + 0xD8, extension.Get(), 8);

	return org_PreCache(this, &wrapper, "");
}

typedef void* (__fastcall * UpdateSkyboxMaterial_t)(void* pCSceneSystem, CMaterial2*** pCMaterial, void* pCSceneWorld);
UpdateSkyboxMaterial_t org_UpdateSkyboxMaterial = nullptr;

void* new_UpdateSkyboxMaterial(void* pCSceneSystem, CMaterial2*** pCMaterial, void* pCSceneWorld) {
	if (g_CustomSky.enabled) {
		auto mat = g_pCResourceSystem->PreCache(g_CustomSky.currentSkyName.c_str());
		return org_UpdateSkyboxMaterial(pCSceneSystem, &mat, pCSceneWorld);
	}

	return org_UpdateSkyboxMaterial(pCSceneSystem, pCMaterial, pCSceneWorld);
}

void updateSkyboxEntities() {
	int highestIndex = GetHighestEntityIndex();
	for(int i = 0; i < highestIndex + 1; i++) {
		if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
			// Note: there are multiple skybox entities. 
			// Seems like the other one is used for fog or smth.
			if (0 != _stricmp("C_EnvSky", ent->GetClientClassName())) continue;

			// we have to remove pointer to object, so it can update
			// see dissasembly for the update function
			// TODO: maybe get this offset from pattern matching
			*(void**)((u_char*)ent + 0xF10) = nullptr;
			org_ForceUpdateSkybox(ent);
		}
	}
}

CON_COMMAND(mirv_skybox, "")
{
	auto argc = args->ArgC();
	auto arg0 = args->ArgV(0);

	if(2 <= argc)
	{
		auto arg1 = args->ArgV(1);
		if(!_stricmp(arg1,"material"))
		{
			if (3 <= argc)
			{
				std::string filePath = args->ArgV(2);
				if (StringEndsWith(filePath.c_str(), "_c")) {
					filePath.erase(filePath.size() - 2);
				}
				g_CustomSky.currentSkyName = filePath.c_str();
				if (g_CustomSky.enabled) updateSkyboxEntities();
				return;
			}

			advancedfx::Message(
				"Usage:\n"
				"%s %s  <sRelativePathToFile> - Set skybox material.\n"
				"Current value: %s\n",
				arg0, arg1,
				g_CustomSky.currentSkyName.c_str()
			);
			return;
		}

		if(!_stricmp(arg1,"enabled"))
		{
			if (3 <= argc)
			{
				g_CustomSky.enabled = 0 != atoi(args->ArgV(2));
				updateSkyboxEntities();
				return;
			}

			advancedfx::Message(
				"Usage:\n"
				"%s %s 0|1 - Enable (1) / disable (0) custom skybox.\n"
				"Current value: %s\n",
				arg0, arg1,
				g_CustomSky.enabled ? "1" : "0"
			);
			return;
		}
	}

	advancedfx::Message(
		"Usage:\n"
		"%s material <sRelativePathToFile> - Set skybox material.\n"
		"%s enabled 0|1 - Enable (1) / disable (0) custom skybox.\n"
		"Note: see wiki on GitHub for this command for details.\n"
		, arg0
		, arg0
	);
}

void HookSceneSystem(HMODULE sceneSystemDll) {
	// has 48 89 9f 10 01 00 00
    // MOV qword ptr [RDI  + 0x110],RBX
	org_UpdateSkyboxMaterial = (UpdateSkyboxMaterial_t)getVTableFn(sceneSystemDll, 39, ".?AVCSceneSystem@@");
	if (0 == org_UpdateSkyboxMaterial) ErrorBox(MkErrStr(__FILE__, __LINE__));

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)org_UpdateSkyboxMaterial, new_UpdateSkyboxMaterial);

	if(NO_ERROR != DetourTransactionCommit()) ErrorBox("Failed to detour SceneSystem functions.");
}
