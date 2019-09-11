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

#define ADVANCEDFX_IFACTORY_DEFINE_UUID(name) ADVANCEDX_DEFINE_UUID(0x3BBBD18B,0x0100,0x4761,0xA588,0x9D,0xF8,0x8F,0xF0,0x52,0xAE)

struct AdvancedfxIFactory {

	void (*Factory)(struct AdvancedfxIFactory * This, AdvancedfxUuid uuid, void* arg);
};

typedef void * AdvancedfxFactoryFn(AdvancedfxUuid uuid, void * arg);

#define ADVANCEDFX_FACTORY_FN_IDENTIFIER AdvancedfxFactory

#ifdef __cplusplus
#define ADVANCEDFX_EXTERNC extern "C"
#else
#define ADVANCEDFX_EXTERNC
#endif

#define ADVANCEDFX_FACTORY_FN ADVANCEDFX_EXTERNC struct AdvancedfxIObject * ADVANCEDFX_FACTORY_FN_IDENTIFIER (struct AdvancedfxIObject * obj)

#endif
