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

// AdvancedfxFactoryFn: ADVANCEDFX_IFACTORY_UUID, AdvancedfxIFactory -> AdvancedfxIFactory
#define ADVANCEDFX_IFACTORY_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x3F3644B5,0xDF45,0x406B,0xB0B4,0x0E,0xFD,0xA7,0xA9,0x42,0xE4)

ADVANCEDFX_IDELETING_DECL(AdvancedfxIFactory, struct AdvancedfxIFactory*)

struct AdvancedfxIFactoryVtable {
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIFactory* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIFactory* This);

	void* (*Factory)(struct AdvancedfxIFactory* This, struct AdvancedfxUuid uuid, void* arg);
};

struct AdvancedfxIFactory
{
	AdvancedfxIFactoryVtable* Vtable;
	
	ADVANCEDFX_IDELETING(AdvancedfxIFactory)* Deleting;
};


// AdvancedfxFactoryFn: ADVANCEDFX_IFACTORY_LIST_UUID, AdvancedfxIFactory -> AdvancedfxIFactoryLists
#define ADVANCEDFX_IFACTORY_ILIST_UUID ADVANCEDFX_UUID(0x0FE7472B,0xF4EC,0x4642,0x827D,0xB8DD8EDDCF28)

ADVANCEDFX_ILIST_DECL(AdvancedfxIFactory, AdvancedfxIFactory*)

struct AdvancedfxIFactoryListVtable {
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIFactory* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIFactory* This);

	struct AdvancedfxUuid (*GetUuid)(struct AdvancedfxIFactory* This);

	void (*Factory)(struct AdvancedfxIFactory* This, struct AdvancedfxUuid uuid, void* arg);
};

struct AdvancedfxIFactoryList
{
	AdvancedfxIFactoryListVtable* Vtable;

	ADVANCEDFX_ILIST(AdvancedfxIFactory)* Factories;
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
