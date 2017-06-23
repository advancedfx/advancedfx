#pragma once

namespace hlae {

//
//	Currently the classes write to and read from the streams directly.
//	This shall be replaced by Seriliziation instead later (in case this is possible).
//	Error handling currently sucks too.
//

#define HLDEMO_DEMO_VERSION		5
#define HLDEMO_NETWORK_VERSION	47

const char HLDEMO_MAGIC[6] = { 'H','L','D','E','M','O' };

//
// hldemo_header_s
//

ref struct hldemo_header_s
{
	hldemo_header_s()
	{
		magic= gcnew array<unsigned char>( 6 );
		map_name =  gcnew array<unsigned char>( 260 );
		game_dll =  gcnew array<unsigned char>( 260 );
	}
	bool write(System::IO::BinaryWriter ^tostream)
	{
		bool bOk = true;
		
		try
		{
			tostream->Write( magic );
			tostream->Write( pad_0x0006 );
			tostream->Write( pad_0x0007 );
			tostream->Write( demo_version );
			tostream->Write( network_version );
			tostream->Write( map_name );
			tostream->Write( game_dll );
			tostream->Write( dwUnknown_0x0218 );
			tostream->Write( dir_offset );
		} catch ( System::Exception ^e )
		{
			Console::WriteLine( "Generic Exception when writing to file: {0}", e );
			bOk=false;
		}

		return bOk;
	}
	bool read(System::IO::BinaryReader ^fromstream)
	{
		bool bOk=true;
		try
		{
			magic = fromstream->ReadBytes( magic->Length );
			pad_0x0006 = fromstream->ReadByte();
			pad_0x0007 = fromstream->ReadByte();
			demo_version = fromstream->ReadUInt32();
			network_version = fromstream->ReadUInt32();
			map_name = fromstream->ReadBytes( map_name->Length );
			game_dll = fromstream->ReadBytes( game_dll->Length );
			dwUnknown_0x0218 = fromstream->ReadUInt32();
			dir_offset = fromstream->ReadUInt32();
		} catch( System::IO::EndOfStreamException ^e )
		{
			Console::WriteLine( "End Of Stream when reading from file: {0}", e );
			bOk=false;
		} catch( System::Exception ^e )
		{
			Console::WriteLine( "Genereic Exception when reading from file: {0}", e );
			bOk=false;
		}

		return bOk;
	}

	static size_t size = 6+1+1+4+4+260+260+4+4;

	array<unsigned char> ^magic;
	unsigned char pad_0x0006;
	unsigned char pad_0x0007;
	unsigned int demo_version;
	unsigned int network_version;
	array<unsigned char> ^map_name;
	array<unsigned char> ^game_dll;
	unsigned int dwUnknown_0x0218;
	unsigned int dir_offset;
};

//
// hldemo_dir_entry_s
//

ref struct hldemo_dir_entry_s
{
	hldemo_dir_entry_s()
	{
		title = gcnew array<unsigned char>( 64 );
	}
	bool write(System::IO::BinaryWriter ^tostream)
	{
		bool bOk = true;
		
		try
		{
			
			tostream->Write( number );
			tostream->Write( title );
			tostream->Write( flags);
			tostream->Write( play );
			tostream->Write( time );
			tostream->Write( frames );
			tostream->Write( offset );
			tostream->Write( length );
		} catch ( System::Exception ^e )
		{
			Console::WriteLine( "Generic Exception when writing to file: {0}", e );
			bOk=false;
		}

		return bOk;
	}
	bool read(System::IO::BinaryReader ^fromstream)
	{
		bool bOk=true;
		try
		{
			number = fromstream->ReadUInt32();
			title = fromstream->ReadBytes( title->Length );
			flags = fromstream->ReadUInt32();
			play = fromstream->ReadUInt32();
			time = fromstream->ReadSingle();
			frames = fromstream->ReadUInt32();
			offset = fromstream->ReadUInt32();
			length = fromstream->ReadUInt32();
		} catch( System::IO::EndOfStreamException ^e )
		{
			Console::WriteLine( "End Of Stream when reading from file: {0}", e );
			bOk=false;
		} catch( System::Exception ^e )
		{
			Console::WriteLine( "Genereic Exception when reading from file: {0}", e );
			bOk=false;
		}

		return bOk;
	}

	unsigned int number;
	array<unsigned char> ^title;
	unsigned int flags;
	unsigned int play;
	float time;
	unsigned int frames;
	unsigned int offset;
	unsigned int length;
};

//
// hldemo_macroblock_header_s
//

ref struct hldemo_macroblock_header_s
{
	bool write(System::IO::BinaryWriter ^tostream)
	{
		bool bOk = true;
		
		try
		{
			tostream->Write( type );
			tostream->Write( time );
			tostream->Write( frame );
		} catch ( System::Exception ^e )
		{
			Console::WriteLine( "Generic Exception when writing to file: {0}", e );
			bOk=false;
		}

		return bOk;
	}
	bool read(System::IO::BinaryReader ^fromstream)
	{
		bool bOk=true;
		try
		{
			type = fromstream->ReadByte();
			time = fromstream->ReadSingle();
			frame = fromstream->ReadUInt32();
		} catch( System::IO::EndOfStreamException ^e )
		{
			Console::WriteLine( "End Of Stream when reading from file: {0}", e );
			bOk=false;
		} catch( System::Exception ^e )
		{
			Console::WriteLine( "Genereic Exception when reading from file: {0}", e );
			bOk=false;
		}

		return bOk;
	}
	unsigned char type;
	float time;
	unsigned int frame;
};

//
// svc (server to client) commands:
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

//
//	Svc to string mapper is not ported yet
//

// sub commands of svc_hltv:
#define HLTV_ACTIVE				0	// tells client that he's an spectator and will get director commands
#define HLTV_STATUS				1	// send status infos about proxy 
#define HLTV_LISTEN				2	// tell client to listen to a multicast stream



} // namespace hlae