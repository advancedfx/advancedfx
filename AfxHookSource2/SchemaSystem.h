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

	struct C_BaseEntity {
		ptrdiff_t m_iTeamNum = 0; // uint8
	} C_BaseEntity;

	struct CBasePlayerController {
		ptrdiff_t m_iszPlayerName = 0; // char[128]
		ptrdiff_t m_steamID = 0; // uint64
	} CBasePlayerController;

	struct CCSPlayerController {
		ptrdiff_t m_sSanitizedPlayerName = 0; // CUtlString
	} CCSPlayerController;
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

		CSchemaDeclaredClassEntry* m_pDeclaredClasses;

		S2_PAD(0xE);

		uint16_t m_nNumDeclaredClasses;
	};

	class CSchemaSystem
	{
	public:
		S2_PAD(0x188);

		uint64_t m_nScopeSize;
		CSchemaSystemTypeScope** m_pScopeArray;
	};
}
