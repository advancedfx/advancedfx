#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H



// Platform definitions ////////////////////////////////////////////////////////


#if (_WIN32 || _WIN64) && _WIN64 || __GNUC__ && __x86_64__ || __ppc64__
#define ADVANCEDFX_ENV64
#else
#define ADVANCEDFX_ENV32
#endif



// UUID related macros /////////////////////////////////////////////////////////


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



// Primitive types /////////////////////////////////////////////////////////////


#define ADVANCEDFX_NULLPTR ((void *)0)


//
// AdvancedfxVoid

#define ADVANCEDFX_VOID_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x6C51D0CF0,x3653,0x43AE,0xB753,0xC1,0x07,0x33,0xDD,0x19,0x9E)


//
// AdvancedfxBool

typedef unsigned char AdvancedfxBool;

#define ADAVANCEDFX_FALSE 0
#define ADAVANCEDFX_TRUE 1

#define ADVANCEDFX_BOOL_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xF5E02116,0xD623,0x499F,0xABBF,0x2B7EAA4AB8DF)


//
// AdvancedfxUInt8

typedef unsigned char AdvancedfxUInt8;

#define ADVANCEDFX_UINT8_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x5760F402,0xCF8D,0x436F,0xB38A,0x0B,0x34,0x0E,0xE4,0x53,0x6C)


//
// AdvancedfxUInt32

typedef unsigned long AdvancedfxUInt32;

#define ADVANCEDFX_UINT32_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x2C0330F6,0xEBD2,0x4C4B,0xAB0F,0x57,0x22,0x08,0x0C,0x59,0x92)


//
// AdvancedfxInt32

typedef signed long AdvancedfxInt32;

#define ADVANCEDFX_INT32_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xBA19A48B,0x6780,0x4391,0x84A5,0x38,0x3D,0x8C,0x55,0xD9,0x0B)


//
// AdvancedfxSize

#ifdef ADVANCEDFX_ENV64
typedef unsigned __int64 AdvancedfxSize;
#else
typedef unsigned int AdvancedfxSize;
#endif

#define ADVANCEDFX_SIZE_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x0C5AFAAD,0xF01F,0x4D60,0x8051,0x86,0xC9,0xBE,0x78,0xFE,0xEF)


//
// AdvancedfxCString

typedef const char* AdvancedfxCString;

#define ADVANCEDFX_CSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x9C6932EB,0x17A1,0x49C0,0xA484,0x59,0x08,0xEB,0x59,0xB8,0x69)


//
// AdvancedfxFloat

typedef float AdvancedfxFloat;


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

#define ADVANCEDFX_UUID_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x9A58B3EA,0xB183,0x44DE,0x8B92,0x3F,0x7D,0xF1,0x25,0x7F,0xF7)


//
// AdvancedfxVersion

struct AdvancedfxVersion {
	AdvancedfxUInt32 major;
	AdvancedfxUInt32 minor;
	AdvancedfxUInt32 revison;
	AdvancedfxUInt32 build;
};


// AdvancedfxRegistry //////////////////////////////////////////////////////////

struct AdvancedfxRegistryShutdownHandlerVtable {
	void (*OnRegistryShutdown)(struct AdvancedfxRegistryShutdownHandler* This, struct AdvancedfxRegistry* registry);
};

struct AdvancedfxRegistryShutdownHandler {
	struct AdvancedfxRegistryShutdownEventVtable* Vtable;
};

struct AdvancedfxRegistryShutdownEventVtable {
	void (*AddRef)(struct AdvancedfxRegistryShutdownEvent * This);
	void (*Release)(struct AdvancedfxRegistryShutdownEvent * This);

	void (*AddHandler)(struct AdvancedfxRegistryShutdownEvent * This, struct AdvancedfxRegistryShutdownHandler* handler);
	void (*RemoveHandler)(struct AdvancedfxRegistryShutdownEvent * This, struct AdvancedfxRegistryShutdownHandler* handler);
};

#define ADVANCEDFX_REGISTRY_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn, 0x6f591e73, 0x14a2, 0x4466, 0x85, 0xe1, 0xd1, 0x83, 0x79, 0x28, 0x72, 0x5a)


struct AdvancedfxRegistryVtable {
	void (*AddRef)(struct AdvancedfxRegistry * This);
	void (*Release)(struct AdvancedfxRegistry * This);

	void* (*Get)(struct AdvancedfxRegistry * This,
		AdvancedfxUInt8 uuidF,
		AdvancedfxUInt8 uuidE,
		AdvancedfxUInt8 uuidD,
		AdvancedfxUInt8 uuidC,
		AdvancedfxUInt8 uuidB,
		AdvancedfxUInt8 uuidA,
		AdvancedfxUInt8 uuid9,
		AdvancedfxUInt8 uuid8,
		AdvancedfxUInt8 uuid7,
		AdvancedfxUInt8 uuid6,
		AdvancedfxUInt8 uuid5,
		AdvancedfxUInt8 uuid4,
		AdvancedfxUInt8 uuid3,
		AdvancedfxUInt8 uuid2,
		AdvancedfxUInt8 uuid1,
		AdvancedfxUInt8 uuid0);

	void (*Set)(struct AdvancedfxRegistry * This,
		AdvancedfxUInt8 uuidF,
		AdvancedfxUInt8 uuidE,
		AdvancedfxUInt8 uuidD,
		AdvancedfxUInt8 uuidC,
		AdvancedfxUInt8 uuidB,
		AdvancedfxUInt8 uuidA,
		AdvancedfxUInt8 uuid9,
		AdvancedfxUInt8 uuid8,
		AdvancedfxUInt8 uuid7,
		AdvancedfxUInt8 uuid6,
		AdvancedfxUInt8 uuid5,
		AdvancedfxUInt8 uuid4,
		AdvancedfxUInt8 uuid3,
		AdvancedfxUInt8 uuid2,
		AdvancedfxUInt8 uuid1,
		AdvancedfxUInt8 uuid0, void* value);
};

struct AdvancedfxRegistry {
	struct AdvancedfxRegistryVtable* Vtable;
};

#define ADVANCEDFX_REGISTRY_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn, 0xb2dd1b8a, 0x7097, 0x4d63, 0x84, 0x8a, 0xc7, 0x12, 0x8f, 0xaa, 0xe8, 0x30)


// Module types ////////////////////////////////////////////////////////////////

typedef void (*AdvancedfxModuleInit)(struct AdvancedfxRegistry * registry);

#endif
