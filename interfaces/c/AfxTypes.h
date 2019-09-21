#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H

#include <stddef.h>


// Remarks about Invalidatable-Reference-Counting //////////////////////////////


// - AdvancedfxIReference (or compatible) objects must only be allocated and freed from the single AfxCore object.
//
// - Unless explictely stated otherwise, AdvancedfxIReference* pointers can be assumed to be non-null.
//
// - Objects returned to you from a function need to be released with ADVANCEDFX_RELEASE, if you don't return them as well, unless stated otherwise.
//
// - Function arguments are only referenced for the duration of the call, if you need a reference after that, use ADVANCEDFX_ADDREF and ADVANCEDFX_RELEASE accordingly.
//
// - Always ensure that a reference is valid e.g. using ADVANCEDFX_VALID, before calling a function in it's Vtable.
//
// - Unless a function is explictely marked with ADVANCEDFX_NO_INVALIDATION you should assume that any references used can be invalid after the call!
//
// - It is recommended to trigger clean-up code, as soon as you observe an invalidate reference: E.g. if you depend on a mandatory reference being valid and it becomes invalid, ADVANCEDFX_DELETE yourself.
//
// - We recommend to use the macros in this header to work with the references:
//   - ADVANCEDFX_VALID - ADAVANCEDFX_FALSE if invalid, other value otherwise
//   - ADVANCEDFX_ADDREF - Increment reference count (e.g. if you need to hold longer onto a passed reference that the current scope.
//   - ADVANCEDFX_DELETE - Calls Delete on a valid reference, otherwise does nothing
//   - ADVANCEDFX_RELEASE - Decrements refernce count. If count becomes zero ADVANCEDFX_DELETE, and the reference is freed using afxCore.
//   - ADVANCEDFX_CREATE - Creates reference of a given size (must be at least sizeof(AdvancedfxIReference) and initializes it (but does not initalize other memory allocated).
//   - ADVANCEDFX_CREATE_INVALID - Creates an invalid reference.


////////////////////////////////////////////////////////////////////////////////


typedef unsigned char AdvancedfxBool;

#define ADAVANCEDFX_FALSE 0
#define ADAVANCEDFX_TRUE 1

struct AdvancedfxUuid
{
	unsigned char ucF;
	unsigned char ucE;
	unsigned char ucD;
	unsigned char ucC;
	unsigned char ucB;
	unsigned char ucA;
	unsigned char uc9;
	unsigned char uc8;
	unsigned char uc7;
	unsigned char uc6;
	unsigned char uc5;
	unsigned char uc4;
	unsigned char uc3;
	unsigned char uc2;
	unsigned char uc1;
	unsigned char uc0;
};

#define ADVANCEDFX_UUID_APPLY_FN(fn,time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) fn(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0)

#define ADVANCEDFX_UUID_BYTE_F(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_low & 0xff000000) >> 12))
#define ADVANCEDFX_UUID_BYTE_E(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_low & 0x00ff0000) >> 8))
#define ADVANCEDFX_UUID_BYTE_D(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_low & 0x0000ff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_C(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_low & 0x000000ff)))
#define ADVANCEDFX_UUID_BYTE_B(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_mid & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_A(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_mid & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_9(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_hi_and_version & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_8(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((time_hi_and_version & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_7(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((clock_seq_hi_and_res__clock_seq_low & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_6(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)((clock_seq_hi_and_res__clock_seq_low & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_5(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc5)
#define ADVANCEDFX_UUID_BYTE_4(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc4)
#define ADVANCEDFX_UUID_BYTE_3(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc3)
#define ADVANCEDFX_UUID_BYTE_2(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc2)
#define ADVANCEDFX_UUID_BYTE_1(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc1)
#define ADVANCEDFX_UUID_BYTE_0(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((unsigned char)node_uc0)

#define ADVANCEDFX_UUID_VAR(name,uuid_fn) struct AdvancedfxUuid name = { \
	uuid_fn(ADVANCEDFX_UUID_BYTE_F) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_E) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_D) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_C) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_B) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_A) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_9) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_8) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_7) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_6) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_5) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_4) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_3) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_2) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_1) \
	, uuid_fn(ADVANCEDFX_UUID_BYTE_0) \
}

#define ADVANCEDFX_UUID_EQUAL(uuid_fn,var) ( \
	uuid_fn(ADVANCEDFX_UUID_BYTE_F) == var.ucF \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_E) == var.ucE \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_D) == var.ucD \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_C) == var.ucC \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_B) == var.ucB \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_A) == var.ucA \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_9) == var.uc9 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_8) == var.uc8 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_7) == var.uc7 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_6) == var.uc6 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_5) == var.uc5 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_4) == var.uc4 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_3) == var.uc3 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_2) == var.uc2 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_1) == var.uc1 \
	&& uuid_fn(ADVANCEDFX_UUID_BYTE_0) == var.uc0 \
)

#define ADVANCEFX_CMP_UUID_VARS(var_a,var_b) (\
	var_a.ucF != var_b.ucF ? (var_a.ucF > var_b.ucF ? 1 : -1) \
	: var_a.ucE != var_b.ucE ? (var_a.ucE > var_b.ucE ? 1 : -1) \
	: var_a.ucD != var_b.ucD ? (var_a.ucD > var_b.ucD ? 1 : -1) \
	: var_a.ucC != var_b.ucC ? (var_a.ucC > var_b.ucC ? 1 : -1) \
	: var_a.ucB != var_b.ucB ? (var_a.ucB > var_b.ucB ? 1 : -1) \
	: var_a.ucA != var_b.ucA ? (var_a.ucA > var_b.ucA ? 1 : -1) \
	: var_a.uc9 != var_b.uc9 ? (var_a.uc9 > var_b.uc9 ? 1 : -1) \
	: var_a.uc8 != var_b.uc8 ? (var_a.uc8 > var_b.uc8 ? 1 : -1) \
	: var_a.uc7 != var_b.uc7 ? (var_a.uc7 > var_b.uc7 ? 1 : -1) \
	: var_a.uc6 != var_b.uc6 ? (var_a.uc6 > var_b.uc6 ? 1 : -1) \
	: var_a.uc5 != var_b.uc5 ? (var_a.uc5 > var_b.uc5 ? 1 : -1) \
	: var_a.uc4 != var_b.uc4 ? (var_a.uc4 > var_b.uc4 ? 1 : -1) \
	: var_a.uc3 != var_b.uc3 ? (var_a.uc3 > var_b.uc3 ? 1 : -1) \
	: var_a.uc2 != var_b.uc2 ? (var_a.uc2 > var_b.uc2 ? 1 : -1) \
	: var_a.uc1 != var_b.uc1 ? (var_a.uc1 > var_b.uc1 ? 1 : -1) \
	: var_a.uc0 != var_b.uc0 ? (var_a.uc0 > var_b.uc0 ? 1 : -1) \
	: 0 \
)

struct AdvancedfxVersion {
	unsigned long major;
	unsigned long minor;
	unsigned long revison;
	unsigned long build;
};

#define ADVANCEDFX_NULLPTR ((void *)0)

struct AdvancedfxIReferenceVtable
{
	//
	// Own:

	void (*Delete)(struct AdvancedfxIReference* This);
};

struct AdvancedfxIReference
{
	//
	// Own:

	size_t RefCountValid;
	struct AdvancedfxIReferenceVtable* Vtable;
};

#define ADVANCEDFX_VALID(pRef) (0 == (pRef->RefCountValid & 0x1))

#define ADVANCEDFX_ADDREF(pRef) ref.RefCountValid = (pRef->RefCountValid + 0x2) | (pRef->RefCountValid & 0x1);

#define ADVANCEDFX_DELETE(pAfxCore,pRef) if(ADVANCEDFX_VALID(pRef)) { ref.Vtable.Delete(ref); ref |= 0x1; }

#define ADVANCEDFX_RELEASE(pAfxCore,pRef) pRef->RefCountValid = (pRef->RefCountValid - 0x2) | (pRef->RefCountValid & 0x1); if(0 == (pRef->RefCountValid >> 1)) { ADVANCEDFX_DELETE(pRef) pAfxCore.FreeRef((AdvancedfxIReference*)ref); }

#define ADVANCEDFX_CREATE(pAfxCore,refType,pVtable,pRef) (pRef = pAfxCore->MemAlloc(sizeof(refType)), pRef->RefCountValid = 0x2, pRef->Vtable = pVtable, pRef) 

#define ADVANCEDFX_CREATE_INVALID(pAfxCore,pRef) (pRef = pAfxCore->MemAlloc(sizeof(struct AdvancedfxIReference)), pRef->RefCountValid = (0x2 | 0x1), pRef) 

struct AdvancedfxCore
{
	void* (*MemAlloc)(size_t bytes);
	void (*Free)(struct AdvancedfxIReference* ref);
};

struct AdvancedfxIString_Vtable
{
	//
	// Implement AdvancedfxIReferenceVtable:

	void (*Delete)(struct AdvancedfxIString* ref);

	//
	// Own:

	const char * (*Get)(struct AdvancedfxIString* This);
};

struct AdvancedfxIString
{
	//
	// Implement AdvancedfxIReference:

	size_t RefCountValid;
	struct AdvancedfxIString_Vtable* Vtable;

	//
	// Own:
};

struct AdvancedfxISetString_Vtable
{
	//
	// Implement AdvancedfxIReferenceVtable:

	void (*Delete)(struct AdvancedfxISetString* ref);

	//
	// Own:

	void (*Set)(struct AdvancedfxISetString* This, const char* value);
};

struct AdvancedfxISetString
{
	//
	// Implement AdvancedfxIReference:

	size_t RefCountValid;
	struct AdvancedfxISetString_Vtable* Vtable;

	//
	// Own:
};


#define ADVANCEDFX_INOTIFY_DECL(type_name,value_type) \
struct type_name ##Vtable { \
	void (*Delete)(struct type_name* This); \
	AdvancedfxBool (*Notify)(struct type_name* This, value_type value); \
}; \
struct type_name { \
	size_t RefCountValid; \
	struct type_name##Vtable* Vtable; \
};

#define ADVANCEDFX_INOTIFY(prefix) \
struct prefix ##Notify

#define ADVANCEDFX_ILIST_DECL_FNS(prefix,this_type_name,notify_type_name,item_type) \
	AdvancedfxBool (*prefix ##Begin)(struct this_type_name* This); \
	AdvancedfxBool (*prefix ##End)(struct this_type_name* This); \
	AdvancedfxBool (*prefix ##Next)(struct this_type_name* This); \
	AdvancedfxBool (*prefix ##Previous)(struct this_type_name* This); \
	void (*prefix ##Insert)(struct this_type_name* This, item_type value); \
	void (*prefix ##BeginNotifyAfterInsert)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##EndNotifyAfterInsert)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##Delete)(struct this_type_name* This); \
	void (*prefix ##BeginNotifyBeforeDelete)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##EndNotifyBeforeDelete)(struct this_type_name* This, struct notify_type_name* notify); \
	item_type (*prefix ##Get)(struct this_type_name* This); \
	void (*prefix ##Set)(struct this_type_name* This, item_type value); \
	void (*prefix ##BeginNotifyBeforeSet)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##EndNotifyBeforeSet)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##BeginNotifyAfterSet)(struct this_type_name* This, struct notify_type_name* notify); \
	void (*prefix ##EndNotifyAfterSet)(struct this_type_name* This, struct notify_type_name* notify);

#endif
