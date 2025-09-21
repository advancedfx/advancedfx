#include "SceneSystem.h"

#include "ClientEntitySystem.h"
#include "Globals.h"
#include "SchemaSystem.h"
#include "MirvColors.h"

#include "../shared/StringTools.h"
#include "../deps/release/Detours/src/detours.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/bufferstring.h"
#include <unordered_map>

typedef void* (__fastcall * FindMaterial_t)(void* This, CMaterial2*** out, const char* materialName);
FindMaterial_t org_FindMaterial = nullptr;

typedef u_char* (__fastcall * MaterialFindParam_t)(CMaterial2* This, const char * paramName);
MaterialFindParam_t org_MaterialFindParam = nullptr;

typedef void (__fastcall * MaterialUpdate_t)(CMaterial2* This);
MaterialUpdate_t org_MaterialUpdate = nullptr;

CResourceSystem* g_pCResourceSystem = nullptr;

struct CBufferStringWrapper {
	SOURCESDK::CS2::CBufferString buf;
	u_char pad[0xE0 - sizeof(SOURCESDK::CS2::CBufferString)];
};

typedef void* (__fastcall * ForceUpdateSkybox_t)(void* This);
ForceUpdateSkybox_t org_ForceUpdateSkybox = nullptr;

struct CustomSkyState {
	std::string currentSkyName = "";

	MyColor color;
	MyColor colorClouds;
	bool drawClouds = true;
	float brightness = 1.0f;
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

void CResourceSystem::GetMaterials(GetMaterialsArrayResult* out) {
	// In materialsystem2.dll see below in func with 
	// "NOTE: Use mat_print_materials instead, and then click on the hyperlink for a given material. That will call this concommand with the correctly formatted argument.\n"
	// (**(code **)(*ResourceSystem + 0x128))(ResourceSystem,0x74616d76,local_198, 7);
	typedef void (__fastcall * GetMaterials_t)(void* This, uint64_t magic, GetMaterialsArrayResult* out, uint8_t unk);
	void** vtable = *(void***)(this);
	auto org_GetMaterials = (GetMaterials_t)vtable[37];

	org_GetMaterials(this, 0x74616d76, out, 7);
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

std::string previousSkybox = "";
std::unordered_map<std::string, bool> cachedMaterials = {};

void resetCachedMaterials () {
	previousSkybox = "";
	g_CustomSky.currentSkyName = "";
	cachedMaterials.clear();
}

void* new_ForceUpdateSkybox(void* This) {
	if (g_CustomSky.currentSkyName.size() != 0) {
		if (previousSkybox.size() == 0) {
			auto curMat = *(CMaterial2***)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_hSkyMaterial);
			if (0 != curMat) previousSkybox = (*curMat)->GetName();
		}

		CMaterial2** newMat = nullptr;
		org_FindMaterial(nullptr, &newMat, g_CustomSky.currentSkyName.c_str());

		if (0 != newMat) {
			*(CMaterial2***)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_hSkyMaterial) = newMat;
		}
	} else if (previousSkybox.size() != 0) {
		CMaterial2** prevMat = nullptr;
		org_FindMaterial(nullptr, &prevMat, previousSkybox.c_str());
		if (0 != prevMat) {
			*(CMaterial2***)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_hSkyMaterial) = prevMat;
		}
	}

	if (g_CustomSky.color.use) {
		*(uint32_t*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_vTintColor) = afxUtils::rgbaToHex(g_CustomSky.color.value);
		*(float*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_flBrightnessScale) = g_CustomSky.brightness;
	}

	return org_ForceUpdateSkybox(This);
}

void updateSkyboxEntities() {
	if (g_CustomSky.currentSkyName.size() != 0) {
		if (cachedMaterials.find(g_CustomSky.currentSkyName) == cachedMaterials.end()) {
			if (0 != g_pCResourceSystem->PreCache(g_CustomSky.currentSkyName.c_str())) {
				cachedMaterials.insert({g_CustomSky.currentSkyName, true});
			}
		}
	}

	int highestIndex = GetHighestEntityIndex();
	for(int i = 0; i < highestIndex + 1; i++) {
		if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
			// Note: there are multiple skybox entities. 
			// Seems like the other one is used for fog or smth.
			if (0 != _stricmp("C_EnvSky", ent->GetClientClassName())) continue;

			// we have to remove pointer to object, so it can update
			// see dissasembly for the update function
			// TODO: maybe get this offset from pattern matching
			*(void**)((u_char*)ent + 0xF18) = nullptr;
			new_ForceUpdateSkybox(ent);
		}
	}

	if (g_CustomSky.currentSkyName.size() == 0 && previousSkybox.size() != 0) {
		previousSkybox = "";
	}

}

struct CBaseSceneData {
	char _pad0[0x18];
	void* sceneObject;
	CMaterial2* material; 
	char _pad1[0x28];
	uint32_t color;
	char _pad2[0x14];
};

typedef void (__fastcall * DrawBaseSceneObject_t)(void* This, void* a2, CBaseSceneData* scene_data, int a4, int a5, void* a6, void* a7);
DrawBaseSceneObject_t org_DrawBaseSceneObject = nullptr;

// sceneData is actually array and can be iterated, a4 is count
// Color and textures can be replace here, but there are some issues with that,
// so we only "toggle" on/off here for now.
void new_DrawBaseSceneObject(void* This, void* a2, CBaseSceneData* sceneData, int a4, int a5, void* a6, void* a7) {
	if (sceneData->material == 0) return org_DrawBaseSceneObject(This, a2, sceneData, a4, a5, a6, a7);

	std::string matName = sceneData->material->GetName();

	// There is some z fight going on, when camera is pointed to sun 
	// e.g. clouds draw regardless, so have to hide some sun materials too
	// Don't have much time to fix it properly, this does the job 99% of times
	if (!g_CustomSky.drawClouds && (matName.find("clouds") != -1 || matName.find("sun_disc_glow") != -1)) {
		 return;
	}

	return org_DrawBaseSceneObject(This, a2, sceneData, a4, a5, a6, a7); 
}

struct FloatColor {
	float r = 0;
	float g = 0;
	float b = 0;
	float a = 0;
};

std::unordered_map<std::string, FloatColor> defaultCloudColors = {};

void resetDefaultCloudColors() {
	g_CustomSky.colorClouds.use = false;
	defaultCloudColors.clear();
}

void updateCloudMaterials() {
	GetMaterialsArrayResult materials = {0};
	g_pCResourceSystem->GetMaterials(&materials);

	for (int i = 0; i < materials.count; i++) {
		auto pMat = *materials.pArrMaterials[i];
		std::string matName = pMat->GetName();
		if (matName.find("clouds") != -1) {
			auto colorParam = (float*)org_MaterialFindParam(pMat, "g_vColorTint");
			if (0 != colorParam) {
				if (defaultCloudColors.find(matName) == defaultCloudColors.end()) {
					defaultCloudColors[matName] = FloatColor { colorParam[0], colorParam[1], colorParam[2], colorParam[3] };
				}

				if (g_CustomSky.colorClouds.use) {
					colorParam[0] = g_CustomSky.colorClouds.value.r / 255.0f;
					colorParam[1] = g_CustomSky.colorClouds.value.g / 255.0f;
					colorParam[2] = g_CustomSky.colorClouds.value.b / 255.0f;
					colorParam[3] = 1.0f; // this doesnt seem to do anything
				} else if (defaultCloudColors.find(matName) != defaultCloudColors.end()) {
					auto defaultColor = defaultCloudColors[matName];
					colorParam[0] = defaultColor.r;
					colorParam[1] = defaultColor.g;
					colorParam[2] = defaultColor.b;
					colorParam[3] = defaultColor.a;
				}

				org_MaterialUpdate(pMat);
			}
		}
	}

}

CON_COMMAND(mirv_sky, "")
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
				auto arg2 = args->ArgV(2);

				if (!_stricmp(arg2, "default")) {
					g_CustomSky.currentSkyName = "";
					updateSkyboxEntities();
					return;
				}

				std::string filePath = arg2;
				if (StringEndsWith(arg2, "_c")) {
					filePath.erase(filePath.size() - 2);
				}
				g_CustomSky.currentSkyName = filePath.c_str();
				updateSkyboxEntities();
				return;
			}

			advancedfx::Message(
				"Usage:\n"
				"%s %s  <sRelativePathToFile> - Set skybox material. It has to be re-applied if map changes.\n"
				"Current value: %s\n",
				arg0, arg1,
				g_CustomSky.currentSkyName.size() == 0 ? "default" : g_CustomSky.currentSkyName.c_str()
			);
			return;
		}

		if(!_stricmp(arg1, "color")) {
			if (3 <= argc && !_stricmp(args->ArgV(2), "default"))
			{
				g_CustomSky.color.use = false;
				updateSkyboxEntities();
				return;
			}

			if (6 <= argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 2);
				if (g_CustomSky.color.setColor(&subArgs)) {
					g_CustomSky.brightness = atoi(args->ArgV(5)) / 255.0;
					if (g_CustomSky.brightness < 0.000001f) g_CustomSky.brightness = 0.000001f;
					updateSkyboxEntities();
				}

				return;
			}

			advancedfx::Message(
				"Usage:\n"
				"%s %s <iR> <iG> <iB> <iA> | default - Set color override in RGBA format, values from 0 to 255.\n"
				"Current value: %s\n",
				arg0, arg1,
				g_CustomSky.color.use ? g_CustomSky.color.userValue.c_str() : "default"
			);
			return;
		}

		if(!_stricmp(arg1, "clouds")) {
			if (3 <= argc)
			{
				auto arg2 = args->ArgV(2);

				if (!_stricmp(arg2, "color")) {
					if (4 <= argc && !_stricmp(args->ArgV(3), "default"))
					{
						g_CustomSky.colorClouds.use = false;
						updateCloudMaterials();
						return;
					}

					if (6 == argc)
					{
						advancedfx::CSubCommandArgs subArgs(args, 3);
						if (g_CustomSky.colorClouds.setColor(&subArgs)) {
							updateCloudMaterials();
							return;
						}

					}

					advancedfx::Message(
						"Usage:\n"
						"%s %s %s <iR> <iG> <iB> | default - Set color override. RGB format, values from 0 to 255. It has to be re-applied if map changes.\n"
						"Current value: %s\n"
						, arg0, arg1, arg2
						, g_CustomSky.colorClouds.use ? g_CustomSky.colorClouds.userValue.c_str() : "default"
					);
					return;
				}

				if (!_stricmp(arg2, "draw")) {
					if (4 <= argc)
					{
						g_CustomSky.drawClouds = 0 != atoi(args->ArgV(3));
						return;
					}

					advancedfx::Message(
						"Usage:\n"
						"%s %s %s 0|1 - Enable (1) / disable (0) drawing of clouds.\n"
						"Current value: %s\n"
						, arg0, arg1, arg2
						, g_CustomSky.drawClouds ? "1" : "0"
					);
					return;
				}

			}
			advancedfx::Message(
				"Usage:\n"
				"%s %s color <iR> <iG> <iB> | default - Set color override. RGB format, values from 0 to 255. It has to be re-applied if map changes.\n"
				"%s %s draw 0|1 - Enable (1) / disable (0) drawing of clouds.\n"
				, arg0, arg1
				, arg0, arg1
			);
			return;
		}

	}

	advancedfx::Message(
		"Usage:\n"
		"%s color <iR> <iG> <iB> <iA> | default - Set color override. RGBA format, values from 0 to 255.\n"
		"%s material <sRelativePathToFile> | default - Set skybox material. It has to be re-applied if map changes.\n"
		"%s clouds [...] - Control clouds.\n"
		"Note: see wiki on GitHub for this command for details.\n"
		, arg0
		, arg0
		, arg0
	);
}

void HookMaterialSystem(HMODULE materialSystemDll) {
	// 1 xref, called in the middle of 23 function of vtable for CMaterialSystem2
	if (auto addr = getAddress(materialSystemDll, "e8 ?? ?? ?? ?? 4c 8b f8 4c 8b f6 48 85 c0")) {
		org_MaterialFindParam = (MaterialFindParam_t)(addr + 5 + *(int32_t*)(addr + 1));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));
	
	// function call in 30 function of vtable for CMaterial
	if (auto addr = getAddress(materialSystemDll, "f6 41 ?? 04 ?? ?? e8 ?? ?? ?? ?? 80 63 ?? fb")) {
		org_MaterialUpdate = (MaterialUpdate_t)(addr + 11 + *(int32_t*)(addr + 7));
	} else ErrorBox(MkErrStr(__FILE__, __LINE__));

	// calls 80th function for CResourceSystem, and has 0x74616d76
	// The 80th for CResourceSystem has following strings:
	// "WARNING: %s resource '%s' (%016llX) requested but is not in the system. (Missing from from a manifest?)\n"
	// "ERROR: %s resource '%s' (%016llX) requested is not loaded and may have been deleted.\n"
	org_FindMaterial = (FindMaterial_t)getVTableFn(materialSystemDll, 14, ".?AVCMaterialSystem2@@");
	if (0 == org_FindMaterial) ErrorBox(MkErrStr(__FILE__, __LINE__));
}

void HookSceneSystem(HMODULE sceneSystemDll) {
	org_DrawBaseSceneObject = (DrawBaseSceneObject_t)getVTableFn(sceneSystemDll, 1, ".?AVCBaseSceneObjectDesc@@");
	if (0 == org_DrawBaseSceneObject) ErrorBox(MkErrStr(__FILE__, __LINE__));

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)org_DrawBaseSceneObject, new_DrawBaseSceneObject);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour SceneSystem functions.");
		return;
	}
}
