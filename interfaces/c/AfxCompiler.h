#ifndef ADVANCEDFX_COMPILER_H
#define ADVANCEDFX_COMPILER_H
#include "AfxTypes.h"


typedef struct AdvancedfxIString AdvancedfxCompilerContextCode;
#define AdvancedxICompilerContextCode_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xEFB58A59,0x536D,0x4466,0x9E7D,0x8F,0x4F,0x21,0xA2,0xB2,0x16)

typedef struct AdvancedfxISetString AdvancedfxCompilerContextLogError;
#define AdvancedfxCompilerContextLogError_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xFAFB5DE5,0x30B4,0x4D99,0x9861,0x0E,0xA6,0x6A,0xB9,0xB6,0x37)

typedef struct AdvancedfxISetString AdvancedfxCompilerContextLogWarning;
#define AdvancedfxCompilerContextLogWarning_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x4668D6CE,0x0422,0x4B59,0xBD9F,0x32,0xB1,0x33,0x63,0xF5,0xE4)

typedef struct AdvancedfxISetString AdvancedfxCompilerContextLogMessage;
#define AdvancedfxCompilerContextLogMessage_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xD8BD0A7F,0x6F5D,0x4F82,0xB7B1,0x11,0x5A,0x1E,0xC6,0x4B,0xC7)


struct AdvancedxICompilerVtable
{
	struct AdvancedfxIReferenced* (*GetAsReferenced)(struct AdvancedxICompiler* This);

	struct AdvancedfxIInterface* (*GetAsInterface)(struct AdvancedxICompiler* This);

	AdvancedfxIRuntimeType*(*Compile)(struct AdvancedxICompiler* This, AdvancedfxIRuntimeType* targetType, AdvancedfxIInterface* context);
};

struct AdvancedxICompiler
{
	struct AdvancedxICompilerVtable* Vtable;
};

#define AdvancedxICompiler_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x620D8BEC,0x3083,0x4043,0xA43F,0xEE,0x6B,0x3F,0xDC,0xFC,0x54)


#endif