#ifndef ADVANCEDFX_AFXHOOKSOURCE_H
#define ADVANCEDFX_AFXHOOKSOURCE_H
#include "AfxTypes.h"
#include "AfxInterface.h"
#include "AfxJit.h"

// UUIDs ///////////////////////////////////////////////////////////////////////


// AdvancedfxFactoryFn: ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID, AdvancedfxIAfxHookSource* -> AdvancedfxIReference*
#define ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3F3644B5,0xDF45,0x406B,0xB0B4,0x0E,0xFD,0xA7,0xA9,0x42,0xE4)

// AdavancedfxIAfxHookSource::GetInterface: ADVANCEDFX_IENTITIES_UUID -> AdvancedfxIEntities*
#define ADVANCEDFX_IENTITIES_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xA7888B75,0x79DF,0x47B9,0xB801,0x03,0x40,0xF4,0x88,0xB5,0x8E)

// AdavancedfxIAfxHookSource::GetInterface: ADVANCEDFX_ISETUPENGINEVIEWOVERRIDES_UUID -> AdvancedfxISetupEngineViewOverrides*
#define ADVANCEDFX_ISETUPENGINEVIEWOVERRIDES_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x8164851B,0xF7F1,0x4F25,0x9A38,0x21,0x6E,0x43,0x79,0x88,0x22)

// AdavancedfxIAfxHookSource::GetInterface: ADVANCEDFX_IAFXHOOKSOURCEJITS_UUID -> AdvancedfxISetupEngineViewOverrides*
#define ADVANCEDFX_IAFXHOOKSOURCE_JITS_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xED0CDB83,0x646B,0x4CCA,0x887F,0x3A,0x13,0xE2,0x6E,0x45,0x75)

// AdvancedfxIJitContext::Get: ADVANCEDFX_JIT_CONTEXT_IAFXHOOKSOURCE_UUID -> AdavancedfxIAfxHookSource*
#define ADVANCEDFX_JIT_CONTEXT_IAFXHOOKSOURCE_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x737E1F66,0x6034,0x4DC6,0x8DE2,0x03,0x3B,0x76,0x83,0x05,0x4F)

// AdavancedfxIAfxHookSource::GetInterface: ADVANCEDFX_IAFXHOOKSOURCE_PYTHON_IJIT_UUID -> AdvancedxIJit*
#define ADVANCEDFX_IAFXHOOKSOURCE_PYTHON_IJIT_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x60B22765,0x9E18,0x410F,0x8401,0x9B,0x1E,0x3C,0xE8,0x74,0xC8)



////////////////////////////////////////////////////////////////////////////////


typedef int AdvancedfxEntityHandle;

typedef struct AdvancedfxQVector {
	float x;
	float y;
	float z;
};

typedef struct AdvancedfxQAngles {
	float x;
	float y;
	float z;
};


// Entities ////////////////////////////////////////////////////////////////////


struct AdvancedfxIEntityVtable
{
	void(*Delete)(struct AdvancedfxIEntity* This);

	AdvancedfxEntityHandle(*GetHandle)(struct AdvancedfxIEntity* This);

	int(*GetEntIndex)(struct AdvancedfxIEntity* This);

	AdvancedfxIEntity*(*GetActiveWeapon)(struct AdvancedfxIEntity* This);

	void(*GetAbsOrigin)(struct AdvancedfxIEntity* This, struct AdvancedfxQVector * outAbsOrigin);

	void(*GetAbsAngles)(struct AdvancedfxIEntity* This, struct AdvancedfxQAngles * outAbsAbgkles);

	AdvancedfxQVector(*EyePosition)(struct AdvancedfxIEntity* This);

	AdvancedfxQAngles(*EyeAngles)(struct AdvancedfxIEntity* This);

	int(*LookUpAttachment)(struct AdvancedfxIEntity* This, const char * attachmentName);

	AdvancedfxBool(*GetAttachment)(struct AdvancedfxIEntity* This, int number, struct advancedfx_QVector * outOrigin, struct advancedfx_QAngles * outAngles);

	void* (*GetRawIClientEntity)(struct AdvancedfxIEntity* This);
};

struct AdvancedfxIEntity {
	size_t RefCountValid;
	struct AdvancedfxIEntityVtable* Vtable;
};

ADVANCEDFX_INOTIFY_DECL(AdvancedfxIEntitiesNotify, struct AdvancedfxIAfxHookSource*)

struct AdvancedfxIEntitiesVtable
{
	//
	// Implement AdvancedfxIInterfaceVtable:

	void(*Delete)(struct AdvancedfxIEntities* This);

	struct AdvancedfxUuid(*GetUuid)(struct AdvancedfxIEntities* This);

	//
	// Own:

	ADVANCEDFX_ILIST_DECL_FNS(, AdvancedfxIEntities, AdvancedfxIEntitiesNotify, struct AdvancedfxIEntity*)

	struct AdvancedfxIEntity* (*EntityFromHandle)(struct AdvancedfxIAfxHookSource** This, AdvancedfxEntityHandle handle);

	struct AdvancedfxIEntity* (*EntityFromIndex)(struct AdvancedfxIAfxHookSource** This, int index);

	struct AdvancedfxIEntity* (*EntityFromSpecKey)(struct AdvancedfxIAfxHookSource** This, int keyNumber);
};

struct AdvancedfxIEntities
{
	//
	// Implement AdvancedfxIInterface:

	size_t RefCountValid;
	struct AdvancedfxIEntitiesVtable* Vtable;

	//
	// Own:
};


// Engine view overrides ///////////////////////////////////////////////////////


struct AdvancedfxISetupEngineViewOverrideVtable
{
	void(*Delete)(struct AdvancedfxISetupEngineViewOverride* This);

	bool(*Handle)(struct AdvancedfxISetupEngineViewOverride * This, bool lastResult, advancedfx_QVector* inOutOrigin, advancedfx_QAngles* inOutAngles, float* inOutfov);
};

struct AdvancedfxISetupEngineViewOverride
{
	size_t RefCountValid;
	struct AdvancedfxISetupEngineViewOverrideVtable* Vtable;
};

ADVANCEDFX_INOTIFY_DECL(AdvancedfxISetupEngineViewOverridesNotify, struct AdvancedfxISetupEngineViewOverrides*)

struct AdvancedfxISetupEngineViewOverridesVtable
{
	//
	// Implement AdvancedfxIInterfaceVtable:

	void(*Delete)(struct AdvancedfxISetupEngineViewOverrides* This);

	struct AdvancedfxUuid(*GetUuid)(struct AdvancedfxISetupEngineViewOverrides* This);

	//
	// Own:

	ADVANCEDFX_ILIST_DECL_FNS(, AdvancedfxISetupEngineViewOverrides, AdvancedfxISetupEngineViewOverridesNotify, struct AdvancedfxISetupEngineViewOverride*)
};

struct AdvancedfxISetupEngineViewOverrides
{
	//
	// Implement AdvancedfxIInterface:

	size_t RefCountValid;
	struct AdvancedfxISetupEngineViewOverridesVtable* Vtable;

	//
	// Own:
};

// AfxIHookSourceDll ///////////////////////////////////////////////////////////

struct AfxIHookSourceDllVtable
{
	//
	// Implement AdvancedfxIReferenceVtable:

	void(*Delete)(struct AfxIHookSourceDll* This);

	//
	// Own:
};

struct AfxIHookSourceDll
{
	//
	// Implement AdvancedfxIReference:

	size_t RefCountValid;
	struct AfxIHookSourceDllVtable* Vtable;

	//
	// Own:
};

// AfxHookSource ///////////////////////////////////////////////////////////////


ADVANCEDFX_INOTIFY_DECL(AdvancedfxIAfxHookSourceNotify, struct AdvancedfxIAfxHookSource*)

struct AdvancedfxIAfxHookSourceVtable
{
	//
	// Implement AdvancedfxIReferenceVtable:

	void(*Delete)(struct AdvancedfxIAfxHookSource* This);

	//
	// Own:

	struct AdvancedfxCore* (*GetCore)(struct AdvancedfxIAfxHookSource* This);

	struct AdvancedfxILocale* (*GetLocale)(struct AdvancedfxIAfxHookSource* This);

	//void (*BeginNotifyBeforeDelete)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceNotify* notify);

	//void (*EndNotifyBeforeDelete)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceNotify* notify);

	/**
	 * Dll is loaded and queried for ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN.
	 */
	struct AfxIHookSourceDll* (*LoadDll)(struct AdvancedfxIAfxHookSource* This, const char* filePath);

	/**
	 * This first calls Delete on the reference returned to LoadDll and then unloads the dll (FreeLibrary).
	 * Please note: On Windows a single DLL can be loaded multiple times by the same process, so the final unload won't happen before it's unloaded as much times as it is still loaded.
	 */
	void (*FreeDll)(struct AdvancedfxIAfxHookSource* This, struct AfxIHookSourceDll* value);

	ADVANCEDFX_ILIST_DECL_FNS(Interfaces, AdvancedfxIAfxHookSource, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxIInterface*)
	
	struct AdvancedfxIInterface* (*GetInterface)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid);
};

struct AdvancedfxIAfxHookSource
{
	//
	// Implement AdvancedfxIReference:

	size_t RefCountValid;
	struct AdvancedfxIAfxHookSourceVtable* Vtable;

	//
	// Own:
};

#endif
