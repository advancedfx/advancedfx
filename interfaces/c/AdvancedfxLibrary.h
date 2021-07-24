#ifndef ADVANCEDFX_LIBRARY_H
#define ADVANCEDFX_LIBRARY_H
#include "AdvancedfxTypes.h"

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



//
// Factory


#define ADVANCEDFX_LIBRARY_FN_IDENTIFIER AdvancedfxModuleInit

#ifdef __cplusplus
#define ADVANCEDFX_EXTERNC extern "C"
#else
#define ADVANCEDFX_EXTERNC
#endif

#define ADVANCEDFX_FACTORY_FN ADVANCEDFX_EXTERNC void ADVANCEDFX_LIBRARY_FN_IDENTIFIER (struct AdvancedfxRegistry * registry)



#endif
