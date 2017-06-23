#pragma once

#include <windows.h>
#include <stdio.h> // FILE, ...

void FilmSound_BlockChannels(bool block);

// attention, Timing and flow asumptions:
// this class makes assumptions about the engine's main loop and function call order which you can understand when looking into the Quake 1 Source.
// i.e. it asumes that Start, AdvanceFrame and Stop happen before the hooks kick in.
class CFilmSound
{
public:
	CFilmSound();

	// Only starts when eFilmSoundState()==FSS_IDLE
	// if starting failed it will return false
	// the targettime should be the delta frametime on the first call (so it is not null) and advance with every frame
	bool Start(wchar_t const * fileName, double dTargetTime, float fUseVolume,
		wchar_t const * extraFileName, double extraTime);

	// this has to be called every engineframe (main loop)
	// to supply a new targettime
	// the targettime should advance with every frame (absoulte to starting)
	// it is suggested that you derive your targettime from the framecount to avoid problems with precision (when accumulating deltatime i.e.)
	void AdvanceFrame(double dTargetTime);

	// We cannot stop instantly, we finally stopped when eFilmSoundState()==FSS_IDLE.
	void Stop();

	void Snd_Supply(WORD leftchan, WORD rightchan);

private:

	// wave header structures designed after:
	// General RIFF description provided by
	// Robert Shuler <rlshuler@aol.com>
	// (downloaded form www.wotsit.org)+hopefully without his mistakes : P

	typedef struct
	{			// CHUNK 8-byte header
		char  id[4];	// identifier, e.g. "fmt " or "data"
		DWORD len;		// remaining chunk length after header
	} chunk_hdr_t;

	struct wave_header_s
	{
		struct {     
			char	id[4];	// identifier string = "RIFF"
			DWORD	len;	// remaining length after this header
		} riff_hdr;

		char wave_id[4];	// WAVE file identifier = "WAVE"

		chunk_hdr_t fmt_chunk_hdr; // Fmt chunk header

		struct
		{
			WORD	wFormatTag;			// Format category
			WORD	wChannels;			// Number of channels
			DWORD	dwSamplesPerSec;	// Sampling rate
			DWORD	dwAvgBytesPerSec;	// For buffer estimation
			WORD	wBlockAlign;		// Data block size

			// PCM specific:
			WORD wBitsPerSample;

		} fmt_chunk_pcm;

		chunk_hdr_t data_chunk_hdr; // Fmt chunk header

	};

	struct FilmSoundFile
	{
		FILE * file_handle;
		wave_header_s wave_header;
		DWORD wave_smaples_written;
	};

	FilmSoundFile * _pWaveFile;
	
	FilmSoundFile * m_pWaveFileExtra;
	double m_ExtraTime;

	FilmSoundFile * _fBeginWave(wchar_t const * fileName, DWORD dwSamplesPerSec);
	void _fWriteWave(FilmSoundFile * pfsf,WORD leftchan,WORD rightchan);
	void _fEndWave(FilmSoundFile * pfsf);
};
