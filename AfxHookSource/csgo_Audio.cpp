#include "stdafx.h"

#include "csgo_Audio.h"

#include "RenderView.h"
#include "SourceInterfaces.h"
#include "addresses.h"
#include <shared/detours.h>

#include <string>
#include <mutex>
#include <map>
#include <sstream>


class CCsgoAudioWaveFile
{
public:
	CCsgoAudioWaveFile(char const * fileName, int numChannels)
	{
		DWORD dwSamplesPerSec = 44100;

		memset(&m_WaveHeader, 0, sizeof(m_WaveHeader)); // clear header

		m_WaveSamplesWritten = 0; // clear written samples num

		m_File = fopen(fileName, "wb");

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

	void Append(int numChannels, WORD * data)
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

	~CCsgoAudioWaveFile()
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


typedef void(__stdcall * CAudioXAudio2_UnkSupplyAudio_t)(DWORD * this_ptr, int numChannels, float * audioData);
typedef void(__cdecl * csgo_MIX_PaintChannels_t)(int paintCountTarget, int unknown);

CAudioXAudio2_UnkSupplyAudio_t detoured_CAudioXAudio2_UnkSupplyAudio;

csgo_MIX_PaintChannels_t detoured_csgo_MIX_PaintChannels;

std::mutex g_csgo_Audio_Mutex;
double g_csgo_Audio_Remainder = 0;
bool g_csgo_Audio_FRAME_START = false;
bool g_CAudioXAudio2_RecordAudio_Active = false;
std::string g_CAudioXAudio2_RecordAudio_Dir;
std::map<DWORD *, CCsgoAudioWaveFile> g_CAudioXAudio2_RecordAudio_Files;

std::vector<WORD> g_CAudioXAudio2_ChannelData;

void __stdcall touring_CAudioXAudio2_UnkSupplyAudio(DWORD * this_ptr, int numChannels, float * audioData)
{
	if (g_CAudioXAudio2_RecordAudio_Active)
	{
		std::map<DWORD *, CCsgoAudioWaveFile>::iterator it = g_CAudioXAudio2_RecordAudio_Files.find(this_ptr);

		if (it == g_CAudioXAudio2_RecordAudio_Files.end())
		{
			std::ostringstream os;
			os << g_CAudioXAudio2_RecordAudio_Dir << "\\audio_" << this_ptr << ".wav";
			std::string fileName = os.str();

			it = g_CAudioXAudio2_RecordAudio_Files.emplace(std::piecewise_construct, std::forward_as_tuple(this_ptr), std::forward_as_tuple(fileName.c_str(), numChannels + 2)).first;
		}

		const int samples = 512;

		if ((int)g_CAudioXAudio2_ChannelData.size() < numChannels)
			g_CAudioXAudio2_ChannelData.resize(numChannels);

		for (int j = 0; j < samples; ++j)
		{
			for (int i = 0; i < numChannels; ++i)
			{
				float fVal = audioData[i*samples + j];
				fVal = min(max(fVal, -32768), 32767);
				fVal = std::round(fVal);
				int iVal = (int)fVal;
				WORD wVal = (WORD)iVal;
				g_CAudioXAudio2_ChannelData[i] = wVal;
			}

			it->second.Append(numChannels, &g_CAudioXAudio2_ChannelData[0]);
		}

	}

	detoured_CAudioXAudio2_UnkSupplyAudio(this_ptr, numChannels, audioData);
}


void __cdecl touring_csgo_MIX_PaintChannels(int paintCountTarget, int unknown)
{
	std::unique_lock<std::mutex> lock(g_csgo_Audio_Mutex);

	if (!g_CAudioXAudio2_RecordAudio_Active)
	{
		detoured_csgo_MIX_PaintChannels(paintCountTarget, unknown);
		return;
	}

	if (!g_csgo_Audio_FRAME_START)
		return;

	g_csgo_Audio_FRAME_START = false;

	WrpGlobals * glob = g_Hook_VClient_RenderView.GetGlobals();

	if (!glob)
		return;

	float frameTime = glob->absoluteframetime_get();

	double fNumCalls = (frameTime * 44100.0 / 512.0) + g_csgo_Audio_Remainder;
	int numCalls = (int)fNumCalls;
	g_csgo_Audio_Remainder = fNumCalls - numCalls;

	while (0 < numCalls)
	{
		--numCalls;
		detoured_csgo_MIX_PaintChannels(paintCountTarget, unknown);

		paintCountTarget += 512;
	}
}


bool csgo_Audio_Install(void)
{
	static bool firstResult = true;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(csgo_CAudioXAudio2_vtable)
		&& AFXADDR_GET(csgo_MIX_PaintChannels))
	{
		detoured_csgo_MIX_PaintChannels = (csgo_MIX_PaintChannels_t)DetourApply((BYTE*)AFXADDR_GET(csgo_MIX_PaintChannels), (BYTE *)touring_csgo_MIX_PaintChannels, (int)AFXADDR_GET(csgo_MIX_PaintChannels_DSZ));

		int * vtable = (int*)AFXADDR_GET(csgo_CAudioXAudio2_vtable);

		DetourIfacePtr((DWORD *)&(vtable[1]), touring_CAudioXAudio2_UnkSupplyAudio, (DetourIfacePtr_fn &)detoured_CAudioXAudio2_UnkSupplyAudio);
	}
	else
		firstResult = false;

	return firstResult;
}

bool csgo_Audio_StartRecording(char const * ansiTakeDir)
{
	if (!csgo_Audio_Install())
		return false;

	csgo_Audio_EndRecording();

	std::unique_lock<std::mutex> lock(g_csgo_Audio_Mutex);

	g_CAudioXAudio2_RecordAudio_Dir = ansiTakeDir;
	g_CAudioXAudio2_RecordAudio_Active = true;
	g_csgo_Audio_Remainder = 0;

	return true;
}

void csgo_Audio_EndRecording(void)
{
	std::unique_lock<std::mutex> lock(g_csgo_Audio_Mutex);

	if (!g_CAudioXAudio2_RecordAudio_Active)
		return;

	g_CAudioXAudio2_RecordAudio_Files.clear();

	g_CAudioXAudio2_RecordAudio_Active = false;
}

void csgo_Audio_FRAME_START(void)
{
	std::unique_lock<std::mutex> lock(g_csgo_Audio_Mutex);

	g_csgo_Audio_FRAME_START = true;
}
