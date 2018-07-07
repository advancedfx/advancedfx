#pragma once

#include <windows.h>
#include <stdio.h>

class CMirvWav
{
public:
	CMirvWav(const wchar_t * fileName, int numChannels, DWORD samplesPerSec);

	void Append(int numChannels, WORD * data);

	~CMirvWav();

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

	FILE * m_File;
	wave_header_s m_WaveHeader;
	DWORD m_WaveSamplesWritten;
};

