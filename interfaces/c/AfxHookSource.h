#ifndef ADVANCEDFX_AFXHOOKSOURCE_H
#define ADVANCEDFX_AFXHOOKSOURCE_H

#include "AdvancedfxTypes.h"

/*
////////////////////////////////////////////////////////////////////////////////


typedef AdvancedfxInt32 AdvancedfxEntityHandle;

struct AdvancedfxQVector {
	float x;
	float y;
	float z;
};

struct AdvancedfxQAngles {
	float x;
	float y;
	float z;
};


// Entities ////////////////////////////////////////////////////////////////////


struct AdvancedfxIEntityVtable
{
	AdvancedfxEntityHandle(*GetHandle)(struct AdvancedfxIEntity* This);

	AdvancedfxInt32(*GetEntIndex)(struct AdvancedfxIEntity* This);

	AdvancedfxIEntity*(*GetActiveWeapon)(struct AdvancedfxIEntity* This);

	void(*GetAbsOrigin)(struct AdvancedfxIEntity* This, struct AdvancedfxQVector * outAbsOrigin);

	void(*GetAbsAngles)(struct AdvancedfxIEntity* This, struct AdvancedfxQAngles * outAbsAbgkles);

	AdvancedfxQVector(*EyePosition)(struct AdvancedfxIEntity* This);

	AdvancedfxQAngles(*EyeAngles)(struct AdvancedfxIEntity* This);

	AdvancedfxInt32(*LookUpAttachment)(struct AdvancedfxIEntity* This, const char * attachmentName);

	AdvancedfxBool(*GetAttachment)(struct AdvancedfxIEntity* This, int number, struct advancedfx_QVector * outOrigin, struct advancedfx_QAngles * outAngles);

	void* (*GetRawIClientEntity)(struct AdvancedfxIEntity* This);
};

struct AdvancedfxIEntity {
	struct AdvancedfxIEntityVtable* Vtable;
};

typedef void* AdvancedfxEntityListIterator;

struct AdvancedfxIEntityEventHandlerVtable
{
	void (*EntityEvent)(struct AdvancedfxIEntityEventHandler* This, struct AdvancedfxIEntity* entity);
};

struct AdvancedfxIEntityEventHandler
{
	struct AdvancedfxIEntityEventHandlerVtable* Vtable;
};

struct AdvancedfxIEntitiesVtable
{
	void (*OnEntityCreatedAdd)(struct AdvancedfxIEntities* This, struct AdvancedfxIEntityEventHandler* handler);

	void (*OnEntityCreatedRemove)(struct AdvancedfxIEntities* This, struct AdvancedfxIEntityEventHandler* handler);

	void (*OnEntityDeletedAdd)(struct AdvancedfxIEntities* This, struct AdvancedfxIEntityEventHandler* handler);

	void (*OnEntityDeletedRemove)(struct AdvancedfxIEntities* This, struct AdvancedfxIEntityEventHandler* handler);

	struct AdvancedfxIEntity* (*EntityListBegin)(struct AdvancedfxIEntities* This);

	struct AdvancedfxIEntit* (*EntityListNext)(struct AdvancedfxIEntities* This);

	struct AdvancedfxIEntity* (*EntityFromHandle)(struct AdvancedfxIEntities* This, AdvancedfxEntityHandle handle);

	struct AdvancedfxIEntity* (*EntityFromIndex)(struct AdvancedfxIEntities* This, int index);

	struct AdvancedfxIEntity* (*EntityFromSpecKey)(struct AdvancedfxIEntities* This, int keyNumber);
};

struct AdvancedfxIEntities
{
	struct AdvancedfxIEntitiesVtable* Vtable;
};

#define ADVANCEDFX_IENTITIES_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xA7888B75,0x79DF,0x47B9,0xB801,0x03,0x40,0xF4,0x88,0xB5,0x8E)



// Engine view overrides ///////////////////////////////////////////////////////


struct AdvancedfxISetupEngineViewOverrideHandlerVtable
{
	AdvancedfxBool(*Handle)(struct AdvancedfxISetupEngineViewOverride * This, AdvancedfxBool lastResult, struct AdvancedfxQAngles* inOutOrigin, struct AdvancedfxQVector* inOutAngles, AdvancedfxFloat* inOutfov);
};

struct AdvancedfxISetupEngineViewOverrideHandler
{
	struct AdvancedfxISetupEngineViewOverrideHandlerVtable* Vtable;
};

struct AdvancedfxISetupEngineViewOverrideVtable
{
	struct AdvancedfxISetupEngineViewOverrideHandler* (*GetOverride)(struct AdvancedfxISetupEngineViewOverride* This);

	void (*SetOverride)(struct AdvancedfxISetupEngineViewOverride* This, struct AdvancedfxISetupEngineViewOverrideHandler* handler);
};

struct AdvancedfxISetupEngineViewOverride
{
	struct AdvancedfxISetupEngineViewOverrideVtable* Vtable;
};


#define ADVANCEDFX_ISETUPENGINEVIEWOVERRIDES_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x8164851B,0xF7F1,0x4F25,0x9A38,0x21,0x6E,0x43,0x79,0x88,0x22)

*/


// AdvancedfxLocale ////////////////////////////////////////////////////////////

struct AdvancedfxLocaleVtable
{
	const char* (*GetLocaleName)(struct AdvancedfxLocale* This);
};

struct AdvancedfxLocale
{
	struct AdvancedfxLocaleVtable* Vtable;
};

#define ADVANCEDFX_LOCALE_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x20B4EE1C,0xB03F,0x4826,0x9AE2,0x1A,0xF2,0x8E,0x36,0xE9,0x79)


// AdvancedfxLoadModule ////////////////////////////////////////////////////////

struct AdvancedfxLoadModuleVtable
{
	void(*LoadModule)(struct AdvancedfxLoadModule* This, const char* filePath);
};

struct AdvancedfxLoadModule {
	struct AdvancedfxLoadModuleVtable* Vtable;
};

#define ADVANCEDFX_LOADMODULE_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x6D9981A3,0x6132,0x431B,0x9A40,0x61,0x96,0xBD,0x37,0x7C,0x1F)


// AdvancedfxConsole ///////////////////////////////////////////////////////////


struct AdvancedfxConsoleCommandArgsVtable
{
	AdvancedfxUInt32(*ArgC)(struct AdvancedfxConsoleCommandArgs* This);

	AdvancedfxUInt32(*ArgV)(struct AdvancedfxConsoleCommandArgs* This, AdvancedfxUInt32 index);
};

struct AdvancedfxConsoleCommandArgs
{
	struct AdvancedfxConsoleCommandArgsVtable* Vtable;
};

struct AdvancedfxConsoleCommandVtable
{
	void (*Execute)(struct  AdvancedfxConsoleCommand* This, struct AdvancedfxConsoleCommandArgs* args);
};

struct AdvancedfxConsoleCommand
{
	struct AdvancedfxConsoleCommandVtable* Vtable;
};

#define ADVANCEDFX_CONSOLE_LOG_LEVEL_WARNING 0
#define ADVANCEDFX_CONSOLE_LOG_LEVEL_MESSAGE 1
#define ADVANCEDFX_CONSOLE_LOG_LEVEL_VERBOSE 2

struct AdvancedfxConsolePrinterVtable {
	void (*Print)(struct AdvancedfxConsolePrinter* This, const char* text, int logLevel);
};

struct AdvancedfxConsolePrinter {
	struct AdvancedfxConsolePrinterVtable* Vtable;
};

struct AdvancedfxConsoleVtable
{
	void (*AddCommand)(struct AdvancedfxConsole* This, const char* name, struct AdvancedfxConsoleCommand* command);

	void (*RemoveCommand)(struct AdvancedfxConsole* This, const char* name, struct AdvancedfxConsoleCommand* command);

	void (*Print)(struct AdvancedfxConsole* This, const char* text, int logLevel);

	void (*Execute)(struct AdvancedfxConsole* This, const char* text);

	void (*AddPrinter)(struct AdvancedfxConsole* This, struct AdvancedfxConsolePrinter* printer);
	void (*RemovePrinter)(struct AdvancedfxConsole* This, struct AdvancedfxConsolePrinter* printer);
};

struct AdvancedfxConsole {
	struct AdvancedfxConsoleVtable* Vtable;
};

#define ADVANCEDFX_CONSOLE_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x4B336709,0x1665,0x4FD5,0x98DC,0xCC,0x1E,0x9C,0x73,0x59,0x76)



#endif
