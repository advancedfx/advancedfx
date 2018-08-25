// BEGIN HLSDK includes
//
// HACK: prevent cldll_int.h from messing the HSPRITE definition,
// HLSDK's HSPRITE --> MDTHACKED_HSPRITE
#pragma push_macro("HSPRITE")
#define HSPRITE MDTHACKED_HSPRITE
//
#include <wrect.h>
#include <cl_dll.h>
#include <cdll_int.h>
#include <edict.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
// END HLSDK includes

//#include <progdefs.h>

#include "cmdregister.h"
#include "hl_addresses.h"

#include <windows.h>



typedef struct enginefuncs_s
{
	void (*_UNUSED_pfnPrecacheModel)(void);
	void (*_UNUSED_pfnPrecacheSound)(void);
	void (*_UNUSED_pfnSetModel)(void);
	void (*_UNUSED_pfnModelIndex)(void);
	void (*_UNUSED_pfnModelFrames)(void);
	void (*_UNUSED_pfnSetSize)(void);
	void (*_UNUSED_pfnChangeLevel)(void);
	void (*_UNUSED_pfnGetSpawnParms)(void);
	void (*_UNUSED_pfnSaveSpawnParms)(void);
	void (*_UNUSED_pfnVecToYaw)(void);
	void (*_UNUSED_pfnVecToAngles)(void);
	void (*_UNUSED_pfnMoveToOrigin)(void);
	void (*_UNUSED_pfnChangeYaw)(void);
	void (*_UNUSED_pfnChangePitch)(void);
	void (*_UNUSED_pfnFindEntityByString)(void);
	void (*_UNUSED_pfnGetEntityIllum)(void);
	void (*_UNUSED_pfnFindEntityInSphere)(void);
	void (*_UNUSED_pfnFindClientInPVS)(void);
	void (*_UNUSED_pfnEntitiesInPVS)(void);
	void (*_UNUSED_pfnMakeVectors)(void);
	void (*_UNUSED_pfnAngleVectors)(void);
	void (*_UNUSED_pfnCreateEntity)(void);

	void		(*pfnRemoveEntity)			(edict_t* e);
	edict_t*	(*pfnCreateNamedEntity)		(int className);

	void (*_UNUSED_pfnMakeStatic)(void);
	void (*_UNUSED_pfnEntIsOnFloor)(void);
	void (*_UNUSED_pfnDropToFloor)(void);
	void (*_UNUSED_pfnWalkMove)(void);
	void (*_UNUSED_pfnSetOrigin)(void);
	void (*_UNUSED_pfnEmitSound)(void);
	void (*_UNUSED_pfnEmitAmbientSound)(void);
	void (*_UNUSED_pfnTraceLine)(void);
	void (*_UNUSED_pfnTraceToss)(void);
	void (*_UNUSED_pfnTraceMonsterHull)(void);
	void (*_UNUSED_pfnTraceHull)(void);
	void (*_UNUSED_pfnTraceModel)(void);
	void (*_UNUSED_pfnTraceTexture)(void);
	void (*_UNUSED_pfnTraceSphere)(void);
	void (*_UNUSED_pfnGetAimVector)(void);
	void (*_UNUSED_pfnServerCommand)(void);
	void (*_UNUSED_pfnServerExecute)(void);
	void (*_UNUSED_pfnClientCommand)(void);
	void (*_UNUSED_pfnParticleEffect)(void);
	void (*_UNUSED_pfnLightStyle)(void);
	void (*_UNUSED_pfnDecalIndex)(void);
	void (*_UNUSED_pfnPovoidContents)(void);
	void (*_UNUSED_pfnMessageBegin)(void);
	void (*_UNUSED_pfnMessageEnd)(void);
	void (*_UNUSED_pfnWriteByte)(void);
	void (*_UNUSED_pfnWriteChar)(void);
	void (*_UNUSED_pfnWriteShort)(void);
	void (*_UNUSED_pfnWriteLong)(void);
	void (*_UNUSED_pfnWriteAngle)(void);
	void (*_UNUSED_pfnWriteCoord)(void);
	void (*_UNUSED_pfnWriteString)(void);
	void (*_UNUSED_pfnWriteEntity)(void);
	void (*_UNUSED_pfnCVarRegister)(void);
	void (*_UNUSED_pfnCVarGetvoid)(void);
	void (*_UNUSED_pfnCVarGetString)(void);
	void (*_UNUSED_pfnCVarSetvoid)(void);
	void (*_UNUSED_pfnCVarSetString)(void);
	void (*_UNUSED_pfnAlertMessage)(void);
	void (*_UNUSED_pfnEngineFprvoidf)(void);
	void (*_UNUSED_pfnPvAllocEntPrivateData)(void);
	void (*_UNUSED_pfnPvEntPrivateData)(void);
	void (*_UNUSED_pfnFreeEntPrivateData)(void);
	void (*_UNUSED_pfnSzFromIndex)(void);

	int			(*pfnAllocString)			(const char *szValue);

	void (*_UNUSED_pfnGetVarsOfEnt)(void);
	void (*_UNUSED_pfnPEntityOfEntOffset)(void);
	void (*_UNUSED_pfnEntOffsetOfPEntity)(void);
	void (*_UNUSED_pfnIndexOfEdict)(void);
	void (*_UNUSED_pfnPEntityOfEntIndex)(void);
	void (*_UNUSED_pfnFindEntityByVars)(void);
	void (*_UNUSED_pfnGetModelPtr)(void);
	void (*_UNUSED_pfnRegUserMsg)(void);
	void (*_UNUSED_pfnAnimationAutomove)(void);
	void (*_UNUSED_pfnGetBonePosition)(void);
	void (*_UNUSED_pfnFunctionFromName)(void);
	void (*_UNUSED_pfnNameForFunction)(void);
	void (*_UNUSED_pfnClientPrvoidf)(void);
	void (*_UNUSED_pfnServerPrvoid)(void);
	void (*_UNUSED_pfnCmd_Args)(void);
	void (*_UNUSED_pfnCmd_Argv)(void);
	void (*_UNUSED_pfnCmd_Argc)(void);
	void (*_UNUSED_pfnGetAttachment)(void);
	void (*_UNUSED_pfnCRC32_Init)(void);
	void (*_UNUSED_pfnCRC32_ProcessBuffer)(void);
	void (*_UNUSED_pfnCRC32_ProcessByte)(void);
	void (*_UNUSED_pfnCRC32_Final)(void);
	void (*_UNUSED_pfnRandomLong)(void);
	void (*_UNUSED_pfnRandomvoid)(void);
	void (*_UNUSED_pfnSetView)(void);
	void (*_UNUSED_pfnTime)(void);
	void (*_UNUSED_pfnCrosshairAngle)(void);
	void (*_UNUSED_pfnLoadFileForMe)(void);
	void (*_UNUSED_pfnFreeFile)(void);
	void (*_UNUSED_pfnEndSection)(void);
	void (*_UNUSED_pfnCompareFileTime)(void);
	void (*_UNUSED_pfnGetGameDir)(void);
	void (*_UNUSED_pfnCvar_RegisterVariable)(void);
	void (*_UNUSED_pfnFadeClientVolume)(void);
	void (*_UNUSED_pfnSetClientMaxspeed)(void);
	void (*_UNUSED_pfnCreateFakeClient)(void);
	void (*_UNUSED_pfnRunPlayerMove)(void);
	void (*_UNUSED_pfnNumberOfEntities)(void);
	void (*_UNUSED_pfnGetInfoKeyBuffer)(void);
	void (*_UNUSED_pfnInfoKeyValue)(void);
	void (*_UNUSED_pfnSetKeyValue)(void);
	void (*_UNUSED_pfnSetClientKeyValue)(void);
	void (*_UNUSED_pfnIsMapValid)(void);
	void (*_UNUSED_pfnStaticDecal)(void);
	void (*_UNUSED_pfnPrecacheGeneric)(void);
	void (*_UNUSED_pfnGetPlayerUserId)(void);
	void (*_UNUSED_pfnBuildSoundMsg)(void);
	void (*_UNUSED_pfnIsDedicatedServer)(void);
	void (*_UNUSED_pfnCVarGetPovoider)(void);
	void (*_UNUSED_pfnGetPlayerWONId)(void);
	void (*_UNUSED_pfnInfo_RemoveKey)(void);
	void (*_UNUSED_pfnGetPhysicsKeyValue)(void);
	void (*_UNUSED_pfnSetPhysicsKeyValue)(void);
	void (*_UNUSED_pfnGetPhysicsInfoString)(void);
	void (*_UNUSED_pfnPrecacheEvent)(void);
	void (*_UNUSED_pfnPlaybackEvent)(void);
	void (*_UNUSED_pfnSetFatPVS)(void);
	void (*_UNUSED_pfnSetFatPAS)(void);
	void (*_UNUSED_pfnCheckVisibility )(void);
	void (*_UNUSED_pfnDeltaSetField)(void);
	void (*_UNUSED_pfnDeltaUnsetField)(void);
	void (*_UNUSED_pfnDeltaAddEncoder)(void);
	void (*_UNUSED_pfnGetCurrentPlayer)(void);
	void (*_UNUSED_pfnCanSkipPlayer)(void);
	void (*_UNUSED_pfnDeltaFindField)(void);
	void (*_UNUSED_pfnDeltaSetFieldByIndex)(void);
	void (*_UNUSED_pfnDeltaUnsetFieldByIndex)(void);
	void (*_UNUSED_pfnSetGroupMask)(void);
	void (*_UNUSED_pfnCreateInstancedBaseline)(void);
	void (*_UNUSED_pfnCvar_DirectSet)(void);
	void (*_UNUSED_pfnForceUnmodified)(void);
	void (*_UNUSED_pfnGetPlayerStats)(void);
	void (*_UNUSED_pfnAddServerCommand)(void);
	void (*_UNUSED_pfnVoice_GetClientListening)(void);
	void (*_UNUSED_pfnVoice_SetClientListening)(void);
	void (*_UNUSED_pfnGetPlayerAuthId)(void);
	void (*_UNUSED_pfnSequenceGet)(void);
	void (*_UNUSED_pfnSequencePickSentence)(void);
	void (*_UNUSED_pfnGetFileSize)(void);
	void (*_UNUSED_pfnGetApproxWavePlayLen)(void);
	void (*_UNUSED_pfnIsCareerMatch)(void);
	void (*_UNUSED_pfnGetLocalizedStringLength)(void);
	void (*_UNUSED_pfnRegisterTutorMessageShown)(void);
	void (*_UNUSED_pfnGetTimesTutorMessageShown)(void);
	void (*_UNUSED_pfnProcessTutorMessageDecayBuffer)(void);
	void (*_UNUSED_pfnConstructTutorMessageDecayBuffer)(void);
	void (*_UNUSED_pfnResetTutorMessageDecayData)(void);
	void (*_UNUSED_pfnQueryClientCvarValue)(void);
    void (*_UNUSED_pfnQueryClientCvarValue2);
} enginefuncs_t;


// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.  INTERFACE VERSION IS FROZEN AT 138

// Passed to pfnKeyValue
typedef struct KeyValueData_s
{
	char	*szClassName;	// in: entity classname
	char	*szKeyName;		// in: name of key
	char	*szValue;		// in: value of key
	__int32	fHandled;		// out: DLL sets to true if key-value pair was understood
} KeyValueData;



enginefuncs_t * pHostFuncs = 0; 
globalvars_t  *pGlobals = 0;

extern cl_enginefuncs_s *pEngfuncs;

#define STRING(offset)		(const char *)(pGlobals->pStringBase + (int)offset)
#define MAKE_STRING(str)	((int)str - (int)STRING(0))

REGISTER_DEBUGCMD_FUNC(wazzup_snow)
{
	pHostFuncs = (enginefuncs_t *)HL_ADDR_GET(p_enginefuncs_s);
	pGlobals = (globalvars_t *)HL_ADDR_GET(p_globalvars_s);

	edict_t * pedict = pHostFuncs->pfnCreateNamedEntity(MAKE_STRING("env_snow"));
	if(pedict && pedict->pvPrivateData) {
		pEngfuncs->Con_Printf("Created.\n");

		//VARS( pent )->origin = pev->origin;
		//pent->v.spawnflags |= SF_NORESPAWN;

		unsigned int dwClient = (unsigned int)GetModuleHandle("client.dll");

		dwClient += 0x2D175F0;

		((entvars_t *)&pedict->v)->owner = 0;
		((entvars_t *)&pedict->v)->origin = Vector(0, 0, 0);
		((entvars_t *)&pedict->v)->angles = Vector(0, 0, 0);

		// Dispatch spawn:
		__asm {
			MOV eax, pedict
			PUSH eax
			MOV eax, dwClient;
			CALL eax;
		};


	}
	else
		pEngfuncs->Con_Printf("Creation failed.\n");

	//pEntity->pev->owner = pentOwner;
	//pEntity->pev->origin = vecOrigin;
	//pEntity->pev->angles = vecAngles;
	//DispatchSpawn( pEntity->edict() );

	//035e4955 68e0d0ce03      push    offset hw!vgui::Frame::`vftable'+0x681e1c (03ced0e0)
//035e495a 68f0b96a03      push    offset hw!vgui::Frame::`vftable'+0x4072c (036ab9f0)
//035e495f ffd0            call    eax
}