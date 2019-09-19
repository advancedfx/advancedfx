#ifndef ADVANCEDFX_AFXHOOKSOURCE_H
#define ADVANCEDFX_AFXHOOKSOURCE_H
#include "AfxTypes.h"
#include "AfxInterface.h"
#include "AfxJit.h"

typedef int advancedfx_JitType;


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

struct AdvancedfxIEntity
{
	AdvancedfxEntityHandle(*GetHandle)(struct AdvancedfxIEntity* This);

	int(*GetEntIndex)(struct AdvancedfxIEntity* This);

	AdvancedfxIEntity*(*GetActiveWeapon)(struct AdvancedfxIEntity* This);

	void(*GetAbsOrigin)(struct AdvancedfxIEntity* This, struct AdvancedfxQVector * outAbsOrigin);

	void(*GetAbsAngles)(struct AdvancedfxIEntity* This, struct AdvancedfxQAngles * outAbsAbgkles);

	AdvancedfxQVector(*EyePosition)(struct AdvancedfxIEntity* This);

	AdvancedfxQAngles(*EyeAngles)(struct AdvancedfxIEntity* This);

	int(*LookUpAttachment)(struct AdvancedfxIEntity* This, const char * attachmentName);

	bool(*GetAttachment)(struct AdvancedfxIEntity* This, int number, struct advancedfx_QVector * outOrigin, struct advancedfx_QAngles * outAngles);

	void* (*GetRawIClientEntity)(struct AdvancedfxIEntity* This);
};

struct AdvancedfxIAfxHookSourceFactoryConnected
{
	void (*Connected)(struct AdvancedfxIAfxHookSourceFactoryConnected* This, struct AdvancedfxUuid uuid, struct AdvancedfxIFactory* factory);
};

ADVANCEDFX_IDELETING_DECL(AdvancedfxIAfxHookSource, struct AdvancedfxIAfxHookSource*)
ADVANCEDFX_ILIST_DECL(AdvancedfxIFactory, struct AdvancedfxIFactory*)
ADVANCEDFX_ILIST_DECL(AdvancedfxISetupEngineViewOverride, struct AdvancedfxISetupEngineViewOverride*)
ADVANCEDFX_ILIST_DECL(AdvancedfxIEntity, struct AdvancedfxIEntity*)

struct AdvancedfxIAfxHookSourceVtable
{
	/**
	 * Dll is loaded and queried for ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN.
	 */
	AdvancedfxIAfxHookSourceDll* (*LoadDll)(struct AdvancedfxIAfxHookSource* This, const char* filePath);

	struct AdvancedfxILocale* (*GetLocale)(struct AdvancedfxIAfxHookSource* This);

	AdvancedfxIEntity* (*EntityFromHandle)(struct AdvancedfxIAfxHookSource** This, AdvancedfxEntityHandle handle);

	AdvancedfxIEntity* (*EntityFromIndex)(struct AdvancedfxIAfxHookSource** This, int index);

	AdvancedfxIEntity* (*EntityFromSpecKey)(struct AdvancedfxIAfxHookSource** This, int keyNumber);
};

struct AdvancedfxIAfxHookSource
{
	AdvancedfxIAfxHookSourceVtable* Vtable;

	ADVANCEDFX_IDELETING(AdvancedfxIAfxHookSource)* Deleting;

	ADVANCEDFX_ILIST(AdvancedfxIFactory)* Factories;

	ADVANCEDFX_ILIST(AdvancedfxISetupEngineViewOverride)* SetupEngineViewHandlers;

	ADVANCEDFX_ILIST(AdvancedfxIEntity)* Entities;

	//
	// Jits

	ADVANCEDFX_ILIST(AdvancedxIJit)* Jits;
};

// AdvancedfxFactoryFn: ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN, AdvancedfxIAfxHookSource -> AdvancedfxIAfxHookSourceDll
#define ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3F3644B5,0xDF45,0x406B,0xB0B4,0x0E,0xFD,0xA7,0xA9,0x42,0xE4)

struct AdvancedfxIAfxHookSourceDllVtable
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIAfxHookSourceDll* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIAfxHookSourceDll* This);

	void (*Connect)(struct AdvancedfxIAfxHookSourceDll* This, struct AdvancedfxIAfxHookSource* afxHookSource);
};

ADVANCEDFX_IDELETING_DECL(AdvancedfxIAfxHookSourceDll, struct AdvancedfxIAfxHookSourceDll*)

struct AdvancedfxIAfxHookSourceDll
{
	AdvancedfxIAfxHookSourceDllVtable* Vtable;

	ADVANCEDFX_IDELETING(AdvancedfxIAfxHookSourceDll)* Deleting;
};

#endif
