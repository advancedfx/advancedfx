#define GAME_CS2

#include <iostream>
#include "WrpConsole.h"
#include "../shared/MirvDeathMsgFilter.h"

#include <cstddef>
#include <Windows.h>
#include "../deps/release/Detours/src/detours.h"
#include "../deps/release/prop/cs2/sdk_src/public/igameevents.h"

#include "DeathMsg.h"
#include "Globals.h"
#include "ClientEntitySystem.h"

advancedfx::Con_Printf_t conMessage = nullptr;
advancedfx::Con_Printf_t conWarning = nullptr;

currentGameCamera g_CurrentGameCamera;

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

	EntityListIterator it;
    int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();
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

			auto xuid = *(uint64_t*)((u_char*)(playerController) + CS2::CBasePlayerController::m_steamID);
			auto name = (char*)((u_char*)(playerController) + CS2::CBasePlayerController::m_iszPlayerName);

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

	EntityListIterator it;
    int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();

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

			auto teamNumber = *(int*)((u_char*)(ent) + CS2::C_BaseEntity::m_iTeamNum);
			if (0 == teamNumber || 1 == teamNumber) continue;

			auto xuid = *(uint64_t*)((u_char*)(ent) + CS2::CBasePlayerController::m_steamID);
			auto name = (char*)((u_char*)(ent) + CS2::CBasePlayerController::m_iszPlayerName);

			result.name = name;
			result.xuid = xuid;
			result.playerController = (u_char*)(ent);
			result.userId = i - 1;

			int slot = 0;
			if (3 == teamNumber) // CT
			{
				slot = 1 + slotCT;
				if (swapPlayerSide) slot += 5;
				++slotCT;
			} else
			if (2 == teamNumber) // T
			{
				slot = 1 + slotT;
				if (!swapPlayerSide) slot += 5;
				++slotT;
			}
			slot = slot % 10;

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
	if (userId < 1) return false;

	switch(Mode)
	{
		case Id_UserId:
			return userId == Id.userId;
			break;
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
			EntityListIterator it;
			int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();
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
			EntityListIterator it;
			int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();
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

struct AfxBasicColor {
	char* name;
	uint32_t value;
};

std::vector<AfxBasicColor> afxBasicColors = {
	// format alpha blue green red
	{"red", 0xFF0000FF},
	{"green", 0xFF00FF00},
	{"blue", 0xFFFF0000},
	{"yellow", 0xFF00FFFF},
	{"cyan", 0xFFFFFF00},
	{"magenta", 0xFFFF00FF},
	{"white", 0xFFFFFFFF},
	{"black", 0xFF000000},
	{"90black", 0xE6000000},
	{"75black", 0xBF000000},
	{"50black", 0x80000000},
	{"25black", 0x40000000},
	{"10black", 0x19000000},
	{"transparent", 0x00000000},

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

			auto hexStr = afxUtils::rgbaToHex(str, advancedfx::Warning);
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
					value = it->value;
					return true;
				}
			}

			uint32_t color;
			if (convertColorFromStrToInt(arg, &color))
			{
				use = true;
				userValue = arg;
				value = color;
				return true;
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
		const auto makeSymbol = *(makeSymbol_t*)((*(u_char**)(*pUIEngine)) + CS2::PanoramaUIEngine::makeSymbol);

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

		// advancedfx::Message("Deathnotices: %llX\n", pDeathnotices);

		for (int i = 0; i < *(int*)pDeathnotices; i++) {
			const auto panel = ((u_char***)pDeathnotices)[1][i];

			typedef const char* (__fastcall *getAttributeString_t)(u_char* ptr, short attributeName, const char* defaultValue);
			const auto getAttributeString = *(getAttributeString_t*)(*(u_char**)panel + CS2::PanoramaUIPanel::getAttributeString);

			typedef void* (__fastcall *setAttributeString_t)(u_char* ptr, short attributeName, const char* defaultValue);
			const auto setAttributeString = *(setAttributeString_t*)(*(u_char**)panel + CS2::PanoramaUIPanel::setAttributeString);

			const auto lifetimeString = getAttributeString(panel, lifeTimeSymbol, "");
			if (!lifetimeString || strlen(lifetimeString) == 0) continue;

			const auto spawnTimeString = getAttributeString(panel, spawnTimeSymbol, "");
			if (!spawnTimeString || strlen(spawnTimeString) == 0) continue;

			setAttributeString(panel, lifeTimeSymbol, "0.001");
			setAttributeString(panel, spawnTimeSymbol, std::to_string(g_CurrentGameCamera.time - 1).c_str());
			// advancedfx::Message("deathnotice panel: %llX\n", panel);
			result = true;
		}

		return result;
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
			*(uint32_t*)(BorderColor.pointer) = BorderColor.use ? BorderColor.value : BorderColor.defaultValue;
		}
		if (nullptr != LocalBackgroundColor.pointer) {
			*(uint32_t*)(LocalBackgroundColor.pointer) = LocalBackgroundColor.use ? LocalBackgroundColor.value : LocalBackgroundColor.defaultValue;
		}	
		if (nullptr != BackgroundColor.pointer) {
			*(uint32_t*)(BackgroundColor.pointer) = BackgroundColor.use ? BackgroundColor.value : BackgroundColor.defaultValue;
		}	
		if (nullptr != CTcolor.pointer) {
			*(uint32_t*)(CTcolor.pointer) = CTcolor.use ? CTcolor.value : CTcolor.defaultValue;
		}
		if (nullptr != Tcolor.pointer) {
			*(uint32_t*)(Tcolor.pointer) = Tcolor.use ? Tcolor.value : Tcolor.defaultValue;
		}
	}

} g_myPanoramaWrapper;

typedef u_char* (__fastcall *g_Original_hashString_t)(uint32_t* pResult, const char* string);
g_Original_hashString_t g_Original_hashString = nullptr;

class MyDeathMsgGameEventWrapper : public SOURCESDK::CS2::IGameEvent, public MyDeathMsgGameEventWrapperBase
{
public:
	MyDeathMsgGameEventWrapper(SOURCESDK::CS2::IGameEvent * event)
	: m_Event(event) { }

	uint32_t hashString(const char * string) {
		uint32_t result;
		g_Original_hashString(&result, string);
		return result;
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
	virtual bool IsEmpty( const u_int &keySymbol ) {
		return m_Event->IsEmpty(keySymbol);
	}
	virtual bool GetBool( const u_int &keySymbol) {
		return m_Event->GetBool(keySymbol);
	}
	virtual int GetInt( const u_int &keySymbol) {

		if (assistedflash.use && hashString("assistedflash") == keySymbol) return assistedflash.value;
		if (headshot.use && hashString("headshot") == keySymbol) return headshot.value;
		if (penetrated.use && hashString("penetrated") == keySymbol) return penetrated.value;
		if (dominated.use && hashString("dominated") == keySymbol) return dominated.value;
		if (revenge.use && hashString("revenge") == keySymbol) return revenge.value;
		if (wipe.use && hashString("wipe") == keySymbol) return wipe.value;
		if (noscope.use && hashString("noscope") == keySymbol) return noscope.value;
		if (thrusmoke.use && hashString("thrusmoke") == keySymbol) return thrusmoke.value;
		if (attackerblind.use && hashString("attackerblind") == keySymbol) return attackerblind.value;
		if (attackerinair.use && hashString("attackerinair") == keySymbol) return attackerinair.value;
		
		return m_Event->GetInt(keySymbol);
	}
	virtual uint64_t GetUint64( const u_int &keySymbol) {
		return m_Event->GetUint64(keySymbol);
	}
	virtual float GetFloat( const u_int &keySymbol) {
		return m_Event->GetFloat(keySymbol);
	}
	virtual const char *GetString( const u_int &keySymbol) {
		if (weapon.use && hashString("weapon") == keySymbol) return weapon.value;
		return m_Event->GetString(keySymbol);
	}
	virtual void *GetPtr( const u_int &keySymbol ) {
		return m_Event->GetPtr(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityHandle GetEHandle( const u_int &keySymbol ) {
		return m_Event->GetEHandle(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetEntity( const u_int &keySymbol) {
		return m_Event->GetEntity(keySymbol);
	}
	virtual void* GetEntityIndex( const u_int &keySymbol ) {
		return m_Event->GetEntityIndex(keySymbol);
	}
	virtual SOURCESDK::CS2::CPlayerSlot GetPlayerSlot( const u_int &keySymbol ) {

		if (attacker.newId.use && hashString("attacker") == keySymbol) return SOURCESDK::CS2::CPlayerSlot(attacker.newId.value.ResolveToUserId());
		if (victim.newId.use && hashString("userid") == keySymbol) return SOURCESDK::CS2::CPlayerSlot(victim.newId.value.ResolveToUserId());
		if (assister.newId.use && hashString("assister") == keySymbol) return SOURCESDK::CS2::CPlayerSlot(assister.newId.value.ResolveToUserId());

		return m_Event->GetPlayerSlot(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetPlayerController( const u_int &keySymbol ) {
		return m_Event->GetPlayerController(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityInstance *GetPlayerPawn( const u_int &keySymbol ) {
		return m_Event->GetPlayerPawn(keySymbol);
	}
	virtual SOURCESDK::CS2::CEntityHandle GetPawnEHandle( const u_int &keySymbol ) {
		return m_Event->GetPawnEHandle(keySymbol);
	}
	virtual void* GetPawnEntityIndex( const u_int &keySymbol ) {
		return m_Event->GetPawnEntityIndex(keySymbol);
	}
	virtual void SetBool( const u_int &keySymbol, bool value ) {
		m_Event->SetBool(keySymbol,value);
	}
	virtual void SetInt( const u_int &keySymbol, int value ) {
		m_Event->SetInt(keySymbol,value);
	}
	virtual void SetUint64( const u_int &keySymbol, uint64_t value ) {
		m_Event->SetUint64(keySymbol,value);
	}
	virtual void SetFloat( const u_int &keySymbol, float value ) {
		m_Event->SetFloat(keySymbol,value);
	}
	virtual void SetString( const u_int &keySymbol, const char *value ) {
		m_Event->SetString(keySymbol,value);
	}
	virtual void SetPtr( const u_int &keySymbol, void *value ) {
		m_Event->SetPtr(keySymbol,value);
	}
	virtual void SetEntity(const u_int &keySymbol, SOURCESDK::CS2::CEntityInstance *value) {
		m_Event->SetEntity(keySymbol,value);
	}
	virtual void SetEntity( const u_int &keySymbol, void* value ) {
		m_Event->SetEntity(keySymbol,value);
	}
	virtual void SetPlayer( const u_int &keySymbol, SOURCESDK::CS2::CEntityInstance *pawn ) {
		m_Event->SetPlayer(keySymbol,pawn);
	}
	virtual void SetPlayer( const u_int &keySymbol, SOURCESDK::CS2::CPlayerSlot value ) {
		m_Event->SetPlayer(keySymbol,value);
	}
	virtual void SetPlayerRaw( const u_int &controllerKeySymbol, const u_int &pawnKeySymbol, SOURCESDK::CS2::CEntityInstance *pawn ) {
		m_Event->SetPlayerRaw(controllerKeySymbol,pawnKeySymbol,pawn);
	}
	virtual bool HasKey( const u_int &keySymbol ) {
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
		if (g_MirvDeathMsgGlobals.activeWrapper->attacker.isLocal.use && g_MirvDeathMsgGlobals.activeWrapper->attacker.isLocal.value) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->attacker;			
			use = true;
		}
		if (g_MirvDeathMsgGlobals.activeWrapper->victim.isLocal.use && g_MirvDeathMsgGlobals.activeWrapper->victim.isLocal.value) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->victim;
			use = true;
		}
		if (g_MirvDeathMsgGlobals.activeWrapper->assister.isLocal.use && g_MirvDeathMsgGlobals.activeWrapper->assister.isLocal.value) {
			entry = g_MirvDeathMsgGlobals.activeWrapper->assister;
			use = true;
		}
	}

	if (!use) return g_Original_getLocalSteamId(param_1);

	switch (entry.newId.value.Mode) {
		case DeathMsgId::Id_Key:
		{
			EntityListIterator it;
			int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();	
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

	if (0 != result) return result;

	return g_Original_getLocalSteamId(param_1);
};

// param 1 CCSGO_HudDeathNotice : panorama::CPanel2D : panorama::IUIPanelClient : CCSGOHudElement : CGameEventListener : IGameEventListener2
// param 2 IGameEvent
typedef void (__fastcall *g_Original_handlePlayerDeath_t)(u_char* param_1, SOURCESDK::CS2::IGameEvent* param_2);
g_Original_handlePlayerDeath_t g_Original_handlePlayerDeath = nullptr;

void __fastcall handleDeathnotice(u_char* hudDeathNotice, SOURCESDK::CS2::IGameEvent* gameEvent) {
	MyDeathMsgGameEventWrapper myWrapper(gameEvent);

	auto pDeathNoticeLifetime = (float*)(hudDeathNotice + 0x74);
	auto pDeathNoticeLocalPlayerLifetimeMod = (float*)(hudDeathNotice + 0x78);

	auto uidAttacker = gameEvent->GetInt(myWrapper.hashString("attacker"));
	auto uidVictim = gameEvent->GetInt(myWrapper.hashString("userid"));
	auto uidAssister = gameEvent->GetInt(myWrapper.hashString("assister"));

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

				nullptr != attackerController ? (char*)((u_char*)attackerController + CS2::CBasePlayerController::m_iszPlayerName) : "null",
				std::to_string(uidAttacker),

				nullptr != victimController ? (char*)((u_char*)victimController + CS2::CBasePlayerController::m_iszPlayerName) : "null",
				std::to_string(uidVictim),

				nullptr != assisterController ? (char*)((u_char*)assisterController + CS2::CBasePlayerController::m_iszPlayerName) : "null",
				std::to_string(nullptr != assisterController ? uidAssister : 0),
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
		strcpy(
			(*(char**)((u_char*)attackerController + CS2::CCSPlayerController::m_sSanitizedPlayerName)),
			myWrapper.attacker.name.value
		);
	}

	if (myWrapper.victim.name.use && nullptr != victimController) {
		strcpy(
			(*(char**)((u_char*)victimController + CS2::CCSPlayerController::m_sSanitizedPlayerName)),
			myWrapper.victim.name.value
		);
	}

	if (myWrapper.assister.name.use && nullptr != assisterController) {
		strcpy(
			(*(char**)((u_char*)assisterController + CS2::CCSPlayerController::m_sSanitizedPlayerName)),
			myWrapper.assister.name.value
		);
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
		*pDeathNoticeLifetime = myWrapper.lifetime.value;
	}

	if (myWrapper.lifetimeMod.use)
	{
		*pDeathNoticeLocalPlayerLifetimeMod = myWrapper.lifetimeMod.value;
	}

	g_MirvDeathMsgGlobals.activeWrapper = &myWrapper;

    g_Original_handlePlayerDeath(hudDeathNotice, &myWrapper);
};

typedef void (__fastcall *g_Original_DeathMsgColors_t)(uint64_t a1, u_char* a2);
g_Original_DeathMsgColors_t g_Original_DeathMsgColors = nullptr;

u_char* findAddress(u_char* startAddress, size_t &range, uint32_t &targetValue) {

	u_char* endAddress = startAddress + range;

    for (auto ptr = startAddress; ptr <= endAddress - sizeof(uint32_t); ptr += sizeof(uint32_t)) {
		uint32_t value = 0;
		memcpy(&value, ptr, sizeof(uint32_t));
        if (value == targetValue) {
            return ptr;
        }
    }


    return nullptr;
}

void __fastcall myDeathMsgColors(uint64_t a1, u_char* a2) {

	if (nullptr == a2 || nullptr == *(uint32_t**)(a2 + 0x8) || nullptr == *(uint32_t**)(a2 + 0x10)) {
		g_myPanoramaWrapper.applyColors();
		g_Original_DeathMsgColors(a1, a2);
		return;
	}

	auto colorProp = *(uint32_t*)(a2 + 0x8);
	auto nextProp = *(uint32_t*)(a2 + 0x10);

	// check for default color and next address value, they are always the same
	// second check is needed because it can get wrong address if checks too early
	if (nullptr == g_myPanoramaWrapper.LocalBackgroundColor.pointer && 0xE7000000 == colorProp && 0x3F800000 == nextProp)
	{
		// address doesn't change later
		g_myPanoramaWrapper.LocalBackgroundColor.pointer = a2 + 0x8;
	}

	if (
		nullptr == g_myPanoramaWrapper.BackgroundColor.pointer
		&& colorProp == 0xA0000000 && nextProp == 0x3F800000
	)
	{
		// because color is not unique and used in multiple places we have to find right one
		// after right one there is another background color property, so we can check against it
		// but the offset is dynamic, so we search in range
	
		u_char* endAddress = a2 + 0x120;
		uint32_t nextColorProp = 0;
		bool found = false;

		for (auto ptr = a2; ptr < endAddress - 4; ptr += 4) {
			if ((uintptr_t)ptr % 4 != 0) continue;

			memcpy(&nextColorProp, ptr, 4);
			if (nextColorProp == 0xD2060663) {
				found = true;
				break;
			}
		}

		if (found) g_myPanoramaWrapper.BackgroundColor.pointer = a2 + 0x8;
	}

	if (
		nullptr == g_myPanoramaWrapper.CTcolor.pointer && nullptr == g_myPanoramaWrapper.Tcolor.pointer &&
		0xFFE69C6F == colorProp && 0x3F800000 == nextProp &&
		nullptr != *(uint32_t**)(a2 + 0x38) && nullptr != *(uint32_t**)(a2 + 0x40)
	)
	{
		auto TcolorProp = *(uint32_t*)(a2 + 0x38);
		auto TcolorNextProp = *(uint32_t*)(a2 + 0x40);
		// not really needed, but just in case
		if (0xFF54BEEA == TcolorProp && 0x3F800000 == TcolorNextProp)
		{
			// these always come together
			g_myPanoramaWrapper.CTcolor.pointer = a2 + 0x8;
			g_myPanoramaWrapper.Tcolor.pointer = a2 + 0x38;
		}
	}

	g_myPanoramaWrapper.applyColors();
	g_Original_DeathMsgColors(a1, a2);
};

typedef void (__fastcall *g_Original_DeathMsgBorderColor_t)(u_char* a1);
g_Original_DeathMsgBorderColor_t g_Original_DeathMsgBorderColor = nullptr;

void __fastcall myDeathMsgBorderColor(u_char* a1) {

	if (nullptr == a1 || nullptr == *(uint32_t**)(a1 + 0x38) || nullptr == *(uint32_t**)(a1 + 0x40)) {
		g_myPanoramaWrapper.applyColors();
		g_Original_DeathMsgBorderColor(a1);
		return;
	}

	auto colorProp = *(uint32_t*)(a1 + 0x38);
	auto nextProp = *(uint32_t*)(a1 + 0x40);

	if (nullptr == g_myPanoramaWrapper.BorderColor.pointer && 0xFF0000E1 == colorProp && 0xFF0000E1 == nextProp)
	{
		g_myPanoramaWrapper.BorderColor.pointer = a1 + 0x38;
	}

	g_myPanoramaWrapper.applyColors();
	g_Original_DeathMsgBorderColor(a1);
};

bool getDeathMsgAddrs(HMODULE clientDll) {
	// can be found with strings like "attacker" and "userid", etc. it basically takes all info from player_death event
	size_t g_Original_handlePlayerDeath_addr = getAddress(clientDll, "48 89 54 24 10 48 89 4C 24 08 55 53 57 41 56 41 57 48 8D AC 24 00 DF FF");
	if (g_Original_handlePlayerDeath_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	};
	// called in multiple places with strings like "userid", "attacker", etc. as second argument
	size_t g_Original_hashString_addr = getAddress(clientDll, "48 89 74 24 10 57 48 81 EC C0 00 00 00 33 C0 48 8B FA 89 01 48 8B");
	if (g_Original_hashString_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	};

	// snippet from function handlePlayerDeath above	
	//   if (*(char *)(lVar17 + 0xb8) == '\0') {
	//     uVar18 = FUN_1808a1a00();
	//
	//     iVar10 = FUN_1808af610(uVar18); // the one we need called here, it returns local steamid, function has 2 xrefs
	//									   // later there is check if attackersteamid is equal to local one
	//
	//     if (((iVar10 != 0) && (plVar14 != (longlong *)0x0)) &&
	//        (piVar13 = (int *)FUN_18056a170(plVar14,&uStackX_20), *piVar13 == iVar10)) {
	//       bVar4 = true;
	//     }
	//   }
	size_t g_Original_getLocalSteamId_addr = getAddress(clientDll,"40 53 48 83 EC ?? 8B 91 ?? ?? ?? ?? 48 8B D9 83 FA ?? 0F 84 ?? ?? ?? ?? 4C 8B 0D");
	if (0 == g_Original_getLocalSteamId_addr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	};

	g_Original_handlePlayerDeath = (g_Original_handlePlayerDeath_t)(g_Original_handlePlayerDeath_addr);
	g_Original_hashString = (g_Original_hashString_t)(g_Original_hashString_addr);
	g_Original_getLocalSteamId = (g_Original_getLocalSteamId_t)(g_Original_getLocalSteamId_addr);

	return true;
};

bool getPanoramaAddrsFromClient(HMODULE clientDll) {
	// credit https://github.com/danielkrupinski/Osiris

	// the only function that has file://{resources}/layout/hud/hud.xml" string
	// hudpanel is DAT that param_1 assigned to
	size_t g_HudPanel_addr = getAddress(clientDll, "89 ?? ?? ?? ?? ?? C6 ?? ?? ?? ?? ?? ?? 48 89 ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48");
	if (g_HudPanel_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	} else {
		g_HudPanel_addr += 16;
	};

	// function has CreatePanelWithCurrentContext string
	// engine is DAT that param_1 assigned to
	size_t g_CUIEngine_addr = getAddress(clientDll, "48 89 78 ?? 48 89 0D ?? ?? ?? ??");
	if (g_CUIEngine_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	} else {
		g_CUIEngine_addr += 7;
	};

	uint32_t g_HudPanel_offset;
	std::memcpy(&g_HudPanel_offset, (void*)(g_HudPanel_addr), sizeof(g_HudPanel_offset));
	g_myPanoramaWrapper.pHudPanel = (u_char**)(g_HudPanel_addr + g_HudPanel_offset + 4);

	uint32_t g_CUIEngine_offset;
	std::memcpy(&g_CUIEngine_offset, (void*)(g_CUIEngine_addr), sizeof(g_CUIEngine_offset));
	g_myPanoramaWrapper.pUIEngine = (u_char**)(g_CUIEngine_addr + g_CUIEngine_offset + 4);

	return true;
};

bool getPanoramaAddrs(HMODULE panoramaDll) {
	// has "Invalid type %d on fillbrush" string
	size_t g_Original_BackgroundColor_addr = getAddress(panoramaDll, "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B DA 48 8B F9 8B 12");
	if (g_Original_BackgroundColor_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	}
	g_Original_DeathMsgColors = (g_Original_DeathMsgColors_t)g_Original_BackgroundColor_addr;

	// has CStylePropertyBorder or ??_7CStylePropertyBorder@panorama@@6B@
	// also can be found in another function that has "border-color" string, it's being set to one of DATs in the end
	// sig looks a bit spooky, but I doubt they will ever change this function
	size_t g_Original_BorderColor_addr = getAddress(panoramaDll, "40 53 48 83 EC ?? 48 8B 05 ?? ?? ?? ?? 48 8B D9 BA ?? ?? ?? ?? 48 8B 08 48 8B 01 FF 50 ?? 48 8B D0 48 85 C0 74 ?? 48 8D 05 ?? ?? ?? ?? 48 89 02 48 8D 05 ?? ?? ?? ?? 0F B6 4B ?? 88 4A ?? 0F B6 4B ?? 48 89 02 88 4A ?? 8B 4B ?? 89 4A ?? 0F 10 43");
	if (g_Original_BorderColor_addr == 0) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return false;
	}
	g_Original_DeathMsgBorderColor = (g_Original_DeathMsgBorderColor_t)g_Original_BorderColor_addr;

	return true;
};

void HookPanorama(HMODULE panoramaDll)
{
	if (g_myPanoramaWrapper.hooked) return;

	if (!getPanoramaAddrs(panoramaDll)) return;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)g_Original_DeathMsgColors, myDeathMsgColors);
	DetourAttach(&(PVOID&)g_Original_DeathMsgBorderColor, myDeathMsgBorderColor);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour panorama functions.");
		return;
	}

	g_myPanoramaWrapper.hooked = true;
};

void HookDeathMsg(HMODULE clientDll) {
	if (g_MirvDeathMsgGlobals.hooked) return;

    if (!getDeathMsgAddrs(clientDll)) return;
	if (!getPanoramaAddrsFromClient(clientDll)) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)g_Original_handlePlayerDeath, handleDeathnotice);
    DetourAttach(&(PVOID&)g_Original_getLocalSteamId, getLocalSteamId);

	if(NO_ERROR != DetourTransactionCommit()) {
		ErrorBox("Failed to detour DeathMsg functions.");
		return;
	};

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
	EntityListIterator it;
    int highestIndex = g_GetHighestEntityIterator(*g_pEntityList, &it)->GetIndex();

	std::vector<std::vector<std::string>> rows = {
		{"name", "userid", "xuid", "speckey"}, {}
	};

    for(int i = 0; i < highestIndex + 1; i++) {
        if(auto ent = (CEntityInstance*)g_GetEntityFromIndex(*g_pEntityList,i)) {
			if(!ent->IsPlayerController()) continue;

			auto teamNumber = *(int*)((u_char*)(ent) + CS2::C_BaseEntity::m_iTeamNum);
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

void colors_PrintHelp_Console(const char * cmd)
{
	std::string msg = "Available colors:\n";

	for (auto it = afxBasicColors.begin(); it != afxBasicColors.end(); ++it)
	{
		msg.append(std::string(it->name) + "\n");
	}

	advancedfx::Message(msg.c_str());
}

struct CS2_MirvDeathMsg : MirvDeathMsg {
	bool colors (IWrpCommandArgs * args) 
	{
		int argc = args->ArgC();
		const char * arg0 = args->ArgV(0);

		const char* options = 
			"Where <option> is one of:\n"
			"default - use default game color\n"
			"\"i,i,i,i\" - color in rgba format e.g. \"255,0,0,255\"\n"
			"\"option\" - one of the default colors e.g. \"red\"\n"
			"use \"mirv_deathmsg help colors\" to see all default colors\n";

		if (3 > argc)
		{
			advancedfx::Message(
				"%s colors ct <option> - Control CT color.\n"
				"%s colors t <option> - Control T color.\n"
				"%s colors border <option> - Control border color of local player.\n"
				"%s colors background <option> - Control background color.\n"
				"%s colors backgroundLocal <option> - Control background color of local player.\n"
				"%s"
				, arg0, arg0, arg0, arg0, arg0, options
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

				advancedfx::Message("%u\n", g_myPanoramaWrapper.BackgroundColor.use ? g_myPanoramaWrapper.BackgroundColor.value : 0);
				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.BackgroundColor.setColor(args->ArgV(3));
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

				advancedfx::Message("%u\n", g_myPanoramaWrapper.LocalBackgroundColor.use ? g_myPanoramaWrapper.LocalBackgroundColor.value : 0);
				return true;
			}

			if (4 == argc)
			{
				g_myPanoramaWrapper.LocalBackgroundColor.setColor(args->ArgV(3));
				return true;
			}
		}

		advancedfx::Message(
			"%s colors ct <option> - Control CT color.\n"
			"%s colors t <option> - Control T color.\n"
			"%s colors border <option> - Control border color of local player.\n"
			"%s colors background <option> - Control background color.\n"
			"%s colors backgroundLocal <option> - Control background color of local player.\n"
			"%s"
			, arg0, arg0, arg0, arg0, arg0, options
		);
		return true;
	};
} g_MirvDeathMsg;

bool mirvDeathMsg_Console(advancedfx::ICommandArgs* args)
{
	if (nullptr == conMessage || nullptr == conWarning)
	{
		conMessage = advancedfx::Message;
		conWarning = advancedfx::Warning;
	}

	const auto arg0 = args->ArgV(0);
	int argc = args->ArgC();

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);
		if (0 == _stricmp("clear", arg1))
		{
			auto result = g_myPanoramaWrapper.clearDeathnotices();
			advancedfx::Message("cleared deathnotices: %s\n", result ? "true" : "false");
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

				if (0 == _stricmp("colors", arg2))
				{
					colors_PrintHelp_Console(arg0);
					return true;
				}
			}
			advancedfx::Message(
				"%s help id - Print help on <id...> usage.\n"
				"%s help players - Print available player ids.\n"
				"%s help colors - Print available basic colors.\n"
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
