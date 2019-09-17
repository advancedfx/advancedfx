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

struct AdvancedfxIEntityDeleting
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIEntityDeleting* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIEntityDeleting* This);

	void (*Deleting)(struct AdvancedfxIEntityDeleting* This, struct AdvancedfxIEntity* entity);
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

struct AdvancedfxIEntity_IListNode;

struct AdvancedfxIEntity_IListNodeNotification
{
	void (*Notfication)(struct AdvancedfxIEntity_IListNodeNotification* This, struct AdvancedfxIEntity_IListNode* node);
};

struct AdvancedfxIEntity_IListNodeNotificationNode;

struct AdvancedfxISetupEngineViewOverride
{
	bool(* Override)(struct AdvancedfxISetupEngineViewOverride* This, bool lastResult, struct AdvancedfxQVector * inOutOrigin, struct AdvancedfxQAngles * inOutAngles, float * inOutfov);
};

struct AdvancedfxISetupEngineViewOverride_IListNode;

struct AdvancedfxISetupEngineViewOverride_IListNodeNotification
{
	void (*Notfication)(struct AdvancedfxISetupEngineViewOverride_IListNodeNotification* This, struct AdvancedfxISetupEngineViewOverride_IListNode* node);
};

struct AdvancedfxISetupEngineViewOverride_IListNodeNotificationNode;

struct AdvancedfxIAfxHookSourceFactoryConnected
{
	void (*Connected)(struct AdvancedfxIAfxHookSourceFactoryConnected* This, struct AdvancedfxUuid uuid, struct AdvancedfxIFactory* factory);
};


struct AdvancedfxISetupEngineViewOverride_IListNode_Vtable
{
	struct AdvancedfxISetupEngineViewOverride_IListNode* (*SetupEngineViewHandlers_SetNext)(struct AdvancedfxISetupEngineViewOverride_IListNode* This, struct AdvancedfxISetupEngineViewOverride_IListNode* value);

	struct AdvancedfxISetupEngineViewOverride_IListNode* (*SetupEngineViewHandlers_SetPrevious)(struct AdvancedfxISetupEngineViewOverride_IListNode* This, struct AdvancedfxISetupEngineViewOverride_IListNode* value);

	struct AdvancedfxISetupEngineViewOverride_IListNode* (*SetupEngineViewHandlers_SetValue)(struct AdvancedfxISetupEngineViewOverride_IListNode* This, struct AdvancedfxISetupEngineViewOverride* value);
};

struct AdvancedfxISetupEngineViewOverride_IListNode
{
	AdvancedfxISetupEngineViewOverride_IListNode_Vtable* Vtable;

	void* Data;
};

struct AdvancedfxIAfxHookSource
{
	void (*BeginDeletingNotification)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceDeleting* afxHookSourceDeleting);

	void (*EndDeletingNotification)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIAfxHookSourceDeleting* afxHookSourceDeleting);

	//
	// Factory management:

	/**
	 * Dll is loaded and queried for ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN.
	 */
	AdvancedfxIAfxHookSourceDll * (*LoadDll)(struct AdvancedfxIAfxHookSource* This, const char* filePath);

	void (*ConnectFactory)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid, struct AdvancedfxIFactory* factory);

	struct AdvancedfxIFactory* (*GetFactory)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid);

	void (*BeginFactoryConnected)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid, struct AdvancedfxIAfxHookSourceFactoryConnected* connected);

	void (*EndFactoryConnected)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxUuid uuid, struct AdvancedfxIAfxHookSourceFactoryConnected* connected);

	//
	// Tools:

	struct AdvancedfxILocale * (*GetLocale)(struct AdvancedfxIAfxHookSource* This);

	//
	// SetupEngineViewHandlers:

	struct AdvancedfxISetupEngineViewOverride_IListNode * SetupEngineViewHandlers;

	//
	// Entities:

	struct AdvancedfxIEntity_IListNode* (*Entities_Next)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNode* node);

	struct AdvancedfxIEntity_IListNode* (*Entities_Previous)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNode* node);

	AdvancedfxIEntity* (*Entities_Value)(struct AdvancedfxIAfxHookSource* This, AdvancedfxIEntity_IListNode* node);

	struct AdvancedfxIEntity_IListNodeNotificationNode* (*Entities_BeginInserted)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNodeNotification* notification);

	void (*Entities_EndInserted)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNodeNotificationNode* notificationNode);

	struct AdvancedfxIEntity_IListNodeNotificationNode* (*Entities_BeginDeleting)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNodeNotification* notification);

	void (*Entities_EndDeleting)(struct AdvancedfxIAfxHookSource* This, struct AdvancedfxIEntity_IListNodeNotificationNode* notificationNode);

	AdvancedfxIEntity*(*EntityFromHandle)(struct AdvancedfxIAfxHookSource** This, AdvancedfxEntityHandle handle);

	AdvancedfxIEntity*(*EntityFromIndex)(struct AdvancedfxIAfxHookSource** This, int index);

	AdvancedfxIEntity*(*EntityFromSpecKey)(struct AdvancedfxIAfxHookSource** This, int keyNumber);

	//
	// Jits

	struct AdvancedxIJit_IListNode* (*Jits_Next)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNode* node);

	struct AdvancedxIJit_IListNode* (*Jits_Previous)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNode* node);

	AdvancedxIJit* (*Jits_Value)(struct AdvancedfxIAfxHookSource* This, AdvancedxIJit_IListNode* node);

	void (*Jits_Delete)(struct AdvancedfxIAfxHookSource* This, AdvancedxIJit_IListNode* node);

	struct AdvancedfxIListNode* (*Jits_InsertBefore)(struct AdvancedfxIAfxHookSource* This, AdvancedxIJit_IListNode* node, struct AdvancedxIJit* value);

	struct AdvancedfxIListNode* (*Jits_InsertAfter)(struct AdvancedfxIAfxHookSource* This, AdvancedxIJit_IListNode* node, struct AdvancedxIJit* value);

	AdvancedxIJit_IListNodeNotificationNode* (*Jits_BeginInserted)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNodeNotification* notification);

	void (*Jits_EndInserted)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNodeNotificationNode* notificationNode);

	AdvancedxIJit_IListNodeNotificationNode* (*Jits_BeginDeleting)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNodeNotification* notification);

	void (*Jits_EndDeleting)(struct AdvancedfxIAfxHookSource* This, struct AdvancedxIJit_IListNodeNotificationNode* notificationNode);

};

// AdvancedfxFactoryFn: ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN, AdvancedfxIAfxHookSource -> AdvancedfxIAfxHookSourceDll
#define ADVANCEDFX_IAFXHOOKSOURCEDLL_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3F3644B5,0xDF45,0x406B,0xB0B4,0x0E,0xFD,0xA7,0xA9,0x42,0xE4)

struct AdvancedfxIAfxHookSourceDllDeleting
{
	void (*Deleting)(struct AdvancedfxIAfxHookSourceDllDeleting* This, struct AdvancedfxIAfxHookSourceDll* dll);
};

struct AdvancedfxIAfxHookSourceDll
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIAfxHookSourceDll* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIAfxHookSourceDll* This);

	void (*BeginDeletingNotification)(struct AdvancedfxIAfxHookSourceDll* This, struct AdvancedfxIAfxHookSourceDllDeleting* dllDeleting);

	void (*EndDeletingNotification)(struct AdvancedfxIAfxHookSourceDll* This, struct AdvancedfxIAfxHookSourceDllDeleting* dllDeleting);

	void (*Connect)(struct AdvancedfxIAfxHookSourceDll* This, struct AdvancedfxIAfxHookSource* afxHookSource);
};

#endif
