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
