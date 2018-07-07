#include "stdafx.h"

#include "MirvWav.h"

CMirvWav::CMirvWav(const wchar_t * fileName, int numChannels, DWORD dwSamplesPerSec)
{
	memset(&m_WaveHeader, 0, sizeof(m_WaveHeader)); // clear header

	m_WaveSamplesWritten = 0; // clear written samples num

	m_File = _wfopen(fileName, L"wb");

	if (!m_File)
	{
		return;
	}

	// write temporary header:
	memcpy(m_WaveHeader.riff_hdr.id, "RIFF", 4);
	m_WaveHeader.riff_hdr.len = 0;

	memcpy(m_WaveHeader.wave_id, "WAVE", 4);

	memcpy(m_WaveHeader.fmt_chunk_hdr.id, "fmt ", 4);
	m_WaveHeader.fmt_chunk_hdr.len = sizeof(m_WaveHeader.fmt_chunk_pcm);

	m_WaveHeader.fmt_chunk_pcm.wFormatTag = 0x0001; // Microsoft PCM
	m_WaveHeader.fmt_chunk_pcm.wChannels = numChannels;
	m_WaveHeader.fmt_chunk_pcm.dwSamplesPerSec = dwSamplesPerSec;
	m_WaveHeader.fmt_chunk_pcm.dwAvgBytesPerSec = numChannels * dwSamplesPerSec * (16 / 8);
	m_WaveHeader.fmt_chunk_pcm.wBlockAlign = numChannels * (16 / 8);
	m_WaveHeader.fmt_chunk_pcm.wBitsPerSample = 16;

	memcpy(m_WaveHeader.data_chunk_hdr.id, "data", 4);
	m_WaveHeader.data_chunk_hdr.len = 0;

	fwrite(&m_WaveHeader, sizeof(m_WaveHeader), 1, m_File);
}

void CMirvWav::Append(int numChannels, WORD * data)
{
	if (!m_File)
		return;

	for (int i = 0; i < m_WaveHeader.fmt_chunk_pcm.wChannels; ++i)
	{
		WORD curData = i < numChannels ? data[i] : 0;

		fwrite(&curData, sizeof(WORD), 1, m_File);
	}

	++m_WaveSamplesWritten;
}

CMirvWav::~CMirvWav()
{
	if (!m_File) return;

	long lfpos = ftell(m_File);

	fseek(m_File, 0, SEEK_SET);

	// we need fo finish the header:
	m_WaveHeader.riff_hdr.len = lfpos - 4;
	m_WaveHeader.data_chunk_hdr.len = m_WaveSamplesWritten * (m_WaveHeader.fmt_chunk_pcm.wBitsPerSample / 8) * m_WaveHeader.fmt_chunk_pcm.wChannels;

	// and write it:
	fwrite(&m_WaveHeader, sizeof(m_WaveHeader), 1, m_File);

	fclose(m_File);
}

