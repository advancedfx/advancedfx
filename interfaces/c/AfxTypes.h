#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H



// TODO:
//
// Deleteable type?



// 
// RULES:

// 1)	Interfaces are sealed and not mutable.
//
// 2)	Objects containing or being AdvancedfxIReferenced returned to you from a function must be released, unless you return them as well or stated otherwise.
//
// 3)	Function arguments are only referenced for the duration of the call.

//
// Recommendations:

// - In order to delete an referenced object, use GetSomeChild to traverse to a leaf child.


////////////////////////////////////////////////////////////////////////////////



#if (_WIN32 || _WIN64) && _WIN64 || __GNUC__ && __x86_64__ || __ppc64__
#define ADVANCEDFX_ENV64
#else
#define ADVANCEDFX_ENV32
#endif


#define ADVANCEDFX_NULLPTR ((void *)0)



// Primitive types /////////////////////////////////////////////////////////////


//
// AdvancedfxVoid

#define AdvancedfxVoid_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x6C51D0CF0,x3653,0x43AE,0xB753,0xC1,0x07,0x33,0xDD,0x19,0x9E)


//
// AdvancedfxBool

typedef unsigned char AdvancedfxBool;

#define ADAVANCEDFX_FALSE 0
#define ADAVANCEDFX_TRUE 1

#define AdvancedfxBool_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xF5E02116,0xD623,0x499F,0xABBF,0x2B7EAA4AB8DF)


//
// AdvancedfxUInt8

typedef unsigned char AdvancedfxUInt8;

#define AdvancedfxUInt8_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x5760F402,0xCF8D,0x436F,0xB38A,0x0B,0x34,0x0E,0xE4,0x53,0x6C)


//
// AdvancedfxUInt32

typedef unsigned long AdvancedfxUInt32;

#define AdvancedfxUInt32_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2C0330F6,0xEBD2,0x4C4B,0xAB0F,0x57,0x22,0x08,0x0C,0x59,0x92)


//
// AdvancedfxInt32

typedef signed long AdvancedfxInt32;

#define AdvancedfxInt32_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xBA19A48B,0x6780,0x4391,0x84A5,0x38,0x3D,0x8C,0x55,0xD9,0x0B)


//
// AdvancedfxSize

#ifdef ADVANCEDFX_ENV64
typedef unsigned __int64 AdvancedfxSize;
#else
typedef unsigned int AdvancedfxSize;
#endif

#define AdvancedfxSize_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x0C5AFAAD,0xF01F,0x4D60,0x8051,0x86,0xC9,0xBE,0x78,0xFE,0xEF)


//
// AdvancedfxCString

typedef const char* AdvancedfxCString;


#define AdvancedfxCString_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x9C6932EB,0x17A1,0x49C0,0xA484,0x59,0x08,0xEB,0x59,0xB8,0x69)


////////////////////////////////////////////////////////////////////////////////


//
// AdvancedfxUuid

struct AdvancedfxUuid
{
	AdvancedfxUInt8 ucF;
	AdvancedfxUInt8 ucE;
	AdvancedfxUInt8 ucD;
	AdvancedfxUInt8 ucC;
	AdvancedfxUInt8 ucB;
	AdvancedfxUInt8 ucA;
	AdvancedfxUInt8 uc9;
	AdvancedfxUInt8 uc8;
	AdvancedfxUInt8 uc7;
	AdvancedfxUInt8 uc6;
	AdvancedfxUInt8 uc5;
	AdvancedfxUInt8 uc4;
	AdvancedfxUInt8 uc3;
	AdvancedfxUInt8 uc2;
	AdvancedfxUInt8 uc1;
	AdvancedfxUInt8 uc0;
};

#define ADVANCEDFX_UUID_APPLY_FN(fn,time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) fn(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0)

#define ADVANCEDFX_UUID_BYTE_F(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_low & 0xff000000) >> 12))
#define ADVANCEDFX_UUID_BYTE_E(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_low & 0x00ff0000) >> 8))
#define ADVANCEDFX_UUID_BYTE_D(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_low & 0x0000ff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_C(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_low & 0x000000ff)))
#define ADVANCEDFX_UUID_BYTE_B(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_mid & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_A(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_mid & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_9(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_hi_and_version & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_8(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((time_hi_and_version & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_7(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((clock_seq_hi_and_res__clock_seq_low & 0xff00) >> 4))
#define ADVANCEDFX_UUID_BYTE_6(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)((clock_seq_hi_and_res__clock_seq_low & 0x00ff)))
#define ADVANCEDFX_UUID_BYTE_5(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc5)
#define ADVANCEDFX_UUID_BYTE_4(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc4)
#define ADVANCEDFX_UUID_BYTE_3(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc3)
#define ADVANCEDFX_UUID_BYTE_2(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc2)
#define ADVANCEDFX_UUID_BYTE_1(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc1)
#define ADVANCEDFX_UUID_BYTE_0(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) ((AdvancedfxUInt8)node_uc0)

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


//
// AdvancedfxVersion

struct AdvancedfxVersion {
	AdvancedfxUInt32 major;
	AdvancedfxUInt32 minor;
	AdvancedfxUInt32 revison;
	AdvancedfxUInt32 build;
};



// Lifecycle management ////////////////////////////////////////////////////////


//
// AdvancedfxIReferenced

struct AdvancedfxIReferencedVtable
{
	void(*AddRef)(struct AdvancedfxIReferenced* This);
	void(*Release)(struct AdvancedfxIReferenced* This);
};


struct AdvancedfxIReferenced
{
	struct AdvancedfxIReferencedVtable* Vtable;
};

#define AdvancedfxIReferenced_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2AD68771,0x741B,0x48D9,0xB91E,0xDA,0x60,0x48,0xA6,0x41,0x10)


//
// AdvancedfxIDeletable

struct AdvancedfxIDeletableVtable
{
	void(*Delete)(struct AdvancedfxIDeletable* This);
};


struct AdvancedfxIDeletable
{
	struct AdvancedfxIDeletableVtable* Vtable;
};

#define AdvancedfxIDeletable_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xD1F3343C,0xA0A1,0x48A2,0xA687,0xD6,0xCE,0xFD,0xAE,0x4B,0x27)



// Interfaces //////////////////////////////////////////////////////////////////


//
// AdvancedfxIInterface


struct AdvancedfxIInterfaceVtable
{
	struct AdvancedfxUuid(*GetUuid)(struct AdvancedfxIInterface* This);

	AdvancedfxBool(*GetAs)(struct AdvancedfxIInterface* This, struct AdvancedfxUuid uuid, void* pOut);
};

struct AdvancedfxIInterface
{
	struct AdvancedfxIInterfaceVtable* Vtable;
};

#define AdvancedfxIInterface_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2DD22007,0xA338,0x4A45,0x9FC6,0xBC,0x72,0x8C,0x4C,0xD8,0x43)


//
// AdvancedfxIInterfaceUuidList

struct AdvancedfxIInterfaceUuidListVtable
{
	AdvancedfxSize(*GetInterfaceUuidCount)(struct AdvancedfxIInterfaceUuidList* This);

	struct AdvancedfxUuid(*GetInterfaceUuid)(struct AdvancedfxIInterfaceUuidList* This, AdvancedfxSize index);
};

struct AdvancedfxIInterfaceUuidList
{
	struct AdvancedfxIInterfaceUuidListVtable* Vtable;
};

#define AdvancedfxIInterfaceUuidList_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2DD22007,0xA338,0x4A45,0x9FC6,0xBC,0x72,0x8C,0x4C,0xD8,0x43)



// Memory management ///////////////////////////////////////////////////////////


struct AdvancedfxIMemoryRootVtable
{
	void* (*Alloc)(struct AdvancefxTypeRoot* This, AdvancedfxSize size);
	void (*Free)(struct AdvancefxTypeRoot* This, void* memory);
	void* (*Realloc)(struct AdvancefxTypeRoot* This, void* memory);
};

struct AdvancedfxIMemory
{
	struct AdvancedfxIMemoryRootVtable* Vtable;
};


// Type discovery //////////////////////////////////////////////////////////////


typedef void* AdvancedfdxRuntimeTypeId;


struct AdvancefxITypeRootVtable
{	
	struct AdvancedfxIRuntimeType* (*GetRuntimeTypeById)(struct AdvancefxTypeRoot* This, AdvancedfdxRuntimeTypeId id);

	struct AdvancedfxIRuntimeType* (*MakeRuntimeTypeFromType)(struct AdvancefxTypeRoot* This, struct AdvancedfxIType* type);

	void (*ReleaseRuntimeType)(struct AdvancefxTypeRoot* This, struct AdvancedfxIRuntimeType* id);
};

struct AdvancefxTypeRoot
{
	struct AdvancefxTypeRootVtable* Vtable;
};

#define AdvancedfxTypeRoot_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x0150AF12,0x8740,0x4A84,0x8E85,0x09,0x00,0x47,0xB0,0x27,0x82)


//
// AdvancedfxIRuntimeType

struct AdvancedfxIRuntimeTypeVtable
{
	struct AdvancefxTypeRoot* (*GetRoot)(struct AdvancedfxIRuntimeType* This);
	
	AdvancedfdxRuntimeTypeId(*GetRuntimeId)(struct AdvancedfxIRuntimeType* This);

	struct AdvancedfxIType* (*GetType)(struct AdvancedfxIRuntimeType* This);
};

struct AdvancedfxIRuntimeType
{
	struct AdvancedfxIRuntimeTypeVtable* Vtable;
};

#define AdvancedfxIRuntimeType_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xF5C4EC8D,0x9B5C,0x43F4,0xA43D,0xB1,0xBB,0x87,0x98,0x70,0x6F)


//
// AdvancedfxIType

struct AdvancedfxITypeVtable
{
	struct AdvancedfxUuid(*GetTypeUiid)(struct AdvancedfxIType* This);

	///<returns>ADVANCEDFX_NULLPTR for primitive types</returns>
	void*(*GetTypeInterface)(struct AdvancedfxIType* This);

	AdvancedfxCString(*GetName)(struct AdvancedfxIType* This);

	AdvancedfxCString(*GetDescription)(struct AdvancedfxIType* This, AdvancedfxSize index);
};

struct AdvancedfxIType
{
	struct AdvancedfxITypeVtable* Vtable;
};


//
// AdvancedfxIPointerType

struct AdvancedfxIPointerTypeVtable
{
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	struct AdvancedfxIType*(*GetPointerType)(struct AdvancedfxIPointerType* This);
};

struct AdvancedfxIPointerType
{
	struct AdvancedfxIPointerTypeVtable* Vtable;
};

#define ADVANCEDFX_TYPE_POINTER_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x5D7EDF2A,0x72EA,0x4E88,0x969F,0xE5,0x8D,0x9B,0xEC,0x6C,0x1A)


//
// AdvancedfxITypedef

struct AdvancedfxITypedefVtable
{	
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	struct AdvancedfxUuid(*GetTypedefUuid)(struct AdvancedfxITypedef* type);

	struct AdvancedfxIType* (*GetTypedefType)(struct AdvancedfxITypedef* This);
};

struct AdvancedfxITypedef
{
	struct AdvancedfxITypedefVtable* Vtable;
};

#define ADVANCEDFX_TYPE_TYPEDEF_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xA909AAF1,0x3F57,0x4F04,0xA7AA,0xA3,0xD2,0xA3,0x95,0x50,0x37)


//
// AdvancedfxIStruct

struct AdvancedfxIStructVtable
{	
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	AdvancedfxSize(*GetMemberCount)(struct AdvancedfxIStruct* This);

	struct AdvancedfxIType* (*GetMemberType)(struct AdvancedfxIStruct* This, AdvancedfxSize index);

	AdvancedfxCString(*GetMemberName)(struct AdvancedfxIStruct* This, AdvancedfxSize index);
	
	AdvancedfxCString(*GetMemberDescription)(struct AdvancedfxIStruct* This, AdvancedfxSize index);
};

struct AdvancedfxIStruct
{
	struct AdvancedfxIStructVtable* Vtable;
};

#define ADVANCEDFX_TYPE_STRUCT_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xC216AFB1,0xF75D,0x4A9C,0x962F,0x67,0xE0,0xCF,0x6E,0x65,0xC1)


//
// AdvancedfxIFunction

struct AdvancedfxIFunctionVtable
{	
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	AdvancedfxSize(*GetParameterCount)(struct AdvancedfxIFunction* This);
	
	struct AdvancedfxIType* (*GetParameterType)(struct AdvancedfxIFunction* This, AdvancedfxSize index);

	AdvancedfxCString(*GetParameterName)(struct AdvancedfxIFunction* This, AdvancedfxSize index);

	AdvancedfxCString(*GetParameterDescription)(struct AdvancedfxIFunction* This, AdvancedfxSize index);

	struct AdvancedfxIType* (*GetReturnType)(struct AdvancedfxIFunction* This);
};

struct AdvancedfxIFunction
{
	struct AdvancedfIxFunctionTypeVtable* Vtable;
};

#define ADVANCEFX_TYPE_FUNCTION_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2B440CAE,0xBA61,0x479A,0x9DC4,0xDF,0xF7,0x5C,0x44,0xEF,0x9A)


//
// AdvancedfxITemplateParameter

struct AdvancedfxITemplateParameterVtable
{
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	struct AdvancedfxITemplate* (*GetTemplate)(struct AdvancedfxITemplateParameter* This);

	struct AdvancedfxIType* (*GetType)(struct AdvancedfxITemplateParameter* This);

	AdvancedfxCString(*GetName)(struct AdvancedfxITemplateParameter* This, AdvancedfxSize index);

	AdvancedfxCString(*GetDescription)(struct AdvancedfxITemplateParameter* This, AdvancedfxSize index);
};

struct AdvancedfxITemplateParameter
{
	struct AdvancedfxITemplateParameterVtable* Vtable;
};

#define ADVANCEFX_TYPE_TEMPLATE_PARAMETER_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x6A530606,0x09CA,0x4B52,0x853F,0x3E,0x9C,0x09,0xC8,0x69,0x63)


//
// AdvancedfxITemplate

struct AdvancedfxITemplateVtable
{
	struct AdvancedfxIType* (*AsType)(struct AdvancedfxIRuntimeType* This);

	AdvancedfxSize(*GetParameterCount)(struct AdvancedfxITemplate* This);

	struct AdvancedfxITemplateParameter* (*GetParameter)(struct AdvancedfxITemplate* This, AdvancedfxSize index);
};

struct AdvancedfxITemplate
{
	struct AdvancedfIxFunctionTypeVtable* Vtable;
};

#define AdvancedfxITemplate_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xC6C362DD,0xF08C,0x4453,0xB26B,0x8D,0x2F,0x20,0x54,0xCE,0x23)



// Signals /////////////////////////////////////////////////////////////////////


//
// IEventSink<value_type>

#define ADVANCEDFX_IEventSink_DECL(type_name,value_type) \
struct type_name ##Vtable { \
	void (*Trigger)(struct type_name* This, value_type value); \
}; \
struct type_name { \
	struct type_name##Vtable* Vtable; \
};


//
// IEventSource<event_sink_type>

#define ADVANCEDFX_IEventSource_DECL(type_name,event_sink_type_name) \
struct type_name ##Vtable { \
	void BeginNotify(struct type_name* This, struct event_sink_type_name* eventSink); \
	void EndNotify(struct type_name* This, struct event_sink_type_name* eventSink); \
}; \
struct type_name { \
	struct type_name##Vtable* Vtable; \
};


// Iterators ///////////////////////////////////////////////////////////////////


//
// IReadonlyList<item_type>

#define ADVANCEDFX_IReadonlyList_DECL(type_name,item_type) \
typedef void* type_name ##Node; \
struct type_name ##NodeValue { \
	type_name ##Node Node; \
	item_type Value; \
}; \
struct type_name ##NodeOther { \
	type_name ##Node Node; \
	type_name ##Node Other; \
}; \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeEventSink, type_name ##Node) \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeValueEventSink, struct type_name ##NodeValue) \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeOther, struct type_name ##NodeOther) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
struct this_type_name ##Vtable { \
	type_name ##Node * (*Begin)(struct type_name* This); \
	type_name ##Node * (*End)(struct type_name* This); \
	type_name ##Node * (*Next)(struct type_name* This, type_name ##Node node); \
	type_name ##Node * (*Previous)(struct type_name* This, type_name ##Node node); \
	\
	struct type_name ##NodeEventSink * (*GetBeforeDelete)(struct type_name* This); \
	struct type_name ##NodeValueEventSink * (*GetBeforeValueChange)(struct type_name* This); \
	struct type_name ##EventSource * (*GetBeforeNextChange)(struct type_name* This); \
	struct type_name ##EventSource * (*GetBeforePreviousChange)(struct type_name* Thise); \
}; \
struct this_type_name {\
	struct this_type_name##Vtable* Vtable; \
};


//
// IList<item_type>

#define ADVANCEDFX_IList_DECL(type_name,item_type) \
ADVANCEDFX_IReadonlyList_DECL(type_name ##Readonly,item_type) \
typedef void* type_name ##Node; \
struct type_name ##NodeValue { \
	type_name ##Node Node; \
	item_type Value; \
}; \
struct type_name ##NodeOther { \
	type_name ##Node Node; \
	type_name ##Node Other; \
}; \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeEventSink, type_name ##Node) \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeValueEventSink, struct type_name ##NodeValue) \
ADVANCEDFX_IEventSink_DECL(type_name ##NodeOther, struct type_name ##NodeOther) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
ADVANCEDFX_IEventSource_DECL(type_name ##NodeEventSource,type_name ##NodeEventSync) \
struct this_type_name ##Vtable { \
	type_name ##Node * (*Begin)(struct type_name* This); \
	type_name ##Node * (*End)(struct type_name* This); \
	type_name ##Node * (*Next)(struct type_name* This, type_name ##Node * node); \
	type_name ##Node * (*Previous)(struct type_name* This, type_name ##Node * node); \
	\
	struct type_name ##NodeEventSink * (*GetBeforeDelete)(struct type_name* This); \
	struct type_name ##NodeValueEventSink * (*GetBeforeValueChange)(struct type_name* This); \
	struct type_name ##EventSource * (*GetBeforeNextChange)(struct type_name* This); \
	struct type_name ##EventSource * (*GetBeforePreviousChange)(struct type_name* This); \
	\
	struct type_name ##NodeSoftReference * (*InsertBefore)(struct type_name* This, type_name ##Node node, item_type value); \
	struct type_name ##NodeSoftReference * (*InsertAfter)(struct type_name* This, type_name ##Node node, item_type value); \
	void (*SetValue)(struct type_name* This, struct type_name ##Node node, item_type value); \
	void (*MoveBefore)(struct type_name* This, struct type_name ##Node target_node, type_name ##Node move_node ); \
	void (*MoveAfter)(struct type_name* This, struct type_name ##Node target_node, type_name ##Node move_node ); \
}; \
struct this_type_name {\
	struct this_type_name##Vtable* Vtable; \
};


#define ADVANCEDFX_IList_DECL(type_name,item_type) \
ADVANCEDFX_IListNode_DECL(type_name ##Node,item_type) \
ADVANCEDFX_ISoftReference_DECL(type_name ##NodeSoftReference, type_name ##Node) \
struct this_type_name ##Vtable { \
	struct AdvancedfxIReferenced* (*GetAsReferenced)(struct type_name* This); \
	\
	struct AdvancedfxIInterface* (*GetAsInterface)(struct type_name* This); \
	\
	struct type_name ##NodeSoftReference * (*Begin)(struct type_name* This); \
	struct type_name ##NodeSoftReference * (*End)(struct type_name* This); \
	struct type_name ##NodeSoftReference * (*InsertBefore)(struct type_name* This, struct type_name ##Node * node, item_type value); \
	struct type_name ##NodeSoftReference * (*InsertAfter)(struct type_name* This, struct type_name ##Node * node, item_type value); \
	void (*MoveBefore)(struct type_name* This, struct type_name ##Node * target_node, struct type_name ##Node * move_node ); \
	void (*MoveAfter)(struct type_name* This, struct type_name ##Node * target_node, struct type_name ##Node * move_node ); \
}; \
struct this_type_name {\
	struct this_type_name##Vtable* Vtable; \
};



// Other types /////////////////////////////////////////////////////////////////

//
// AdvancedfxIString

struct AdvancedfxIString_Vtable
{
	struct AdvancedfxIReferenced* (*GetAsReferenced)(struct AdvancedfxIString* This);

	struct AdvancedfxIInterface* (*GetAsInterface)(struct AdvancedfxIString* This);

	AdvancedfxCString(*Get)(struct AdvancedfxIString* This);
};

struct AdvancedfxIString
{
	struct AdvancedfxIString_Vtable* Vtable;
};

#define AdvancedfxIString_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x74BF43F4,0x451A,0x4DB4,0x8001,0xDE,0xA8,0x2B,0xA9,0x58,0xEA)


//
// AdvancedfxISetString

struct AdvancedfxISetString_Vtable
{
	struct AdvancedfxIReferenced* (*GetAsReferenced)(struct AdvancedfxISetString* This);

	struct AdvancedfxIInterface* (*GetAsInterface)(struct AdvancedfxISetString* This);

	void (*Set)(struct AdvancedfxISetString* This, AdvancedfxCString value);
};

struct AdvancedfxISetString
{
	struct AdvancedfxISetString_Vtable* Vtable;
};

#define AdvancedfxISetString_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x87F12812,0x9107,0x4C1A,0xA81F,0xDC,0x59,0x71,0xEC,0x9D,0xC2)



#endif
