#include "stdafx.h"

#include "demotools.h"

#include <tchar.h>

#include <shared/hldemo/hldemo_clr.h>

using namespace hlae;
using namespace AfxCppCli::old::tools;

#define DEBUG_MESSAGE(master,message) MessageDummy()
#define ERROR_MESSAGE(master,message) MessageDummy()
#define INFO_MESSAGE(master,message) MessageDummy()
#define VERBOSE_MESSAGE(master,message) MessageDummy()
#define WARNING_MESSAGE(master,message) MessageDummy()

void MessageDummy(void)
{
}

bool compare_bytes (const char *bytes1, const char *bytes2, size_t ilen)
{
	while (ilen--) if (*(bytes1++)!=*(bytes2++)) return false;
	return true;
}

CHlaeDemoFix::CHlaeDemoFix()
{
	_bEnableDirectoryFix = false;
	_bEnableDemoCleanUp = false;
	_bEnableHltvFix = false;
	_bEnableWaterMarks = true;

	int i=0;

	_watermark64 = gcnew array<unsigned char>(64);
	System::String ^tstr = gcnew System::String(HLAE_WATERMARK64);
	for (i=0;i<64-1;i++)
		_watermark64[i] = (char)(tstr[i]);
	delete tstr;
	_watermark64[i]=0; //

	_watermark260 = gcnew array<unsigned char>(260);
	System::String ^tstr2 = gcnew System::String(HLAE_WATERMARK260);
	for (i=0;i<260-1;i++)
		_watermark260[i] = (char)(tstr2[i]);
	delete tstr2;
	_watermark260[i]=0;

	m_LastPercentage = 0;
}

CHlaeDemoFix::~CHlaeDemoFix()
{
	ClearCommandMap();
}

void CHlaeDemoFix::EnableDirectoryFix(bool bEnable)
{
	_bEnableDirectoryFix = bEnable;
}
void CHlaeDemoFix::EnableDemoCleanUp(bool bEnable)
{
	_bEnableDemoCleanUp = bEnable;
}

void CHlaeDemoFix::EnableHltvFix(bool bEnable)
{
	_bEnableHltvFix = bEnable;
}

void CHlaeDemoFix::EnableWaterMarks(bool bEnable)
{
	_bEnableWaterMarks = bEnable;
}

unsigned char CHlaeDemoFix::GetHltvFixBell()
{
	return _ucHltvFixBell;
}

void CHlaeDemoFix::AddCommandMapping ( System::String ^srcmap, System::String ^targetmap)
{
	cmd_mapping_s ^mymapping = gcnew cmd_mapping_s;

	for (int i=0;i<64-1;i++)
	{
		if (i>=srcmap->Length)
		{
			mymapping->src[i]=0; // terminate string
			break;
		}

		mymapping->src[i] = (char)(srcmap[i]);
	}

	for (int i=0;i<64-1;i++)
	{
		if (i>=targetmap->Length)
		{
			mymapping->dst[i]=0; // terminate string
			break;
		}

		mymapping->dst[i] = (char)(targetmap[i]);
	}

	_CommandMap.AddFirst(mymapping);
}

void CHlaeDemoFix::ClearCommandMap( void )
{
	_CommandMap.Clear();
}

bool CHlaeDemoFix::Run ( System::String ^infilename, System::String ^outfilename)
{
	bool bOK = false;
	System::IO::BinaryReader ^ infile = nullptr;
	System::IO::BinaryWriter ^ outfile = nullptr;

	_ucHltvFixBell = 2;

	try {

		try
		{
			infile = gcnew System::IO::BinaryReader( System::IO::File::Open( infilename, System::IO::FileMode::Open, System::IO::FileAccess::Read ) );
		} catch (System::Exception ^e)
		{
			ERROR_MESSAGE( debugMaster, System::String::Format("Exception when opening inputfile: {0}",e) );
			if(infile) infile->Close();
			infile = nullptr;
		}

		try
		{
			outfile = gcnew System::IO::BinaryWriter( System::IO::File::Open( outfilename, System::IO::FileMode::Create, System::IO::FileAccess::Write ) );
		}
		catch (System::Exception ^e)
		{
			ERROR_MESSAGE( debugMaster, System::String::Format("Exception when opening outfile: {0}",e) );
			if(outfile) outfile->Close();
			outfile = nullptr;
		}

		if (
			nullptr != infile && nullptr != outfile
			&& infile->BaseStream->CanRead && infile->BaseStream->CanSeek
			&& outfile->BaseStream->CanWrite && outfile->BaseStream->CanSeek // seek required(?)
		)
		{
			if (_bEnableDirectoryFix)
				bOK = fix_demo(infile, outfile);
			else
				bOK = normal_demo(infile, outfile);

		} else {
			if (!(infile && infile->BaseStream && infile->BaseStream->CanRead && infile->BaseStream->CanSeek)) ERROR_MESSAGE( debugMaster, "Could not open input file for reading and seeking." );
			if (!(outfile && outfile->BaseStream && outfile->BaseStream->CanWrite && outfile->BaseStream->CanSeek)) ERROR_MESSAGE( debugMaster, "Could not open output file for writing and seeking." );
		}
	}
	finally {
		if(infile) infile->Close();
		if(outfile) outfile->Close();

		delete infile;
		delete outfile;
	}

	return bOK;
}

bool CHlaeDemoFix::normal_demo(System::IO::BinaryReader ^ infile, System::IO::BinaryWriter ^ outfile)
{
	bool bOK=false;


	//wxString tstr;
	hldemo_header_s ^header = gcnew hldemo_header_s; 

	VERBOSE_MESSAGE( debugMaster, "reading demo header ..." );

	if (read_header(infile,header))
	{
		if (_bEnableWaterMarks) watermark_header(header);

		if(!(header->dir_offset))
		{
			ERROR_MESSAGE( debugMaster, "Directory entries not present, aborting. Use DemoFix.");
		}
		else if (write_header(outfile,header))
		{

			bool bSearchNewSegment = true && (infile->BaseStream->Position < header->dir_offset);
			int iNumSegments = 0;
			copy_macroblock_e eCopyState=copy_macroblock_e::CPMB_OKSTOP;



			while (bSearchNewSegment)
			{
				VERBOSE_MESSAGE( debugMaster, System::String::Format( "Processing segment {0}",iNumSegments) );

				//tstr.Printf(_T("Segment: %i"),iNumSegments);
				//tickerdlg->Update(infile->Tell(),tstr);
				//tickerdlg->TempSet((100*infile->BaseStream->Position)/infile->BaseStream->Length);

				hldemo_macroblock_header_s ^lastheader = gcnew hldemo_macroblock_header_s();
				lastheader->type = 5;
				lastheader->time = 0.0f;
				lastheader->frame = 0;

				eCopyState = copy_macroblock(infile, outfile, lastheader);

				while (eCopyState==copy_macroblock_e::CPMB_OK && (infile->BaseStream->Position < header->dir_offset))
				{
					//if (! ( tickerdlg->Update(infile->BaseStream->Position) ) )
					//{
					//	eCopyState=CPMB_USERABORT;
					//	break;
					//}
					eCopyState = copy_macroblock(infile, outfile, lastheader);
				}
				
				if (eCopyState==copy_macroblock_e::CPMB_OKSTOP)
				{
					// normal stop

					// munch repeated stops:
					long long fpos = infile->BaseStream->Position;
					hldemo_macroblock_header_s ^mbheader = gcnew hldemo_macroblock_header_s();

					while ( (infile->BaseStream->Position < header->dir_offset) && mbheader->read( infile ) && mbheader->type == 5 )
					{
						lastheader = mbheader;
						fpos = infile->BaseStream->Position;
						mbheader->write( outfile );
					}

					// rewind behind last stop:
					infile->BaseStream->Seek( fpos, System::IO::SeekOrigin::Begin );

				} else if (eCopyState==copy_macroblock_e::CPMB_ERROR)
				{
					ERROR_MESSAGE( debugMaster, "DemoFix: Found incomplete or unknown message or file end, aborting. Use DemoFix." );
				} else if (eCopyState==copy_macroblock_e::CPMB_USERABORT) {
					// fatal error (user abort)
					ERROR_MESSAGE( debugMaster, "DemoFix: User aborted." );
				} else {
					// fatal error
					ERROR_MESSAGE( debugMaster, "DemoFix: Cannot recover from last error, failed." );
				}

				if (eCopyState==copy_macroblock_e::CPMB_OKSTOP)
				{
					iNumSegments++;
				}

				bSearchNewSegment = (eCopyState == copy_macroblock_e::CPMB_OKSTOP) && (infile->BaseStream->Position < header->dir_offset);
			}

			// no new segments, copy directory entries (if any):
			if (eCopyState==copy_macroblock_e::CPMB_OKSTOP)
			{
				//tstr.Printf(_T("copying %i directory entries"),iNumSegments);
				//tickerdlg->Update(infile->BaseStream->Position,tstr);
				VERBOSE_MESSAGE( debugMaster, System::String::Format("copying {0} directory entries",iNumSegments) );

				if(copy_bytes(infile,outfile,infile->BaseStream->Length - infile->BaseStream->Position))
				{
					// that's it guys.
					INFO_MESSAGE( debugMaster, "DemoTools: Finished.");
					//tickerdlg->Update(infile->Length());
					bOK=true;
				};
			}

		} else ERROR_MESSAGE( debugMaster, "Failed to write demo header." );

	} else ERROR_MESSAGE( debugMaster, "Failed to read demo header." );

	delete header;

	return bOK;
}

bool CHlaeDemoFix::fix_demo(System::IO::BinaryReader ^ infile, System::IO::BinaryWriter ^ outfile)
{
	bool bOK=false;

	//wxString tstr;
	hldemo_header_s ^header = gcnew hldemo_header_s();

	VERBOSE_MESSAGE( debugMaster, "DemoFix: reading demo header ..." );

	if (read_header(infile,header))
	{
		if (_bEnableWaterMarks) watermark_header(header);

		if(header->dir_offset)
		{
			WARNING_MESSAGE( debugMaster, "DemoFix: Directory entries already present, ignoring." );
			header->dir_offset = 0;
		}

		if (write_header(outfile,header))
		{
			bool bSearchNewSegment=true;
			int iNumSegments=0;
			copy_macroblock_e eCopyState;

			System::Collections::Generic::Queue <hldemo_dir_entry_s ^> direntries_que;
			
			while (bSearchNewSegment)
			{
				VERBOSE_MESSAGE( debugMaster, System::String::Format("Scanning for segment {0}",iNumSegments) );

				//tstr.Printf(_T("Segment: %i"),iNumSegments);
				//tickerdlg->Update(infile->Tell(),tstr);

				unsigned int segmentoffset=infile->BaseStream->Position;
				unsigned int totalframes=0;
				float starttime=0;

				hldemo_macroblock_header_s ^lastheader = gcnew hldemo_macroblock_header_s();
				lastheader->type = 5;
				lastheader->time = 0.0f;
				lastheader->frame = 0;

				eCopyState = copy_macroblock(infile, outfile, lastheader);

				if(eCopyState==copy_macroblock_e::CPMB_OK || eCopyState==copy_macroblock_e::CPMB_OKSTOP)
					starttime=lastheader->time;

				while (eCopyState==copy_macroblock_e::CPMB_OK)
				{
					totalframes++;
					//if (! ( tickerdlg->Update(infile->BaseStream->Position) ) )
					//{
					//	eCopyState=CPMB_USERABORT;
					//	break;
					//}
					eCopyState = copy_macroblock(infile, outfile, lastheader);
				}
				
				if (eCopyState==copy_macroblock_e::CPMB_OKSTOP)
				{
					// normal stop

					// munch repeated stops:
					long long fpos = infile->BaseStream->Position;
					hldemo_macroblock_header_s ^mbheader = gcnew hldemo_macroblock_header_s();

					while ( mbheader->read( infile ) && mbheader->type == 5)
					{
						lastheader = mbheader;
						totalframes++;
						fpos = infile->BaseStream->Position;
						mbheader->write( outfile );
					}

					// rewind behind last stop:
					infile->BaseStream->Seek( fpos, System::IO::SeekOrigin::Begin );

				} else if (eCopyState==copy_macroblock_e::CPMB_ERROR)
				{
					// some error, we drop the rest and add the missing stop:
					if (totalframes>0)
					{
						VERBOSE_MESSAGE( debugMaster, "DemoFix: Found incomplete or unknown message or file end. Asuming file end, finishing segment (appending STOP), dropping rest." );
						hldemo_macroblock_header_s ^mbheader = gcnew hldemo_macroblock_header_s();

						mbheader->type = 0x05;
						mbheader->frame = totalframes;
						mbheader->time = lastheader->time;
						mbheader->write( outfile );
						totalframes++;
					} else VERBOSE_MESSAGE( debugMaster, "DemoFix: Found incomplete or unknown message or file end. Asuming file end, ignoring segment (was empty), dropping rest." );
				} else if (eCopyState==copy_macroblock_e::CPMB_USERABORT) {
					// fatal error (user abort)
					ERROR_MESSAGE( debugMaster, "DemoFix: User aborted." );
				} else {
					// fatal error
					ERROR_MESSAGE( debugMaster, "DemoFix: Cannot recover from last error, failed." );
				}

				if ((eCopyState==copy_macroblock_e::CPMB_OKSTOP || eCopyState==copy_macroblock_e::CPMB_ERROR)&&totalframes>0)
				{
					// add to directory entry
					hldemo_dir_entry_s ^direntry = gcnew hldemo_dir_entry_s();
					direntry->number = iNumSegments;
					if (iNumSegments==0)
					{
						int i=0;
						for each(System::Char achar in System::String("LOADING"))
							direntry->title[i++] = (unsigned char)achar;
						for (;i<direntry->title->Length;i++)
							direntry->title[i]=0; // terminate / fill with zero
					}
					else
					{
						int i=0;
						for each(System::Char achar in System::String("Playback"))
							direntry->title[i++] = (unsigned char)achar;
						for (;i<direntry->title->Length;i++)
							direntry->title[i]=0; // terminate / fill with zero
					}
					direntry->flags = 0;
					direntry->play = 0x0FF;
					direntry->time = lastheader->time - starttime;
					direntry->frames = totalframes;
					direntry->offset = segmentoffset;
					direntry->length = infile->BaseStream->Position - segmentoffset;

					direntries_que.Enqueue( direntry );
					iNumSegments++;
				}

				if (eCopyState!=copy_macroblock_e::CPMB_OKSTOP)
					bSearchNewSegment=false;
			}

			// no new segments, create directory entries (if any):
			if (eCopyState==copy_macroblock_e::CPMB_OKSTOP || eCopyState==copy_macroblock_e::CPMB_ERROR)
			{
				//tstr.Printf(_T("Building %i directory entries"),iNumSegments);
				//tickerdlg->Update(infile->Tell(),tstr);
				unsigned int fdiroffset = outfile->BaseStream->Position; // remember offset of directory entries

				VERBOSE_MESSAGE( debugMaster, System::String::Format("DemoFix: building directroy entries for {0} segments ...", iNumSegments) );

				outfile->Write( iNumSegments ); // write number of directroy entries;

				// spew out directory entries (if any):
				while (iNumSegments>0)
				{

					hldemo_dir_entry_s ^curentry = direntries_que.Dequeue();
					iNumSegments--;

					curentry->write( outfile );
				}

				// patch dir_offset in header:
				outfile->Seek( hldemo_header_s::size-sizeof(unsigned int), System::IO::SeekOrigin::Begin );
				outfile->Write( fdiroffset );

				// that's it guys.
				INFO_MESSAGE( debugMaster, "DemoFix: Finished." );
				//tickerdlg->Update(infile->Length());
				bOK=true;
			}

		} else ERROR_MESSAGE( debugMaster, "DemoFix: Failed to write demo header." );

	} else ERROR_MESSAGE( debugMaster, "DemoFix: Failed to read demo header." );

	delete header;

	return bOK;
}

bool CHlaeDemoFix::read_header( System::IO::BinaryReader ^infile, hldemo_header_s ^header )
{
	int ilen;
	int i;
	bool bOk;
	System::Text::Encoding^ encoding = System::Text::Encoding::UTF8;

	//wxString tstr;

	if (!(header->read( infile ))) return false;

	bOk=true;
	for (int i=0;i<header->magic->Length;i++)
		bOk = bOk && (header->magic[i] == HLDEMO_MAGIC[i]);
	if (! bOk )
	{
		ERROR_MESSAGE( debugMaster, "DemoTools: file identifier invalid" );
		return false;
	}
	VERBOSE_MESSAGE( debugMaster, System::String::Format( "File identifier: {0}", encoding->GetString(header->magic)) );

	//iread = sizeof(header->demo_version);
	//if (iread != infile->Read(&(header->demo_version),iread)) return false;
	if (HLDEMO_DEMO_VERSION != header->demo_version)
	{
		WARNING_MESSAGE( debugMaster, System::String::Format("Demo version is: {0}, but I expected {1}. (ignoring)",header->demo_version,HLDEMO_DEMO_VERSION) );
	} else {
		VERBOSE_MESSAGE( debugMaster, System::String::Format("Demo version: {0}",header->demo_version) );
	}

	//iread = sizeof(header->network_version);
	//if (iread != infile->Read(&(header->network_version),iread)) return false;
	if (HLDEMO_NETWORK_VERSION != header->network_version)
	{
		WARNING_MESSAGE( debugMaster, System::String::Format("Network version is: {0}, but I expected {1}. (ignoring)",header->network_version,HLDEMO_NETWORK_VERSION) );
	} else {
		VERBOSE_MESSAGE( debugMaster, System::String::Format("Network version: {0}",header->network_version) );
	}

	ilen=header->map_name->Length;
	bOk=false;
	for (i=0;i<ilen;i++)
		if (header->map_name[i]==0)
		{
			bOk=true;
			break;
		}
	;
	if (!bOk)
	{
		WARNING_MESSAGE( debugMaster, "map_name string malformed, forcing term \\0" );
		header->map_name[ilen-1]=0;
	}
	VERBOSE_MESSAGE( debugMaster, System::String::Format("Map name: {0}", encoding->GetString(header->map_name)) );

	ilen=header->game_dll->Length;
	bOk=false;
	for (i=0;i<ilen;i++)
		if (header->game_dll[i]==0)
		{
			bOk=true;
			break;
		}
	;
	if (!bOk)
	{
		WARNING_MESSAGE( debugMaster, "game_dll string malformed, forcing term \\0" );
		header->game_dll[ilen-1]=0;
	}
	VERBOSE_MESSAGE( debugMaster, System::String::Format("Game DLL: {0}", encoding->GetString(header->game_dll)) );

	// apply header overrides:

	if( this->bSet_NetworkVersion )
	{
		header->network_version = this->uiSet_NetWorkVersion;
	}

	if( this->bSet_GameDll )
	{
		System::Text::Encoding^ uenc = System::Text::Encoding::Unicode;
		array<unsigned char> ^trgt = uenc->GetBytes( this->strSet_GameDll );

		trgt = System::Text::Encoding::Convert( System::Text::Encoding::Unicode, System::Text::Encoding::UTF8, trgt);
		int i;
		int l = trgt->Length;
		if( l > 259 ) l = 259;
		for( i=0; i < l; i++)
		{
			header->game_dll[i] = trgt[i];
		}
		header->game_dll[i] = 0;
	}

	//iread = sizeof(header->dir_offset);
	//if (iread != infile->Read(&(header->dir_offset),iread)) return false;
	return true;
}

void CHlaeDemoFix::watermark_header(hldemo_header_s ^header)
{
	bool bMark1=false;
	bool bMark2=false;
	for (int i=0;i<260;i++)
	{
		if (bMark1)
			header->game_dll[i] = _watermark260[i];
		else
			bMark1 = 0 == header->game_dll[i];
		if (bMark2)
			header->map_name[i] = _watermark260[i];
		else
			bMark2 = 0 == header->map_name[i];
	}
}

bool CHlaeDemoFix::write_header( System::IO::BinaryWriter ^outfile, hldemo_header_s ^header )
{
	return header->write( outfile );
}

bool CHlaeDemoFix::copy_bytes( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, size_t cbCopy )
{
	if (cbCopy<0)
		return false;

	#define BUFFER_SIZE 1024

	bool bRes = true;
	array<unsigned char> ^pdata;
	int readbytes = BUFFER_SIZE;

	
	while (cbCopy>0)
	{
		try
		{
			if (cbCopy<BUFFER_SIZE) readbytes=(int)cbCopy; 

			pdata = infile->ReadBytes( readbytes );
			outfile->Write( pdata );

			cbCopy -= readbytes;
		} catch ( System::Exception ^e )
		{
			ERROR_MESSAGE( debugMaster, System::String::Format("Generic Exception when copying bytes: {0}", e) );
			bRes=false;
			break;
		}
	}

	return bRes;
}

CHlaeDemoFix::copy_macroblock_e CHlaeDemoFix::copy_macroblock( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, hldemo_macroblock_header_s ^pblock_header )
{
	copy_macroblock_e bReturn=copy_macroblock_e::CPMB_OK;

	unsigned int dwreadbytes;
	
	long long fsize;
	long long fpos;
	long long ftarget;

	hldemo_macroblock_header_s ^macroblock_header=gcnew hldemo_macroblock_header_s();

	// let's define two things using the tools of the devil (macros):

	#define COPY_BYTES(numofbytes) \
		copy_bytes((infile->BaseStream->Seek(fpos, System::IO::SeekOrigin::Begin),infile),outfile,numofbytes)

	#define RETURN_REWIND(returnthis) \
		( \
		fpos < fsize ? infile->BaseStream->Seek(fpos, System::IO::SeekOrigin::Begin), returnthis : copy_macroblock_e::CPMB_FATALERROR \
		)

	#define RETURN_BLOCK(returnthis) \
		( \
		(pblock_header ? (void)(pblock_header=macroblock_header) : (void)0 ), returnthis \
		)

	#if HLAE_HLDEM_ENABLESLOWDEBUG
		#pragma message("warning: CHlaeDemoFix::copy_macroblock_e: DebugMessages enabled, this can be extremely slow!")
		#define MYDEBUGMESSAGE(thismsg) DEBUG_MESSAGE( debugMaster, System::String::Format( "{0}: {1}",(unsigned int)fpos,thismsg) );
	#else
		#define MYDEBUGMESSAGE(thismsg)
	#endif

	// get filesize and backup filepos:
	fsize=infile->BaseStream->Length;
	fpos=infile->BaseStream->Position;

	{
		int newPercentage = fsize ? (fpos*100) / fsize : 100;
		if(newPercentage != m_LastPercentage && nullptr != OnDemoFixProgress) {
			m_LastPercentage = newPercentage;
			OnDemoFixProgress(this, newPercentage);
		}
	}

	// read macroblock_header:
	if (! macroblock_header->read( infile ) )
		return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

	// parse macroblock:
	switch (macroblock_header->type)
	{
	case 0:
	case 1:
		MYDEBUGMESSAGE("0/1 game data")

		if (infile->BaseStream->Seek(464,System::IO::SeekOrigin::Current) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
		try { dwreadbytes = infile->ReadUInt32(); } catch ( System::Exception ^) { return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR); }

		if (_bEnableHltvFix || this->bSet_ProtocolVersion )
		{
			if ((ftarget = infile->BaseStream->Position)+dwreadbytes > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
			if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
			
			if (copy_gamedata(infile,outfile,dwreadbytes)!= copy_macroblock_e::CPMB_OK) return copy_macroblock_e::CPMB_FATALERROR;
		} else {
			if ((ftarget = infile->BaseStream->Position+dwreadbytes) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
			if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		}
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 2:
		MYDEBUGMESSAGE("2 unknown empty")

		ftarget = infile->BaseStream->Position;
		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 3:
		MYDEBUGMESSAGE("3 client command")

		if (_bEnableDemoCleanUp)
		{
			if((ftarget = infile->BaseStream->Position) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
			if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
			if (copy_command(infile,outfile)!=copy_macroblock_e::CPMB_OK) return copy_macroblock_e::CPMB_FATALERROR;
		} else {
			if((ftarget = infile->BaseStream->Position+64) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
			if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		}

		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 4:
		MYDEBUGMESSAGE("4 unknown data")

		if((ftarget=infile->BaseStream->Position+32) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 5:
		MYDEBUGMESSAGE("5 last in segment")

		ftarget = infile->BaseStream->Position;
		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OKSTOP);
	case 6:
		MYDEBUGMESSAGE("6 unknown")

		if((ftarget=infile->BaseStream->Position+84) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 7:
		MYDEBUGMESSAGE("7 unknown")

		if((ftarget=infile->BaseStream->Position+8) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 8:
		MYDEBUGMESSAGE("8 sound")

		if(infile->BaseStream->Seek(4,System::IO::SeekOrigin::Current) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
		try { dwreadbytes = infile->ReadUInt32(); } catch (System::Exception ^) { return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR); }
		if (infile->BaseStream->Seek(dwreadbytes,System::IO::SeekOrigin::Current) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
		if ((ftarget = infile->BaseStream->Position+16) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	case 9:
		MYDEBUGMESSAGE("9 dynamic length data")

		try { dwreadbytes = infile->ReadUInt32(); } catch (System::Exception ^) { return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR); }
		if ((ftarget = infile->BaseStream->Position+dwreadbytes) > fsize) return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);

		if (!COPY_BYTES(ftarget-fpos)) return copy_macroblock_e::CPMB_FATALERROR;
		return RETURN_BLOCK(copy_macroblock_e::CPMB_OK);
	}

	// unsupported block type
	MYDEBUGMESSAGE("unknown block type")

	// return OK:
	return RETURN_REWIND(copy_macroblock_e::CPMB_ERROR);
}

CHlaeDemoFix::copy_macroblock_e CHlaeDemoFix::copy_command( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile )
{
	copy_macroblock_e bReturn=copy_macroblock_e::CPMB_OK;
	array<unsigned char> ^stmp;
	
	// read:
	try { stmp = infile->ReadBytes( 64 ); } catch (System::Exception ^)
	{
		ERROR_MESSAGE( debugMaster, "failed reading command from demo" );
		return copy_macroblock_e::CPMB_FATALERROR;
	}

	// check in debug mode:
	#ifdef _DEBUG
		//if (strnlen(stmp,64)>=64) g_debug.SendMessage(_T("demo command not null terminated, forcing \\0"),hlaeDEBUG_WARNING);
	#endif

	stmp[63]=0; // force term

	// process mappings:
	for each (cmd_mapping_s ^curmap in _CommandMap)
	{
		bool bEqual = true;
		int ilen = curmap->src->Length;
		for (int i=0;i<ilen;i++)
		{
			bEqual = bEqual && (curmap->src[i] == stmp[i]);
			if (0==stmp[i]) break;
		}
		if (bEqual)
		{
			bool bWaterMark=false;
			for (int i=0;i<stmp->Length;i++)
			{
				if (bWaterMark)
					stmp[i]=_watermark64[i];
				else
					stmp[i]=curmap->dst[i];
				if(0==stmp[i])
					bWaterMark = _bEnableWaterMarks;
			}
			break;
		}
	}

	// write:
	try { outfile->Write( stmp ); } catch (System::Exception ^)
	{
		ERROR_MESSAGE( debugMaster, "failed writing command to demo" );
		return copy_macroblock_e::CPMB_FATALERROR;
	}

	return bReturn;
}

CHlaeDemoFix::copy_macroblock_e CHlaeDemoFix::copy_gamedata( System::IO::BinaryReader ^infile, System::IO::BinaryWriter ^outfile, unsigned int dwreadbytes )
{
	// for debugging errors:
	//wxString tstr;
	unsigned int dwreadorg = dwreadbytes;
	unsigned int dwreadlast = dwreadbytes;
	size_t initialpos = infile->BaseStream->Position;

	unsigned char cmdcode;

	#define PRINT_FERROR(cmdcodethis,returnthis) \
		( \
			ERROR_MESSAGE( debugMaster, System::String::Format("DemoTools: copy_gamedata failed at {0} in block {1} when parsing cmd {2} at {3}",(unsigned int)initialpos+dwreadorg-dwreadbytes,(unsigned int)initialpos,(unsigned int)cmdcode,(unsigned int)initialpos+dwreadorg-dwreadlast) ) \
			, returnthis \
		)

	unsigned char ctmp;
	unsigned int uitmp;
	unsigned short ustmp;

	while (dwreadbytes>0)
	{
		dwreadlast = dwreadbytes;

		// get cmd code:
		try {
			cmdcode = infile->ReadByte();
			outfile->Write( cmdcode );
		} catch (System::Exception ^)
		{
			return PRINT_FERROR(0,copy_macroblock_e::CPMB_FATALERROR);
		}

		dwreadbytes--;

		switch (cmdcode)
		{
		case svc_nop: // 1
			continue;
		
		case svc_time: // 7
			if (dwreadbytes<4) break; // invalid, let copy bytes handle it
			if (!copy_bytes(infile,outfile,4)) return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
			dwreadbytes-=4;
			continue;
		
		case svc_print: // 8
			// Read string:
			while (dwreadbytes>0)
			{
				dwreadbytes--;
				try {
					ctmp = infile->ReadByte();
					outfile->Write( ctmp );
				} catch (System::Exception ^)
				{
					return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
				}
				if (ctmp==0) break; // string end, get out of here
			}
			continue;

		case svc_serverinfo: // 11
			DEBUG_MESSAGE( debugMaster, System::String::Format("found svc_serverinfo at {0}",(unsigned int)initialpos+dwreadorg-dwreadbytes));
			uitmp = 4 +		4 + 4 + 16;					
			if (dwreadbytes<uitmp+1) break; // invalid, let copy bytes handle it

			if( this->bSet_ProtocolVersion )
			{
				try {
					ustmp = infile->ReadUInt16();
					ustmp = (unsigned short)(this->ui_ProtoVersion);
					outfile->Write( ustmp );
					uitmp -= 2;
					dwreadbytes -= 2;
				} catch (System::Exception ^)
				{
					return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
				}
			}

			if (!copy_bytes(infile,outfile,uitmp)) return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
			dwreadbytes-=uitmp;
			
			// now we are at the Holy Grail of maxclients
			try { ctmp = infile->ReadByte(); } catch (System::Exception ^) { return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR); }
			if (ctmp<DEMOFIX_MAXPLAYERS)
			{
				if( this->_bEnableHltvFix ) ctmp++; // add a slut slot
				_ucHltvFixBell = 0; // ring the bell happy :)
			} else if (_ucHltvFixBell) _ucHltvFixBell = 1; // ring the bell sad :..(
			try { outfile->Write( ctmp ); } catch (System::Exception ^) { return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR); }

			VERBOSE_MESSAGE( debugMaster, System::String::Format("dem_forcehltv 1 fix: {0}: serverinfo maxplayers: {1} -> {2}",(unsigned int)(infile->BaseStream->Position),(unsigned int)(ctmp==DEMOFIX_MAXPLAYERS? ctmp : ctmp-1),(unsigned int)ctmp) );

			dwreadbytes-=1;

			if( this->bFuckOff )
			{
				// Copy "till end of "cstrike":
				uitmp = 9;
				if (!copy_bytes(infile,outfile,uitmp)) return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
				dwreadbytes-=uitmp;

				// patch blocklength:
				size_t mcurpos = outfile->BaseStream->Position;
				outfile->Seek( - (infile->BaseStream->Position - initialpos) - 4, System::IO::SeekOrigin::Current );
				outfile->Write( dwreadorg +5 );
				outfile->Seek( mcurpos, System::IO::SeekOrigin::Begin );

				// insert "_beta":
				ctmp = '_'; outfile->Write(ctmp);
				ctmp = 'b'; outfile->Write(ctmp);
				ctmp = 'e'; outfile->Write(ctmp);
				ctmp = 't'; outfile->Write(ctmp);
				ctmp = 'a'; outfile->Write(ctmp);

			}

			break; // our work is done here, get us out of here!
		
		case svc_hltv: // 50
			if (dwreadbytes<2) break; // dunno let copy bytes handle it

			try {
				ctmp = infile->ReadByte();
				dwreadbytes--;
				outfile->Write( ctmp );
			} catch (System::Exception ^)
			{
				return PRINT_FERROR(cmdcode,copy_macroblock_e::CPMB_FATALERROR);
			}

			if(HLTV_ACTIVE != ctmp) break; // dunno let copy bytes handle it

			continue;
		}

		break; // not handled / unknown, cannot continue
	}

	// copy unhandled data:
	if (dwreadbytes>0 && !copy_bytes(infile,outfile,dwreadbytes)) return PRINT_FERROR(0,copy_macroblock_e::CPMB_FATALERROR);

	return copy_macroblock_e::CPMB_OK;
}