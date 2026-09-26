#include "SceneSystem.h"

#include "ClientEntitySystem.h"
#include "Globals.h"
#include "SchemaSystem.h"
#include "MirvColors.h"
#include "StreamSettings.h"

#include "../shared/StringTools.h"
#include "../deps/release/Detours/src/detours.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier0/memalloc.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/bufferstring.h"

#include <atomic>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>

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

uint32_t g_Skybox_UnkPtr_Offset;

struct CustomSkyState {
	std::string currentSkyName = "";

	MyColor color;
	MyColor colorClouds;
	bool drawClouds = true;
	float brightness = 1.0f;
} g_CustomSky;

std::map<void *, size_t> g_PickerEntities;

bool g_PickerActive =  false;
bool g_PickerCollecting = false;
bool g_PickerPrint = false;

void Picker_Pick(bool wasVisible)
{
	if (!g_PickerActive)
	{
		g_PickerActive = true;
		g_PickerCollecting = true;
		g_PickerPrint = true;
	}
	else
	{
		if (g_PickerCollecting)
			g_PickerCollecting = false;

		size_t index = 0;

		for (auto it = g_PickerEntities.begin(); it != g_PickerEntities.end(); )
		{
			size_t oldIndex = it->second;

			if ((1 == (oldIndex & 0x1)) == wasVisible)
			{
				it = g_PickerEntities.erase(it);
			}
			else
			{
				it->second = index;
				++index;
				++it;
			}
		}
	}

	g_PickerPrint = true;
}

void Picker_Stop(void)
{
	if(g_PickerActive)
	{
		g_PickerEntities.clear();
		g_PickerActive = false;
		advancedfx::Message("==== Picker stopped. ====\n");
	}
}

void PickerPrintReset(){
	if(g_PickerPrint) {
		g_PickerPrint = false;

		bool determinedEntities = g_PickerEntities.size() <= 1;
		if (determinedEntities) Picker_Stop();
	}
}

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
	auto org_GetMaterials = (GetMaterials_t)vtable[32];

	org_GetMaterials(this, 0x74616d76, out, 7);
}

CMaterial2** CResourceSystem::PreCache (const char* name) {
	// resourcesystem.dll
	// "Resource \"%s\" was explicitly loaded with a blocking load.\n"
	// There are 2 precache functions in vtable next to each other, this is first one
	typedef CMaterial2** (__fastcall * PreCacheFn_t)(void* This, CBufferStringWrapper* name, const char* unk);
	void** vtable = *(void***)(this);
	auto org_PreCache = (PreCacheFn_t)vtable[40];

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
bool didSaveOriginalSkyColor = false;
uint32_t orgSkyColor = -1;
float orgSkyBrightness = 0.0f;

void resetCachedMaterials () {
	previousSkybox = "";
	g_CustomSky.currentSkyName = "";
	cachedMaterials.clear();
	didSaveOriginalSkyColor = false;
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
			auto counter = *(uint32_t*)((u_char*)newMat + 0x20);
			counter = counter + 1;
			*(uint32_t*)((u_char*)newMat + 0x20) = counter;
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
		if (!didSaveOriginalSkyColor) {
			orgSkyColor = *(uint32_t*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_vTintColor);
			orgSkyBrightness = *(float*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_flBrightnessScale);
			didSaveOriginalSkyColor = true;
		}
		*(uint32_t*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_vTintColor) = afxUtils::rgbaToHex(g_CustomSky.color.value);
		*(float*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_flBrightnessScale) = g_CustomSky.brightness;
	} else if (didSaveOriginalSkyColor) {
		*(uint32_t*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_vTintColor) = orgSkyColor;
		*(float*)((u_char*)This + g_clientDllOffsets.C_EnvSky.m_flBrightnessScale) = orgSkyBrightness;
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
			if (0 != _stricmp("env_sky", ent->GetClassName())) continue;

			// we have to remove pointer to object, so it can update
			// see dissasembly for the update function
			*(void**)((u_char*)ent + g_Skybox_UnkPtr_Offset) = nullptr;
			new_ForceUpdateSkybox(ent);
		}
	}

	if (g_CustomSky.currentSkyName.size() == 0 && previousSkybox.size() != 0) {
		previousSkybox = "";
	}

	if (!g_CustomSky.color.use && didSaveOriginalSkyColor) {
		didSaveOriginalSkyColor = false;
	}

}

size_t g_SceneLayer_pszViewPass_Offset = -1;
size_t g_SceneLayer_Flags_Offset = -1;

struct CBaseSceneData {
	char _pad0[0x18];
	void* sceneObject;
	CMaterial2* material;
	char _pad1[0x28];
	uint32_t color;
	char _pad2[0x14];
};

size_t g_SceneObject_pSceneObjectDesc_Offset = -1;

static_assert(sizeof(CBaseSceneData) == 0x68, "Unexpected CBaseSceneData size.");

enum class SceneObjectDrawPolicy {
	Draw,
	Hide,
	DepthPassesOnly
};

SceneObjectDrawPolicy g_BaseSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
SceneObjectDrawPolicy g_AnimatableSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
SceneObjectDrawPolicy g_AggregateSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
SceneObjectDrawPolicy g_OverlaysPolicy = SceneObjectDrawPolicy::Draw;
int g_iSceneFilterDebug = 0;

enum class SceneSemanticGroup : int {
	ViewModel = 0,
	Particles = 1,
	FirstPersonLegs = 2,
	Shells = 3,
	Weapons = 4,
	Players = 5,
	World = 6,
	Sky = 7,
	Smoke = 8,
	Count = 9
};

SceneObjectDrawPolicy g_SceneSemanticPolicies[(int)SceneSemanticGroup::Count] = {
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw,
	SceneObjectDrawPolicy::Draw
};

std::atomic_bool g_bSceneFilterSystemActive = false;

void UpdateSceneFilterSystemActive() {
	bool bActive =
		0 < g_iSceneFilterDebug
		|| g_BaseSceneObjectsPolicy != SceneObjectDrawPolicy::Draw
		|| g_AnimatableSceneObjectsPolicy != SceneObjectDrawPolicy::Draw
		|| g_AggregateSceneObjectsPolicy != SceneObjectDrawPolicy::Draw
		|| g_OverlaysPolicy != SceneObjectDrawPolicy::Draw
		|| g_PickerActive;

	for(int i = 0; !bActive && i < (int)SceneSemanticGroup::Count; i++) {
		bActive = g_SceneSemanticPolicies[i] != SceneObjectDrawPolicy::Draw;
	}

	g_bSceneFilterSystemActive = bActive;
}

void ClearSceneFliterSystemPolicies() {
	g_BaseSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
	g_AnimatableSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
	g_AggregateSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
	g_OverlaysPolicy = SceneObjectDrawPolicy::Draw;

	for(int i = 0; i < (int)SceneSemanticGroup::Count; i++) {
		g_SceneSemanticPolicies[i] = SceneObjectDrawPolicy::Draw;
	}

	UpdateSceneFilterSystemActive();
}

void SetupSceneFilterPolicies(const class CStreamSettings & settings) {
	switch(settings.ViewModelAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::ViewModel] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::ViewModel] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::ViewModel] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.ParticlesAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Particles] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Particles] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Particles] = SceneObjectDrawPolicy::Draw;
		break;
	}	
	switch(settings.FirstPersonLegsAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::FirstPersonLegs] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::FirstPersonLegs] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::FirstPersonLegs] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.ShellsAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Shells] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Shells] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Shells] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.WeaponsAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Weapons] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Weapons] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Weapons] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.PlayersAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Players] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Players] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Players] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.WorldAction) {
	case CStreamSettings::Action::NoDraw:
		g_AggregateSceneObjectsPolicy = SceneObjectDrawPolicy::Hide;
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::World] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_AggregateSceneObjectsPolicy = SceneObjectDrawPolicy::DepthPassesOnly;
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::World] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_AggregateSceneObjectsPolicy = SceneObjectDrawPolicy::Draw;
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::World] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.SkyAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Sky] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Sky] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Sky] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.SmokeAction) {
	case CStreamSettings::Action::NoDraw:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Smoke] = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Smoke] = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_SceneSemanticPolicies[(int)SceneSemanticGroup::Smoke] = SceneObjectDrawPolicy::Draw;
		break;
	}
	switch(settings.OverlaysAction) {
	case CStreamSettings::Action::NoDraw:
		g_OverlaysPolicy = SceneObjectDrawPolicy::Hide;
		break;
	case CStreamSettings::Action::ZOnly:
		g_OverlaysPolicy = SceneObjectDrawPolicy::DepthPassesOnly;
		break;
	default:
		g_OverlaysPolicy = SceneObjectDrawPolicy::Draw;
		break;
	}

	UpdateSceneFilterSystemActive();
}

enum class SceneObjectFilterClass {
	Base,
	Animatable,
	Aggregate,
	SkyBox,
	SkyBox6Face,
	ProjectedDecal,
	SmokeVolume,
	Particles,
	Unknown
};

std::map<void**,SceneObjectFilterClass> g_VtableToSceneObjectFilterClass;

struct SceneLayerContext {
	const char* ViewName = nullptr;
	const char* ViewPass = nullptr;
	uint32_t Flags = 0;
};

static const char* SceneObjectFilterClassToString(SceneObjectFilterClass filterClass) {
	switch (filterClass) {
	case SceneObjectFilterClass::Base: return "base";
	case SceneObjectFilterClass::Animatable: return "animatable";
	case SceneObjectFilterClass::Aggregate: return "aggregate";
	case SceneObjectFilterClass::SkyBox: return "skybox";
	case SceneObjectFilterClass::SkyBox6Face: return "skybox6face";
	case SceneObjectFilterClass::ProjectedDecal: return "projecteddecal";
	case SceneObjectFilterClass::SmokeVolume: return "smokevolume";
	case SceneObjectFilterClass::Particles: return "particles";
	default: return "unknown";
	}
}

static const char* SceneObjectDrawPolicyToString(SceneObjectDrawPolicy policy) {
	switch (policy) {
	case SceneObjectDrawPolicy::Draw: return "draw";
	case SceneObjectDrawPolicy::Hide: return "hide";
	case SceneObjectDrawPolicy::DepthPassesOnly: return "zonly";
	default: return "unknown";
	}
}

static bool TryParseSceneObjectDrawPolicy(const char* value, SceneObjectDrawPolicy& outPolicy) {
	if (0 == _stricmp(value, "1") || 0 == _stricmp(value, "draw")) {
		outPolicy = SceneObjectDrawPolicy::Draw;
		return true;
	}
	if (0 == _stricmp(value, "0") || 0 == _stricmp(value, "hide")) {
		outPolicy = SceneObjectDrawPolicy::Hide;
		return true;
	}
	if (0 == _stricmp(value, "zonly")) {
		outPolicy = SceneObjectDrawPolicy::DepthPassesOnly;
		return true;
	}
	return false;
}

static const char* SceneSemanticGroupToString(SceneSemanticGroup group) {
	switch (group) {
	case SceneSemanticGroup::ViewModel: return "viewModel";
	case SceneSemanticGroup::Particles: return "particles";
	case SceneSemanticGroup::FirstPersonLegs: return "firstPersonLegs";
	case SceneSemanticGroup::Shells: return "shells";
	case SceneSemanticGroup::Weapons: return "weapons";
	case SceneSemanticGroup::Players: return "players";
	case SceneSemanticGroup::World: return "world";
	case SceneSemanticGroup::Sky: return "sky";
	case SceneSemanticGroup::Smoke: return "smoke";
	default: return "unknown";
	}
}

static bool TryGetSceneSemanticGroup(const char* value, SceneSemanticGroup& outGroup) {
	if (0 == _stricmp(value, "viewModel")) {
		outGroup = SceneSemanticGroup::ViewModel;
		return true;
	}
	if (0 == _stricmp(value, "particles")) {
		outGroup = SceneSemanticGroup::Particles;
		return true;
	}
	if (0 == _stricmp(value, "firstPersonLegs")) {
		outGroup = SceneSemanticGroup::FirstPersonLegs;
		return true;
	}
	if (0 == _stricmp(value, "weapons")) {
		outGroup = SceneSemanticGroup::Weapons;
		return true;
	}
	if (0 == _stricmp(value, "shells")) {
		outGroup = SceneSemanticGroup::Shells;
		return true;
	}
	if (0 == _stricmp(value, "players") || 0 == _stricmp(value, "player")) {
		outGroup = SceneSemanticGroup::Players;
		return true;
	}
	if (0 == _stricmp(value, "world")) {
		outGroup = SceneSemanticGroup::World;
		return true;
	}
	if (0 == _stricmp(value, "sky") || 0 == _stricmp(value, "skybox")) {
		outGroup = SceneSemanticGroup::Sky;
		return true;
	}
	if (0 == _stricmp(value, "smoke")) {
		outGroup = SceneSemanticGroup::Smoke;
		return true;
	}
	return false;
}

static SceneObjectDrawPolicy GetSceneObjectClassPolicy(SceneObjectFilterClass filterClass) {
	switch (filterClass) {
	case SceneObjectFilterClass::Base: return g_BaseSceneObjectsPolicy;
	case SceneObjectFilterClass::Animatable: return g_AnimatableSceneObjectsPolicy;
	case SceneObjectFilterClass::Aggregate: return g_AggregateSceneObjectsPolicy;
	case SceneObjectFilterClass::SkyBox: return SceneObjectDrawPolicy::Draw;
	case SceneObjectFilterClass::SkyBox6Face: return SceneObjectDrawPolicy::Draw;
	case SceneObjectFilterClass::ProjectedDecal: return SceneObjectDrawPolicy::Draw;
	case SceneObjectFilterClass::Unknown:
	default: return SceneObjectDrawPolicy::Draw;
	}
}

static bool SceneObjectFilterClassIsSkyBox(SceneObjectFilterClass filterClass) {
	return filterClass == SceneObjectFilterClass::SkyBox
		|| filterClass == SceneObjectFilterClass::SkyBox6Face;
}

static bool SceneObjectFilterClassIsProjectedDecal(SceneObjectFilterClass filterClass) {
	return filterClass == SceneObjectFilterClass::ProjectedDecal;
}

static bool SceneObjectFilterClassHasKnownSceneDataLayout(SceneObjectFilterClass filterClass) {
	return true;
	/*return filterClass == SceneObjectFilterClass::Base
		|| filterClass == SceneObjectFilterClass::Animatable
		|| filterClass == SceneObjectFilterClass::Aggregate;*/
}

static bool StringContains(const char* value, const char* pattern) {
	return value && pattern && nullptr != strstr(value, pattern);
}

static bool SceneLayerContextIsViewModel(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "ViewModel");
}

static bool SceneLayerContextIsFirstPersonLegs(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "FirstpersonLegs");
}

static bool SceneLayerContextIsSky(const SceneLayerContext& context) {
	return StringContains(context.ViewName, "Csgo3DSkyboxView")
		|| StringContains(context.ViewPass, "3DSkybox")
		|| StringContains(context.ViewPass, "Skybox")
		|| StringContains(context.ViewPass, "Sky");
}

static bool SceneLayerContextIsVisiblity(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "Visibility") // e.g. PlayerVisibility...
		|| StringContains(context.ViewPass, "Occlusion") // e.g. PlayerOcclusion...
	;
}

static bool SceneLayerContextIsPlayers(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "Characters")
		|| StringContains(context.ViewPass, "Player");
}

static bool SceneLayerContextIsShadowPass(const SceneLayerContext& context) {
	return StringContains(context.ViewName, "CSM")
		|| StringContains(context.ViewName, "DrawShadowMapsForLight")
		|| StringContains(context.ViewPass, "DrawShadowMapsForLight");
}

static bool SceneLayerContextIsDecals(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "Decals");
}

static bool SceneLayerContextIsStencil(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "Stencil");

}

static bool SceneLayerContextIsCables(const SceneLayerContext& context) {
	return StringContains(context.ViewPass, "Cables") ;
}

static bool SceneLayerContextIsDepthPassesOnlyExcluded(const SceneLayerContext& context) {
	return SceneLayerContextIsDecals(context)
		|| SceneLayerContextIsCables(context)
		|| SceneLayerContextIsStencil(context)
		|| StringContains(context.ViewPass, "Translucent")
		|| StringContains(context.ViewPass, "Overlay");
}

static bool SceneDataMaterialIsPlayer(SceneObjectFilterClass filterClass, const char* materialName) {
	if (materialName == nullptr) return false;

	return StringContains(materialName, "characters/models/")
		|| StringContains(materialName, "agents/models/")
		|| StringContains(materialName, "models/player/")
		|| (filterClass == SceneObjectFilterClass::Animatable && StringContains(materialName, "compmat_")); // skinned weapons/gloves
}

static bool SceneDataMaterialIsWeapon(SceneObjectFilterClass filterClass, const char* materialName, const CBaseSceneData * pSceneData, const SceneLayerContext& context) {
	if (materialName == nullptr) return false;

	if (StringContains(materialName, "weapons/keychains/") 
		// || StringContains(materialName, "weapons/models/shared/stattrak")
		// || StringContains(materialName, "weapons/models/shared/nametag")
		|| StringContains(materialName, "weapons/models/")
		|| StringContains(materialName, "models/weapons/")
	) {
		return !SceneLayerContextIsPlayers(context);
	}

	if (StringContains(materialName, "compmat_"))
		// && filterClass == SceneObjectFilterClass::Animatable)
	{
		if (nullptr == pSceneData) return false;
		auto sceneObject = pSceneData->sceneObject;
		if (nullptr == sceneObject) return false;
		auto meshInstance = *(u_char**)((u_char*)sceneObject + 0x10);
		if (nullptr == meshInstance) return false;
		auto model = **(u_char***)(meshInstance + 0x8);
		auto modelName = *(const char**)(model + 0x8);

		if (StringContains(modelName, "weapons/models")) {// double check because we might get gloves in case they have skin
			return !SceneLayerContextIsPlayers(context);
			// auto modelState = (*(u_char**)((u_char*)sceneObject + 0x110)) + 0x10;
			// // see 3 fn in vtable for ".?AVNetworkVar_m_modelState@CSkeletonInstance@@"
			// auto ent = *(CEntityInstance**)(modelState - 0x140 - 0x38);
			// auto ownerHandle = SOURCESDK::CS2::CEntityHandle::CEntityHandle(*(uint32_t*)((u_char*)ent + g_clientDllOffsets.C_BaseEntity.m_hOwnerEntity));
			// if (ownerHandle.IsValid()) return false;
			// return true;
		}
	}

	return false;
}

static bool SceneDataMaterialIsShell(SceneObjectFilterClass filterClass, const char* materialName) {
	if (materialName == nullptr) return false;

	return StringContains(materialName, "materials/models/weapons/shared/shells/")
		|| StringContains(materialName, "weapons/models/shared/shells/");
}

static bool SceneDataIsCables(SceneObjectFilterClass filterClass, const CBaseSceneData * pSceneData, const SceneLayerContext& context) {
	if(filterClass != SceneObjectFilterClass::Particles) return false;
	if(SceneLayerContextIsCables(context)) return true;
	// pSceneObject + 0x10 -> object of ".?AVCParticleSystemDefinition@@"
	// poi(poi(poi(poi(poi(pSceneObject+0x10)+0x90)))-8)+c --> ".?AVC_OP_RopeSpringConstraint@@"
	// poi(poi(pSceneObject+0x10)+0x250) --> // particles/entity/path_particle_cable_static.vpcf
	if (nullptr == pSceneData) return false;
	auto sceneObject = pSceneData->sceneObject;
	auto particleSystemDefinition = *(u_char**)((u_char*)sceneObject + 0x10);
	if (nullptr == particleSystemDefinition) return false;	
	const char * particleDefinitionFilePath = (const char *)*(u_char**)((u_char*)particleSystemDefinition + 0x250);
	return nullptr != particleDefinitionFilePath && StringContains(particleDefinitionFilePath, "particle_cable"); // There are multiple!
}

static SceneSemanticGroup ClassifySceneObject(SceneObjectFilterClass filterClass, const SceneLayerContext& context, const char* materialName, const CBaseSceneData * pSceneData) {
	// Precedence is intentional.
	if (!SceneDataIsCables(filterClass,pSceneData,context) && filterClass == SceneObjectFilterClass::Particles) return SceneSemanticGroup::Particles;
	if (SceneLayerContextIsViewModel(context)) return SceneSemanticGroup::ViewModel;
	if (SceneLayerContextIsFirstPersonLegs(context)) return SceneSemanticGroup::FirstPersonLegs;
	if (SceneDataMaterialIsShell(filterClass, materialName)) return SceneSemanticGroup::Shells;
	if (SceneDataMaterialIsWeapon(filterClass, materialName, pSceneData, context)) return SceneSemanticGroup::Weapons;
	if (SceneDataMaterialIsPlayer(filterClass, materialName)) return SceneSemanticGroup::Players;
	if (SceneLayerContextIsPlayers(context)) return SceneSemanticGroup::Players;
	if (SceneLayerContextIsSky(context)) return SceneSemanticGroup::Sky;
	if (filterClass == SceneObjectFilterClass::SmokeVolume) return SceneSemanticGroup::Smoke;
	return SceneSemanticGroup::World;
}

static bool SceneLayerContextIsDepthPass(const SceneLayerContext& context) {
	if (StringContains(context.ViewName, "CSM")) return true;
	if (StringContains(context.ViewPass, "DrawDepthOpaque")) return true;
	if (StringContains(context.ViewPass, "Depth Prepass")) return true;
	if (StringContains(context.ViewPass, "Depth prepass")) return true;

	return false;
}

static SceneObjectDrawPolicy ApplyLayerAwarePolicy(SceneObjectFilterClass filterClass, const SceneLayerContext & context, const char* materialName, SceneObjectDrawPolicy policy) {
	if (policy != SceneObjectDrawPolicy::Draw) {
		if(SceneLayerContextIsShadowPass(context)) {
			return SceneObjectDrawPolicy::Draw;
		}
	}

	if (policy == SceneObjectDrawPolicy::DepthPassesOnly) {
		if(SceneLayerContextIsDepthPass(context) && !SceneLayerContextIsDepthPassesOnlyExcluded(context)
			|| SceneLayerContextIsVisiblity(context)) {
			return SceneObjectDrawPolicy::Draw;
		}
	}

	return nullptr == materialName && filterClass == SceneObjectFilterClass::Unknown
		&& SceneLayerContextIsStencil(context)
		? SceneObjectDrawPolicy::Draw
		: policy
	;
}

static bool TryGetSceneSemanticPolicy(SceneObjectFilterClass filterClass, const SceneLayerContext & context, const char* materialName, SceneObjectDrawPolicy& outPolicy, const CBaseSceneData * sceneData) {
	SceneSemanticGroup group = ClassifySceneObject(filterClass, context, materialName, sceneData);
	SceneObjectDrawPolicy policy = g_SceneSemanticPolicies[(int)group];
	// Semantic draw is the default state; descriptor-class policies still apply unless a semantic group is hidden / depth-pass-only.
	if (policy == SceneObjectDrawPolicy::Draw) return false;

	outPolicy = ApplyLayerAwarePolicy(filterClass, context, materialName, policy);
	return true;
}

static const char* GetSceneObjectDescName(const CBaseSceneData& sceneData) {
	if (sceneData.sceneObject == nullptr) return nullptr;

	void* desc = *(void**)((unsigned char*)sceneData.sceneObject + g_SceneObject_pSceneObjectDesc_Offset);
	if (desc == nullptr) return nullptr;

 	// See notes in HookSceneSystem about DebugSceneData for vtabel offset:
	void** vtable = *(void***)desc;
	return ((const char* (__fastcall*)(void*))vtable[0])(desc);
}

/* This doesn't seem quite correct, removing feature for now.
static uint32_t GetSceneDataSort(const CBaseSceneData& sceneData) {
	return *(uint32_t*)((const unsigned char*)&sceneData + 0x58);
}*/

/* This is probably correct, but we don't need it for now.
static uint16_t GetSceneDataFlags(const CBaseSceneData& sceneData) {
	return *(uint16_t*)((const unsigned char*)&sceneData + g_SceneData_Flags_Offset);
}*/

static bool DebugPrintSceneData(const char * pContextTitle, SceneObjectFilterClass filterClass, const SceneLayerContext & context, const CBaseSceneData* sceneData, int count) {
	if (g_iSceneFilterDebug <= 0 && !g_PickerActive || count <= 0) return true;

	bool canReadSceneDataFields = sceneData && SceneObjectFilterClassHasKnownSceneDataLayout(filterClass);

	bool hidden = false;

	for (int i = 0; i < count; ++i) {
		const char* materialName = canReadSceneDataFields ? (sceneData[i].material ? sceneData[i].material->GetName() : nullptr) : nullptr;
		const char* descName = canReadSceneDataFields ? GetSceneObjectDescName(sceneData[i]) : nullptr;
		void* sceneObject = canReadSceneDataFields ? sceneData[i].sceneObject : nullptr;

		bool bInList = false;
		bool bIHidden = false;

		if(g_PickerActive && sceneObject) {
			if (!g_PickerCollecting)
			{
				auto itEnt = g_PickerEntities.find(sceneObject);
				bInList = g_PickerEntities.end() != itEnt;
				if(bInList){
					bIHidden = bInList && ((itEnt->second) & 0x1) == 1;
				}
			}
			else
			{
				auto itEnt = g_PickerEntities.lower_bound(sceneObject);
				if (itEnt == g_PickerEntities.end() || (sceneObject < itEnt->first))
				{
					itEnt = g_PickerEntities.emplace_hint(itEnt, std::piecewise_construct, std::forward_as_tuple(sceneObject), std::forward_as_tuple(g_PickerEntities.size()));
				}

				bInList = true;
				bIHidden = ((itEnt->second) & 0x1) == 1;
			}
		}

		hidden = hidden || bIHidden;

		if((!bInList || bIHidden || !g_PickerPrint) && g_iSceneFilterDebug <= 0) continue;

		advancedfx::Message(
			"AFXDEBUG: mirv_scene_filter [%s] %s[%i/%i] layer=%s:%s group=%s desc=%s material=%s sceneObject=0x%p, flags=0x%08x\n",
			pContextTitle,
			SceneObjectFilterClassToString(filterClass),
			i,
			count,
			context.ViewName ? context.ViewName : "?",
			context.ViewPass ? context.ViewPass : "?",
			SceneSemanticGroupToString(ClassifySceneObject(filterClass, context, materialName, sceneData)),
			descName ? descName : "?",
			materialName ? materialName : "?",
			sceneObject,
			context.Flags
		);
	}

	return !hidden;
}

static SceneObjectDrawPolicy GetSceneDataPolicy(SceneObjectFilterClass filterClass, const SceneLayerContext & context, const CBaseSceneData * pSceneData) {

	if (SceneObjectFilterClassIsSkyBox(filterClass)) {
		SceneObjectDrawPolicy policy = ApplyLayerAwarePolicy(filterClass, context, nullptr, g_SceneSemanticPolicies[(int)SceneSemanticGroup::Sky]);
		return policy == SceneObjectDrawPolicy::DepthPassesOnly ? SceneObjectDrawPolicy::Hide : policy;
	}

	if (SceneObjectFilterClassIsProjectedDecal(filterClass)) {
		SceneObjectDrawPolicy policy = g_SceneSemanticPolicies[(int)SceneSemanticGroup::World];
		return policy == SceneObjectDrawPolicy::DepthPassesOnly ? SceneObjectDrawPolicy::Hide : policy;
	}

	if(SceneLayerContextIsCables(context)) {
		SceneObjectDrawPolicy policy = g_SceneSemanticPolicies[(int)SceneSemanticGroup::World];
		return policy == SceneObjectDrawPolicy::DepthPassesOnly ? SceneObjectDrawPolicy::Hide : policy;
	}

	if (!SceneObjectFilterClassHasKnownSceneDataLayout(filterClass) || nullptr == pSceneData || pSceneData->material == nullptr) {
		SceneObjectDrawPolicy semanticPolicy;
		if (TryGetSceneSemanticPolicy(filterClass, context, nullptr, semanticPolicy, pSceneData)) return semanticPolicy;
		return ApplyLayerAwarePolicy(filterClass, context, nullptr, GetSceneObjectClassPolicy(filterClass));
	}

	const char* materialName = pSceneData->material->GetName();
	if (materialName == nullptr) {
		SceneObjectDrawPolicy semanticPolicy;
		if (TryGetSceneSemanticPolicy(filterClass, context, nullptr, semanticPolicy, pSceneData)) return semanticPolicy;
		return ApplyLayerAwarePolicy(filterClass, context, nullptr, GetSceneObjectClassPolicy(filterClass));
	}

	// Custom Sky related:
	//
	// There is some z fight going on, when camera is pointed to sun
	// e.g. clouds draw regardless, so have to hide some sun materials too
	// Don't have much time to fix it properly, this does the job 99% of times
	if (!g_CustomSky.drawClouds) {
		std::string matName = materialName;
		if (matName.find("clouds") != std::string::npos || matName.find("sun_disc_glow") != std::string::npos) {
			return SceneObjectDrawPolicy::Hide;
		}
	}	

	SceneObjectDrawPolicy semanticPolicy;
	if (TryGetSceneSemanticPolicy(filterClass, context, materialName, semanticPolicy, pSceneData)) return semanticPolicy;
	return ApplyLayerAwarePolicy(filterClass, context, materialName, GetSceneObjectClassPolicy(filterClass));
}

std::shared_timed_mutex g_BlockedSoftwareCommandListsMutex;
std::set<void *> g_BlockedSoftwareCommandLists;
std::atomic<void *> g_BeforeUi_SoftwareCommandLists;

void ClearThreadSceneLayerContexts(){
	if(g_bSceneFilterSystemActive) {
		g_PickerPrint = false;
		{
			std::unique_lock<std::shared_timed_mutex> lock(g_BlockedSoftwareCommandListsMutex);
			g_BlockedSoftwareCommandLists.clear();
		}
	}
}

extern bool BlockColorDepth(void * pCRenderContextDx11_SoftwareCommandList, bool bColor, bool bDepth);

typedef void (__fastcall * RenderLayerDrawListPart_t)(void * pSceneSystem, void * param_2, void * pCSceneLayer, unsigned char * pDrawingData, unsigned int count, void *param_6);
RenderLayerDrawListPart_t org_RenderLayerDrawListPart = nullptr;

/*
void __fastcall new_RenderLayerDrawListPart(void * pSceneSystem, void * param_2, void * pCSceneLayer, unsigned char * pDrawingData, unsigned int count, void *param_6) {
	if(g_bSceneFilterSystemActive)
	{
		SceneLayerContext context;
		void* pCSceneView = *(void**)((unsigned char*)pCSceneLayer + 0x6f0);
		if (pCSceneView) {
			void** vtable = *(void***)pCSceneView;
			context.ViewName = ((const char* (__fastcall*)(void*))vtable[0])(pCSceneView);
		}
		context.ViewPass = (const char*)pCSceneLayer + g_SceneLayer_pszViewPass_Offset;
		context.Flags = *(uint32_t*)((unsigned char*)pCSceneLayer + g_SceneLayer_Flags_Offset);
		
		{
			std::unique_lock<std::shared_timed_mutex> lock(g_RenderParam4ToSceneLayerContextsMutex);
			auto result = g_RenderParam4ToSceneLayerContexts.emplace(&(pDrawingData[i]), context);
			if(!result.second) result.first->second = context;
		}
	}

	org_RenderLayerDrawListPart(pSceneSystem, param_2, pCSceneLayer, pDrawingData, count, param_6);
}
*/

void BlockColorDepth(void * pThisSoftwareCommandList){
	bool blocked = false;
	{
		std::unique_lock<std::shared_timed_mutex> lock(g_BlockedSoftwareCommandListsMutex);
		blocked = !g_BlockedSoftwareCommandLists.emplace(pThisSoftwareCommandList).second;
	}
	if(!blocked) BlockColorDepth(pThisSoftwareCommandList,true,true);
}

void CheckAndDo_Untoggle_BlockColorDepth(void * pThisSoftwareCommandList) {
	if(g_bSceneFilterSystemActive) {
		bool blocked;
		{
			std::shared_lock<std::shared_timed_mutex> lock(g_BlockedSoftwareCommandListsMutex);
			blocked = g_BlockedSoftwareCommandLists.find(pThisSoftwareCommandList) != g_BlockedSoftwareCommandLists.end();
		}
		if(blocked) {
			std::unique_lock<std::shared_timed_mutex> lock(g_BlockedSoftwareCommandListsMutex);
			auto it = g_BlockedSoftwareCommandLists.find(pThisSoftwareCommandList);
			blocked = it != g_BlockedSoftwareCommandLists.end();
			if(blocked) g_BlockedSoftwareCommandLists.erase(it);
		}
		if(blocked) {
			BlockColorDepth(pThisSoftwareCommandList,false,false);
		}
	}
}

void QueueCallbackBeforeUi(void* pCRenderContextDx11_SoftwareCommandList, int value);

typedef void * (__fastcall * SoftwareCommandList_Commit_t)(void * pThisSoftwareCommandList);
SoftwareCommandList_Commit_t org_SoftwareCommandList_Commit = nullptr;
void * __fastcall new_SoftwareCommandList_Commit(void * pThisSoftwareCommandList) {
	CheckAndDo_Untoggle_BlockColorDepth(pThisSoftwareCommandList);
	if(pThisSoftwareCommandList == g_BeforeUi_SoftwareCommandLists) {
		g_BeforeUi_SoftwareCommandLists = nullptr;
		QueueCallbackBeforeUi(pThisSoftwareCommandList, 0);
	}
	return org_SoftwareCommandList_Commit(pThisSoftwareCommandList);
}

void SetContextFromDrawingData(SceneLayerContext & context, void * pDrawingData) {
	void * pSceneView = ((void**)pDrawingData)[0];
	void * pSceneLayer = ((void**)pDrawingData)[1];
	
	void** pSceneViewVtable = *(void***)pSceneView;
	context.ViewName = ((const char* (__fastcall*)(void*))pSceneViewVtable[0])(pSceneView); // see DebugSceneData function for how to know it's fist function in vtable.

	context.ViewPass = (const char*)pSceneLayer + g_SceneLayer_pszViewPass_Offset;

	context.Flags = *(uint32_t*)((unsigned char*)pSceneLayer + g_SceneLayer_Flags_Offset);	
}

// Since the 2026-09-23 build there is a 5th (stack) argument: optional name suffix, formatted as "/%s" when not null.
// It must be forwarded, otherwise the original formats garbage from our stack frame and crashes in tier0.
typedef void (__fastcall * InitDrawingData_t)(unsigned char * pDrawingData,void *pSceneView,void *pSceneLayer,uint32_t unkFlags4,const char *pszNameSuffix);
InitDrawingData_t org_InitDrawingData = nullptr;
void __fastcall new_InitDrawingData(unsigned char * pDrawingData,void *pSceneView,void *pSceneLayer,uint32_t unkFlags4,const char *pszNameSuffix) {
	org_InitDrawingData(pDrawingData,pSceneView,pSceneLayer,unkFlags4,pszNameSuffix);

	void * pCRenderContextDx11_SoftwareCommandList = ((void **)pDrawingData)[4];
	
	if(org_SoftwareCommandList_Commit == nullptr) {
		void** vtable = *(void***)pCRenderContextDx11_SoftwareCommandList;
		org_SoftwareCommandList_Commit = (SoftwareCommandList_Commit_t)vtable[11];

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)org_SoftwareCommandList_Commit, new_SoftwareCommandList_Commit);

		if(NO_ERROR != DetourTransactionCommit()) {
			ErrorBox("Failed to detour SoftwareCommandList::Commit.");
			return;
		}				
	}

	SceneLayerContext context;
	SetContextFromDrawingData(context, pDrawingData);

	if(0 == strcmp("Player 0", context.ViewName)) {
		if(0 == strcmp("PostProcessing", context.ViewPass)) g_BeforeUi_SoftwareCommandLists = pCRenderContextDx11_SoftwareCommandList;
		else if(0 == strcmp("Legacy Sniper Scope", context.ViewPass)) {
			QueueCallbackBeforeUi(pCRenderContextDx11_SoftwareCommandList, -1); // abort, let render scope first.
			g_BeforeUi_SoftwareCommandLists = pCRenderContextDx11_SoftwareCommandList;
		}
	}

	if(g_bSceneFilterSystemActive && pDrawingData) {
		CheckAndDo_Untoggle_BlockColorDepth(pCRenderContextDx11_SoftwareCommandList);

		/*void** pSceneViewVtable = *(void***)pSceneView;
		SceneLayerContext context;
		context.ViewName = ((const char* (__fastcall*)(void*))pSceneViewVtable[0])(pSceneView); // see DebugSceneData function for how to know it's fist function in vtable.
		context.ViewPass = (const char*)pSceneLayer + g_SceneLayer_pszViewPass_Offset;
		context.Flags = *(uint32_t*)((unsigned char*)pSceneLayer + g_SceneLayer_Flags_Offset);*/
		/*{
			std::unique_lock<std::shared_timed_mutex> lock(g_RenderParam4ToSceneLayerContextsMutex);
			auto result = g_RenderParam4ToSceneLayerContexts.emplace(pDrawingData, context);
			if(!result.second) result.first->second = context;
		}*/

		if(0 < g_iSceneFilterDebug) {
			advancedfx::Message("AFXDEBUG: InitDrawingData layer=%s:%s flags=0x%08x unkFlags4=0x%08x\n",
				context.ViewName ? context.ViewName : "?",
				context.ViewPass ? context.ViewPass : "?",
				context.Flags,
				unkFlags4
			);
		}

		if(g_OverlaysPolicy != SceneObjectDrawPolicy::Draw && context.ViewPass && (
			0 == strcmp(context.ViewPass,"OverlaySmoke") // We want to let through "OverlaySmokeUpdate"
			|| StringContains(context.ViewPass,"FlashbangOverlay")
			|| StringContains(context.ViewPass,"Fade To Black")
			)
		) {	
			BlockColorDepth(pCRenderContextDx11_SoftwareCommandList);
		}
	}
}

typedef void (__fastcall * DrawSceneData_t)(void * pDrawingData, CBaseSceneData * pSceneData);
DrawSceneData_t org_DrawSceneData = nullptr;

typedef void (__fastcall * DrawCurrentPrimitives_t)(void * pDrawingData);
DrawCurrentPrimitives_t org_DrawCurrentPrimitives = nullptr;

void __fastcall new_DrawSceneData(void * pDrawingData, CBaseSceneData* pSceneData) {

	if(g_bSceneFilterSystemActive && nullptr != pDrawingData) {
		SceneLayerContext context;
		SetContextFromDrawingData(context, pDrawingData);

		SceneObjectFilterClass filterClass = SceneObjectFilterClass::Unknown;
		if(pSceneData != nullptr && pSceneData->sceneObject) {
			if(void* desc = *(void**)((unsigned char*)pSceneData->sceneObject + g_SceneObject_pSceneObjectDesc_Offset)) {
				void** vtable = *(void***)desc;
				auto it = g_VtableToSceneObjectFilterClass.find(vtable);
				if(it != g_VtableToSceneObjectFilterClass.end())
					filterClass = it->second;
			}
		}
		
		SceneObjectDrawPolicy policy;
		if(DebugPrintSceneData("DrawSceneData", filterClass, context, pSceneData, 1)) {
			policy = GetSceneDataPolicy(filterClass, context, pSceneData);
		} else {
			policy = SceneObjectDrawPolicy::Hide;
		}

		void * pCRenderContextDx11_SoftwareCommandList = ((void **)pDrawingData)[4];
		switch (policy) {
		default:
		case SceneObjectDrawPolicy::Draw:
			break;
		case SceneObjectDrawPolicy::DepthPassesOnly:
			BlockColorDepth(pCRenderContextDx11_SoftwareCommandList, true, false);
			break;
		case SceneObjectDrawPolicy::Hide:
			BlockColorDepth(pCRenderContextDx11_SoftwareCommandList, true, true);
			break;
		}
		org_DrawSceneData(pDrawingData, pSceneData);
		org_DrawCurrentPrimitives(pDrawingData); // Force draw to prevent merging and get correct state.
		switch (policy) {
		default:
		case SceneObjectDrawPolicy::Draw:
			break;
		case SceneObjectDrawPolicy::DepthPassesOnly:
		case SceneObjectDrawPolicy::Hide:
			BlockColorDepth(pCRenderContextDx11_SoftwareCommandList, false, false);
			break;
		}
		return;
	}

	org_DrawSceneData(pDrawingData, pSceneData);
}

/*
void __fastcall new_DrawCurrentPrimitives(void * pDrawingData) {
	if(g_bSceneFilterSystemActive && nullptr != pDrawingData) {
		SceneLayerContext context;
		SetContextFromDrawingData(context, pDrawingData);

		SceneObjectFilterClass filterClass = SceneObjectFilterClass::Unknown;	
			
		SceneObjectDrawPolicy policy;
		if(DebugPrintSceneData("DrawCurrentPrimitives", filterClass, context, nullptr, 1)) {
			policy = GetSceneDataPolicy(filterClass, context, nullptr);
		} else {
			policy = SceneObjectDrawPolicy::Hide;
		}

		switch (policy) {
		default:
		case SceneObjectDrawPolicy::Draw:
			break;
		case SceneObjectDrawPolicy::DepthPassesOnly:
		case SceneObjectDrawPolicy::Hide:
			{
				void * pCRenderContextDx11_SoftwareCommandList = ((void **)pDrawingData)[4];
				ToggleDrawing(pCRenderContextDx11_SoftwareCommandList, true);
				org_DrawCurrentPrimitives(pDrawingData);
				ToggleDrawing(pCRenderContextDx11_SoftwareCommandList, false);
			}
			return;
		}
	}

	org_DrawCurrentPrimitives(pDrawingData);
}
*/

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

CON_COMMAND(__mirv_scene_filter, "")
{
	auto argc = args->ArgC();
	auto arg0 = args->ArgV(0);

	if (3 <= argc)
	{
		auto arg1 = args->ArgV(1);
		auto arg2 = args->ArgV(2);
		SceneObjectDrawPolicy policy;
		SceneSemanticGroup semanticGroup;

		if (!_stricmp(arg1, "base"))
		{
			if (TryParseSceneObjectDrawPolicy(arg2, policy))
			{
				g_BaseSceneObjectsPolicy = policy;
				UpdateSceneFilterSystemActive();
				return;
			}
		}

		if (!_stricmp(arg1, "animatable"))
		{
			if (TryParseSceneObjectDrawPolicy(arg2, policy))
			{
				g_AnimatableSceneObjectsPolicy = policy;
				UpdateSceneFilterSystemActive();
				return;
			}
		}

		if (!_stricmp(arg1, "aggregate"))
		{
			if (TryParseSceneObjectDrawPolicy(arg2, policy))
			{
				g_AggregateSceneObjectsPolicy = policy;
				UpdateSceneFilterSystemActive();
				return;
			}
		}

		if (!_stricmp(arg1, "overlays"))
		{
			if (TryParseSceneObjectDrawPolicy(arg2, policy))
			{
				g_OverlaysPolicy = policy == SceneObjectDrawPolicy::Hide
					? SceneObjectDrawPolicy::Hide
					: SceneObjectDrawPolicy::Draw;
				UpdateSceneFilterSystemActive();
				return;
			}
		}		

		if (TryGetSceneSemanticGroup(arg1, semanticGroup))
		{
			if (TryParseSceneObjectDrawPolicy(arg2, policy))
			{
				g_SceneSemanticPolicies[(int)semanticGroup] = policy;
				UpdateSceneFilterSystemActive();
				return;
			}
		}

		if (!_stricmp(arg1, "debug"))
		{
			g_iSceneFilterDebug = atoi(arg2);
			if (g_iSceneFilterDebug < 0) g_iSceneFilterDebug = 0;
			UpdateSceneFilterSystemActive();
			return;
		}

		if (!_stricmp(arg1, "picker"))
		{
			if (!_stricmp(arg2, "ent") && 4 <= argc)
			{
				//curBaseFx->Console_DisableFastPathRequired();

				bool value = 0 != atoi(args->ArgV(3));
				Picker_Pick(value);
				UpdateSceneFilterSystemActive();
				return;
			}
			else
			if (!_stricmp(arg2, "stop"))
			{
				Picker_Stop();
				UpdateSceneFilterSystemActive();
				return;
			}
		}		
	}

	advancedfx::Message(
		"Usage:\n"
		"%s base draw|hide|zonly|0|1 - Controls CBaseSceneObjectDesc entries in the hooked scene draw path.\n"
		"%s animatable draw|hide|zonly|0|1 - Controls CAnimatableSceneObjectDesc entries in the hooked scene draw path.\n"
		"%s aggregate draw|hide|zonly|0|1 - Controls CAggregateSceneObjectDesc entries in the hooked scene draw path.\n"
		"%s smoke draw|hide|0|1 - Controls CSmokeVolumeObjectDesc by patching its vtable while hidden.\n"
		"%s overlays draw|hide|0|1 - Controls overlays pass.\n"
		"%s viewModel draw|hide|zonly - Controls viewmodel layers.\n"
		"%s particles draw|hide|zonly - Controls viewmodel effect layers.\n"
		"%s firstPersonLegs draw|hide|zonly - Controls first person legs layers.\n"
		"%s weapons draw|hide|zonly - Controls weapons layers.\n"
		"%s players draw|hide|zonly - Controls player character layers.\n"
		"%s world draw|hide|zonly - Controls layers not matched by another semantic group.\n"
		"%s sky draw|hide|zonly - Controls 3D skybox layers matched in the scene draw path.\n"
		"%s debug <iCount> - Print up to iCount entries for each hooked scene draw call. Use 0 to disable.\n"
		"%s picker ent 0|1 - Tell picker if entity is visible (1) or not (0). (Or start picking with 1.)\n"
		"%s picker stop - Stop picking.\n"
		"Current values: base=%s animatable=%s aggregate=%s smoke=%s overlays=%s viewModel=%s particles=%s firstPersonLegs=%s players=%s world=%s sky=%s debug=%i\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, SceneObjectDrawPolicyToString(g_BaseSceneObjectsPolicy)
		, SceneObjectDrawPolicyToString(g_AnimatableSceneObjectsPolicy)
		, SceneObjectDrawPolicyToString(g_AggregateSceneObjectsPolicy)
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::Smoke])
		, SceneObjectDrawPolicyToString(g_OverlaysPolicy)
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::ViewModel])
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::Particles])
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::FirstPersonLegs])
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::Players])
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::World])
		, SceneObjectDrawPolicyToString(g_SceneSemanticPolicies[(int)SceneSemanticGroup::Sky])
		, g_iSceneFilterDebug
	);
}

void HookMaterialSystem(HMODULE materialSystemDll) {
	// 1 xref, called in the middle of 23 function of vtable for CMaterialSystem2
	//
	// LAB_18003d0a1                                   XREF[1]:     18003d077 (j)   
    // 18003d0a1 49  8b  d4       MOV        RDX ,R12
    // 18003d0a4 48  8b  cd       MOV        RCX ,RBP
    // 18003d0a7 e8  84  4d       CALL       FUN_180011e30 <------
    //           fd  ff
    // 18003d0ac 4c  8b  e8       MOV        R13 ,RAX
    // 18003d0af 4c  8b  fd       MOV        R15 ,RBP
    // 18003d0b2 48  85  c0       TEST       RAX ,RAX
    // 18003d0b5 0f  84  16       JZ         LAB_18003d1d1
    //           01  00  00
    // 18003d0bb 0f  10  44       MOVUPS     XMM0 ,xmmword ptr [RSP  + local_38[0] ]
    //           24  30
    // 18003d0c0 c6  40  38  00    MOV        byte ptr [RAX  + 0x38 ],0x0
    // 18003d0c4 0f  11  00       MOVUPS     xmmword ptr [RAX ],XMM0
    // 18003d0c7 8b  86  68       MOV        EAX ,dword ptr [RSI  + 0x568 ]
    //           05  00  00
    // 18003d0cd 89  44  24  78    MOV        dword ptr [RSP  + local_res10 ],EAX
    // 18003d0d1 3b  86  78       CMP        EAX ,dword ptr [RSI  + 0x578 ]
    //           05  00  00
    //
	if (auto addr = getAddress(materialSystemDll, "E8 ?? ?? ?? ?? 4C 8B E8 4C 8B FD 48 85 C0")) {
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

void GetSceneObjectVtable(HMODULE dll, const char * mangledClass, SceneObjectFilterClass filterClass) {
	if (void ** vtable = (void **)Afx::BinUtils::FindClassVtable(dll, mangledClass, 0, 0)) {
		// vtable[0] - GetDesc()
		// vtable[1] - DrawArray()
		// vtable[3] - DrawSingle()
		g_VtableToSceneObjectFilterClass[vtable] = filterClass;
	} else {
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		ErrorBox(mangledClass);
	}
}

void GetClientDllSceneObjectVtable(HMODULE clientDll) {
	GetSceneObjectVtable(clientDll, ".?AVCSmokeVolumeSceneObjectDesc@@", SceneObjectFilterClass::SmokeVolume);
}

void GetParticlesDllSceneObjectVtable(HMODULE particlesDll) {
	GetSceneObjectVtable(particlesDll, ".?AVCParticleObjectDesc@@", SceneObjectFilterClass::Particles);
}

void HookSceneSystem(HMODULE sceneSystemDll) {
	GetSceneObjectVtable(sceneSystemDll, ".?AVCBaseSceneObjectDesc@@", SceneObjectFilterClass::Base);
	GetSceneObjectVtable(sceneSystemDll, ".?AVCAnimatableSceneObjectDesc@@", SceneObjectFilterClass::Animatable);
	GetSceneObjectVtable(sceneSystemDll, ".?AVCAggregateSceneObjectDesc@@", SceneObjectFilterClass::Aggregate);
	GetSceneObjectVtable(sceneSystemDll, ".?AVCSkyBoxObjectDesc@@", SceneObjectFilterClass::SkyBox);
	GetSceneObjectVtable(sceneSystemDll, ".?AVC6FaceSkyboxObjectDesc@@", SceneObjectFilterClass::SkyBox6Face);
	GetSceneObjectVtable(sceneSystemDll, ".?AVCProjectedDecalSceneObjectDesc@@", SceneObjectFilterClass::ProjectedDecal);

/*
// References "RenderLayerDrawListPart"
void FUN_1800ea9e0(longlong pSceneSystem,longlong *param_2,uint *pCSceneLayer,ulonglong param_4,
                  uint count,int *param_6)

{
	// [...]
	
    FUN_18009c590(param_4,uVar6,param_3,param_5); // <-- This is the InitDrawingData function
    local_118 = 0;

    if (0 < *param_6) {
      do {
        puVar25 = *(uint **)(*(longlong *)(param_6 + 2) + (longlong)puVar22);
        puVar23 = puVar25 + 2;
        local_f8 = puVar25;
        if (*(CThreadedJob **)((longlong)local_d8 + (longlong)puVar22) == (CThreadedJob *)0x0) {
          FUN_1800e7590(0,param_3,puVar23,*puVar25); // This is the DebugSceneData function
        }
        else {
          CThreadedJob::WaitOrImmediatelyExecute
                    (*(CThreadedJob **)((longlong)local_d8 + (longlong)puVar22));
        }
        uVar29 = *puVar25;
        uVar24 = (ulonglong)uVar29;
        if ((param_3[0x12] & 0x28) == 0) {
          if (uVar29 != 0) {
            do {
              uVar29 = puVar23[0x18];
              bVar12 = (byte)uVar29 & 3;
              bVar17 = *(byte *)(*(longlong *)(puVar23 + 6) + 0x9a) & 3;
              if (bVar17 < bVar12) {
                bVar17 = bVar12;
              }
              if (bVar17 != 0) {
                plVar9 = *(longlong **)(*(longlong *)(puVar23 + 6) + 0x18);
                (**(code **)(*plVar9 + 0x18))
                          (plVar9,*(undefined8 *)(param_4 + 0x20),puVar23,uVar6,param_3,bVar17);
              }
              if (((byte)uVar29 & 8) == 0) {
                FUN_18009c880(param_4,puVar23); // <--- This is the DrawSceneData function we are after, it's also called in a deeper tree further bellow from FUN_1800eb800 / RenderLayerDrawListPart
              }
              else {
                FUN_18009cca0(param_4); // <-- This is DrawCurrentPrimitives function we are after
              }
              puVar23 = puVar23 + 0x1c;
              uVar24 = uVar24 - 1;
              puVar25 = local_f8;
            } while (uVar24 != 0);
          }
        }
        else if (uVar29 != 0) {
			// ...
		}
	}

    if (0 < *param_6)
        ...
        uVar26 = *puVar22;
        uVar21 = (ulonglong)uVar26;
        if ((pCSceneLayer[0x12] & 8) == 0) {
          if (uVar26 != 0) {
            do {
              uVar26 = puVar20[0x18];
              bVar9 = (byte)uVar26 & 3;
              bVar14 = *(byte *)(*(longlong *)(puVar20 + 6) + 0x9a) & 3;
              if (bVar14 < bVar9) {
                bVar14 = bVar9;
              }
              if (bVar14 != 0) {
                plVar7 = *(longlong **)(*(longlong *)(puVar20 + 6) + 0x18);
                (**(code **)(*plVar7 + 0x18))
                          (plVar7,*(undefined8 *)(param_4 + 0x20),puVar20,uVar4,pCSceneLayer,bVar14)
                ;
              }
              if (((byte)uVar26 & 8) == 0) {
                FUN_1800971c0(param_4,puVar20); // <--- This is the DrawSceneData function we are after, it's also called in a deeper tree further bellow from FUN_180096ed0
              }
              else {
                FUN_1800975e0(param_4); // <-- This is DrawCurrentPrimitives function we are after
              }
              puVar20 = puVar20 + 0x1a;
              uVar21 = uVar21 - 1;
              puVar22 = viewDrawList;
            } while (uVar21 != 0);
          }
        }
        else if (uVar26 != 0) { ... }
		...
    }
...
CTSListBase::Push((CTSListBase *)(pSceneSystem + 0xed0),(TSLNodeBase_t *)(puVar22 + -2));
...
CTSListBase::Push((CTSListBase *)(pSceneSystem + 0xed0),(TSLNodeBase_t *)(piVar5 + -2));
...
}

void FUN_1800971c0(longlong param_1,longlong param_2)
{
...
  cVar2 = FUN_1800978a0(param_1);
  if (cVar2 == '\0') {
    FUN_1800975e0(param_1);
  }
...
}

// DebugSceneData function
//
// - "Batchsort"
// - "Fullsort"
// - "shaded pass"
// - "depth pass"
// - "%s:%s after sort (%s %s)\n" (recommended)
// - "null material"
// - "C:\\buildworker\\csgo_rel_win64\\build\\src\\scenesystem\\scenesystem.cpp"
void FUN_1800e7590(undefined8 param_1,longlong pSceneLayer,longlong param_3,uint unkMaybeDictSize)
{
  // ...
  if (param_4 != 0) {
    uVar1 = *(uint *)(pSceneLayer + 0x48); // --> g_SceneLayer_Flags_Offset == 0x48
    uVar13 = (ulonglong)param_4;
	// ...
        uVar2 = *(uint *)(pSceneLayer + 0x48);
        uVar9 = (**(code **)**(undefined8 **)(pSceneLayer + 0x6f0))(); // pSceneLayer + 0x6f0 == pSceneView
        pcVar8 = "shaded pass";
        if ((uVar1 & 0x1000000) != 0) {
          pcVar8 = "depth pass";
        }
        pcVar12 = "Batchsort";
        if ((uVar2 & 1) != 0) {
          pcVar12 = "Fullsort";
        }
        Warning("%s:%s after sort (%s %s)\n",uVar9,pSceneLayer + 0x4b8,pcVar12,pcVar8); // --> 0x4b8 is g_SceneLayer_pszViewPass_Offset
	// ...
		plVar7 = (longlong *)(pSceneData + 0x18); // plVar7 == &(pSceneData->SceneObject)
			//...
		    uVar9 = (**(code **)**(undefined8 **)(*plVar7 + 0x18))(); // 0x18 is g_SceneObject_pSceneObjectDesc_Offset (pSceneData->SceneObject->SceneObjectDesc::GetName)
            Warning("  [%i] %s %s %s IndexCount: %d\n",(int)plVar7[8],uVar9,pcVar12,pcVar8,uVar11);
	// ...
}

// DrawSceneData function
void FUN_18009c880(longlong param_1,longlong param_2)
{
	// ...
  *(undefined8 *)(param_1 + 0x38) = *(undefined8 *)(*(longlong *)(param_2 + 0x18) + 0x18);
  if ((*(byte *)(param_2 + 0x62) & 8) == 0) {
    uVar6 = *(undefined8 *)(param_2 + 0x20);
  }
  *(undefined8 *)(param_1 + 0x40) = uVar6;
  uVar7 = *(ushort *)(param_1 + 0x48);
  if (*(ushort *)(param_1 + 0x48) <= *(ushort *)(param_2 + 0x5e)) {
    uVar7 = *(ushort *)(param_2 + 0x5e);
  }
  *(ushort *)(param_1 + 0x48) = uVar7;
  *(undefined8 *)(param_1 + 0x50) = *(undefined8 *)(param_2 + 0x38);
  *(undefined2 *)(param_1 + 0x4a) = *(undefined2 *)(param_2 + 0x62); // g_SceneData_Flags_Offset
}
*/
	//org_RenderLayerDrawListPart = (RenderLayerDrawListPart_t)getAddress(sceneSystemDll, "4c 89 4c 24 20 4c 89 44 24 18 48 89 54 24 10 48 89 4c 24 08 55 53 56 57 41 57 48 8d 6c 24 e0 48 81 ec 20 01 00 00");
	//if (0 == org_RenderLayerDrawListPart) ErrorBox(MkErrStr(__FILE__, __LINE__));

	// The drawing data flags byte moved (0x230 -> 0x250 in 2026-09-23 build), so it's wildcarded in the patterns below.
	org_InitDrawingData = (InitDrawingData_t)getAddress(sceneSystemDll, "48 89 5c 24 08 48 89 6c 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 81 ec ?? ?? 00 00 0f b6 81 ?? ?? 00 00 48 8b d9 4c 8b fa");
	if (0 == org_InitDrawingData) ErrorBox(MkErrStr(__FILE__, __LINE__));

	org_DrawSceneData = (DrawSceneData_t)getAddress(sceneSystemDll, "48 89 5c 24 20 55 48 83 ec 30 f6 81 ?? ?? 00 00 40");
	if (0 == org_DrawSceneData) ErrorBox(MkErrStr(__FILE__, __LINE__));

	org_DrawCurrentPrimitives = (DrawCurrentPrimitives_t)getAddress(sceneSystemDll, "4c 8b dc 53 48 81 ec d0 00 00 00 83 79 30 01 48 8b d9 0f 8c ?? ?? ?? ?? 48 8b 49 20 48 8d 15 ?? ?? ?? ??");
	if (0 == org_DrawCurrentPrimitives) ErrorBox(MkErrStr(__FILE__, __LINE__));

	// See notes in HookSceneSystem about DebugSceneData above:
	g_SceneLayer_pszViewPass_Offset = 0x4b8;
	g_SceneLayer_Flags_Offset = 0x48;
	g_SceneObject_pSceneObjectDesc_Offset = 0x18;

	// See notes in HookSceneSystem about DrawSceneData above:
	//g_SceneData_Flags_Offset = 0x62;

	if(//org_RenderLayerDrawListPart &&
		org_InitDrawingData && org_DrawSceneData && org_DrawCurrentPrimitives) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		//DetourAttach(&(PVOID&)org_RenderLayerDrawListPart, new_RenderLayerDrawListPart);
		DetourAttach(&(PVOID&)org_InitDrawingData, new_InitDrawingData);
		DetourAttach(&(PVOID&)org_DrawSceneData, new_DrawSceneData);
		//DetourAttach(&(PVOID&)org_DrawCurrentPrimitives, new_DrawCurrentPrimitives);

		if(NO_ERROR != DetourTransactionCommit()) {
			ErrorBox("Failed to detour SceneSystem functions.");
			return;
		}
	}
}
