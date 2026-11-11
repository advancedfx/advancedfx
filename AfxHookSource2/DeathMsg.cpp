#include "stdafx.h"

#include <iostream>
#include "WrpConsole.h"
#include "../shared/MirvDeathMsgFilter.h"

#include <cstddef>
#include <Windows.h>
#include "../shared/binutils.h"
#include "../deps/release/Detours/src/detours.h"

#include "../deps/release/prop/AfxHookSource/SourceSdkShared.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameevents.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/utlmap.h"
#include "../deps/release/prop/cs2/sdk_src/public/tier1/utlstring.h"

#include "DeathMsg.h"
#include "Globals.h"
#include "ClientEntitySystem.h"
#include "SchemaSystem.h"
#include "MirvColors.h" 

#include "addresses.h"

#include <set>
#include <algorithm>
#include <vector>

// TODO: move panorama stuff out after addresses.cpp is done
// decompose/change myPanoramaWrapper too
// doing it messy way here for now because lazy

// credit https://github.com/danielkrupinski/Osiris

void* g_CStylePropertyOpacity_vtable = 0;

typedef void(__fastcall *g_CPanelStyleSetStyleProperty_t)(void* This, void* property, bool transition);
g_CPanelStyleSetStyleProperty_t g_CPanelStyleSetStyleProperty = nullptr;

struct CPanel2D {
	const char* getClassName() {
		void * pClientClass = ((void * (__fastcall *)(void *)) (*(void***)this)[AFXADDR_GET(cs2_client_CPanel2D_GetClientClass_vtable_idx)]) (this);

		if(pClientClass) {
			return *(const char**)((unsigned char*)pClientClass + 0x8);
		}

		return nullptr;
	}
};

struct StylePropertySymbolMap {
    typedef uint8_t* (__fastcall *Resolve_t)(uint8_t* out, const char* stylePropertyName);

    uint8_t findSymbol(const char* stylePropertyName) {
        if (resolve) {
            uint8_t result = 0xFF;
            resolve(&result, stylePropertyName);
            return result;
        }

        if (!symbols) return 0xFF;

		for (int i = 0; i < symbols->numElements; ++i) {
            if (std::strcmp(symbols->memory[i].key.Get(), stylePropertyName) == 0)
                return symbols->memory[i].value;
        }

        return 0xFF;
    }

    Resolve_t resolve = nullptr;
    SOURCESDK::CS2::CUtlMap<SOURCESDK::CS2::CUtlString, uint8_t>* symbols = nullptr;
} g_PanoramaStylePropertySymbols;

CON_COMMAND(__mirv_panorama_dump_style_symbols, "") {
	auto symbols = g_PanoramaStylePropertySymbols.symbols;
	if (!symbols) {
		advancedfx::Warning("AFXWARNING: Panorama style-symbol dumping is unavailable for this CS2 build.\n");
		return;
	}

	for (int i = 0; i < symbols->numElements; ++i) {
		auto node = symbols->memory[i];
		advancedfx::Message("%i: %s\n", node.value, node.key.Get());
	}
}

struct StylePropertyOpacity {
	void* vtable;
	uint8_t id;
	bool disallowTransition = false;
	u_char pad[0x6];
	float value;

	StylePropertyOpacity() {} 

	StylePropertyOpacity(void* vt, uint8_t i, float v) 
		: vtable(vt), id(i), value(v) {}

};

bool makeOpacityProperty(StylePropertyOpacity* out, float value) {
	auto id = g_PanoramaStylePropertySymbols.findSymbol("opacity");
	if (g_CStylePropertyOpacity_vtable == nullptr || id == 0xFF) return false;

	*out = StylePropertyOpacity { g_CStylePropertyOpacity_vtable, id, value};

	return true;
}

struct CUIPanel {
	bool setOpacity(float value) {
		if (nullptr == g_CPanelStyleSetStyleProperty) return false;

		auto style = (u_char*)(this + CS2::PanoramaUIPanel::panelStyle);

		StylePropertyOpacity styleProp;
		if (!makeOpacityProperty(&styleProp, value)) return false;

		g_CPanelStyleSetStyleProperty(style, &styleProp, true);

		return true;
	}
};

currentGameCamera g_CurrentGameCamera;

namespace CS2 {
	namespace PanoramaPanelStyle {
		ptrdiff_t setPanelStyleProperty = 0;
	}
};

struct PlayerInfo {
	char* name;
	uint64_t xuid;
	int specKey;
	int userId;
	u_char* playerController;
};

PlayerInfo getSpectatedPlayer() 
{
	PlayerInfo result = {0,0,0,-1,0};
	auto cameraOrigin = g_CurrentGameCamera.origin;
	auto cameraAngles = g_CurrentGameCamera.angles;

    int highestIndex = GetHighestEntityIndex();
	for(int i = 0; i < highestIndex + 1; i++)
	{
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
		{
			if(!ent->IsPlayerPawn()) continue;

			float entityOrigin[3];
			ent->GetRenderEyeOrigin(entityOrigin);
			float entityAngles[3];
			ent->GetRenderEyeAngles(entityAngles);

			std::vector<double> deltaList = {
				std::abs(std::abs(entityOrigin[0]) - std::abs(cameraOrigin[0])),
				std::abs(std::abs(entityOrigin[1]) - std::abs(cameraOrigin[1])),
				std::abs(std::abs(entityOrigin[2]) - std::abs(cameraOrigin[2])),
				std::abs(std::abs(entityAngles[0]) - std::abs(cameraAngles[0])),
				std::abs(std::abs(entityAngles[1]) - std::abs(cameraAngles[1])),
				std::abs(std::abs(entityAngles[2]) - std::abs(cameraAngles[2]))
			};

			if (
				deltaList[0] > 0.2f || deltaList[1] > 0.2f || deltaList[2] > 0.2f ||
				deltaList[3] > 0.2f || deltaList[4] > 0.2f || deltaList[5] > 0.2f
			) continue;

			auto controllerIndex = ent->GetPlayerControllerHandle().GetEntryIndex();
			if (-1 == controllerIndex) continue;

			auto playerController = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,controllerIndex);

			auto xuid = *(uint64_t*)((u_char*)(playerController) + g_clientDllOffsets.CBasePlayerController.m_steamID);
			auto name = (char*)((u_char*)(playerController) + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);

            if (0 == xuid) advancedfx::Warning("Error: could not find xuid for entity %i\n", controllerIndex);
            if (nullptr == name || 0 == strlen(name)) advancedfx::Warning("Error: could not find name for entity %i\n", controllerIndex);

			result.name = name;
			result.xuid = xuid;
			result.userId = controllerIndex - 1;
			result.playerController = (u_char*)(playerController);
			// speckey is not really needed here
			break;
		}
	}

	if (-1 == result.userId) 
		advancedfx::Warning(
			"Could not find spectated player.\n"
			"Make sure you're in pov mode and camera fully switched.\n"		
		);

	return result;
};

PlayerInfo getPlayerInfoFromControllerIndex(int entindex)
{
	PlayerInfo result = {0,0,0,0,0};

	// Left screen side keys: 1, 2, 3, 4, 5
	// Right screen side keys: 6, 7, 8, 9, 0
	int slotCT = 0;
	int slotT = 0;

	bool swapPlayerSide = false;
	// apparently in CS2 CT is always on the left side, so we don't have to check for swap
	// but in case they change it in future we can probably check for gamephase like below

    int highestIndex = GetHighestEntityIndex();

	// find gamerules, maybe we can save it since it's pointer
    // for(int i = 0; i < highestIndex + 1; i++)
	// {
    //     if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
	// 	{
	// 		if (0 != strcmp("cs_gamerules", ent->GetClassName())) continue;

	// 		auto gameRules = *(u_char**)((u_char*)(ent) + CS2::C_CSGameRulesProxy::m_pGameRules);

	// 		auto gamePhase = *(int*)((gameRules) + CS2::C_CSGameRules::m_gamePhase);
	// 		auto overtimes = *(int*)((gameRules) + CS2::C_CSGameRules::m_nOvertimePlaying);

	// 		advancedfx::Message("gamePhase: %i\n", gamePhase);
	// 		advancedfx::Message("overtimes: %i\n", overtimes); // have to check if overtimes are affecting this or not

	// 		if(3 == gamePhase) swapPlayerSide = true;

	// 		// gamePhase:
	// 		// 	2 = "first"
	// 		// 	3 = "second"
	// 		// 	4 = "halftime"
	// 		// 	5 = "postgame

	// 		break;
	// 	}
	// }

    for(int i = 0; i < highestIndex + 1; i++)
	{
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
		{
			if(!ent->IsPlayerController()) continue;

			auto teamNumber = *(int*)((u_char*)(ent) + g_clientDllOffsets.C_BaseEntity.m_iTeamNum);
			if (0 == teamNumber || 1 == teamNumber) continue;

			int slot = 0;
			if (3 == teamNumber) // CT
			{
				slot = 1 + slotCT;
				if (swapPlayerSide) slot += 5;
				++slotCT;
			} 
			else if (2 == teamNumber) // T
			{
				slot = 1 + slotT;
				if (!swapPlayerSide) slot += 5;
				++slotT;
			}
			slot = slot % 10;

			if(i != entindex) continue;

			auto xuid = *(uint64_t*)((u_char*)(ent) + g_clientDllOffsets.CBasePlayerController.m_steamID);
			auto name = (char*)((u_char*)(ent) + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName);

            if (0 == xuid) advancedfx::Warning("Error: could not find xuid for entity %i\n", i);
            if (nullptr == name || 0 == strlen(name)) advancedfx::Warning("Error: could not find name for entity %i\n", i);

			result.name = name;
			result.xuid = xuid;
			result.playerController = (u_char*)(ent);
			result.userId = i - 1;
			result.specKey = slot;

			if (i == entindex) break;
        }
    }

	return result;
}

void DeathMsgId::operator=(char const * consoleValue) {
	if (!consoleValue)
		return;

	if (StringBeginsWith(consoleValue, "k"))
	{
		this->Mode = Id_Key;
		this->Id.specKey = atoi(consoleValue +1);
	}
	else if (StringBeginsWith(consoleValue, "x"))
	{
		uintptr_t val;
		
		if (0 == _stricmp("xTrace", consoleValue))
		{
			auto player = getSpectatedPlayer();
			if (-1 != player.userId)
				val = player.xuid;
			else 
				val = 0;
		}
		else
			val = strtoull(consoleValue + 1, 0, 10);

		this->operator=(val);
	}
	else
	{
		int val;

		if (0 == _stricmp("trace", consoleValue))
		{
			auto player = getSpectatedPlayer();
			if (-1 != player.userId)
				val = player.userId;
			else 
				val = 0;
		}
		else
			val = atoi(consoleValue);

		this->operator=(val);
	}
};

bool DeathMsgId::EqualsUserId(int userId)
{
	if (Mode == Id_UserId) return userId == Id.userId;

	if (userId < 0)
		return false;

	switch(Mode)
	{
		case Id_Key:
			// in CS2 playercontroller entityindex is userId + 1
			return getPlayerInfoFromControllerIndex(userId + 1).specKey == Id.specKey;
			break;

		case Id_Xuid:
			return getPlayerInfoFromControllerIndex(userId + 1).xuid == Id.xuid;
			break;
	}

	return false;
};

int DeathMsgId::ResolveToUserId()
{
	switch(Mode)
	{
	case Id_Key:
		{
			int highestIndex = GetHighestEntityIndex();
			for(int i = 0; i < highestIndex + 1; i++)
			{
				if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
				{
					if(!ent->IsPlayerController()) continue;
					auto player = getPlayerInfoFromControllerIndex(i);
					if (player.specKey == Id.specKey) return i - 1;
				}
			}
		}
		return 0;
	case Id_Xuid:
		{
			int highestIndex = GetHighestEntityIndex();
			for(int i = 0; i < highestIndex + 1; i++)
			{
				if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
				{
					if(!ent->IsPlayerController()) continue;
					auto player = getPlayerInfoFromControllerIndex(i);
					if (player.xuid == Id.xuid) return i - 1;
				}
			}
		}			
		return 0;
	}

	return Id.userId;
};

struct myPanoramaWrapper {
// credit https://github.com/danielkrupinski/Osiris 
// for engine and panel methods
	bool hooked = false;

	short lifeTimeSymbol = -1;
	short spawnTimeSymbol = -1;
	u_char** pUIEngine = nullptr;
	u_char** pHudPanel = nullptr;

	struct ColorEntry {
		u_char* pointer;
		bool use;
		uint32_t value;
		uint32_t defaultValue;
		std::string userValue = "";

		bool convertColorFromStrToInt (const char* str, uint32_t* outColor) {
			if (nullptr == str || nullptr == outColor) return false;

			auto hexStr = afxUtils::rgbaToHex(str, " ");
			if (hexStr.length() != 8) return false;

			*outColor = afxUtils::hexStrToInt(hexStr);
			return true;
		}; 

		bool setColor(const char* arg) {
			if (nullptr == arg) return false;
			if (0 == _stricmp("default", arg))
			{
				use = false;
				value = defaultValue;
				return true;
			}

			for (auto it = afxBasicColors.begin(); it != afxBasicColors.end(); ++it)
			{
				if (0 == _stricmp(it->name, arg))
				{
					use = true;
					userValue = arg;
					value = afxUtils::rgbaToHex(it->value);

					return true;
				}
			}
			return false;
		}

		bool setColor(advancedfx::ICommandArgs* args) {
			auto argc = args->ArgC();

			if (argc == 5)
			{
				uint32_t color;
				std::string str = "";
				str.append(args->ArgV(1));
				str.append(" ");
				str.append(args->ArgV(2));
				str.append(" ");
				str.append(args->ArgV(3));
				str.append(" ");
				str.append(args->ArgV(4));

				if (convertColorFromStrToInt(str.c_str(), &color))
				{
					use = true;
					userValue = str.c_str();
					value = color;
					return true;
				}
			}

			return false;
		};
	};

	ColorEntry BorderColor = { nullptr, false, 0xFF0000E1, 0xFF0000E1 };
	ColorEntry BackgroundColor = { nullptr, false, 0xA0000000, 0xA0000000 };
	ColorEntry LocalBackgroundColor = { nullptr, false, 0xE7000000, 0xE7000000 };
	ColorEntry CTcolor = { nullptr, false, 0xFFE69C6F, 0xFFE69C6F };
	ColorEntry Tcolor = { nullptr, false, 0xFF54BEEA, 0xFF54BEEA };

	void initSymbols() {
		if (-1 != lifeTimeSymbol) return;

		if (nullptr == pUIEngine){
			advancedfx::Warning("pUIEngine is null\n");
			return;
		}

		typedef short(__fastcall *makeSymbol_t)(u_char* ptr, int type, const char* name);
		const auto makeSymbol = (makeSymbol_t)((*(void***)(*pUIEngine))[AFXADDR_GET(cs2_client_CUIEngine_MakeSymbol_vtable_idx)]);

		lifeTimeSymbol = makeSymbol(*pUIEngine, 0, "Lifetime");
		spawnTimeSymbol = makeSymbol(*pUIEngine, 0, "SpawnTime");
	};

	u_char* getDeathnotices(){
		if (nullptr == pHudPanel){
			advancedfx::Warning("pHudPanel is null\n");
			return nullptr;
		}

		const auto hudPanel = ((u_char***)pHudPanel)[0][1];
		if (!hudPanel) return nullptr;
		const auto hudDeathNotice = findChildInLayoutFile(hudPanel, "HudDeathNotice");
		if (!hudDeathNotice) return nullptr;
		const auto visibleNotices = findChildInLayoutFile(hudDeathNotice, "VisibleNotices");
		if (!visibleNotices) return nullptr;

		auto deathnotices = visibleNotices + CS2::PanoramaUIPanel::children;

		return deathnotices;
	};

	bool clearDeathnotices(){
		initSymbols();

		const auto pDeathnotices = getDeathnotices(); // dunno how to invalidate it, so will get it each time
		if (nullptr == pDeathnotices) return false;

		if (*(int*)pDeathnotices == 0) return false;

		bool result = false;

		for (int i = 0; i < *(int*)pDeathnotices; i++) {
			const auto panel = ((u_char***)pDeathnotices)[1][i];

			typedef const char* (__fastcall *getAttributeString_t)(u_char* ptr, short attributeName, const char* defaultValue);
			const auto getAttributeString = (getAttributeString_t)((*(void***)panel)[AFXADDR_GET(cs2_client_CUIPanel_GetAttributeString_vtable_idx)]);

			typedef void* (__fastcall *setAttributeString_t)(u_char* ptr, short attributeName, const char* defaultValue);
			const auto setAttributeString = (setAttributeString_t)((*(void***)panel)[AFXADDR_GET(cs2_client_CUIPanel_SetAttributeString_vtable_idx)]);

			const auto lifetimeString = getAttributeString(panel, lifeTimeSymbol, "");
			if (!lifetimeString || strlen(lifetimeString) == 0) continue;

			const auto spawnTimeString = getAttributeString(panel, spawnTimeSymbol, "");
			if (!spawnTimeString || strlen(spawnTimeString) == 0) continue;

			setAttributeString(panel, lifeTimeSymbol, "0.001");
			setAttributeString(panel, spawnTimeSymbol, std::to_string(g_CurrentGameCamera.time - 1).c_str());
			result = true;
		}

		return result;
	}

	void printChildren(const char* parentId) {
		auto parentPanel = ((u_char***)pHudPanel)[0][1];
		if (!parentPanel) return;

		if (strlen(parentId) > 0) {
			auto r = findChildInLayoutFile(parentPanel, parentId);
			if (r) {
				parentPanel = r;
			} else {
				auto r2 = findChildrenInLayoutFileByClassName(parentPanel, parentId);
				if (!r2.empty()) parentPanel = r2[0]; // could be multiple there
			}
		} 

		auto parentPanelId = *(char**)(parentPanel + CS2::PanoramaUIPanel::panelId);
		auto parentPanel2D = *(CPanel2D**)(parentPanel + 0x8);

		advancedfx::Message("ClientClass / PanelId:\n");
		if (0 != parentPanel2D)
			advancedfx::Message("%s / %s\n", parentPanel2D->getClassName(), parentPanelId);
		else 
			advancedfx::Message("%s / %s\n", "null", parentPanelId);

		const auto children = parentPanel + CS2::PanoramaUIPanel::children;
		if (!children) {
			advancedfx::Warning("No children found.\n");
			return;
		};

		for (int i = 0; i < *(int*)children; ++i) {
			const auto panel = ((u_char***)children)[1][i];
			const auto panelId = *(char**)(panel + CS2::PanoramaUIPanel::panelId);
			auto panel2D = *(CPanel2D**)(panel + 0x8);

			if (!panelId && !panel2D) continue;

			if (0 != panel2D)
				advancedfx::Message("\t%s / %s\n", panel2D->getClassName(), panelId);
			else 
				advancedfx::Message("\t%s / %s\n", "null", panelId);
		}

	}

	std::vector<u_char*> findChildrenInLayoutFileByClassName(u_char* parentPanel, const char* classNameToFind) {
		std::vector<u_char*> res;
		if (!parentPanel) return res;

		const auto children = parentPanel + CS2::PanoramaUIPanel::children;
		if (!children) return res;

		for (int i = 0; i < *(int*)children; ++i) {
			const auto panel = ((u_char***)children)[1][i];
			auto panel2D = *(CPanel2D**)(panel + 0x8);
			if (!panel2D) continue;

			auto panelClassName = panel2D->getClassName();

			if (strcmp(panelClassName, classNameToFind) == 0) {
				res.emplace_back(panel);
			};
		}

		for (int i = 0; i < *(int*)children; ++i) {
			const auto panel = ((u_char***)children)[1][i];
			const auto panelFlags = (u_char)(panel + CS2::PanoramaUIPanel::panelFlags);
			if ((panelFlags & CS2::PanoramaUIPanel::k_EPanelFlag_HasOwnLayoutFile) == 0) {
				auto found = findChildrenInLayoutFileByClassName(panel, classNameToFind);
				if (!found.empty()) {
					for (auto i : found) {
						res.emplace_back(i);
					}
				}
			}
		}

		return res;
	}

	u_char* findChildInLayoutFile(u_char* parentPanel, const char* idToFind){
		if (!parentPanel) return nullptr;

		const auto children = parentPanel + CS2::PanoramaUIPanel::children;
		if (!children) return nullptr;

		for (int i = 0; i < *(int*)children; ++i) {
			const auto panel = ((u_char***)children)[1][i];
			const auto panelId = *(char**)(panel + CS2::PanoramaUIPanel::panelId);
			if (!panelId) continue;
			if (strcmp(panelId, idToFind) == 0) {
				return panel;
			};
		}

		for (int i = 0; i < *(int*)children; ++i) {
			const auto panel = ((u_char***)children)[1][i];
			const auto panelFlags = (u_char)(panel + CS2::PanoramaUIPanel::panelFlags);
			if ((panelFlags & CS2::PanoramaUIPanel::k_EPanelFlag_HasOwnLayoutFile) == 0) {
				if (const auto found = findChildInLayoutFile(panel, idToFind)) {
					return found;
				}
			}
		}

		return nullptr;
	};

	void applyColors() {
		if (nullptr != BorderColor.pointer) {
			*(uint32_t*)(BorderColor.pointer + 0x38 + 4*0) = BorderColor.use ? BorderColor.value : BorderColor.defaultValue;
			*(uint32_t*)(BorderColor.pointer + 0x38 + 4*1) = BorderColor.use ? BorderColor.value : BorderColor.defaultValue;
			*(uint32_t*)(BorderColor.pointer + 0x38 + 4*2) = BorderColor.use ? BorderColor.value : BorderColor.defaultValue;
			*(uint32_t*)(BorderColor.pointer + 0x38 + 4*3) = BorderColor.use ? BorderColor.value : BorderColor.defaultValue;
		}
		if (nullptr != LocalBackgroundColor.pointer) {
			*(uint32_t*)(LocalBackgroundColor.pointer +0x20) = LocalBackgroundColor.use ? LocalBackgroundColor.value : LocalBackgroundColor.defaultValue;
		}	
		if (nullptr != BackgroundColor.pointer) {
			*(uint32_t*)(BackgroundColor.pointer +0x20) = BackgroundColor.use ? BackgroundColor.value : BackgroundColor.defaultValue;
		}	
		if (nullptr != CTcolor.pointer) {
			*(uint32_t*)(CTcolor.pointer + 0x20) = CTcolor.use ? CTcolor.value : CTcolor.defaultValue;
		}
		if (nullptr != Tcolor.pointer) {
			*(uint32_t*)(Tcolor.pointer + 0x20) = Tcolor.use ? Tcolor.value : Tcolor.defaultValue;
		}
	}

	/*
	void updateHudPanelStyles() {
		// currently not required.
		const auto hudPanel = ((u_char***)pHudPanel)[0][1];
		if (hudPanel) {
			// Function is called also in if after refrence to "CUIPanel::AddClassesInternal - apply old dirty styles":
			// and is also at vtable entry 71 of panorama panel class.
			void (__fastcall * applyStyleFn)(void *, signed short int) = (void (__fastcall *)(void *, signed short int))((*(void***)hudPanel)[71]);
			applyStyleFn(hudPanel, 0);
		}
	}
	*/

} g_myPanoramaWrapper;

CON_COMMAND(__mirv_panorama_print_children, "") {
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);
		g_myPanoramaWrapper.printChildren(arg1);
	} else {
		g_myPanoramaWrapper.printChildren("");
	}
}

typedef unsigned int (__fastcall *g_Original_hashString_t)(const char* string, unsigned int length, unsigned int lengthXorSeed);
g_Original_hashString_t g_Original_hashString = nullptr;


class MyDeathMsgGameEventWrapper : public SOURCESDK::CS2::IGameEvent, public MyDeathMsgGameEventWrapperBase
{
public:
	MyDeathMsgGameEventWrapper(SOURCESDK::CS2::IGameEvent * event)
	: m_Event(event) { }

	SOURCESDK::CS2::CKV3MemberName hashString(const char * string) {
		size_t len = strlen(string);
		unsigned int hash = g_Original_hashString(string, (unsigned int)len, (unsigned int)len ^ 0x31415926);
		return SOURCESDK::CS2::CKV3MemberName((int)hash, -1, string);
	}

	bool IsHashStringEqual(const char * a, const SOURCESDK::CS2::CKV3MemberName & b) {
		return IsHashEqual(hashString(a),b);
	}

private:
	bool IsHashEqual(const SOURCESDK::CS2::CKV3MemberName & a, const SOURCESDK::CS2::CKV3MemberName & b) {
		return a.GetHashCode() == b.GetHashCode();
	}

public:
	virtual ~MyDeathMsgGameEventWrapper() {};
	virtual const char *GetName() const {
		return m_Event->GetName();
	}
	virtual int GetID() const {
		return m_Event->GetID();
	}
	virtual bool IsReliable() const {
		return m_Event->IsReliable();
	}
	virtual bool IsLocal() const {
		return m_Event->IsLocal();
	}
	virtual bool IsEmpty( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->IsEmpty(keySymbol);
	}
	virtual bool GetBool( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {
		return m_Event->GetBool(keySymbol);
	}
	virtual int GetInt( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {

		if (assistedflash.use && IsHashStringEqual("assistedflash", keySymbol)) return assistedflash.value;
		if (headshot.use && IsHashStringEqual("headshot", keySymbol)) return headshot.value;
		if (penetrated.use && IsHashStringEqual("penetrated", keySymbol)) return penetrated.value;
		if (dominated.use && IsHashStringEqual("dominated", keySymbol)) return dominated.value;
		if (revenge.use && IsHashStringEqual("revenge", keySymbol)) return revenge.value;
		if (wipe.use && IsHashStringEqual("wipe", keySymbol)) return wipe.value;
		if (noscope.use && IsHashStringEqual("noscope", keySymbol)) return noscope.value;
		if (thrusmoke.use && IsHashStringEqual("thrusmoke", keySymbol)) return thrusmoke.value;
		if (attackerblind.use && IsHashStringEqual("attackerblind", keySymbol)) return attackerblind.value;
		if (attackerinair.use && IsHashStringEqual("attackerinair", keySymbol)) return attackerinair.value;
		
		return m_Event->GetInt(keySymbol);
	}
	virtual uint64_t GetUint64( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {
		return m_Event->GetUint64(keySymbol);
	}
	virtual float GetFloat( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {
		return m_Event->GetFloat(keySymbol);
	}
	virtual const char *GetString( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {
		if (weapon.use && IsHashStringEqual("weapon", keySymbol)) return weapon.value;
		return m_Event->GetString(keySymbol);
	}
	virtual void *GetPtr( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetPtr(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityHandle GetEHandle( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetEHandle(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetEntity( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol) {
		return m_Event->GetEntity(keySymbol);
	}
	virtual void* GetEntityIndex( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetEntityIndex(keySymbol);
	}
	virtual SOURCESDK::CS2::CPlayerSlot GetPlayerSlot( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {

		if (attacker.newId.use && IsHashStringEqual("attacker", keySymbol)) return SOURCESDK::CS2::CPlayerSlot(attacker.newId.value.ResolveToUserId());
		if (victim.newId.use && IsHashStringEqual("userid", keySymbol)) return SOURCESDK::CS2::CPlayerSlot(victim.newId.value.ResolveToUserId());
		if (assister.newId.use && IsHashStringEqual("assister", keySymbol)) return SOURCESDK::CS2::CPlayerSlot(assister.newId.value.ResolveToUserId());

		return m_Event->GetPlayerSlot(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetPlayerController( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetPlayerController(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetPlayerPawn( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetPlayerPawn(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityHandle GetPawnEHandle( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetPawnEHandle(keySymbol);
	}
	virtual void* GetPawnEntityIndex( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->GetPawnEntityIndex(keySymbol);
	}
	virtual void SetBool( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, bool value ) {
		m_Event->SetBool(keySymbol,value);
	}
	virtual void SetInt( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, int value ) {
		m_Event->SetInt(keySymbol,value);
	}
	virtual void SetUint64( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, uint64_t value ) {
		m_Event->SetUint64(keySymbol,value);
	}
	virtual void SetFloat( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, float value ) {
		m_Event->SetFloat(keySymbol,value);
	}
	virtual void SetString( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, const char *value ) {
		m_Event->SetString(keySymbol,value);
	}
	virtual void SetPtr( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, void *value ) {
		m_Event->SetPtr(keySymbol,value);
	}
	virtual void SetEntity(const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, SOURCESDK::CS2::CEntityInstance *value) {
		m_Event->SetEntity(keySymbol,value);
	}
	virtual void SetEntity( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, void* value ) {
		m_Event->SetEntity(keySymbol,value);
	}
	virtual void SetPlayer( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, SOURCESDK::CS2::CEntityInstance *pawn ) {
		m_Event->SetPlayer(keySymbol,pawn);
	}
	virtual void SetPlayer( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol, SOURCESDK::CS2::CPlayerSlot value ) {
		m_Event->SetPlayer(keySymbol,value);
	}
	virtual void SetPlayerRaw( const SOURCESDK::CS2::GameEventKeySymbol_t &controllerKeySymbol, const SOURCESDK::CS2::GameEventKeySymbol_t &pawnKeySymbol, SOURCESDK::CS2::CEntityInstance *pawn ) {
		m_Event->SetPlayerRaw(controllerKeySymbol,pawnKeySymbol,pawn);
	}
	virtual bool HasKey( const SOURCESDK::CS2::GameEventKeySymbol_t &keySymbol ) {
		return m_Event->HasKey(keySymbol);
	}
	virtual void CreateVMTable( void* &Table ) {
		m_Event->CreateVMTable(Table);
	}
	virtual struct SOURCESDK::CS2::KeyValues3* GetDataKeys() const {
		return m_Event->GetDataKeys();
	}

private:
	SOURCESDK::CS2::IGameEvent * m_Event;
};

struct CS2_MirvDeathMsgGlobals : MirvDeathMsgGlobals {
	bool hooked = false; 
	MyDeathMsgGameEventWrapper* activeWrapper = nullptr;
} g_MirvDeathMsgGlobals;

typedef uint64_t (__fastcall *g_Original_getLocalSteamId_t)(void* param_1);
g_Original_getLocalSteamId_t g_Original_getLocalSteamId = nullptr;

uint64_t __fastcall getLocalSteamId(void* param_1) {
	uint64_t result = 0;
	MyDeathMsgPlayerEntry entry;
	bool use = false;

	if (nullptr != g_MirvDeathMsgGlobals.activeWrapper) { 
		if (g_MirvDeathMsgGlobals.activeWrapper->attacker.isLocal.use) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->attacker;			
			use = true;
		}
		else if (g_MirvDeathMsgGlobals.activeWrapper->victim.isLocal.use) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->victim;
			use = true;
		}
		else if (g_MirvDeathMsgGlobals.activeWrapper->assister.isLocal.use) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->assister;
			use = true;
		}
	}

	if (g_MirvDeathMsgGlobals.useHighlightId)
	{
		entry.newId.value = g_MirvDeathMsgGlobals.highlightId;
		entry.isLocal.value = true;
		use = true;
	}

	if (use && !entry.isLocal.value) return 0;

	if (!use) return g_Original_getLocalSteamId(param_1);

	switch (entry.newId.value.Mode) {
		case DeathMsgId::Id_Key:
		{
			int highestIndex = GetHighestEntityIndex();	
			for(int i = 0; i < highestIndex + 1; i++)
			{
				if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i))
				{
					if(!ent->IsPlayerController()) continue;
					auto player = getPlayerInfoFromControllerIndex(i);	
					if (player.specKey == entry.newId.value.Id.specKey)
					{
						result = player.xuid != 0 ? player.xuid : result;
						break;
					}
					
				}
			}
		}

		break;
		case DeathMsgId::Id_Xuid:
			result = entry.newId.value.Id.xuid != 0 ? entry.newId.value.Id.xuid : result;
		break;
		case DeathMsgId::Id_UserId:
		{
			auto player = getPlayerInfoFromControllerIndex(entry.newId.value.Id.userId + 1);
			result = player.xuid != 0 ? player.xuid : result;
		}
		break;
	};

	return result;
};

// param 1 CCSGO_HudDeathNotice : panorama::CPanel2D : panorama::IUIPanelClient : CCSGOHudElement : CGameEventListener : IGameEventListener2
// param 2 IGameEvent
typedef void (__fastcall *g_Original_handlePlayerDeath_t)(u_char* param_1, SOURCESDK::CS2::IGameEvent* param_2);
g_Original_handlePlayerDeath_t g_Original_handlePlayerDeath = nullptr;

void __fastcall handleDeathnotice(u_char* hudDeathNotice, SOURCESDK::CS2::IGameEvent* gameEvent) {

	if (AFXADDR_INVALID == AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_lifetime_offset) || AFXADDR_INVALID == AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset)) {
		return g_Original_handlePlayerDeath(hudDeathNotice, gameEvent);
	}

	auto lifetimeOffset = AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_lifetime_offset);
	auto lifetimeModOffset = AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_lifetimemod_offset);

	float orgDeathNoticeLifetime, orgDeathNoticeLocalPlayerLifetimeMod;

	MyDeathMsgGameEventWrapper myWrapper(gameEvent);

	auto pDeathNoticeLifetime = (float*)(hudDeathNotice + lifetimeOffset);
	auto pDeathNoticeLocalPlayerLifetimeMod = (float*)(hudDeathNotice + lifetimeModOffset);

	auto uidAttacker = (int)(int16_t)gameEvent->GetInt(myWrapper.hashString("attacker"));
	auto uidVictim = (int)(int16_t)gameEvent->GetInt(myWrapper.hashString("userid"));
	auto uidAssister = (int)(int16_t)gameEvent->GetInt(myWrapper.hashString("assister"));

	myWrapper.attacker.newId.value.Id.userId = uidAttacker;
	myWrapper.victim.newId.value.Id.userId = uidVictim;
	myWrapper.assister.newId.value.Id.userId = uidAssister;

	auto attackerController = gameEvent->GetPlayerController(myWrapper.hashString("attacker"));
	auto victimController = gameEvent->GetPlayerController(myWrapper.hashString("userid"));
	auto assisterController = gameEvent->GetPlayerController(myWrapper.hashString("assister"));

	if (g_MirvDeathMsgGlobals.Settings.Debug)
	{

		std::vector<std::vector<std::string>> rows = {
			{
				"weapon",
				"attackerName",
				"attackerUserId",
				"victimName",
				"victimUserId",
				"assisterName",
				"assisterUserId",
			},
			{
				gameEvent->GetString(myWrapper.hashString("weapon")),

				nullptr != attackerController ? (char*)((u_char*)attackerController + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName) : "null",
				std::to_string(uidAttacker),

				nullptr != victimController ? (char*)((u_char*)victimController + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName) : "null",
				std::to_string(uidVictim),

				nullptr != assisterController ? (char*)((u_char*)assisterController + g_clientDllOffsets.CBasePlayerController.m_iszPlayerName) : "null",
				std::to_string(uidAssister),
			}
		};

		advancedfx::Message(
			"player_death\n"
			"%s", afxUtils::createTable(rows, " | ", "-").c_str()
		);

	}

	for(std::list<DeathMsgFilterEntry>::iterator it = g_MirvDeathMsgGlobals.Filter.begin(); it != g_MirvDeathMsgGlobals.Filter.end(); it++)
	{
		DeathMsgFilterEntry & e = *it;

		bool attackerBlocked;
		switch(e.attacker.mode)
		{
		case DMBM_ANY:
			attackerBlocked = true;
			break;
		case DMBM_EXCEPT:
			attackerBlocked = !e.attacker.id.EqualsUserId(uidAttacker);
			break;
		case DMBM_EQUAL:
		default:
			attackerBlocked = e.attacker.id.EqualsUserId(uidAttacker);
			break;
		}

		bool victimBlocked;
		switch(e.victim.mode)
		{
		case DMBM_ANY:
			victimBlocked = true;
			break;
		case DMBM_EXCEPT:
			victimBlocked = !e.victim.id.EqualsUserId(uidVictim);
			break;
		case DMBM_EQUAL:
		default:
			victimBlocked = e.victim.id.EqualsUserId(uidVictim);
			break;
		}

		bool assisterBlocked;
		switch(e.assister.mode)
		{
		case DMBM_ANY:
			assisterBlocked = true;
			break;
		case DMBM_EXCEPT:
			assisterBlocked = !e.assister.id.EqualsUserId(uidAssister);
			break;
		case DMBM_EQUAL:
		default:
			assisterBlocked = e.assister.id.EqualsUserId(uidAssister);
			break;
		}

		bool matched = attackerBlocked && victimBlocked && assisterBlocked;

		if(matched)
		{
			myWrapper.ApplyDeathMsgFilterEntry(e);

			uidAttacker = myWrapper.GetInt(myWrapper.hashString("attacker"));
			uidVictim = myWrapper.GetInt(myWrapper.hashString("userid"));
			uidAssister = myWrapper.GetInt(myWrapper.hashString("assister"));

			if (e.lastRule) break;
		}
	}

	if (myWrapper.block.use && myWrapper.block.value) {
		return;
	}

	if (g_MirvDeathMsgGlobals.useHighlightId)
	{
		myWrapper.attacker.isLocal.use = true;
		myWrapper.attacker.isLocal.value = g_MirvDeathMsgGlobals.highlightId.EqualsUserId(uidAttacker);

		myWrapper.victim.isLocal.use = true;
		myWrapper.victim.isLocal.value = g_MirvDeathMsgGlobals.highlightId.EqualsUserId(uidVictim);

		myWrapper.assister.isLocal.use = true;
		myWrapper.assister.isLocal.value = g_MirvDeathMsgGlobals.highlightId.EqualsUserId(uidAssister);
	}

	if (myWrapper.attacker.name.use && nullptr != attackerController) {
		((SOURCESDK::CS2::CUtlString *)((u_char*)attackerController + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName))->Set(myWrapper.attacker.name.value);
	}

	if (myWrapper.victim.name.use && nullptr != victimController) {
		((SOURCESDK::CS2::CUtlString *)((u_char*)victimController + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName))->Set(myWrapper.victim.name.value);
	}

	if (myWrapper.assister.name.use && nullptr != assisterController) {
		((SOURCESDK::CS2::CUtlString *)((u_char*)assisterController + g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName))->Set(myWrapper.assister.name.value);
	}

	if (g_MirvDeathMsgGlobals.Lifetime.use)
	{
		myWrapper.lifetime.use = true;
		myWrapper.lifetime.value = g_MirvDeathMsgGlobals.Lifetime.value;
	}

	if (g_MirvDeathMsgGlobals.LifetimeMod.use)
	{
		myWrapper.lifetimeMod.use = true;
		myWrapper.lifetimeMod.value = g_MirvDeathMsgGlobals.LifetimeMod.value;
	}

	if (myWrapper.lifetime.use)
	{
		orgDeathNoticeLifetime = *pDeathNoticeLifetime;
		*pDeathNoticeLifetime = myWrapper.lifetime.value;
	}

	if (myWrapper.lifetimeMod.use)
	{
		orgDeathNoticeLocalPlayerLifetimeMod = *pDeathNoticeLocalPlayerLifetimeMod;
		*pDeathNoticeLocalPlayerLifetimeMod = myWrapper.lifetimeMod.value;
	}

	g_MirvDeathMsgGlobals.activeWrapper = &myWrapper;

    g_Original_handlePlayerDeath(hudDeathNotice, &myWrapper);

	if (myWrapper.lifetimeMod.use) {
		*pDeathNoticeLocalPlayerLifetimeMod = orgDeathNoticeLocalPlayerLifetimeMod;
	}
	if (myWrapper.lifetime.use) {
		*pDeathNoticeLifetime = orgDeathNoticeLifetime;
	}

	g_MirvDeathMsgGlobals.activeWrapper = nullptr;
};


typedef int (__fastcall * Panorama_CLayoutFile_LoadFromFile_t)(void * This, const char * pFilePath, unsigned char _unk02);
typedef unsigned char (__fastcall * Panorama_CStyleProperty_Parse_t)(void * This, void* _unk01, const char * pValueStr);
typedef void (__fastcall * Panorama_CStyleProperty_Clone_t)(void * This, void * pTarget);

bool g_b_In_Panorama_CLayoutFile_LoadFromFile = false;
bool g_b_In_Panorama_CLayoutFile_LoadFromFile_HudReticle = false;

Panorama_CLayoutFile_LoadFromFile_t g_Org_Panorama_CLayoutFile_LoadFromFile = nullptr;
Panorama_CStyleProperty_Parse_t g_Org_Panorama_CStylePropertyForegroundColor_Parse = nullptr;
Panorama_CStyleProperty_Parse_t g_Org_Panorama_CStylePropertyBackgroundColor_Parse = nullptr;
Panorama_CStyleProperty_Parse_t g_Org_Panorama_CStylePropertyBorder_Parse = nullptr;

Panorama_CStyleProperty_Parse_t g_Org_Panorama_CStylePropertyWashColor_Parse = nullptr;
Panorama_CStyleProperty_Clone_t g_Org_Panorama_CStylePropertyWashColor_Clone = nullptr;

std::set<u_char*> g_pHudReticle_WashColor_T;
std::set<u_char*> g_pHudReticle_WashColor_CT;

void SetHudReticleWashColorT(uint32_t value) {
	for(auto it= g_pHudReticle_WashColor_T.begin(); it != g_pHudReticle_WashColor_T.end(); it++) {
		*(uint32_t*)(*it + 0x10) = value;
	}
}

void SetHudReticleWashColorCT(uint32_t value) {
	for(auto it= g_pHudReticle_WashColor_CT.begin(); it != g_pHudReticle_WashColor_CT.end(); it++) {
		*(uint32_t*)(*it + 0x10) = value;
	}
}

int __fastcall My_Panorama_CLayoutFile_LoadFromFile(void * This, const char * pFilePath, unsigned char _unk02) {
	if(0 == strcmp("panorama\\layout\\hud\\huddeathnotice.xml",pFilePath)) {		
		g_b_In_Panorama_CLayoutFile_LoadFromFile = true;
		int result = g_Org_Panorama_CLayoutFile_LoadFromFile(This,pFilePath,_unk02);
		g_b_In_Panorama_CLayoutFile_LoadFromFile = false;
		return result;
	}

	if(0 == strcmp("panorama\\layout\\hud\\hudreticle.xml",pFilePath)) {
		g_pHudReticle_WashColor_T.clear();
		g_pHudReticle_WashColor_CT.clear();
		g_b_In_Panorama_CLayoutFile_LoadFromFile_HudReticle = true;
		int result = g_Org_Panorama_CLayoutFile_LoadFromFile(This,pFilePath,_unk02);
		g_b_In_Panorama_CLayoutFile_LoadFromFile_HudReticle = false;
		return result;
	}

	return g_Org_Panorama_CLayoutFile_LoadFromFile(This,pFilePath,_unk02);
}

unsigned char __fastcall My_Panorama_CStylePropertyForegroundColor_Parse(void * This, void* _unk01, const char * pValueStr) {
	unsigned char result = g_Org_Panorama_CStylePropertyForegroundColor_Parse(This,_unk01,pValueStr);
	if(g_b_In_Panorama_CLayoutFile_LoadFromFile) {
		if(0 == strcmp(pValueStr,"#6f9ce6")) {
			g_myPanoramaWrapper.CTcolor.pointer = (u_char*)This;
		}
		else if(0 == strcmp(pValueStr,"#eabe54")) {
			g_myPanoramaWrapper.Tcolor.pointer = (u_char*)This;
		}		
	}
	return result;
}

unsigned char __fastcall My_Panorama_CStylePropertyBackgroundColor_Parse(void * This, void* _unk01, const char * pValueStr) {
	unsigned char result = g_Org_Panorama_CStylePropertyBackgroundColor_Parse(This,_unk01,pValueStr);
	if(g_b_In_Panorama_CLayoutFile_LoadFromFile) {
		if(0 == strcmp(pValueStr,"#000000a0")) {
			g_myPanoramaWrapper.BackgroundColor.pointer = (u_char*)This;
		}
		else if(0 == strcmp(pValueStr,"#000000e7")) {
			g_myPanoramaWrapper.LocalBackgroundColor.pointer = (u_char*)This;
		}		
	}
	return result;
}

unsigned char __fastcall My_Panorama_CStylePropertyBorder_Parse(void * This, void* _unk01, const char * pValueStr) {
	unsigned char result = g_Org_Panorama_CStylePropertyBorder_Parse(This,_unk01,pValueStr);
	if(g_b_In_Panorama_CLayoutFile_LoadFromFile) {
		if(0 == strcmp(pValueStr,"2px solid #e10000")) {
			g_myPanoramaWrapper.BorderColor.pointer = (u_char*)This;
		}
	}
	return result;
}


unsigned char __fastcall My_Panorama_CStylePropertyWashColor_Parse(void * This, void* _unk01, const char * pValueStr) {
	unsigned char result = g_Org_Panorama_CStylePropertyWashColor_Parse(This,_unk01,pValueStr);
	if(g_b_In_Panorama_CLayoutFile_LoadFromFile_HudReticle) {
		if(0 == strcmp(pValueStr,"rgb(150, 200, 250)")) {
			g_pHudReticle_WashColor_CT.emplace((u_char*)This);
		}
		else if(0 == strcmp(pValueStr,"#eabe54")) {
			g_pHudReticle_WashColor_T.emplace((u_char*)This);
		}		
	}
	return result;
}

void __fastcall My_Panorama_CStylePropertyWashColor_Clone(void * This, void * pTarget) {
	g_Org_Panorama_CStylePropertyWashColor_Clone(This, pTarget);
	if(g_b_In_Panorama_CLayoutFile_LoadFromFile_HudReticle) {
		auto itCT = g_pHudReticle_WashColor_CT.find((u_char*)This);
		if(itCT != g_pHudReticle_WashColor_CT.end()) {
			g_pHudReticle_WashColor_CT.emplace((u_char*)pTarget);
		}
		auto itT = g_pHudReticle_WashColor_T.find((u_char*)This);
		if(itT != g_pHudReticle_WashColor_T.end()) {
			g_pHudReticle_WashColor_T.emplace((u_char*)pTarget);
		}
	}
}

bool getDeathMsgAddrs() {
	if (0 == AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath)
		|| 0 == AFXADDR_GET(cs2_client_HashString)
		|| 0 == AFXADDR_GET(cs2_client_GetLocalSteamId))
		return false;

	g_Original_handlePlayerDeath = (g_Original_handlePlayerDeath_t)AFXADDR_GET(cs2_client_CCSGO_HudDeathNotice_HandlePlayerDeath);
	g_Original_hashString = (g_Original_hashString_t)AFXADDR_GET(cs2_client_HashString);
	g_Original_getLocalSteamId = (g_Original_getLocalSteamId_t)AFXADDR_GET(cs2_client_GetLocalSteamId);

	return true;
};

bool getPanoramaAddrsFromClient() {
	if (AFXADDR_INVALID == AFXADDR_GET(cs2_client_CUIPanel_GetAttributeString_vtable_idx)
		|| AFXADDR_INVALID == AFXADDR_GET(cs2_client_CUIPanel_SetAttributeString_vtable_idx)
		|| AFXADDR_INVALID == AFXADDR_GET(cs2_client_CUIEngine_MakeSymbol_vtable_idx)
		|| 0 == AFXADDR_GET(cs2_client_g_pHudPanel)
		|| 0 == AFXADDR_GET(cs2_client_g_pUIEngine))
		return false;

	g_myPanoramaWrapper.pHudPanel = (u_char**)AFXADDR_GET(cs2_client_g_pHudPanel);
	g_myPanoramaWrapper.pUIEngine = (u_char**)AFXADDR_GET(cs2_client_g_pUIEngine);

	return true;
};

bool getPanoramaAddrs() {
	if (0 == AFXADDR_GET(cs2_panorama_CLayoutFile_LoadFromFile)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyForegroundColor_Parse)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyBackgroundColor_Parse)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyBorder_Parse)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyWashColor_Clone)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyWashColor_Parse)
		|| 0 == AFXADDR_GET(cs2_panorama_CStylePropertyOpacity_vtable)
		|| (0 == AFXADDR_GET(cs2_panorama_CStyleSymbol_Resolve) && 0 == AFXADDR_GET(cs2_panorama_g_StylePropertySymbols)))
		return false;

	g_Org_Panorama_CLayoutFile_LoadFromFile = (Panorama_CLayoutFile_LoadFromFile_t)AFXADDR_GET(cs2_panorama_CLayoutFile_LoadFromFile);
	g_Org_Panorama_CStylePropertyForegroundColor_Parse = (Panorama_CStyleProperty_Parse_t)AFXADDR_GET(cs2_panorama_CStylePropertyForegroundColor_Parse);
	g_Org_Panorama_CStylePropertyBackgroundColor_Parse = (Panorama_CStyleProperty_Parse_t)AFXADDR_GET(cs2_panorama_CStylePropertyBackgroundColor_Parse);
	g_Org_Panorama_CStylePropertyBorder_Parse = (Panorama_CStyleProperty_Parse_t)AFXADDR_GET(cs2_panorama_CStylePropertyBorder_Parse);
	g_Org_Panorama_CStylePropertyWashColor_Clone = (Panorama_CStyleProperty_Clone_t)AFXADDR_GET(cs2_panorama_CStylePropertyWashColor_Clone);
	g_Org_Panorama_CStylePropertyWashColor_Parse = (Panorama_CStyleProperty_Parse_t)AFXADDR_GET(cs2_panorama_CStylePropertyWashColor_Parse);
	g_CStylePropertyOpacity_vtable = (void**)AFXADDR_GET(cs2_panorama_CStylePropertyOpacity_vtable);

	g_PanoramaStylePropertySymbols.resolve = (StylePropertySymbolMap::Resolve_t)AFXADDR_GET(cs2_panorama_CStyleSymbol_Resolve);
	g_PanoramaStylePropertySymbols.symbols = (SOURCESDK::CS2::CUtlMap<SOURCESDK::CS2::CUtlString, uint8_t>*)AFXADDR_GET(cs2_panorama_g_StylePropertySymbols);

	g_CPanelStyleSetStyleProperty = (g_CPanelStyleSetStyleProperty_t)AFXADDR_GET(cs2_panorama_CPanelStyle_SetStyleProperty);

	return true;
};

void HookPanorama(HMODULE panoramaDll)
{
	if (g_myPanoramaWrapper.hooked) return;

	if (!getPanoramaAddrs()) return;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Org_Panorama_CLayoutFile_LoadFromFile, My_Panorama_CLayoutFile_LoadFromFile);
	DetourAttach(&(PVOID&)g_Org_Panorama_CStylePropertyForegroundColor_Parse, My_Panorama_CStylePropertyForegroundColor_Parse);
	DetourAttach(&(PVOID&)g_Org_Panorama_CStylePropertyBackgroundColor_Parse, My_Panorama_CStylePropertyBackgroundColor_Parse);
	DetourAttach(&(PVOID&)g_Org_Panorama_CStylePropertyBorder_Parse, My_Panorama_CStylePropertyBorder_Parse);
	DetourAttach(&(PVOID&)g_Org_Panorama_CStylePropertyWashColor_Clone, My_Panorama_CStylePropertyWashColor_Clone);
	DetourAttach(&(PVOID&)g_Org_Panorama_CStylePropertyWashColor_Parse, My_Panorama_CStylePropertyWashColor_Parse);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour panorama functions.");
		return;
	}

	g_myPanoramaWrapper.hooked = true;
};

void HookDeathMsg(HMODULE clientDll) {
	if (g_MirvDeathMsgGlobals.hooked) return;

	if (!getPanoramaAddrsFromClient()) return;
	if (!getDeathMsgAddrs()) return;

	DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)g_Original_handlePlayerDeath, handleDeathnotice);
    DetourAttach(&(PVOID&)g_Original_getLocalSteamId, getLocalSteamId);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour DeathMsg functions.");
		return;
	}

	g_MirvDeathMsgGlobals.hooked = true;
};

void deathMsgId_PrintHelp_Console(const char * cmd)
{
	advancedfx::Message(
		"%s accepts the following as <id...>:\n"
		"<iNumber> - UserID. Example: 9\n"
		"x<iNumber> - XUID. Example: x76561198106931330\n"
		"k<iNumber> - Spectator key number.\n"
		"trace - UserID from a screen trace (e.g. current POV).\n"
		"xTrace - XUID from a screen trace (e.g. current POV).\n"
		"We recommend getting the numbers from the output of \"mirv_deathmsg help players\".\n"
		, cmd
	);
};

void deathMsgPlayers_PrintHelp_Console()
{
    int highestIndex = GetHighestEntityIndex();

	std::vector<std::vector<std::string>> rows = {
		{"name", "userid", "xuid", "speckey"}, {}
	};

    for(int i = 0; i < highestIndex + 1; i++) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
			if(!ent->IsPlayerController()) continue;

			auto teamNumber = *(int*)((u_char*)(ent) + g_clientDllOffsets.C_BaseEntity.m_iTeamNum);
			if (0 == teamNumber || 1 == teamNumber) continue;

			// I know it's nested loop, but on this scale it doesn't matter
			auto playerInfo = getPlayerInfoFromControllerIndex(i);

			rows.push_back({
				playerInfo.name,
				std::to_string(playerInfo.userId), // apparently in CS2 userid is playercontroller entityindex - 1
				std::string("x").append(std::to_string(playerInfo.xuid)),
				std::string("k").append(std::to_string(playerInfo.specKey))
			});
        }
    }

	if (rows.size() == 2) return;

	advancedfx::Message(
		"%s", afxUtils::createTable(rows, " | ", "-").c_str()
	);
};

struct CS2_MirvDeathMsg : MirvDeathMsg {
	bool colors (IWrpCommandArgs * args) 
	{
		int argc = args->ArgC();
		const char * arg0 = args->ArgV(0);

		std::string colors = "";

		for (int i = 0; i < afxBasicColors.size(); i++)
		{
			auto color = afxBasicColors[i];
			colors.append(color.name);
			if (i < afxBasicColors.size() - 1) colors.append(", ");
		}

		const char* options = 
			"Where <option> is one of:\n"
			"default - use default game color\n"
			"<0-255> <0-255> <0-255> <0-255> - color in RGBA format e.g. 255 0 0 255\n"
			"<color> - one of the default colors e.g. red\n";

		if (3 > argc)
		{
			advancedfx::Message(
				"%s colors ct <option> - Control CT color.\n"
				"%s colors t <option> - Control T color.\n"
				"%s colors border <option> - Control border color of local player.\n"
				"%s colors background <option> - Control background color.\n"
				"%s colors backgroundLocal <option> - Control background color of local player.\n"
				"\n"
				"%s"
				"\n"
				"Available colors:\n"
				"%s\n"
				, arg0, arg0, arg0, arg0, arg0, options, colors.c_str()
			);
			return true;	
		}

		const char* arg2 = args->ArgV(2);
		
		if (0 == _stricmp("ct", arg2))
		{
			if (3 == argc)
			{
				advancedfx::Message(
					"%s colors %s <option> - Control CT color in death messages.\n"
					"Current value: %s\n"
					, arg0, arg2
					, g_myPanoramaWrapper.CTcolor.use ? g_myPanoramaWrapper.CTcolor.userValue.c_str() : "default"
				);
				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.CTcolor.setColor(args->ArgV(3));
				g_myPanoramaWrapper.applyColors();
				return true;
			}

			if (7 == argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 3);
				g_myPanoramaWrapper.CTcolor.setColor(&subArgs);
				g_myPanoramaWrapper.applyColors();
				return true;
			}
		}

		if (0 == _stricmp("t", arg2))
		{
			if (3 == argc)
			{
				advancedfx::Message(
					"%s colors %s <option> - Control T color in death messages.\n"
					"Current value: %s\n"
					, arg0, arg2
					, g_myPanoramaWrapper.Tcolor.use ? g_myPanoramaWrapper.Tcolor.userValue.c_str() : "default"
				);
				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.Tcolor.setColor(args->ArgV(3));
				g_myPanoramaWrapper.applyColors();
				return true;
			}

			if (7 == argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 3);
				g_myPanoramaWrapper.Tcolor.setColor(&subArgs);
				g_myPanoramaWrapper.applyColors();
				return true;
			}
		}

		if (0 == _stricmp("border", arg2))
		{
			if (3 == argc)
			{
				advancedfx::Message(
					"%s colors %s <option> - Control border color of local player in death messages.\n"
					"Current value: %s\n"
					, arg0, arg2
					, g_myPanoramaWrapper.BorderColor.use ? g_myPanoramaWrapper.BorderColor.userValue.c_str() : "default"
				);
				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.BorderColor.setColor(args->ArgV(3));
				g_myPanoramaWrapper.applyColors();
				return true;
			}

			if (7 == argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 3);
				g_myPanoramaWrapper.BorderColor.setColor(&subArgs);
				g_myPanoramaWrapper.applyColors();
				return true;
			}
		}

		if (0 == _stricmp("background", arg2))
		{
			if (3 == argc)
			{
				advancedfx::Message(
					"%s colors %s <option> - Control background color of death messages.\n"
					"Current value: %s\n"
					, arg0, arg2
					, g_myPanoramaWrapper.BackgroundColor.use ? g_myPanoramaWrapper.BackgroundColor.userValue.c_str() : "default"
				);

				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.BackgroundColor.setColor(args->ArgV(3));
				g_myPanoramaWrapper.applyColors();
				return true;
			}

			if (7 == argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 3);
				g_myPanoramaWrapper.BackgroundColor.setColor(&subArgs);
				g_myPanoramaWrapper.applyColors();
				return true;
			}
		}

		if (0 == _stricmp("backgroundLocal", arg2))
		{
			if (3 == argc)
			{
				advancedfx::Message(
					"%s colors %s <option> - Control background color of local player.\n"
					"Current value: %s\n"
					, arg0, arg2
					, g_myPanoramaWrapper.LocalBackgroundColor.use ? g_myPanoramaWrapper.LocalBackgroundColor.userValue.c_str() : "default"
				);

				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.LocalBackgroundColor.setColor(args->ArgV(3));
				g_myPanoramaWrapper.applyColors();
				return true;
			}

			if (7 == argc)
			{
				advancedfx::CSubCommandArgs subArgs(args, 3);
				g_myPanoramaWrapper.LocalBackgroundColor.setColor(&subArgs);
				g_myPanoramaWrapper.applyColors();
				return true;
			}
		}

		advancedfx::Message(
			"%s colors ct <option> - Control CT color.\n"
			"%s colors t <option> - Control T color.\n"
			"%s colors border <option> - Control border color of local player.\n"
			"%s colors background <option> - Control background color.\n"
			"%s colors backgroundLocal <option> - Control background color of local player.\n"
			"\n"
			"%s"
			"\n"
			"Available colors:\n"
			"%s\n"
			, arg0, arg0, arg0, arg0, arg0, options, colors.c_str()
		);
		return true;
	};
} g_MirvDeathMsg;

bool mirvDeathMsg_Console(advancedfx::ICommandArgs* args)
{
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);
		if (0 == _stricmp("clear", arg1))
		{
			auto result = g_myPanoramaWrapper.clearDeathnotices();
			return true;
		} else
		if (0 == _stricmp("filter", arg1))
		{
			return g_MirvDeathMsg.filter(args, g_MirvDeathMsgGlobals);
		} else
		if (0 == _stricmp("lifetime", arg1)) {
			return g_MirvDeathMsg.lifetime(args, g_MirvDeathMsgGlobals);
		} else
		if (0 == _stricmp("lifetimeMod", arg1)) {
			return g_MirvDeathMsg.lifetimeMod(args, g_MirvDeathMsgGlobals);
		} else
		if (0 == _stricmp("localPlayer", arg1)) {
			return g_MirvDeathMsg.localPlayer(args, g_MirvDeathMsgGlobals);
		} else
		if (0 == _stricmp("debug", arg1))
		{
			return g_MirvDeathMsg.debug(args, g_MirvDeathMsgGlobals);
		} else
		if (0 == _stricmp("colors", arg1)) {
			return g_MirvDeathMsg.colors(args);
		}
		if (0 == _stricmp("help", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("id", arg2))
				{
					deathMsgId_PrintHelp_Console(arg0);
					return true;
				}

				if (0 == _stricmp("players", arg2))
				{
					deathMsgPlayers_PrintHelp_Console();
					return true;
				}

			}
			advancedfx::Message(
				"%s help id - Print help on <id...> usage.\n"
				"%s help players - Print available player ids.\n"
				, arg0, arg0, arg0
			);
			return true;
		}
	}

	advancedfx::Message(
		"%s clear - Clears all deathnotices.\n"
		"%s filter [...] - Filter death messages.\n"
		"%s lifetime [...] - Controls lifetime of death messages.\n"
		"%s lifetimeMod [...] - Controls lifetime modifier of death messages for the \"local\" player.\n"
		"%s localPlayer [...] - Controls what is considered \"local\" player (and thus highlighted in death notices).\n"
		"%s debug [...] - Enable / Disable debug spew upon death messages.\n"
		"%s colors [...] - Controls colors of death messages.\n"
		"%s help [...] - Print help.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;
};

CON_COMMAND(mirv_deathmsg, "controls death notification options")
{
	mirvDeathMsg_Console(args);
};

enum panelMatchType {
	ID = 0,
	CLASS_NAME
};

void applyStyleProperty_Console(IWrpCommandArgs * args) {
	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	panelMatchType matchType = panelMatchType::ID;
	std::string panelId = "";
	// TODO: match by property type, when add new ones
	bool didMatchProperty = false;
	float opacity = 0;

	for (int i = 1; i < argc; ++i)
	{
		const char * argI = args->ArgV(i);
		if (StringIBeginsWith(argI, "panelId="))
		{
			panelId = argI + strlen("panelId=");
			matchType = panelMatchType::ID;
		}
		else if (StringIBeginsWith(argI, "panelClassName="))
		{
			panelId = argI + strlen("panelClassName=");
			matchType = panelMatchType::CLASS_NAME;
		}
		else if (StringIBeginsWith(argI, "opacity="))
		{
			opacity = float(atof(argI + strlen("opacity=")));
			didMatchProperty = true;
		}
	}

	if (panelId.empty()) {
		advancedfx::Warning("PanelId cannot be empty.\n");
		return;
	}

	if (!didMatchProperty) {
		advancedfx::Warning("Did not match any style property.\n");
		return;
	}

	auto parentPanel = ((u_char***)g_myPanoramaWrapper.pHudPanel)[0][1];
	if (!parentPanel) {
		advancedfx::Warning("Root panel is 0\n");
		return;
	}


	if (matchType == panelMatchType::ID) {
		u_char* targetPanel = g_myPanoramaWrapper.findChildInLayoutFile(parentPanel, panelId.c_str());

		if (0 == targetPanel) {
			advancedfx::Warning("Could not find panel %s\n", panelId.c_str());
			return;
		}

		auto res = ((CUIPanel*)targetPanel)->setOpacity(std::clamp(opacity, 0.0f, 1.0f));
		if (!res) {
			advancedfx::Warning("Could not set opacity property for %s\n", panelId.c_str());
		}
	} else if (matchType == panelMatchType::CLASS_NAME) {
		auto foundPanels = g_myPanoramaWrapper.findChildrenInLayoutFileByClassName(parentPanel, panelId.c_str());
		if (foundPanels.empty()) {
			advancedfx::Warning("Could not find panels with className %s\n", panelId.c_str());
		} else {
			for (auto panel : foundPanels) {
				((CUIPanel*)panel)->setOpacity(std::clamp(opacity, 0.0f, 1.0f));
			}
		}
	}
}

CON_COMMAND(mirv_panorama, "")
{
	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("panelStyle", arg1)) {
			if (3 <= argc) {
				CSubWrpCommandArgs subArgs(args, 2);
				applyStyleProperty_Console(&subArgs);
			} else {
				advancedfx::Message(
					"%s %s panelStyle <option> <option>\n"
					"Where <option> at least 2 arguments are required: panelId or class and property to set.\n"
					"%s %s:\n"
					"\tpanelId=<str>\n"
					"\tpanelClassName=<str>\n"
					"\topacity=<fValue>\n"
					"Example:\n"
					"%s %s panelId=trueview_row opacity=0\n"
					"%s %s panelClassName=HudPerfStatsBasics opacity=0\n"
					"Warning: if matching by className the style would be applied to all instances.\n"
					, arg0, arg1
					, arg0, arg1
					, arg0, arg1
					, arg0, arg1
				);
			}
			return;
		}
	}

	advancedfx::Message(
		"%s panelStyle [...] - Set style for specific panorama panel.\n"
		, arg0
	);
}

