#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H

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

#define ADVANCEDX_DEFINE_UUID(name,time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) AdvancedfxUuid name = { \
	(time_low & 0xff000000) >> 12 \
	, (time_low & 0x00ff0000) >> 8 \
	, (time_low & 0x0000ff00) >> 4 \
	, (time_low & 0x000000ff) \
	, (time_mid & 0xff00) >> 4 \
	, (time_mid & 0x00ff) \
	, (time_hi_and_version & 0xff00) >> 4 \
	, (time_hi_and_version & 0x00ff) \
	, (clock_seq_hi_and_res__clock_seq_low & 0xff00) >> 4 \
	, (clock_seq_hi_and_res__clock_seq_low & 0x00ff) \
	, node_5 \
	, node_4 \
	, node_3 \
	, node_2 \
	, node_1 \
	, node_0 \
};

#define ADVANCEDFX_UUIDS_EQUAL(a,b) ( \
	a.ucF == b.ucF \
	&& a.ucE == b.ucE \
	&& a.ucD == b.ucD \
	&& a.ucC == b.ucC \
	&& a.ucB == b.ucB \
	&& a.ucA == b.ucA \
	&& a.uc9 == b.uc9 \
	&& a.uc8 == b.uc8 \
	&& a.uc7 == b.uc7 \
	&& a.uc6 == b.uc6 \
	&& a.uc5 == b.uc5 \
	&& a.uc4 == b.uc4 \
	&& a.uc3 == b.uc3 \
	&& a.uc2 == b.uc2 \
	&& a.uc1 == b.uc1 \
	&& a.uc0 == b.uc0 \
)

#define ADVANCEDFX_UUIDS_CMP(a,b)  ( \
	a.ucF != b.ucF ? (a.ucF > b.ucF ? 1 : -1) \
	: a.ucE != b.ucE ? (a.ucE > b.ucE ? 1 : -1) \
	: a.ucD != b.ucD ? (a.ucD > b.ucD ? 1 : -1) \
	: a.ucC != b.ucC ? (a.ucC > b.ucC ? 1 : -1) \
	: a.ucB != b.ucB ? (a.ucB > b.ucB ? 1 : -1) \
	: a.ucA != b.ucA ? (a.ucA > b.ucA ? 1 : -1) \
	: a.uc9 != b.uc9 ? (a.uc9 > b.uc9 ? 1 : -1) \
	: a.uc8 != b.uc8 ? (a.uc8 > b.uc8 ? 1 : -1) \
	: a.uc7 != b.uc7 ? (a.uc7 > b.uc7 ? 1 : -1) \
	: a.uc6 != b.uc6 ? (a.uc6 > b.uc6 ? 1 : -1) \
	: a.uc5 != b.uc5 ? (a.uc5 > b.uc5 ? 1 : -1) \
	: a.uc4 != b.uc4 ? (a.uc4 > b.uc4 ? 1 : -1) \
	: a.uc3 != b.uc3 ? (a.uc3 > b.uc3 ? 1 : -1) \
	: a.uc2 != b.uc2 ? (a.uc2 > b.uc2 ? 1 : -1) \
	: a.uc1 != b.uc1 ? (a.uc1 > b.uc1 ? 1 : -1) \
	: (a.uc0 > b.uc0 ? 1 : -1) \
)

struct AdvancedfxVersion {
	unsigned long major;
	unsigned long minor;
	unsigned long revison;
	unsigned long build;
};

#define ADVANCEDFX_NULLPTR ((void *)0)

struct AdvancedfxIDependable {

};

struct AdvancedfxIDependent {

	void (*ReleaseFrom)(struct AdvnacedfxIDependent* This, struct AdvancedfxIDependable* dependable);
};

//struct AdvancedfxIDependable* (*AddRef)(struct AdvancedfxIString* This, struct AdvancedfxIDependent* dependent);

//void (*Release)(struct AdvancedfxIString* This, struct AdvancedfxIDependent* dependent);


#endif
