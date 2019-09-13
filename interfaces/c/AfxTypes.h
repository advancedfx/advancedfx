#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H

// Remarks about Reference Counting
//
// - Always make use of BeginDeletingNotification / EndDeletingNotification whenever you can instead.
// - Only use this if you _own_ the object: Meaning it's passed to you as function argument or you are enclosing object.
// - Hold only as long onto a reference as you really have to.
// - Everything else will cause havoc!
// - Objects created or returned need to be Released, if you don't return them as well, unless stated otherwise.
// - Before using function arguments you need to call AddRef on them, to prevent them being able to be released while you use them.

typedef unsigned char AdvancedfxBool;

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

#define _ADVANCEDFX_APPLY(arg) (arg)
#define ADVANCEDFX_APPLY(fn,arg) fn _ADVANCEDFX_APPLY(arg)

#define _ADVANCEDFX_APPLY(arg1,arg2) (arg1,arg2)
#define ADVANCEDFX_APPLY_2(fn,arg1,arg2) fn _ADVANCEDFX_APPLY(arg1,arg2)

#define ADVANCEDFX_UUID(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0

// Example usage: ADVANCEDFX_APPLY_FN(ADVANCEDFX_UUID_BYTES,ADVANCEDFX_IFACTORY_UUID)
#define ADVANCEDFX_UUID_BYTES(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) \
	(unsigned char)((time_low & 0xff000000) >> 12) \
	, (unsigned char)((time_low & 0x00ff0000) >> 8) \
	, (unsigned char)((time_low & 0x0000ff00) >> 4) \
	, (unsigned char)((time_low & 0x000000ff)) \
	, (unsigned char)((time_mid & 0xff00) >> 4) \
	, (unsigned char)((time_mid & 0x00ff)) \
	, (unsigned char)((time_hi_and_version & 0xff00) >> 4) \
	, (unsigned char)((time_hi_and_version & 0x00ff)) \
	, (unsigned char)((clock_seq_hi_and_res__clock_seq_low & 0xff00) >> 4) \
	, (unsigned char)((clock_seq_hi_and_res__clock_seq_low & 0x00ff)) \
	, (unsigned char)(node_uc5) \
	, (unsigned char)(node_uc4) \
	, (unsigned char)(node_uc3) \
	, (unsigned char)(node_uc2) \
	, (unsigned char)(node_uc1) \
	, (unsigned char)(node_uc0)

// Example usage: ADVANCEDFX_APPLY_FN_2(ADVANCEDFX_UUID_VAR,name,ADVANCEDFX_APPLY_FN(ADVANCEDFX_UUID_BYTES,ADVANCEDFX_IFACTORY_UUID));
#define ADVANCEDFX_UUID_VAR(name,aF,aE,aD,aC,aB,aA,a9,a8,a7,a6,a5,a4,a3,a2,a1,a0) struct AdvancedfxUuid name = { \
	aF,aE,aD,aC,aB,aA,a9,a8,a7,a6,a5,a4,a3,a2,a1,a0 \
}

#define ADVANCEDFX_UUID_VAR_BYTES(name) name.ucF,name.ucE,name.ucD,name.ucC,name.ucB,name.ucA,name.uc9,name.uc8,name.uc7,name.uc6,name.uc5,name.uc4,name.uc3,name.uc2,name.uc1

#define ADVANCEDFX_UUID_BYTES_EQUAL(aF,aE,aD,aC,aB,aA,a9,a8,a7,a6,a5,a4,a3,a2,a1,a0,bF,bE,bD,bC,bB,bA,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) ( \
	aF == bF \
	&& aE == bE \
	&& aD == bD \
	&& aC == bC \
	&& aB == bB \
	&& aA == bA \
	&& a9 == b9 \
	&& a8 == b8 \
	&& a7 == b7 \
	&& a6 == b6 \
	&& a5 == b5 \
	&& a4 == b4 \
	&& a3 == b3 \
	&& a2 == b2 \
	&& a1 == b1 \
	&& a0 == b0 \
)

#define ADVANCEFX_UUID_BYTES_CMP(aF,aE,aD,aC,aB,aA,a9,a8,a7,a6,a5,a4,a3,a2,a1,a0,bF,bE,bD,bC,bB,bA,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) (\
	aF != bF ? (aF > bF ? 1 : -1) \
	: aE != bE ? (aE > bE ? 1 : -1) \
	: aD != bD ? (aD > bD ? 1 : -1) \
	: aC != bC ? (aC > bC ? 1 : -1) \
	: aB != bB ? (aB > bB ? 1 : -1) \
	: aA != bA ? (aA > bA ? 1 : -1) \
	: a9 != b9 ? (a9 > b9 ? 1 : -1) \
	: a8 != b8 ? (a8 > b8 ? 1 : -1) \
	: a7 != b7 ? (a7 > b7 ? 1 : -1) \
	: a6 != b6 ? (a6 > b6 ? 1 : -1) \
	: a5 != b5 ? (a5 > b5 ? 1 : -1) \
	: a4 != b4 ? (a4 > b4 ? 1 : -1) \
	: a3 != b3 ? (a3 > b3 ? 1 : -1) \
	: a2 != b2 ? (a2 > b2 ? 1 : -1) \
	: a1 != b1 ? (a1 > b1 ? 1 : -1) \
	: a0 != b0 ? (a0 > b0 ? 1 : -1) \
	: 0 \
)

struct AdvancedfxVersion {
	unsigned long major;
	unsigned long minor;
	unsigned long revison;
	unsigned long build;
};

#define ADVANCEDFX_NULLPTR ((void *)0)

struct AdvancedfxIString
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxIString* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxIString* This);

	const char * (*Get)(struct AdvancedfxIString* This);
};

struct AdvancedfxISetString
{
	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*AddRef)(struct AdvancedfxISetString* This);

	/**
	 * See "Remarks about Reference Counting" in AfxTypes.h!
	 */
	void (*Release)(struct AdvancedfxISetString* This);

	void (*Set)(struct AdvancedfxISetString* This, const char* value);
};

#endif
