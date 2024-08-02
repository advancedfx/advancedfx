#include "stdafx.h"

#include "csgo_CHudDeathNotice.h"

#include "addresses.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "RenderView.h"
#include "csgo/ClientToolsCSgo.h"

#include "../shared/MirvDeathMsgFilter.h"

#include "AfxStreams.h"

#include <shared/AfxDetours.h>
#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <list>
#include <queue>
#include <sstream>

/*

sub_1011CD40

void CHudBaseDeathNotice::FireGameEvent_UnkDoNotice( IGameEvent *event )

event->GetInt("attacker",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetInt("userid",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo
event->GetInt("assister",0) -> VEngineClient014->GetPlayerForUserID --> VEngineClient014->GetPlayerInfo,  IClientNetworkable::entindex((IClientNetworkable)localPlayer)
event->GetInt("assistedflash",0) 
event->GetString("weapon",0)
event->GetInt("headshot",0)
event->GetInt("penetrated",0)
event->GetInt("dominated",0)
event->GetInt("revenge",0)
event->GetInt("wipe",0)
event->GetInt("noscope",0) // new
event->GetInt("thrusmoke",0) // new
event->GetInt("attackerblind",0) // new

- SpawnTime
- Lifetime
- [LifetimeMod]
".?AVCUIPanel@panorama@@" (2nd ref) [thiscall 282] UnkSetFloatProp(word propId, float value)


*/



int GetSpecKeyNumber(int entindex)
{
	// Left screen side keys: 1, 2, 3, 4, 5
	// Right screen side keys: 6, 7, 8, 9, 0

	static bool (*fnPlayerSidesWappedOnScreen)(void) = (bool (*)(void))AFXADDR_GET(csgo_Unknown_GetTeamsSwappedOnScreen);

	bool swapPlayerSide = fnPlayerSidesWappedOnScreen && fnPlayerSidesWappedOnScreen();

	int slotCT = 0;
	int slotT = 0;

	WrpGlobals* gl = g_Hook_VClient_RenderView.GetGlobals();
	int imax = gl ? gl->maxclients_get() : 0;

	for (int i = 1; i <= imax; ++i)
	{
		SOURCESDK::IClientEntity_csgo* ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
		SOURCESDK::C_BaseEntity_csgo* be = ce ? ce->GetBaseEntity() : 0;
		if (be && be->IsPlayer())
		{
			int be_team = be->GetTeamNumber();

			if (3 == be_team) // CT
			{
				if (be->entindex() == entindex)
				{
					int slot = 1 + slotCT;
					if (swapPlayerSide) slot += 5;
					slot = slot % 10;
					return slot;
				}

				++slotCT;
			}
			else if (2 == be_team) // T
			{
				if (be->entindex() == entindex)
				{
					int slot = 1 + slotT;
					if (!swapPlayerSide) slot += 5;
					slot = slot % 10;
					return slot;
				}

				++slotT;
			}
		}
	}

	return -1;
}


extern WrpVEngineClient * g_VEngineClient;

advancedfx::Con_Printf_t conMessage = nullptr;
advancedfx::Con_Printf_t conWarning = nullptr;

MirvDeathMsg g_MirvDeathMsg;

typedef void csgo_C_CSPlayer_IUnk_t;
typedef int(__fastcall * csgo_C_CSPlayer_IClientNetworkable_entindex_t)(csgo_C_CSPlayer_IUnk_t * This, void * edx);
csgo_C_CSPlayer_IClientNetworkable_entindex_t Truecsgo_C_CSPlayer_IClientNetworkable_entindex = 0;


typedef void CCSGO_HudDeathNotice_t;
typedef void (__fastcall *CCSGO_HudDeathNotice_FireGameEvent_t)(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event);
CCSGO_HudDeathNotice_FireGameEvent_t TrueCCSGO_HudDeathNotice_FireGameEvent = 0;
CCSGO_HudDeathNotice_t * CCSGO_HudDeathNotice_FireGameEvent_This = 0;

typedef void(__fastcall *CCSGO_HudDeathNotice_UnkRemoveNotices_t)(CCSGO_HudDeathNotice_t * This, void * edx);
CCSGO_HudDeathNotice_UnkRemoveNotices_t TrueCCSGO_HudDeathNotice_UnkRemoveNotices = 0;

typedef void csgo_panorama_CUIPanel;
typedef void(__fastcall * csgo_panorama_CUIPanel_UnkSetFloatProp_t)(csgo_panorama_CUIPanel * This, void * edx, WORD propId, float value);
csgo_panorama_CUIPanel_UnkSetFloatProp_t Truecsgo_panorama_CUIPanel_UnkSetFloatProp = 0;

extern SOURCESDK::CSGO::IEngineTrace * g_pClientEngineTrace;

class CMyDeatMsgTraceFilter : public SOURCESDK::CSGO::CTraceFilter
{
public:
	CMyDeatMsgTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passentity);
	virtual bool ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask);

private:
	const SOURCESDK::CSGO::IHandleEntity *m_pPassEnt;
};

CMyDeatMsgTraceFilter::CMyDeatMsgTraceFilter(const SOURCESDK::CSGO::IHandleEntity *passedict)
{
	m_pPassEnt = passedict;
}

bool CMyDeatMsgTraceFilter::ShouldHitEntity(SOURCESDK::CSGO::IHandleEntity *pHandleEntity, int contentsMask)
{
	return pHandleEntity && pHandleEntity != m_pPassEnt;
}

int EntIndexFromTrace() {
	if (!(CClientToolsCsgo::Instance() && g_pClientEngineTrace))
	{
		Tier0_Warning("Not supported for your engine / missing hooks,!\n");
		return 0;
	}

	double forward[3], right[3], up[3];

	double Rx = (g_Hook_VClient_RenderView.LastCameraAngles[0]);
	double Ry = (g_Hook_VClient_RenderView.LastCameraAngles[1]);
	double Rz = (g_Hook_VClient_RenderView.LastCameraAngles[2]);

	MakeVectors(Rz, Rx, Ry, forward, right, up);

	double Tx = (g_Hook_VClient_RenderView.LastCameraOrigin[0] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[0] - 0 * right[0] + 0 * up[0]);
	double Ty = (g_Hook_VClient_RenderView.LastCameraOrigin[1] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[1] - 0 * right[1] + 0 * up[1]);
	double Tz = (g_Hook_VClient_RenderView.LastCameraOrigin[2] + SOURCESDK_CSGO_MAX_TRACE_LENGTH * forward[2] - 0 * right[2] + 0 * up[2]);

	SOURCESDK::Vector vecAbsStart;
	vecAbsStart.Init((float)g_Hook_VClient_RenderView.LastCameraOrigin[0], (float)g_Hook_VClient_RenderView.LastCameraOrigin[1], (float)g_Hook_VClient_RenderView.LastCameraOrigin[2]);

	SOURCESDK::Vector vecAbsEnd;
	vecAbsEnd.Init((float)Tx, (float)Ty, (float)Tz);

	SOURCESDK::CSGO::Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	SOURCESDK::C_BaseEntity_csgo * localPlayer = reinterpret_cast<SOURCESDK::C_BaseEntity_csgo *>(CClientToolsCsgo::Instance()->GetClientToolsInterface()->GetLocalPlayer());
	CMyDeatMsgTraceFilter traceFilter(localPlayer);

	SOURCESDK::CSGO::trace_t tr;

	g_pClientEngineTrace->TraceRay(ray, SOURCESDK_CSGO_MASK_ALL, &traceFilter, &tr);

	if (tr.DidHit() && tr.m_pEnt)
	{
		return tr.GetEntityIndex();
	}

	return 0;
}

void DeathMsgId::operator=(const char* consoleValue)
{
	if (!consoleValue)
		return;

	if (StringBeginsWith(consoleValue, "k"))
	{
		this->Mode = Id_Key;
		this->Id.specKey = atoi(consoleValue +1);
	}
	else if (StringBeginsWith(consoleValue, "x"))
	{
		unsigned long long val;
		
		if (0 == _stricmp("xTrace", consoleValue))
		{
			SOURCESDK::player_info_t_csgo pinfo;

			if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(EntIndexFromTrace(), &pinfo))
			{
				val = pinfo.xuid;
			}
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
			SOURCESDK::player_info_t_csgo pinfo;

			if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(EntIndexFromTrace(), &pinfo))
			{
				val = pinfo.userID;
			}
			else
				val = 0;
		}
		else
			val = atoi(consoleValue);

		this->operator=(val);
	}
};

int DeathMsgId::ResolveToUserId()
{
	switch(Mode)
	{
	case Id_Key:
		{
			SOURCESDK::player_info_t_csgo pinfo;
			SOURCESDK::IVEngineClient_014_csgo * vengineClient = g_VEngineClient->GetVEngineClient_csgo();
			int maxClients = g_VEngineClient->GetMaxClients();
			for (int i = 0; i < maxClients; ++i)
			{
				if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(i, &pinfo))
				{
					if(GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID)) == Id.specKey)
						return pinfo.userID;
				}
			}
		}
		return 0;
	case Id_Xuid:
		{
			SOURCESDK::player_info_t_csgo pinfo;
			SOURCESDK::IVEngineClient_014_csgo * vengineClient = g_VEngineClient->GetVEngineClient_csgo();
			int maxClients = g_VEngineClient->GetMaxClients();
			for (int i = 0; i < maxClients; ++i)
			{
				if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(i, &pinfo) && pinfo.xuid == Id.xuid)
				{
					return pinfo.userID;
				}
			}
		}			
		return 0;
	}
	
	return Id.userId;
};

bool DeathMsgId::EqualsUserId(int userId)
{
	if (Mode == Id_UserId) return userId == Id.userId;

	if (userId < 1)
		return false;

	switch(Mode)
	{
	case Id_Key:
		if (g_VEngineClient)
		{
			if (SOURCESDK::IVEngineClient_014_csgo * pEngine = g_VEngineClient->GetVEngineClient_csgo())
			{
				int entIndex = g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(userId);
				return GetSpecKeyNumber(entIndex) == Id.specKey;
			}
		}
		break;

	case Id_Xuid:
		if (g_VEngineClient)
		{
			if (SOURCESDK::IVEngineClient_014_csgo * pEngine = g_VEngineClient->GetVEngineClient_csgo())
			{
				SOURCESDK::player_info_t_csgo pInfo;
				return (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(userId), &pInfo) && pInfo.xuid == Id.xuid);
			}
		}
		break;
				
	}

	return false;
};

static std::list<DeathMsgFilterEntry> deathMessageFilter;

class MyDeathMsgGameEventWrapper : public SOURCESDK::CSGO::IGameEvent, public MyDeathMsgGameEventWrapperBase
{
public:
	MyDeathMsgGameEventWrapper(SOURCESDK::CSGO::IGameEvent * event)
		: m_Event(event)
	{

	}
public:
	virtual ~MyDeathMsgGameEventWrapper() {
	}
	
	virtual const char * GetName() const {
		return m_Event->GetName();
	}

	virtual bool IsReliable() const {
		return m_Event->IsReliable();
	}

	virtual bool IsLocal() const {
		return m_Event->IsLocal();
	}

	virtual bool IsEmpty(const char *keyName = NULL) {
		return m_Event->IsEmpty(keyName);
	}

	virtual bool GetBool(const char *keyName = NULL, bool defaultValue = false) {

		return m_Event->GetBool(keyName, defaultValue);
	}

	virtual int GetInt(const char *keyName = NULL, int defaultValue = 0) {

		if (attacker.newId.use && 0 == strcmp("attacker", keyName)) return attacker.newId.value.ResolveToUserId();
		if (victim.newId.use && 0 == strcmp("userid", keyName)) return victim.newId.value.ResolveToUserId();
		if (assister.newId.use && 0 == strcmp("assister", keyName)) return assister.newId.value.ResolveToUserId();
		if (assistedflash.use && 0 == strcmp("assistedflash", keyName)) return assistedflash.value;
		if (headshot.use && 0 == strcmp("headshot", keyName)) return headshot.value;
		if (penetrated.use && 0 == strcmp("penetrated", keyName)) return penetrated.value;
		if (dominated.use && 0 == strcmp("dominated", keyName)) return dominated.value;
		if (revenge.use && 0 == strcmp("revenge", keyName)) return revenge.value;
		if (wipe.use && 0 == strcmp("wipe", keyName)) return wipe.value;
		if (noscope.use && 0 == strcmp("noscope", keyName)) return noscope.value;
		if (thrusmoke.use && 0 == strcmp("thrusmoke", keyName)) return thrusmoke.value;
		if (attackerblind.use && 0 == strcmp("attackerblind", keyName)) return attackerblind.value;

		return m_Event->GetInt(keyName, defaultValue);
	}

	virtual SOURCESDK::uint64 GetUint64(const char *keyName = NULL, SOURCESDK::uint64 defaultValue = 0) {
		return m_Event->GetUint64(keyName, defaultValue);
	}
	
	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) {
		return m_Event->GetFloat(keyName, defaultValue);
	}

	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") {

		if (weapon.use && 0 == strcmp("weapon", keyName)) return weapon.value;

		return m_Event->GetString(keyName, defaultValue);
	}

	virtual const wchar_t *GetWString(char const *keyName = NULL, const wchar_t *defaultValue = L"") {
		return m_Event->GetWString(keyName, defaultValue);
	}
	virtual const void *GetPtr( const char *keyname = NULL, const void *defaultValues = NULL ) {
		return m_Event->GetPtr(keyname, defaultValues);
	}

	virtual void SetBool(const char *keyName, bool value) {
		m_Event->SetBool(keyName, value);
	}

	virtual void SetInt(const char *keyName, int value) {
		m_Event->SetInt(keyName, value);
	}

	virtual void SetUint64(const char *keyName, SOURCESDK::uint64 value) {
		m_Event->SetUint64(keyName, value);
	}

	virtual void SetFloat(const char *keyName, float value) {
		m_Event->SetFloat(keyName, value);
	}

	virtual void SetString(const char *keyName, const char *value) {
		m_Event->SetString(keyName, value);
	}

	virtual void SetWString( const char *keyName, const wchar_t *value ) {
		m_Event->SetWString(keyName, value);
	}
	
	virtual void SetPtr( const char *keyname, const void *value ) {
		m_Event->SetPtr(keyname, value);
	}

private:
	SOURCESDK::CSGO::IGameEvent * m_Event;
};

struct CHudDeathNoticeHookGlobals : MirvDeathMsgGlobals {

	MyDeathMsgGameEventWrapper * activeWrapper = 0;

	int entindex_callnum;
	int panorama_CUIPanel_UnkSetFloatProp_callnum;

	std::queue<int> nextRealCUnknownGetPlayerNameState;
	std::queue<int> nextRealEntindexState;

} g_HudDeathNoticeHookGlobals;

void __fastcall MyCCSGO_HudDeathNotice_FireGameEvent(CCSGO_HudDeathNotice_t * This, void * edx, SOURCESDK::CSGO::IGameEvent * event)
{
	CCSGO_HudDeathNotice_FireGameEvent_This = This;

	MyDeathMsgGameEventWrapper myWrapper(event); // TODO: Consider to re-use this instead of allocating every time on stack.

	g_HudDeathNoticeHookGlobals.entindex_callnum = 0;
	g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum = 0;
	g_HudDeathNoticeHookGlobals.activeWrapper = &myWrapper;

	if (g_HudDeathNoticeHookGlobals.Lifetime.use)
	{
		myWrapper.lifetime.use = true;
		myWrapper.lifetime.value = g_HudDeathNoticeHookGlobals.Lifetime.value;
	}

	if (g_HudDeathNoticeHookGlobals.LifetimeMod.use)
	{
		myWrapper.lifetimeMod.use = true;
		myWrapper.lifetimeMod.value = g_HudDeathNoticeHookGlobals.LifetimeMod.value;
	}

	int uidAttacker = event->GetInt("attacker");
	int uidVictim = event->GetInt("userid");
	int uidAssister = event->GetInt("assister");

	if(0 < g_HudDeathNoticeHookGlobals.Settings.Debug)
	{
		Tier0_Msg("CHudDeathNotice::FireGameEvent: uidAttacker=%i, uidVictim=%i, uidAssister=%i weapon=\"%s\"\n", uidAttacker, uidVictim, uidAssister, event->GetString("weapon"));
	}

	for(std::list<DeathMsgFilterEntry>::iterator it = g_HudDeathNoticeHookGlobals.Filter.begin(); it != g_HudDeathNoticeHookGlobals.Filter.end(); it++)
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

			uidAttacker = myWrapper.GetInt("attacker", 0);
			uidVictim = myWrapper.GetInt("userid", 0);
			uidAssister = myWrapper.GetInt("assister", 0);

			if (e.lastRule) break;
		}
	}

	if (g_HudDeathNoticeHookGlobals.useHighlightId)
	{

		myWrapper.attacker.isLocal.use = true;
		myWrapper.attacker.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidAttacker);

		myWrapper.victim.isLocal.use = true;
		myWrapper.victim.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidVictim);

		myWrapper.assister.isLocal.use = true;
		myWrapper.assister.isLocal.value = g_HudDeathNoticeHookGlobals.highlightId.EqualsUserId(uidAssister);
	}

	if (!(myWrapper.block.use && myWrapper.block.value) ) {

		while (!g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.pop();
		}

		if (uidAttacker) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(1);
		if (uidVictim) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(2);
		if (uidAssister) g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.push(3);

		while(!g_HudDeathNoticeHookGlobals.nextRealEntindexState.empty())
		{
			Assert(0);
			g_HudDeathNoticeHookGlobals.nextRealEntindexState.pop();
		}

		if (uidAttacker) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(2);
		if (uidVictim) g_HudDeathNoticeHookGlobals.nextRealEntindexState.push(1);

		TrueCCSGO_HudDeathNotice_FireGameEvent(This, edx, &myWrapper);
	}

	g_HudDeathNoticeHookGlobals.activeWrapper = 0;
}

class MyDeathMsgGameEventFallback : public SOURCESDK::CSGO::IGameEvent
{
public:
	MyDeathMsgGameEventFallback() {
	}

public:
	virtual ~MyDeathMsgGameEventFallback() {
	}

	virtual const char * GetName() const {
		return "player_death";
	}

	virtual bool IsReliable() const {
		return true;
	}

	virtual bool IsLocal() const {
		return true;
	}

	virtual bool IsEmpty(const char *keyName = NULL) {
		return true;
	}

	virtual bool GetBool(const char *keyName = NULL, bool defaultValue = false) {
		if (0 == strcmp("realtime_passthrough", keyName)) return true;

		return defaultValue;
	}

	virtual int GetInt(const char *keyName = NULL, int defaultValue = 0) {
		return defaultValue;
	}

	virtual SOURCESDK::uint64 GetUint64(const char *keyName = NULL, SOURCESDK::uint64 defaultValue = 0) {
		return defaultValue;
	}

	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) {
		return defaultValue;
	}

	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") {
		return defaultValue;
	}

	virtual const wchar_t *GetWString(char const *keyName = NULL, const wchar_t *defaultValue = L"") {
		return defaultValue;
	}
	virtual const void *GetPtr( const char *keyname = NULL, const void *defaultValues = NULL ) {
		return defaultValues;
	}

	virtual void SetBool(const char *keyName, bool value) {
		return;
	}

	virtual void SetInt(const char *keyName, int value) {
		return;
	}

	virtual void SetUint64(const char *keyName, SOURCESDK::uint64 value) {
		return;
	}

	virtual void SetFloat(const char *keyName, float value) {
		return;
	}

	virtual void SetString(const char *keyName, const char *value) {
		return;
	}

	virtual void SetWString( const char *keyName, const wchar_t *value ) {
		return;
	}

	virtual void SetPtr( const char *keyname, const void *value ) {
		return;
	}

private:

};

struct CCsgoReplaceNameEntry
{
	DeathMsgId id;
	std::string name;
	int mode;

	CCsgoReplaceNameEntry(const char * id, const char * name, int mode)
	{
		this->id = id;
		this->name = name;
		this->mode = mode;
	}
};

struct CCsgoREplaceTeamNameEntry {
	bool Use_ClanName = false;
	std::string ClanName;
	bool Use_FlagImageString = false;
	std::string FlagImageString;
	bool Use_LogoImageString = false;
	std::string LogoImageString;
};

bool g_csgo_ReplaceTeamNameDebug = false;
std::map<int,CCsgoREplaceTeamNameEntry> g_csgo_ReplaceTeamNameList;

bool g_csgo_ReplacePlayerNameDebug = false;
std::list<CCsgoReplaceNameEntry> g_csgo_ReplacePlayerNameList;

// This function has a bug: entindex != entnum (returned by GetPlayerForUserID), but it's fine for now here.

int __fastcall Mycsgo_C_CSPlayer_IClientNetworkable_entindex(csgo_C_CSPlayer_IUnk_t * This, void * edx) {

	if (g_HudDeathNoticeHookGlobals.activeWrapper)
	{
		if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealEntindexState.empty())
		{
			int state = g_HudDeathNoticeHookGlobals.nextRealEntindexState.front();
			g_HudDeathNoticeHookGlobals.nextRealEntindexState.pop();

			switch (state)
			{
			case 1:
				if (g_HudDeathNoticeHookGlobals.activeWrapper->attacker.isLocal.use)
					return g_HudDeathNoticeHookGlobals.activeWrapper->attacker.isLocal.value ? g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("attacker", 0)) : 0;
				break;
			case 2:
				if (g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.use)
					return g_HudDeathNoticeHookGlobals.activeWrapper->victim.isLocal.value ? g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(g_HudDeathNoticeHookGlobals.activeWrapper->GetInt("userid", 0)) : 0;
				break;
			}
		}
	}

	return Truecsgo_C_CSPlayer_IClientNetworkable_entindex(This, edx);
}

typedef char *(__fastcall *csgo_C_Team_Get_ClanName_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );
typedef char *(__fastcall *csgo_C_Team_Get_FlagImageString_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );
typedef char *(__fastcall *csgo_C_Team_Get_LogoImageString_t)( SOURCESDK::C_BaseEntity_csgo * This, void * Edx );

csgo_C_Team_Get_ClanName_t g_Old_csgo_C_Team_Get_ClanName = nullptr;
csgo_C_Team_Get_FlagImageString_t g_Old_csgo_C_Team_Get_FlagImageString = nullptr;
csgo_C_Team_Get_LogoImageString_t g_Old_csgo_C_Team_Get_LogoImageString = nullptr;

char * __fastcall My_csgo_C_Team_Get_ClanName( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_ClanName( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_ClanName: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_ClanName)
	{
		result = const_cast<char *>(it->second.ClanName.c_str());
	}

	return result;
}

char * __fastcall My_csgo_C_Team_Get_FlagImageString( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_FlagImageString( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_FlagImageString: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_FlagImageString)
	{
		result = const_cast<char *>(it->second.FlagImageString.c_str());
	}

	return result;
}

char * __fastcall My_csgo_C_Team_Get_LogoImageString( SOURCESDK::C_BaseEntity_csgo * This, void * Edx ) {
	char * result = g_Old_csgo_C_Team_Get_LogoImageString( This, Edx );

	int teamNumber = This->GetTeamNumber();

	if(g_csgo_ReplaceTeamNameDebug)
	{
		Tier0_Msg("C_Team::Get_LogoImageString: %i -> %s\n", teamNumber, result);
	}

	auto it = g_csgo_ReplaceTeamNameList.find(teamNumber);
	if(it != g_csgo_ReplaceTeamNameList.end() && it->second.Use_LogoImageString)
	{
		result = const_cast<char *>(it->second.LogoImageString.c_str());
	}

	return result;
}


bool Hookcsgo_Team_GetFunctions(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		AFXADDR_GET(csgo_C_Team_vtable)
		&& AFXADDR_GET(csgo_client_C_Team_Get_ClanName_vtable_index)
		&& AFXADDR_GET(csgo_client_C_Team_Get_FlagImageString_vtable_index)
		&& AFXADDR_GET(csgo_client_C_Team_Get_LogoImageString_vtable_index)
	)
	{
		LONG error = NO_ERROR;


		void **vtable = (void **)AFXADDR_GET(csgo_C_Team_vtable);
		
		g_Old_csgo_C_Team_Get_ClanName = (csgo_C_Team_Get_ClanName_t)vtable[AFXADDR_GET(csgo_client_C_Team_Get_ClanName_vtable_index)];
		g_Old_csgo_C_Team_Get_FlagImageString = (csgo_C_Team_Get_FlagImageString_t)vtable[AFXADDR_GET(csgo_client_C_Team_Get_FlagImageString_vtable_index)];
		g_Old_csgo_C_Team_Get_LogoImageString = (csgo_C_Team_Get_LogoImageString_t)vtable[AFXADDR_GET(csgo_client_C_Team_Get_LogoImageString_vtable_index)];
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_ClanName, My_csgo_C_Team_Get_ClanName);
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_FlagImageString, My_csgo_C_Team_Get_FlagImageString);
		DetourAttach(&(PVOID&)g_Old_csgo_C_Team_Get_LogoImageString, My_csgo_C_Team_Get_LogoImageString);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

typedef const char *(__fastcall *csgo_CPlayerResource_GetPlayerName_t)( void* This, void* Edx, int index );

csgo_CPlayerResource_GetPlayerName_t g_Touring_csgo_CPlayerResource_GetPlayerName = nullptr;

const char * __fastcall My_csgo_CPlayerResource_GetPlayerName( void* This, void* Edx, int index ) {
	const char * result = g_Touring_csgo_CPlayerResource_GetPlayerName(This, Edx, index);

	SOURCESDK::player_info_t_csgo pinfo;

	if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(index, &pinfo))
	{
		if (g_csgo_ReplacePlayerNameDebug) {

			Tier0_Msg("CPlayerResource::GetPlayerName: %i -> %s\n", pinfo.userID, result);
		}

		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if(!(e.mode & (1 << 0))) continue;

			if (
				(e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo.userID)
				|| (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo.xuid)
				|| e.id.Mode == DeathMsgId::Id_Key && e.id.Id.specKey == GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID))
			) {
				result = it->name.c_str();
				break;
			}
		}
	}

	return result;
}

bool Hookcsgo_CPlayerResource_GetPlayerName(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		g_VEngineClient->GetVEngineClient_csgo()
		&& AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable)
		&& AFXADDR_GET(csgo_client_CPlayerResource_GetPlayerName_vtable_index)
	)
	{
		LONG error = NO_ERROR;


		void **vtable = (void **)AFXADDR_GET(csgo_C_CS_PlayerResource_IGameResources_vtable);

		g_Touring_csgo_CPlayerResource_GetPlayerName = (csgo_CPlayerResource_GetPlayerName_t)vtable[AFXADDR_GET(csgo_client_CPlayerResource_GetPlayerName_vtable_index)];
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Touring_csgo_CPlayerResource_GetPlayerName, My_csgo_CPlayerResource_GetPlayerName);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

typedef wchar_t * (__fastcall *csgo_CUnknown_GetPlayerName_t)(void * This, void * Edx, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1);

csgo_CUnknown_GetPlayerName_t g_Old_csgo_CUnknown_GetPlayerName;

wchar_t * __fastcall My_csgo_CUnknown_GetPlayerName(void * This, void * Edx, int entIndex, wchar_t * targetBuffer, DWORD targetByteCount, DWORD unknownArg1)
{
	wchar_t * result = g_Old_csgo_CUnknown_GetPlayerName(This, Edx, entIndex, targetBuffer, targetByteCount, unknownArg1);

	SOURCESDK::player_info_t_csgo pinfo;
	MyDeathMsgPlayerEntry * playerEntry = nullptr;

	if (g_HudDeathNoticeHookGlobals.activeWrapper && !g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.empty())
	{
		int state = g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.front();
		g_HudDeathNoticeHookGlobals.nextRealCUnknownGetPlayerNameState.pop();

		switch (state)
		{
		case 1:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->attacker);
			break;
		case 2:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->victim);
			break;
		case 3:
			playerEntry = &(g_HudDeathNoticeHookGlobals.activeWrapper->assister);
			break;
		}
	}

	if (g_VEngineClient->GetVEngineClient_csgo()->GetPlayerInfo(entIndex, &pinfo))
	{
		if (g_csgo_ReplacePlayerNameDebug) {

			std::string utf8PlayerName;

			if (WideStringToUTF8String(result, utf8PlayerName))
				Tier0_Msg("CUnknown::GetPlayerName: %i -> %s\n", pinfo.userID, utf8PlayerName.c_str());
			else
				Tier0_Msg("CUnknown::GetPlayerName: %i ERROR: could not get UTF8-String for player name.\n", pinfo.userID);
		}

		for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it) {
			CCsgoReplaceNameEntry & e = *it;

			if(!(e.mode & (1 << 1))) continue;

			if (
				(e.id.Mode == DeathMsgId::Id_UserId && e.id.Id.userId == pinfo.userID)
				|| (e.id.Mode == DeathMsgId::Id_Xuid && e.id.Id.xuid == pinfo.xuid)
				|| e.id.Mode == DeathMsgId::Id_Key && e.id.Id.specKey == GetSpecKeyNumber(g_VEngineClient->GetVEngineClient_csgo()->GetPlayerForUserID(pinfo.userID))
			) {
				std::wstring widePlayerName;
				if (UTF8StringToWideString(it->name.c_str(), widePlayerName))
				{
					wcscpy_s(targetBuffer, targetByteCount / sizeof(wchar_t), widePlayerName.c_str());
				}
				break;
			}
		}

		if (playerEntry && playerEntry->name.use) {
			std::wstring widePlayerName;
			if (UTF8StringToWideString(playerEntry->name.value, widePlayerName))
			{
				wcscpy_s(targetBuffer, targetByteCount / sizeof(wchar_t), widePlayerName.c_str());
			}
		}

		if (g_HudDeathNoticeHookGlobals.activeWrapper)
		{
			switch (g_HudDeathNoticeHookGlobals.showNumbers)
			{
				case MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::Numbers:
				case MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::PrependNumbers:
				{
					wchar_t number[33];
					if (0 == _itow_s(GetSpecKeyNumber(entIndex), number, 10))
					{
						size_t len = wcslen(number);
						size_t oldLen = wcslen(targetBuffer);
						if (g_HudDeathNoticeHookGlobals.showNumbers == MirvDeathMsgGlobals::DeathnoticeShowNumbers_e::PrependNumbers)
						{
							size_t pos = len + 1 + oldLen;
							if (pos < targetByteCount)
							{
								for(;len <= pos; --pos)
								{
									targetBuffer[pos] = targetBuffer[pos - len - 1];
								}
								targetBuffer[len] = L' ';
								for (pos = 0; pos < len; ++pos)
								{
									targetBuffer[pos] = number[pos];
								}
							}
						}
						else
						{
							len = len + 1;
							if (len <= targetByteCount / sizeof(wchar_t))
							{
								wcscpy_s(targetBuffer, len, number);
							}
						}
					}
				}
				break;
			}
		}
	}

	return result;
}


bool Hook_csgo_CUnknown_GetPlayerName(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		g_VEngineClient->GetVEngineClient_csgo()
		&& AFXADDR_GET(csgo_CUnknown_GetPlayerName)
	)
	{
		LONG error = NO_ERROR;

		g_Old_csgo_CUnknown_GetPlayerName = (csgo_CUnknown_GetPlayerName_t)AFXADDR_GET(csgo_CUnknown_GetPlayerName);
		
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_Old_csgo_CUnknown_GetPlayerName, My_csgo_CUnknown_GetPlayerName);
		error = DetourTransactionCommit();
		
		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

void __fastcall Mycsgo_panorama_CUIPanel_UnkSetFloatProp(csgo_panorama_CUIPanel * This, void * edx, WORD propId, float value) {

	if (g_HudDeathNoticeHookGlobals.activeWrapper)
	{
		switch (g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum)
		{
		case 0:
			// SpawnTime.
			break;
		case 1:
			// Lifetime.
			if (g_HudDeathNoticeHookGlobals.activeWrapper->lifetime.use) value = g_HudDeathNoticeHookGlobals.activeWrapper->lifetime.value;
			break;
		case 2:
			// LifetimeMod.
			if (g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.use
				&& value != 1.0f // multiplier is used
			) value = g_HudDeathNoticeHookGlobals.activeWrapper->lifetimeMod.value;
			break;
		}

		++g_HudDeathNoticeHookGlobals.panorama_CUIPanel_UnkSetFloatProp_callnum;
	}

	Truecsgo_panorama_CUIPanel_UnkSetFloatProp(This, edx, propId, value);
}


bool csgo_CHudDeathNotice_Install_Panorama(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if(!firstRun) return firstResult;
	firstRun = false;
	
	if(
		Hook_csgo_CUnknown_GetPlayerName()
		&& AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent)
		&& AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp)
		&& AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex)
	)
	{
		LONG error = NO_ERROR;

		TrueCCSGO_HudDeathNotice_FireGameEvent = (CCSGO_HudDeathNotice_FireGameEvent_t)AFXADDR_GET(csgo_CCSGO_HudDeathNotice_FireGameEvent);
		Truecsgo_panorama_CUIPanel_UnkSetFloatProp = (csgo_panorama_CUIPanel_UnkSetFloatProp_t)AFXADDR_GET(csgo_panorama_AVCUIPanel_UnkSetFloatProp);
		Truecsgo_C_CSPlayer_IClientNetworkable_entindex = (csgo_C_CSPlayer_IClientNetworkable_entindex_t)AFXADDR_GET(csgo_C_CSPlayer_IClientNetworkable_entindex);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueCCSGO_HudDeathNotice_FireGameEvent, MyCCSGO_HudDeathNotice_FireGameEvent);
		DetourAttach(&(PVOID&)Truecsgo_panorama_CUIPanel_UnkSetFloatProp, Mycsgo_panorama_CUIPanel_UnkSetFloatProp);
		DetourAttach(&(PVOID&)Truecsgo_C_CSPlayer_IClientNetworkable_entindex, Mycsgo_C_CSPlayer_IClientNetworkable_entindex);
		error = DetourTransactionCommit();

		firstResult = NO_ERROR == error;
	}

	return firstResult;
}

bool csgo_CHudDeathNotice_Install(void)
{
	return csgo_CHudDeathNotice_Install_Panorama();
}

void Console_DeathMsgId_PrintHelp(const char * cmd)
{
	Tier0_Msg(
		"%s accepts the following as <id...>:\n"
		"<iNumber> - UserID. Example: 2\n"
		"trace - UserID from a screen trace (e.g. current POV).\n"
		"x<iNumber> - XUID. Example: x76561197961927915\n"
		"xTrace - XUID from a screen trace (e.g. current POV).\n"
		"k<iNumber> - Spectator key number.\n"
		"We recommend getting the numbers from the output of \"mirv_listentities isPlayer=1\".\n"
		, cmd
	);
}

bool csgo_CHudDeathNotice_Console(IWrpCommandArgs * args)
{
	if (!csgo_CHudDeathNotice_Install())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	if (nullptr == conMessage || nullptr == conWarning)
	{
		conMessage = Tier0_Msg;
		conWarning = Tier0_Warning;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("filter", arg1))
		{
			return g_MirvDeathMsg.filter(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("fake", arg1)) {
			if (!CCSGO_HudDeathNotice_FireGameEvent_This)
			{
				Tier0_Warning("One message must have been triggered first, before this will work.\n");
				return true;
			}

			if (3 <= argc)
			{
				CSubWrpCommandArgs subArgs(args, 2);
				MyDeathMsgGameEventFallback fallBack;

				g_HudDeathNoticeHookGlobals.Filter.emplace_front(&subArgs);

				MyCCSGO_HudDeathNotice_FireGameEvent(CCSGO_HudDeathNotice_FireGameEvent_This, 0, &fallBack);

				g_HudDeathNoticeHookGlobals.Filter.erase(g_HudDeathNoticeHookGlobals.Filter.begin());

				return true;
			}

			std::string prefix(arg0);
			prefix += " fake";
			Console_DeathMsgArgs_PrintHelp(prefix.c_str(), false);
			return true;
		}
		else if (0 == _stricmp("help", arg1)) {
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("id", arg2)) {
					Console_DeathMsgId_PrintHelp(arg0);
					return true;
				}
			}
		}
		else if (0 == _stricmp("lifetime", arg1)) {
			return g_MirvDeathMsg.lifetime(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("lifetimeMod", arg1)) {
			return g_MirvDeathMsg.lifetimeMod(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("localPlayer", arg1)) {
			return g_MirvDeathMsg.localPlayer(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("showNumbers", arg1)) {
			return g_MirvDeathMsg.showNumbers(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("debug", arg1))
		{
			return g_MirvDeathMsg.debug(args, g_HudDeathNoticeHookGlobals);
		}
		else if (0 == _stricmp("deprecated", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			Tier0_Msg(
					"Usage:\n"
					"mirv_deathmsg block [...] - Block specific death messages.\n"
					"mirv_deathmsg cfg [...] - Configure death message properties, e.g. noticeLifeTime.\n"
					"mirv_deathmsg highLightId [...] - Control highlighting.\n"
					"mirv_deathmsg help id - Display help on the ids that can be used.\n"
					"mirv_deathmsg debug [...] - Enable debug message in console.\n"
			);

			return true;
		}
		else if(0 == _stricmp("block", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			if(4 <= argc)
			{
				char const * uidAttacker = args->ArgV(2);
				char const * uidVictim = args->ArgV(3);
				char const * uidAssister = 5 <= argc ? args->ArgV(4) : "*";

				CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
				fakeArgs.AddArg("filter");
				fakeArgs.AddArg("add");

				std::string arg;				
				
				arg = "attackerMatch=";
				arg += uidAttacker;
				fakeArgs.AddArg(arg.c_str());

				arg = "victimMatch=";
				arg += uidVictim;
				fakeArgs.AddArg(arg.c_str());

				arg = "assisterMatch=";
				arg += uidAssister;
				fakeArgs.AddArg(arg.c_str());

				fakeArgs.AddArg("block=1");

				csgo_CHudDeathNotice_Console(&fakeArgs);

				return true;
			}
			else
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("list", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("filter");
					fakeArgs.AddArg("print");
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				else
				if(0 == _stricmp("clear", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("filter");
					fakeArgs.AddArg("clear");
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg block <idAttacker> <idVictim> - block these ids\n"
				"\tRemarks: * to match any id, use !x to match any id apart from x.\n"
				"mirv_deathmsg block <idAttacker> <idVictim> <idAssister> - block these ids\n"
				"\tRemarks: * to match any id, use !x to match any id apart from x.\n"
				"mirv_deathmsg block list - list current blocks\n"
				"mirv_deathmsg block clear - clear current blocks\n"
				"(Use mirv_deathmsg help id to get help on the ids.)\n"
			);
			return true;
		}
		else
		if(0 == _stricmp("modTime", arg1))
		{
			Tier0_Warning("This command has been removed.\n");

			return true;
		}
		else
		if(0 == _stricmp("highLightId", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			int argc = args->ArgC();
			const char * arg0 = args->ArgV(0);

			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (atoi(arg2) < 0)
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = false;
				}
				else
				{
					g_HudDeathNoticeHookGlobals.useHighlightId = true;
					g_HudDeathNoticeHookGlobals.highlightId = arg2;
				}

				return true;
			}

			Tier0_Msg(
				"Usage:\n"
				"%s -1|0|<id> - -1 is default behaviour, 0 is never highlight, otherwise <id> is the ID (you can get it from mirv_deathmsg debug) of the player you want to highlight.\n"
				"Current setting: ",
				arg0
			);
			if (!g_HudDeathNoticeHookGlobals.useHighlightId) Tier0_Msg("-1");
			else g_HudDeathNoticeHookGlobals.highlightId.Console_Print();
			Tier0_Msg(
				"\n"
			);
			return true;
		}
		else
		if(0 == _stricmp("highLightAssists", arg1))
		{
			Tier0_Warning("This command has been removed.\n");

			return true;
		}
		else
		if(0 == _stricmp("cfg", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			if(4 <= argc)
			{
				char const * arg2 = args->ArgV(2);
				float arg3 = (float)atof(args->ArgV(3));

				if(0 == _stricmp("noticeLifeTime", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("lifetime");
					fakeArgs.AddArg(arg3 < 0 ? "default" : args->ArgV(3));
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
				else
				if(0 == _stricmp("localPlayerLifeTimeMod", arg2))
				{
					CFakeWrpCommandArgs fakeArgs("mirv_deathmsg");
					fakeArgs.AddArg("lifetimeMod");
					fakeArgs.AddArg(arg3 < 0 ? "default" : args->ArgV(3));
					csgo_CHudDeathNotice_Console(&fakeArgs);

					return true;
				}
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg cfg noticeLifeTime f - This is what you want. Current: %f\n"
				"mirv_deathmsg cfg localPlayerLifeTimeMod f - Current: %f\n"
				"Where f is a floating point value in seconds. Use -1 (a negative value) to use the orginal value instead.\n",
				g_HudDeathNoticeHookGlobals.Lifetime.use ? g_HudDeathNoticeHookGlobals.Lifetime.value : -1,
				g_HudDeathNoticeHookGlobals.LifetimeMod.use ? g_HudDeathNoticeHookGlobals.LifetimeMod.value : -1
			);
			return true;
		}
	}


	Tier0_Msg(
		"%s filter [...] - Filter death messages.\n"
		"%s fake [...] - Fake a death message (needs one message being triggered earlier).\n"
		"%s help id [...] - Print help on <id...> usage.\n"
		"%s lifetime [...] - Controls lifetime of death messages.\n"
		"%s lifetimeMod [...] - Controls lifetime modifier of death messages for the \"local\" player.\n"
		"%s localPlayer [...] - Controls what is considered \"local\" player (and thus highlighted in death notices).\n"
		"%s showNumbers [...] - Controls if and in what style to show numbers instead of player names.\n"
		"%s debug [...] - Enable / Disable debug spew upon death messages.\n"
		"%s deprecated - Print deprecated commands.\n"
		"Hint: Rewind the demo (to the round start for example) to clear death messages.\n"
		, arg0
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

}

bool csgo_ReplacePlayerName_Console(IWrpCommandArgs * args)
{
	if (!Hook_csgo_CUnknown_GetPlayerName() || !Hookcsgo_CPlayerResource_GetPlayerName())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	int argc = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if (0 == _stricmp("filter", arg1))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("add", arg2)) {

					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						g_csgo_ReplacePlayerNameList.emplace_back(arg3, arg4, 6 <= argc ? atoi(args->ArgV(5)) : 3);
						return true;
					}

					Tier0_Msg(
						"%s filter add <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> <sNewName> [<iMode>] - <iMode>: 1 name only, 2 decorated, 3 both (default)\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("edit", arg2))
				{
					if (4 <= argc)
					{
						const char* arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::advance(sourceIt, listNr);

						if (5 <= argc)
						{
							const char* arg4 = args->ArgV(4);

							if (0 == _stricmp("id", arg4))
							{
								if (6 <= argc)
								{
									sourceIt->id = args->ArgV(5);
									return true;
								}

								Tier0_Msg(
									"%s filter edit %i id <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> - Set new ID.\n"
									"Current value: "
									, arg0, listNr
								);
								sourceIt->id.Console_Print();
								Tier0_Msg("\n");

								return true;
							}
							else if (0 == _stricmp("name", arg4))
							{
								if (6 <= argc)
								{
									sourceIt->name = args->ArgV(5);
									return true;
								}

								Tier0_Msg(
									"%s filter edit %i name <sName> - Set new name.\n"
									"Current value: %s\n"
									, arg0, listNr
									, sourceIt->name.c_str()
								);

								return true;
							}
						}

						Tier0_Msg(
							"%s filter edit %i id [...]\n"
							"%s filter edit %i name [...]\n"
							, arg0, listNr
							, arg0, listNr
						);
						return true;
					}

					Tier0_Msg(
						"%s filter edit <listNr> [...] - Edit entry.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("move", arg2))
				{
					if (5 <= argc)
					{
						const char * arg3 = args->ArgV(3);
						const char * arg4 = args->ArgV(4);

						int listNr = atoi(arg3);
						int targetNr = atoi(arg4);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}

						if (targetNr < 0 || targetNr > (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <tragetNr>\n", targetNr);
							return true;
						}

						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::list<CCsgoReplaceNameEntry>::iterator targetIt = g_csgo_ReplacePlayerNameList.begin();

						if (listNr <= targetNr)
						{
							std::advance(sourceIt, listNr);

							targetIt = sourceIt;

							std::advance(targetIt, targetNr -listNr);
						}
						else
						{
							std::advance(targetIt, targetNr);

							sourceIt = targetIt;

							std::advance(sourceIt, listNr - targetNr);
						}

						g_csgo_ReplacePlayerNameList.splice(targetIt, g_csgo_ReplacePlayerNameList, sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter move <listNr> <targetListNr> - Move entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("remove", arg2)) {

					if (4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						int listNr = atoi(arg3);

						if (listNr < 0 || listNr >= (int)g_csgo_ReplacePlayerNameList.size())
						{
							Tier0_Warning("Error: %i is not in valid range for <listNr>\n", listNr);
							return true;
						}
						std::list<CCsgoReplaceNameEntry>::iterator sourceIt = g_csgo_ReplacePlayerNameList.begin();

						std::advance(sourceIt, listNr);

						g_csgo_ReplacePlayerNameList.erase(sourceIt);

						return true;
					}

					Tier0_Msg(
						"%s filter remove <listNr> - Remove entry in list.\n"
						, arg0
					);
					return true;
				}
				else if (0 == _stricmp("removeMatching", arg2))
				{
					if (4 <= argc)
					{
						CSubWrpCommandArgs subArgs(args, 3);
						DeathMsgId id;
						id = args->ArgV(3);

						int userId = id.ResolveToUserId();
						if(userId)
						{
							for(auto it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); )
							{
								if(it->id.EqualsUserId(userId))
								{
									auto itDelete = it;
									++it;
									g_csgo_ReplacePlayerNameList.erase(itDelete);
								}
								else ++it;
							}
						}

						return true;
					}

					Tier0_Msg(
						"%s filter removeMatching <idUser>|trace|x<XUID>|xTrace|k<spectatorKey> - Remove entries that _currently_ match.\n"
						, arg0
					);
					return true;
				}				
				else if (0 == _stricmp("clear", arg2)) {

					g_csgo_ReplacePlayerNameList.clear();
				
					return true;
				}			
				else if (0 == _stricmp("print", arg2)) {

					Tier0_Msg("nr: id, name\n");

					int nr = 0;

					for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it)
					{
						Tier0_Msg(
							"%i: "
							, nr
						);

						it->id.Console_Print();

						Tier0_Msg(", \"%s\"\n", it->name.c_str());

						++nr;
					}
					Tier0_Msg("---- EOL ----\n");

					return true;
				}					
			}

			Tier0_Msg(
				"%s filter add [...] - Add an entry.\n"
				"%s filter edit [...] - Edit an entry.\n"
				"%s filter move [...] - Move an entry.\n"
				"%s filter remove [...] - Remove an entry.\n"
				"%s filter removeMatching [...] - Remove matching entries.\n"
				"%s filter clear - Clear filter list.\n"
				"%s filter print - Print current list entries.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("help", arg1)) {
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp("id", arg2)) {
					Console_DeathMsgId_PrintHelp(arg0);
					return true;
				}
			}
		}
		else if (0 == _stricmp("deprecated", arg1))
		{
			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			Tier0_Msg(
				"Usage:\n"
				"%s <playerId> <name> - replace <playerId> with given <name>.\n"
				"%s debug 0|1 - print <playerId> -> <name> pairs into console as they get queried by the game.\n"
				"%s delete <playerId> - delete replacement for <playerId>.\n"
				"%s list - list all <playerId> -> <name> replacements currently active.\n"
				"%s clear - clear all replacements.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return true;
		}
		else if (0 == _stricmp("list", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			CFakeWrpCommandArgs fakeArgs(args->ArgV(0));
			fakeArgs.AddArg("filter");
			fakeArgs.AddArg("print");

			csgo_ReplacePlayerName_Console(&fakeArgs);

			return true;
		}
		else if (0 == _stricmp("clear", arg1)) {

			//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

			CFakeWrpCommandArgs fakeArgs(args->ArgV(0));
			fakeArgs.AddArg("filter");
			fakeArgs.AddArg("clear");

			csgo_ReplacePlayerName_Console(&fakeArgs);

			return true;
		}
		else if (3 <= argc)
		{
			const char * arg2 = args->ArgV(2);

			if (0 == _stricmp("debug", arg1))
			{
				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				g_csgo_ReplacePlayerNameDebug = 0 != atoi(arg2);
				return true;
			}
			else if (0 == _stricmp("delete", arg1)) {

				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				int id = atoi(arg2);

				for (std::list<CCsgoReplaceNameEntry>::iterator it = g_csgo_ReplacePlayerNameList.begin(); it != g_csgo_ReplacePlayerNameList.end(); ++it)
				{
					CCsgoReplaceNameEntry & entry = *it;

					if(id == entry.id.ResolveToUserId())
					{
						it = g_csgo_ReplacePlayerNameList.erase(it);
						if (it == g_csgo_ReplacePlayerNameList.end())
							break;
					}
				}
				return true;
			}
			else if (1 <= strlen(arg1) && (isdigit(arg1[0]) || 'x' == tolower(arg1[0]))) {

				//Tier0_Warning("This command is deprecated.\n"); // Deprecated.

				g_csgo_ReplacePlayerNameList.emplace_back(arg1, arg2, 3);
				return true;

			}
		}
	}


	Tier0_Msg(
		"%s filter [...] - Filter names.\n"
		"%s help id [...]- Print help on <id...> usage.\n"
		"%s deprecated - Print deprecated commands.\n"
		"%s debug 0|1"
		"Please note: The engine might cache names, so set the filter up early enough!\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;
}


bool csgo_ReplaceTeamName_Console(IWrpCommandArgs * args)
{
	if (!Hookcsgo_Team_GetFunctions())
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return true;
	}

	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if(2 <= argC)
	{
		const char * arg1 = args->ArgV(1);

		if(0 == _stricmp("add", arg1) && 3 <= argC)
		{
			int index = atoi(args->ArgV(2));
			CCsgoREplaceTeamNameEntry entry;

			for(int i = 3; i < argC; ++i)
			{
				const char * curArg = args->ArgV(i);
				if(StringBeginsWith(curArg, "clanName="))
				{
					entry.Use_ClanName = true;
					entry.ClanName = curArg + strlen("clanName=");
				}
				else if(StringBeginsWith(curArg, "flagImageString="))
				{
					entry.Use_FlagImageString = true;
					entry.FlagImageString = curArg + strlen("flagImageString=");
				}
				else if(StringBeginsWith(curArg, "logoImageString="))
				{
					entry.Use_LogoImageString = true;
					entry.LogoImageString = curArg + strlen("logoImageString=");
				}
				else {
					Tier0_Warning("Argument %i (\"%s\") is not valid.\n", i, curArg);
					return true;
				}
			}

			g_csgo_ReplaceTeamNameList[index] = entry;
			return true;
		}
		else if(0 == _stricmp("remove", arg1) && 3 <= argC)
		{
			int index = atoi(args->ArgV(2));
			auto it = g_csgo_ReplaceTeamNameList.find(index);
			if(it != g_csgo_ReplaceTeamNameList.end())
			{
				g_csgo_ReplaceTeamNameList.erase(it);
			}
			else Tier0_Warning("AFXERROR: Invalid index.\n");

			return true;
		}
		else if(0 == _stricmp("clear", arg1))
		{
			g_csgo_ReplaceTeamNameList.clear();
			return true;
		}
		else if(0 == _stricmp("print", arg1))
		{
			for(auto it=g_csgo_ReplaceTeamNameList.begin(); it !=g_csgo_ReplaceTeamNameList.end(); ++it)
			{
				CCsgoREplaceTeamNameEntry &entry = it->second;

				Tier0_Msg("%i ->", it->first);
				if(entry.Use_ClanName) Tier0_Msg(" \"clanName=%s\"", entry.ClanName.c_str());
				if(entry.Use_FlagImageString) Tier0_Msg(" \"flagImageString=%s\"", entry.FlagImageString.c_str());
				if(entry.Use_LogoImageString) Tier0_Msg(" \"logoImageString=%s\"", entry.LogoImageString.c_str());
				Tier0_Msg("\n");
			}
			return true;
		}
		else if(0 == _stricmp("debug", arg1) && 3 <= argC)
		{
			g_csgo_ReplaceTeamNameDebug = 0 != atoi(args->ArgV(2));
			return true;
		}
	}

	Tier0_Msg(
		"%s add <iTeamNumber> <sTeamName> (\"clanName=<sText>\"|\"flagImageString=<sText>\"|\"logoImageString=<sText>\")* - Replace team text(s) (team numbers: 1 = Spectator, 2 = TERRORIST, 3 = CT, ...)\n"
		"%s remove <iTeamNumber>\n"
		"%s clear\n"
		"%s print\n"
		"%s debug 0|1\n"
		"Please note: The engine might cache names, so set the filter up early enough!\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
	return true;
}
