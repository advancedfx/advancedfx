#ifndef ADVANCEDFX_INTERFACE_H
#define ADVANCEDFX_INTERFACE_H
#include "AfxTypes.h"

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define ADVANCEFDX_DLL_EXPORT __attribute__ ((dllexport))
#define ADVANCEFDX_DLL_IMPORT __attribute__ ((dllimport))
#else
#define ADVANCEFDX_DLL_EXPORT __declspec(dllexport)
#define ADVANCEFDX_DLL_IMPORT __declspec(dllimport)
#endif
#define ADVANCEFDX_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define ADVANCEFDX_DLL_EXPORT __attribute__ ((visibility ("default")))
#define ADVANCEFDX_DLL_IMPORT __attribute__ ((visibility ("default")))
#define ADVANCEFDX_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ADVANCEFDX_DLL_EXPORT
#define ADVANCEFDX_DLL_IMPORT
#define ADVANCEFDX_DLL_LOCAL
#endif
#endif

ADVANCEDFX_INOTIFY_DECL(AdvancedfxIFactoryNotify, struct AdvancedfxIFactory*)

struct AdvancedfxIFactoryVtable {
	void(*Delete)(struct AdvancedfxIFactory* This);

	void(*NotifyBeforeDeleteBegin)(struct AdvancedfxIFactory* This, struct AdvancedfxIFactoryNotify* notify);

	void (*NotifyBeforeDeleteEnd)(struct AdvancedfxIFactory* This, struct AdvancedfxIFactoryNotify* notify);

	struct AdvancedfxUuid(*GetUuid)(struct AdvancedfxIFactory* This);

	void* (*Factory)(struct AdvancedfxIFactory* This, void* arg);
};

struct AdvancedfxIFactory
{
	size_t RefCountValid;
	struct AdvancedfxIFactoryVtable* Vtable;
};

// AdvancedfxFactoryFn: ADVANCEDFX_IFACTORY_LIST_UUID, AdvancedfxCore -> AdvancedfxIFactoryLists
#define ADVANCEDFX_IFACTORY_ILIST_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x0FE7472B,0xF4EC,0x4642,0x827D,0xB8,0xDD,0x8E,0xDD,0xCF,0x28)

ADVANCEDFX_INOTIFY_DECL(AdvancedfxIUuidListNotify, struct AdvancedfxIUuidList*)

struct AdvancedfxIUuidListVtable {
	void(*Delete)(struct AdvancedfxIFactoryListVtable* This);

	ADVANCEDFX_ILIST_DECL_FNS(Uuids, AdvancedfxIFactoryList, AdvancedfxIAfxHookSourceNotify, struct AdvancedfxUuid)
};

struct AdvancedfxIUuidList
{
	size_t RefCountValid;
	struct AdvancedfxIUuidListVtable* Vtable;
};

typedef void * AdvancedfxFactoryFn(struct AdvancedfxUuid uuid, void * arg);

#define ADVANCEDFX_FACTORY_FN_IDENTIFIER AdvancedfxFactory

#ifdef __cplusplus
#define ADVANCEDFX_EXTERNC extern "C"
#else
#define ADVANCEDFX_EXTERNC
#endif

#define ADVANCEDFX_FACTORY_FN ADVANCEDFX_EXTERNC struct AdvancedfxIObject * ADVANCEDFX_FACTORY_FN_IDENTIFIER (struct AdvancedfxIObject * obj)

#endif
