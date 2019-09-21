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

struct AdvancedfxIAfxHookSourceFactoryConnected
{
	void (*Connected)(struct AdvancedfxIAfxHookSourceFactoryConnected* This, struct AdvancedfxUuid uuid, struct AdvancedfxIFactory* factory);
};

ADVANCEDFX_INOTIFY_DECL(AdvancedfxIAfxHookSourceNotify, struct AdvancedfxIAfxHookSource*)

struct AdvancedfxIAfxHookSourceVtable
{
	void(*Delete)(struct AdvancedfxIAfxHookSource* This);

	void (*NotifyBeforeDeleteBegin)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceNotify* notify);

	void (*NotifyBeforeDeleteEnd)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceNotify* notify);

	struct AdvancedfxILocale* (*GetLocale)(struct AdvancedfxIAfxHookSource* This);


	/**
	 * Dll is loaded and queried for ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN.
	 */
	struct AdvancedfxIAfxHookSourceDll* (*LoadDll)(struct AdvancedfxIAfxHookSource* This, const char* filePath);

	void (*AddFactory)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIFactory* factory);

	ADVANCEDFX_ILIST_DECL_FNS(FactoryUuids, AdvancedfxIAfxHookSource, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxUuid)

	struct AdvancedfxIFactory* (*GetFactory)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid);

	struct AdvancedfxIEntity* (*EntityFromHandle)(struct AdvancedfxIAfxHookSource** This, AdvancedfxEntityHandle handle);

	struct AdvancedfxIEntity* (*EntityFromIndex)(struct AdvancedfxIAfxHookSource** This, int index);

	struct AdvancedfxIEntity* (*EntityFromSpecKey)(struct AdvancedfxIAfxHookSource** This, int keyNumber);

	ADVANCEDFX_ILIST_DECL_FNS(SetupEngineViewHandlers, AdvancedfxIAfxHookSource, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxISetupEngineViewOverride*)

	ADVANCEDFX_ILIST_DECL_FNS(Entities, AdvancedfxIAfxHookSource, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxIEntity*)

	ADVANCEDFX_ILIST_DECL_FNS(Jits, AdvancedfxIAfxHookSource, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxIJit*)
};

struct AdvancedfxIAfxHookSource
{
	size_t RefCountValid;
	struct AdvancedfxIAfxHookSourceVtable* Vtable;
};

// AdvancedfxFactoryFn: ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN, AdvancedfxIAfxHookSource -> AdvancedfxIFactory
#define ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3F3644B5,0xDF45,0x406B,0xB0B4,0x0E,0xFD,0xA7,0xA9,0x42,0xE4)

#endif
