#pragma once

// when we don't use  manual packing, use this one:
#define HLDEMO_CPP_PACK 8

#pragma pack(push)
#pragma pack( HLDEMO_CPP_PACK )

// force manual packing for compiler packed structures that support it:
// 0 - disable
// 1 - enable
#define HLDEMO_CPP_MANUALPACK 0

// data type sizes:
// char         :  8 bit (signed)
// unsigned int : 32 bit
// float        : 32 bit

/*

#define HLDEMO_DEMO_VERSION		5
#define HLDEMO_NETWORK_VERSION	47

const char HLDEMO_MAGIC[6] = { 'H','L','D','E','M','O' };

#if HLDEMO_CPP_MANUALPACK
	#pragma pack(push)
	#pragma pack(1)
#endif
	struct hldemo_header_s
	{
		char magic[6];
		#if HLDEMO_CPP_MANUALPACK
			char pad_0x0006;
			char pad_0x0007;
		#endif
		unsigned int demo_version;
		unsigned int network_version;
		char map_name[260];
		char game_dll[260];
		unsigned int dwUnknown_0x0218;
		unsigned int dir_offset;
	};
#if HLDEMO_CPP_MANUALPACK
	#pragma pack(pop)
#endif

#if HLDEMO_CPP_MANUALPACK
	#pragma pack(push)
	#pragma pack(1)
#endif
	struct hldemo_dir_entry_s
	{
		unsigned int number;
		char title[64];
		unsigned int flags;
		unsigned int play;
		float time;
		unsigned int frames;
		unsigned int offset;
		unsigned int length;
	};
#if HLDEMO_CPP_MANUALPACK
	#pragma pack(pop)
#endif


#pragma pack(push)
#pragma pack(1)
// we always pack this manually, since this more a sequence of operations than a struct
struct hldemo_macroblock_header_s
{
	unsigned char type;
	float time;
	unsigned int frame;
};
#pragma pack(pop)

*/

//
// server to client commands:
//

// from loaded hw.dll at 0x01ece720:

#define	svc_bad						0
#define	svc_nop						1
#define	svc_disconnect				2
#define	svc_event					3
#define	svc_version					4
#define	svc_setview					5
#define	svc_sound					6
#define	svc_time					7
#define	svc_print					8
#define	svc_stufftext				9
#define	svc_setangle				10
#define	svc_serverinfo				11
#define	svc_lightstyle				12
#define	svc_updateuserinfo			13
#define	svc_deltadescription		14
#define	svc_clientdata				15
#define	svc_stopsound				16
#define	svc_pings					17
#define	svc_particle				18
#define	svc_damage					19
#define	svc_spawnstatic				20
#define	svc_event_reliable			21
#define	svc_spawnbaseline			22
#define	svc_temp_entity				23
#define	svc_setpause				24
#define	svc_signonnum				25
#define	svc_centerprint				26
#define	svc_killedmonster			27
#define	svc_foundsecret				28
#define	svc_spawnstaticsound		29
#define	svc_intermission			30
#define	svc_finale					31
#define	svc_cdtrack					32
#define	svc_restore					33
#define	svc_cutscene				34
#define	svc_weaponanim				35
#define	svc_decalname				36
#define	svc_roomtype				37
#define	svc_addangle				38
#define	svc_newusermsg				39
#define	svc_packetentities			40
#define	svc_deltapacketentities		41
#define	svc_choke					42
#define	svc_resourcelist			43
#define	svc_newmovevars				44
#define	svc_resourcerequest			45
#define	svc_customization			46
#define	svc_crosshairangle			47
#define	svc_soundfade				48
#define	svc_filetxferfailed			49
#define	svc_hltv					50
#define	svc_director				51
#define	svc_voiceinit				52
#define	svc_voicedata				53
#define	svc_sendextrainfo			54
#define	svc_timescale				55
#define	svc_resourcelocation		56
#define	svc_sendcvarvalue			57
#define	svc_sendcvarvalue2			58
#define	svc_END_OF_LIST				255

/*
struct svc_entry_s
{
	unsigned int cmd;
	char* name;
	void (* func)(void);
};

typedef svc_entry_s svc_mapper_t[];

// uses this like: svc_mapper_t mymapper = HLDEMO_INIT_SVC_MAPPER;
#define HLDEMO_INIT_SVC_MAPPER { \
	{ svc_bad, "svc_bad", 0 }, \
	{ svc_nop, "svc_nop", 0 }, \
	{ svc_disconnect, "svc_disconnect", 0 }, \
	{ svc_event, "svc_event", 0 }, \
	{ svc_version, "svc_version", 0 }, \
	{ svc_setview, "svc_setview", 0 }, \
	{ svc_sound, "svc_sound", 0 }, \
	{ svc_time, "svc_time", 0 }, \
	{ svc_print, "svc_print", 0 }, \
	{ svc_stufftext, "svc_stufftext", 0 }, \
	{ svc_setangle, "svc_setangle", 0 }, \
	{ svc_serverinfo, "svc_serverinfo", 0 }, \
	{ svc_lightstyle, "svc_lightstyle", 0 }, \
	{ svc_updateuserinfo, "svc_updateuserinfo", 0 }, \
	{ svc_deltadescription, "svc_deltadescription", 0 }, \
	{ svc_clientdata, "svc_clientdata", 0 }, \
	{ svc_stopsound, "svc_stopsound", 0 }, \
	{ svc_pings, "svc_pings", 0 }, \
	{ svc_particle, "svc_particle", 0 }, \
	{ svc_damage, "svc_damage", 0 }, \
	{ svc_spawnstatic, "svc_spawnstatic", 0 }, \
	{ svc_event_reliable, "svc_event_reliable", 0 }, \
	{ svc_spawnbaseline, "svc_spawnbaseline", 0 }, \
	{ svc_temp_entity, "svc_temp_entity", 0 }, \
	{ svc_setpause, "svc_setpause", 0 }, \
	{ svc_signonnum, "svc_signonnum", 0 }, \
	{ svc_centerprint, "svc_centerprint", 0 }, \
	{ svc_killedmonster, "svc_killedmonster", 0 }, \
	{ svc_foundsecret, "svc_foundsecret", 0 }, \
	{ svc_spawnstaticsound, "svc_spawnstaticsound", 0 }, \
	{ svc_intermission, "svc_intermission", 0 }, \
	{ svc_finale, "svc_finale", 0 }, \
	{ svc_cdtrack, "svc_cdtrack", 0 }, \
	{ svc_restore, "svc_restore", 0 }, \
	{ svc_cutscene, "svc_cutscene", 0 }, \
	{ svc_weaponanim, "svc_weaponanim", 0 }, \
	{ svc_decalname, "svc_decalname", 0 }, \
	{ svc_roomtype, "svc_roomtype", 0 }, \
	{ svc_addangle, "svc_addangle", 0 }, \
	{ svc_newusermsg, "svc_newusermsg", 0 }, \
	{ svc_packetentities, "svc_packetentities", 0 }, \
	{ svc_deltapacketentities, "svc_deltapacketentities", 0 }, \
	{ svc_choke, "svc_choke", 0 }, \
	{ svc_resourcelist, "svc_resourcelist", 0 }, \
	{ svc_newmovevars, "svc_newmovevars", 0 }, \
	{ svc_resourcerequest, "svc_resourcerequest", 0 }, \
	{ svc_customization, "svc_custo mization", 0 }, \
	{ svc_crosshairangle, "svc_crosshairangle", 0 }, \
	{ svc_soundfade, "svc_soundfade", 0 }, \
	{ svc_filetxferfailed, "svc_filetxferfailed", 0 }, \
	{ svc_hltv, "svc_hltv", 0 }, \
	{ svc_director, "svc_director", 0 }, \
	{ svc_voiceinit, "svc_voiceinit", 0 }, \
	{ svc_voicedata, "svc_voicedata", 0 }, \
	{ svc_sendextrainfo, "svc_sendextrainfo", 0 }, \
	{ svc_timescale, "svc_timescale", 0 }, \
	{ svc_resourcelocation, "svc_resourcelocation", 0 }, \
	{ svc_sendcvarvalue, "svc_sendcvarvalue", 0 }, \
	{ svc_sendcvarvalue2, "svc_sendcvarvalue2", 0 }, \
	{ svc_END_OF_LIST, "End of List", 0 } \
};

*/

#pragma pack(pop)

