#include "SchemaSystem.h"
#include "Globals.h"
#include <winsock.h>

ClientDllOffsets_t g_clientDllOffsets;

// module name -> class name -> field name -> offset
std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::ptrdiff_t>>> g_SchemaSystemOffsets;

void getOffsetsFromSchemaSystem(SDK::CSchemaSystem* pSchemaSystem)
{
	void** pScopeArray = (void**)(pSchemaSystem->m_pScopeArray);

	for (uint64_t i = 0; pSchemaSystem->m_nScopeSize > i; ++i)
	{
		SDK::CSchemaSystemTypeScope* pSchemaScope = (SDK::CSchemaSystemTypeScope*)(pScopeArray[i]);

		// we don't need other modules for now
		if (!pSchemaScope || !pSchemaScope->m_pDeclaredClasses || 0 != strcmp(pSchemaScope->m_szName, "client.dll"))
		{
			continue;
		}

		std::vector<SDK::CSchemaDeclaredClassEntry> declaredClassEntries(pSchemaScope->m_nNumDeclaredClasses + 1);
		memcpy(declaredClassEntries.data(), pSchemaScope->m_pDeclaredClasses, (pSchemaScope->m_nNumDeclaredClasses + 1) * sizeof(SDK::CSchemaDeclaredClassEntry));

		for (uint16_t j = 0; j <= pSchemaScope->m_nNumDeclaredClasses; ++j)
		{
			SDK::CSchemaDeclaredClass* pDeclaredClass = declaredClassEntries[j].m_pDeclaredClass;
			if (!pDeclaredClass) continue;

			SDK::CSchemaClass* pClass = pDeclaredClass->m_Class;
			if (!pClass) continue;

			const char* className = pClass->m_szName;

			uintptr_t pClassFields = (uintptr_t)(pClass->m_pFields);
			if (pClassFields)
			{
				for (uint16_t k = 0; pClass->m_nNumFields > k; ++k)
				{
					SDK::CSchemaField* pField = (SDK::CSchemaField*)(pClassFields + sizeof(SDK::CSchemaField) * k);

					if (!pField) continue;
					if (!pField->m_pType) continue; 
					if (!pField->m_szName) continue;

					auto fieldName = pField->m_szName;
		
					size_t fieldNameSize = strlen(fieldName);
					bool isNameValid = (fieldNameSize > 0);

					for (size_t n = 0; n < fieldNameSize; ++n) {
						if (!isascii(fieldName[n])) {
							isNameValid = false;
							break;
						}
					}

					if (!isNameValid) continue;

					g_SchemaSystemOffsets[pSchemaScope->m_szName][className][fieldName] = pField->m_nOffset;
				}
			}
		
		}
	}
}

bool getOffset(ptrdiff_t* offset, std::string moduleName, std::string className, std::string fieldName)
{
	if(g_SchemaSystemOffsets.find(moduleName) == g_SchemaSystemOffsets.end()) return false;
	auto& module = g_SchemaSystemOffsets.at(moduleName);

	if(module.find(className) == module.end()) return false;
	auto& classFields = module.at(className);

	if(classFields.find(fieldName) == classFields.end()) return false;
	*offset = classFields.at(fieldName);

	return true;
}

void initSchemaSystemOffsets()
{
	bool bOk = true;

	bOk = bOk && getOffset(&g_clientDllOffsets.C_CSGameRulesProxy.m_pGameRules, "client.dll", "C_CSGameRulesProxy", "m_pGameRules");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_CSGameRules.m_gamePhase, "client.dll", "C_CSGameRules", "m_gamePhase");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_CSGameRules.m_nOvertimePlaying, "client.dll", "C_CSGameRules", "m_nOvertimePlaying");
	bOk = bOk && getOffset(&g_clientDllOffsets.CEntityInstance.m_pEntity, "client.dll", "CEntityInstance", "m_pEntity");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BaseEntity.m_iTeamNum, "client.dll", "C_BaseEntity", "m_iTeamNum");
	bOk = bOk && getOffset(&g_clientDllOffsets.CBasePlayerController.m_iszPlayerName, "client.dll", "CBasePlayerController", "m_iszPlayerName");
	bOk = bOk && getOffset(&g_clientDllOffsets.CBasePlayerController.m_steamID, "client.dll", "CBasePlayerController", "m_steamID");
	bOk = bOk && getOffset(&g_clientDllOffsets.CBasePlayerController.m_hPawn, "client.dll", "CBasePlayerController", "m_hPawn");
	bOk = bOk && getOffset(&g_clientDllOffsets.CCSPlayerController.m_sSanitizedPlayerName, "client.dll", "CCSPlayerController", "m_sSanitizedPlayerName");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BasePlayerPawn.m_hController, "client.dll", "C_BasePlayerPawn", "m_hController");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BasePlayerPawn.m_pWeaponServices, "client.dll", "C_BasePlayerPawn", "m_pWeaponServices");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BasePlayerPawn.m_pObserverServices, "client.dll", "C_BasePlayerPawn", "m_pObserverServices");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BasePlayerPawn.m_pCameraServices, "client.dll", "C_BasePlayerPawn", "m_pCameraServices");
	bOk = bOk && getOffset(&g_clientDllOffsets.CPlayer_WeaponServices.m_hActiveWeapon, "client.dll", "CPlayer_WeaponServices", "m_hActiveWeapon");
	bOk = bOk && getOffset(&g_clientDllOffsets.CPlayer_CameraServices.m_hViewEntity, "client.dll", "CPlayer_CameraServices", "m_hViewEntity");
	bOk = bOk && getOffset(&g_clientDllOffsets.CPlayer_ObserverServices.m_iObserverMode, "client.dll", "CPlayer_ObserverServices", "m_iObserverMode");
	bOk = bOk && getOffset(&g_clientDllOffsets.CPlayer_ObserverServices.m_hObserverTarget, "client.dll", "CPlayer_ObserverServices", "m_hObserverTarget");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_bCanCreateGrenadeTrail, "client.dll", "C_BaseCSGrenadeProjectile", "m_bCanCreateGrenadeTrail");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_nSnapshotTrajectoryEffectIndex, "client.dll", "C_BaseCSGrenadeProjectile", "m_nSnapshotTrajectoryEffectIndex");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_BaseCSGrenadeProjectile.m_flTrajectoryTrailEffectCreationTime, "client.dll", "C_BaseCSGrenadeProjectile", "m_flTrajectoryTrailEffectCreationTime");
	bOk = bOk && getOffset(&g_clientDllOffsets.C_SmokeGrenadeProjectile.m_vSmokeColor, "client.dll", "C_SmokeGrenadeProjectile", "m_vSmokeColor");

	if (!bOk) ErrorBox(MkErrStr(__FILE__, __LINE__));	
}

void HookSchemaSystem(HMODULE schemaSystemDll)
{

   // 18000ab79 48  89  05       MOV        qword ptr [DAT_18005c710 ],RAX
   //           90  1b  05  00
   // 18000ab80 4c  8d  0d       LEA        R9,[s_schema_list_bindings_<substring>_1800449   = "schema_list_bindings <substri
   //           09  9e  03  00
   // 18000ab87 0f  b6  45  e8   MOVZX      EAX ,byte ptr [RBP  + local_28[8] ]
   // 18000ab8b 4c  8d  45  e0   LEA        R8=>local_28 ,[RBP  + -0x20 ]
   // 18000ab8f 33  f6           XOR        ESI ,ESI
	size_t instructionAddr = getAddress(schemaSystemDll, "48 89 05 ?? ?? ?? ?? 4C 8D 0D ?? ?? ?? ?? 0F B6 45 E8 4C 8D 45 E0 33 F6");
	if (0 == instructionAddr) {
		ErrorBox(MkErrStr(__FILE__, __LINE__));	
		return;
	}

	uintptr_t _SchemaSystemInterface = instructionAddr + *(int32_t*)(instructionAddr + 3) + 7;
	SDK::CSchemaSystem* schemaSystem = (SDK::CSchemaSystem*)(_SchemaSystemInterface);

	if (!schemaSystem)
	{
		ErrorBox(MkErrStr(__FILE__, __LINE__));
		return;
	}

	getOffsetsFromSchemaSystem(schemaSystem);

	initSchemaSystemOffsets();

	g_SchemaSystemOffsets.clear();
}
