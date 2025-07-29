#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <windows.h>

void HookSchemaSystem(HMODULE schemaSystemDll);

// these resolved in runtime
struct ClientDllOffsets_t {
	struct C_CSGameRulesProxy {
		ptrdiff_t m_pGameRules = 0; // C_CSGameRules*
	} C_CSGameRulesProxy;

	struct C_CSGameRules {
		ptrdiff_t m_gamePhase = 0; // int32
		ptrdiff_t m_nOvertimePlaying = 0; // int32
	} C_CSGameRules;

	struct CEntityInstance {
		ptrdiff_t m_pEntity = 0; // CEntityIdentity*
	} CEntityInstance;

	struct C_BaseEntity {
		ptrdiff_t m_iTeamNum = 0; // uint8
	} C_BaseEntity;

	struct C_BaseCSGrenadeProjectile {
		ptrdiff_t m_bCanCreateGrenadeTrail = 0x11BD; // bool
		ptrdiff_t m_nSnapshotTrajectoryEffectIndex = 0; // ParticleIndex_t
		ptrdiff_t m_flTrajectoryTrailEffectCreationTime = 0; // float32

	} C_BaseCSGrenadeProjectile;

	struct C_SmokeGrenadeProjectile {
		ptrdiff_t m_vSmokeColor = 0; // Vector
	} C_SmokeGrenadeProjectile;

	struct CBasePlayerController {
		ptrdiff_t m_iszPlayerName = 0; // char[128]
		ptrdiff_t m_steamID = 0; // uint64
		ptrdiff_t m_hPawn = 0; // CHandle< C_CSPlayerPawnBase >
	} CBasePlayerController;

	struct CCSPlayerController {
		ptrdiff_t m_sSanitizedPlayerName = 0; // CUtlString
	} CCSPlayerController;

	struct C_BasePlayerPawn {
		ptrdiff_t m_hController = 0; // CHandle< CBasePlayerController >
		ptrdiff_t m_pWeaponServices = 0; // CPlayer_WeaponServices*
		ptrdiff_t m_pObserverServices = 0; // CPlayer_ObserverServices*
		ptrdiff_t m_pCameraServices = 0; // CPlayer_CameraServices*
	} C_BasePlayerPawn;

	struct CPlayer_CameraServices {
		ptrdiff_t m_hViewEntity = 0; // CHandle< CBaseEntity >
	} CPlayer_CameraServices;

	struct CPlayer_WeaponServices {
		ptrdiff_t m_hActiveWeapon = 0; // CHandle< CBasePlayerWeapon >
	} CPlayer_WeaponServices;

	struct CPlayer_ObserverServices {
		ptrdiff_t m_iObserverMode = 0; // uint8                                   
		ptrdiff_t m_hObserverTarget  = 0; // CHandle< CBaseEntity >
	} CPlayer_ObserverServices;
};

extern struct ClientDllOffsets_t g_clientDllOffsets;

// https://github.com/sneakyevil/CS2-SchemaDumper/blob/main/CSchemaSystem.hpp

#define S2_PAD_INSERT(x, y) x ## y
#define S2_PAD_DEFINE(x, y) S2_PAD_INSERT(x, y)
#define S2_PAD(size) char S2_PAD_DEFINE(padding_, __LINE__)[size]

namespace SDK
{
	class CSchemaField
	{
	public:
		const char* m_szName;
		void* m_pType;
		uint32_t m_nOffset;
		uint32_t m_nMetadataSize;
		void* m_nMetadata;
	};

	class CSchemaClass
	{
	public:
		void* vfptr;
		const char* m_szName;
		const char* m_szModuleName;
		uint32_t m_nSize;
		uint16_t m_nNumFields;

		S2_PAD(0x2);

		uint16_t m_nStaticSize;
		uint16_t m_nMetadataSize;

		S2_PAD(0x4);

		CSchemaField* m_pFields;
	};

	class CSchemaDeclaredClass
	{
	public:
		void* vfptr;
		const char* m_szName;
		const char* m_szModuleName;
		const char* m_szUnknownStr;
		CSchemaClass* m_Class;
	};

	class CSchemaDeclaredClassEntry
	{
	public:
		uint64_t m_nHash[2];
		CSchemaDeclaredClass* m_pDeclaredClass;
	};

	class CSchemaSystemTypeScope
	{
	public:
		void* vfptr;
		char m_szName[256];

		S2_PAD(0x338);

		uint16_t m_nNumDeclaredClasses;

		S2_PAD(0x6);

		CSchemaDeclaredClassEntry* m_pDeclaredClasses;
	};

	class CSchemaSystem
	{
	public:
		S2_PAD(0x188);

		uint64_t m_nScopeSize;
		CSchemaSystemTypeScope** m_pScopeArray;
	};
}
