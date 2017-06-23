#pragma once

// Description:
// Classes that help fixing and manipulating demos.


using namespace System;

namespace hlae {
ref struct hldemo_header_s;
ref struct hldemo_macroblock_header_s;
} // namespace hlae {

using namespace hlae;

namespace AfxCppCli {
namespace old {
namespace tools {

// this enables additional (slow) messages, which are also pretty many, so you
// might want to use debug mode and set no drop in menu ;)
#define HLAE_HLDEM_ENABLESLOWDEBUG 0

#define HLAE_WATERMARK64 "It is so great! I simply love it <3: Half-Life Advanced Effects"
#define HLAE_WATERMARK260 "Half-Life Advanced Effects is great! I love Half-Life Advanced Effects! I really love Half-Life Advanced Effects! Did I already say how much I love Half-Life Advanced Effects? Dude, Half-Life Advanced Effects is really great! I luv Half-Life Advanced Effects!"

delegate void OnDemoFixProgressDelegate(System::Object ^ sender, int percentage);

ref class CHlaeDemoFix
{
public:
	OnDemoFixProgressDelegate ^ OnDemoFixProgress;

	#define DEMOFIX_MAXPLAYERS 32
	CHlaeDemoFix();
	~CHlaeDemoFix();

	void EnableDirectoryFix() { EnableDirectoryFix(true); };
	void EnableDirectoryFix(bool bEnable);
	// if enabled this rebuilds the directory headers
	// if the headers are already present this may result in additonal headers
	// with junk data

	void EnableDemoCleanUp() { EnableDemoCleanUp(true); }
	void EnableDemoCleanUp(bool bEnable);
	// if enabled this activates usage of the command mappings

	void EnableHltvFix() { EnableHltvFix(true); }
	void EnableHltvFix(bool bEnable);
	// if enabled the tries to add an extra slot for the demo_forcehltv 1 spectator

	void EnableWaterMarks() { EnableWaterMarks(true); }
	void EnableWaterMarks(bool bEnable);
	// if enabled HLAE adds some additional watermarks to the output


	bool bSet_NetworkVersion;
	unsigned int uiSet_NetWorkVersion;
	bool bSet_GameDll;
	String ^strSet_GameDll;
	bool bSet_ProtocolVersion;
	unsigned int ui_ProtoVersion;
	bool bFuckOff;

	unsigned char GetHltvFixBell();
	// 0 - ok, fixed
	// 1 - serverinfo found, but maxplayers was already reached
	// 2 - bell not rung (serverinfo not found)

	void AddCommandMapping ( System::String ^srcmap, System::String ^targetmap);
	// adds a command mapping, use EnableDemoCleanUp to enable them.
	// The strings are converted to wxConvISO8859_1 and if their length is >= 64 the rest is truncated.
	// If there are identical mappings after conversion, the first one is used and the rest is ignored.

	void ClearCommandMap( void );
	// call this to clear the command map (frees memory)
	// will be called automatically on class deletion

	bool Run ( System::String ^infilename, System::String ^outfilename);
	// run CHlaeDemoFix with the settings made before
	// infilename: file to read from
	// outfilename: filte to write to

private:
	int m_LastPercentage;


	enum class copy_macroblock_e
	{
		CPMB_OK=0,			// copy ok
		CPMB_OKSTOP,		// copy ok, last block signaled stop
		CPMB_ERROR,			// last block errorenous, no bytes copied
		CPMB_USERABORT,		// user aborted operation
		CPMB_FATALERROR		// fatal error, can not be recovered
	};

	bool _bEnableDirectoryFix;
	bool _bEnableDemoCleanUp;
	bool _bEnableHltvFix;
	bool _bEnableWaterMarks;

	unsigned char _ucHltvFixBell;

	array<unsigned char> ^_watermark64; // 64 bytes
	array<unsigned char> ^_watermark260; // 260 bytes

	ref struct cmd_mapping_s
	{
		cmd_mapping_s()
		{
			src=gcnew array<unsigned char>(64);
			dst=gcnew array<unsigned char>(64);
		}
		~cmd_mapping_s()
		{
			delete dst;
			delete src;
		}
		array<unsigned char> ^src;
		array<unsigned char> ^dst;
	};

	System::Collections::Generic::LinkedList <cmd_mapping_s ^> _CommandMap;

	bool normal_demo(System::IO::BinaryReader ^ infile, System::IO::BinaryWriter ^ outfile);
	// used when EnableDirectoryFix is not set, obeys _bEnableDemoCleanUp and _bEnableHltvFix
	// infilename: file to read from
	// outfilename: filte to write to

	bool fix_demo(System::IO::BinaryReader ^ infile, System::IO::BinaryWriter ^ outfile);
	// used when EnableDirectoryFix is set, obeys _bEnableDemoCleanUp and _bEnableHltvFix
	// infilename: file to read from
	// outfilename: filte to write to

	bool read_header( System::IO::BinaryReader ^infile, hldemo_header_s ^header );
	// reads the demo header from a demo file:
	// infile - file to read from, on fail file position is undefined
	// header - pointer on header structure to read to, on fail content is undefined
	// returns: true on success, false on fail

	void watermark_header(hldemo_header_s ^header);

	bool write_header( System::IO::BinaryWriter ^outfile, hldemo_header_s ^header );
	// writes a demo header to outfile.
	// outfile - file to write to, on fail file position is undefined
	// header - header that shall be writte
	// returns: true on success, false on fail

	bool copy_bytes( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, size_t cbCopy );
	// copy bytes instantly, so it should be checked before reading if there are enough bytes available!
	// infile - file to read from, on fail file position is undefined
	// outfile - file to write to, on fail file position is undefined
	// cbCopy - number of bytes to copy
	// returns true on success, false on fail

	copy_macroblock_e copy_macroblock( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, hldemo_macroblock_header_s ^pblock_header );
	// tires to read a macroblock with best effort
	// infile - file to read from, on CPMB_FATALERROR file position is undefined, otherwise it's the same as before
	// outfile - file to write to, on CPMB_FATALERROR file position is undefined, otherwise it's the same as before
	// pblockheader - if this is not null and the return is CPMB_OK or CPMBO_OKSTOP, the funtion will return a copy of the last read macro_block_header, otherwise the pointed structure stays untouched.
	// returns:
	//	CPMB_OK - block read, no problems
	//	CPMB_OKSTOP - stop block (0x05) read, no problems
	//  CPMB_ERROR - error when reading from file (no more data, malformed block, ...) , successfully rewinded to last valid position (the one when the function was called), suggesting to do no further reads
	//	CPMB_FATALERROR - s.th. went wrong (read error, write error, ...) and it cannot be recovered, the output file shall be asumed to be broken.

	copy_macroblock_e copy_command( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile );
	// this is only called on _bEnableDemoCleanUp == true
	// it's job is to proccess the democleanup entries, by filtering the 64 byte command strings
	// before calling you should make sure that there are 64 bytes to read from
	// if the 64th char isn't null terminated, this function will terminate it, only in debug mode a warning is thrown
	// infile - file to read from, on CPMB_OK it's 64 bytes ahead, otherwise it's undefined
	// outfile - file to write to, on CPMB_OK it's 64 bytes ahead, otherwise it's undefined

	copy_macroblock_e copy_gamedata( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, unsigned int dwreadbytes );
	// this is only called on _bEnableHltvFix
	// if it retursn with anything else than CPMB_OK, the file postions are undefined, otherwise they are past the copied data and ready for the next block

};


} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {
