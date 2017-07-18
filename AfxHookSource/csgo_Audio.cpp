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

typedef void(__stdcall * CAudioXAudio2_UnkSupplyAudio_t)(DWORD * this_ptr, int numChannels, float * audioData);
typedef void(__cdecl * csgo_MIX_PaintChannels_t)(int paintCountTarget, int unknown);

CAudioXAudio2_UnkSupplyAudio_t detoured_CAudioXAudio2_UnkSupplyAudio;

csgo_MIX_PaintChannels_t detoured_csgo_MIX_PaintChannels;

std::mutex g_csgo_Audio_Mutex;
double g_csgo_Audio_Remainder = 0;
bool g_csgo_Audio_FRAME_START = false;
bool g_CAudioXAudio2_RecordAudio_Active = false;
std::string g_CAudioXAudio2_RecordAudio_Dir;
struct CAudioXAudio2_RecordAudio_Files
{
	std::map<int, FILE *> ChannelFiles;
};
std::map<DWORD *, CAudioXAudio2_RecordAudio_Files> g_CAudioXAudio2_RecordAudio_Files;

void __stdcall touring_CAudioXAudio2_UnkSupplyAudio(DWORD * this_ptr, int numChannels, float * audioData)
{
	if (g_CAudioXAudio2_RecordAudio_Active)
	{
		if (g_CAudioXAudio2_RecordAudio_Files.find(this_ptr) == g_CAudioXAudio2_RecordAudio_Files.end())
		{
			for (int i = 0; i < numChannels; ++i)
			{
				std::ostringstream os;
				os << g_CAudioXAudio2_RecordAudio_Dir << "\\audio_" << this_ptr << "_" << i << ".raw";

				std::string fileName = os.str();

				g_CAudioXAudio2_RecordAudio_Files[this_ptr].ChannelFiles[i] = fopen(fileName.c_str(), "wb");
			}
		}

		const int samples = 512;

		for (int i = 0; i < numChannels; ++i)
		{
			FILE * file = g_CAudioXAudio2_RecordAudio_Files[this_ptr].ChannelFiles[i];

			for (int j = 0; j < samples; ++j)
			{
				float fVal = audioData[i*samples + j];
				fVal = min(max(fVal, -32768), 32767);
				fVal = std::round(fVal);
				int iVal = (int)fVal;
				WORD wVal = (WORD)iVal;

				fwrite(&wVal, sizeof(wVal), 1, file);
			}
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

	for (std::map<DWORD *, CAudioXAudio2_RecordAudio_Files>::iterator it = g_CAudioXAudio2_RecordAudio_Files.begin(); it != g_CAudioXAudio2_RecordAudio_Files.end(); ++it)
	{
		for (std::map<int, FILE *>::iterator it2 = it->second.ChannelFiles.begin(); it2 != it->second.ChannelFiles.end(); ++it2)
		{
			fclose(it2->second);
		}
	}

	g_CAudioXAudio2_RecordAudio_Files.clear();

	g_CAudioXAudio2_RecordAudio_Active = false;
}

void csgo_Audio_FRAME_START(void)
{
	g_csgo_Audio_FRAME_START = true;
}