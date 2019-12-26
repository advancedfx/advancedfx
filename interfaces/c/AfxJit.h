#ifndef ADVANCEDFX_JIT_H
#define ADVANCEDFX_JIT_H
#include "AfxTypes.h"
#include "AfxInterface.h"

#define ADVANCEDFX_JIT_CONTEXT_CODE_ISTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xEFB58A59,0x536D,0x4466,0x9E7D,0x8F,0x4F,0x21,0xA2,0xB2,0x16)

#define ADVANCEDFX_JIT_CONTEXT_LOG_ERROR_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xFAFB5DE5,0x30B4,0x4D99,0x9861,0x0E,0xA6,0x6A,0xB9,0xB6,0x37)

#define ADVANCEDFX_JIT_CONTEXT_LOG_WARNING_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x4668D6CE,0x0422,0x4B59,0xBD9F,0x32,0xB1,0x33,0x63,0xF5,0xE4)

#define ADVANCEDFX_JIT_CONTEXT_LOG_MESSAGE_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xD8BD0A7F,0x6F5D,0x4F82,0xB7B1,0x11,0x5A,0x1E,0xC6,0x4B,0xC7)

struct AdvancedfxIJitContextVtable
{
	void(*Delete)(struct AdvancedfxIJitContext* This);

	ADVANCEDFX_ILIST_DECL_FNS(Uuids, AdvancedfxIJitContext, AdvancedfxIJitContextNotify, struct AdvancedfxUuid)

	AdvancedfxIReference* (*Get)(struct AdvancedfxIJitContext* This, struct AdvancedfxUuid);
};

struct AdvancedfxIJitContext {
	size_t RefCountValid;
	struct AdvancedfxIJitContextVtable* Vtable;
};

struct AdvancedxIJitVtable {
	//
	// Implement AdvancedfxIInterfaceVTable:

	void(*Delete)(struct AdvancedxIJit* This);

	struct AdvancedfxUuid(*GetUuid)(struct AdvancedxIJit* This);

	//
	// Own:

	void (*Execute)(struct AdvancedxIJit* This, AdvancedfxIJitContext* context);
};

struct AdvancedxIJit
{
	//
	// Implement AdvancedfxIInterface:

	size_t RefCountValid;
	struct AdvancedfxIFactoryVtable* Vtable;

	//
	// Own:
};


#endif