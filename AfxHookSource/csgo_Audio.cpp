#include "stdafx.h"

#include "csgo_Audio.h"

#include "RenderView.h"
#include "SourceInterfaces.h"
#include "addresses.h"
#include "MirvWav.h"
#include "MirvTime.h"
#include "WrpConsole.h"

#include <shared/AfxDetours.h>

#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <map>
#include <sstream>
#include <set>


using namespace std::chrono_literals;

typedef void * WaveAppendTmpFile_t;
typedef void(__fastcall* CVideoMode_Common_WriteMovieFrame_t)(void* This, void* Edx, void* pInfo);
typedef bool(__fastcall* CEngineVgui_ConIsVisible_t)(void* This, void* Edx);
typedef void(__cdecl* S_ExtraUpdate_t)(void);
typedef void(__cdecl* S_Update__t)(float mixAheadTime);
typedef void(__cdecl* CL_StartMovie_t)(void);

WaveAppendTmpFile_t detoured_WaveAppendTmpFile;
CVideoMode_Common_WriteMovieFrame_t detoured_CVideoMode_Common_WriteMovieFrame;
CEngineVgui_ConIsVisible_t detoured_CEngineVgui_ConIsVisible;

DWORD g_csgo_Audio_EngineThreadId = 0;
bool g_csgo_Audio_Record = false;
std::wstring g_CAudioXAudio2_RecordAudio_Dir;
CMirvWav* g_CAudioXAudio2_RecordAudio_File = nullptr;;

std::vector<WORD> g_CAudioXAudio2_ChannelData;


bool __cdecl My_WaveAppendTmpFile(void* buffer, int sampleBits, int numSamples)
{
	if (g_csgo_Audio_Record && GetCurrentThreadId() == g_csgo_Audio_EngineThreadId)
	{
		//Tier0_Msg("Calling CAudioXAudio2_UnkSupplyAudio.\n");

		const int samplesPerChannel = 512;
		const int numChannels = numSamples / samplesPerChannel;

		if (nullptr == g_CAudioXAudio2_RecordAudio_File)
		{
			std::wostringstream os;
			os << g_CAudioXAudio2_RecordAudio_Dir << L"\\audio.wav";
			std::wstring fileName = os.str();

			g_CAudioXAudio2_RecordAudio_File = new CMirvWav(fileName.c_str(), numChannels, 44100);
		}

		if ((int)g_CAudioXAudio2_ChannelData.size() < numChannels)
			g_CAudioXAudio2_ChannelData.resize(numChannels);

		for(int i=0; i < samplesPerChannel; i++)
			g_CAudioXAudio2_RecordAudio_File->Append(numChannels, &(((WORD *)buffer)[i*numChannels]));

		// Pass through in case someone is recording WAV with startmovie atm:
		if (0 != strcmp(":afx", (char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename)))
			return true;

		return false;
	}

	return true;
}

__declspec(naked) void __cdecl touring_WaveAppendTmpFile() {
	__asm push eax
	__asm push ecx
	__asm push ebp
	__asm mov ebp, esp

	__asm sub esp, __LOCAL_SIZE

	__asm mov eax, [ebp + 0x14]
	__asm push eax // numSamples
	__asm push 16 // sampleBits
	__asm push edx // buffer
	__asm call My_WaveAppendTmpFile
	__asm pop edx

	__asm cmp al, 0
	__asm jnz __continue_call
	__asm mov esp, ebp
	__asm pop ebp
	__asm pop ecx
	__asm pop eax
	__asm ret

	__asm __continue_call:
	__asm mov esp, ebp
	__asm pop ebp
	__asm pop ecx
	__asm pop eax
	__asm jmp [detoured_WaveAppendTmpFile]
}

bool __fastcall touring_CEngineVgui_ConIsVisible(void* This, void* Edx) {

	// The engine checks when recording startmovie wav (which we force it temporarily into) if the console is down (if yes it doesn't record), so let it think it is not.
	if (g_csgo_Audio_Record && GetCurrentThreadId() == g_csgo_Audio_EngineThreadId && 0 == strcmp(":afx", (char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename)))
		return false;

	return detoured_CEngineVgui_ConIsVisible(This, Edx);
}

void __fastcall touring_CVideoMode_Common_WriteMovieFrame(void* This, void* Edx, void* pInfo) {

	// Prevent video frame capture if no one is using startmovie atm:
	if (g_csgo_Audio_Record && GetCurrentThreadId() == g_csgo_Audio_EngineThreadId && 0 == strcmp(":afx", (char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename)))
		return;

	detoured_CVideoMode_Common_WriteMovieFrame(This, Edx, pInfo);
}


typedef float (__cdecl * csgo_engine_S_MixAsync_t)(void);
csgo_engine_S_MixAsync_t detoured_csgo_engine_S_MixAsync;
std::mutex g_SoundRecordingMutex;
bool g_SoundRecordingValue = false;

float __cdecl touring_csgo_engine_S_MixAsync(void) {
	static float lastResult = 120.0f;

	std::unique_lock<std::mutex> lock(g_SoundRecordingMutex);

	if(g_SoundRecordingValue) {
		return -1;
	}

	lastResult = detoured_csgo_engine_S_MixAsync();
	return lastResult;
}

bool csgo_Audio_Install(void)
{
	static bool firstResult = true;
	static bool firstRun = true;

	if (!firstRun) return firstResult;
	firstRun = false;

	if (
		AFXADDR_GET(csgo_engine_WaveAppendTmpFile)
		&& AFXADDR_GET(csgo_engine_cl_movieinfo_moviename)
		&& AFXADDR_GET(csgo_engine_CVideoMode_Common_vtable)
		&& AFXADDR_GET(csgo_engine_CEngineVGui_vtable)
		&& AFXADDR_GET(csgo_engine_CL_StartMovie)
		&& AFXADDR_GET(csgo_engine_S_MixAsync)
		) {

		detoured_WaveAppendTmpFile = (WaveAppendTmpFile_t)AFXADDR_GET(csgo_engine_WaveAppendTmpFile);
		detoured_CVideoMode_Common_WriteMovieFrame = (CVideoMode_Common_WriteMovieFrame_t)(((void**)AFXADDR_GET(csgo_engine_CVideoMode_Common_vtable))[23]);
		detoured_CEngineVgui_ConIsVisible = (CEngineVgui_ConIsVisible_t)(((void**)AFXADDR_GET(csgo_engine_CEngineVGui_vtable))[18]);
		detoured_csgo_engine_S_MixAsync = (csgo_engine_S_MixAsync_t)AFXADDR_GET(csgo_engine_S_MixAsync);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)detoured_WaveAppendTmpFile, touring_WaveAppendTmpFile);
		DetourAttach(&(PVOID&)detoured_CVideoMode_Common_WriteMovieFrame, touring_CVideoMode_Common_WriteMovieFrame);
		DetourAttach(&(PVOID&)detoured_CEngineVgui_ConIsVisible, touring_CEngineVgui_ConIsVisible);
		DetourAttach(&(PVOID&)detoured_csgo_engine_S_MixAsync, touring_csgo_engine_S_MixAsync);
		LONG error = DetourTransactionCommit();
		firstResult = NO_ERROR == error;
	}
	else
		firstResult = false;

	return firstResult;
}

__declspec(naked) void __fastcall call_CL_StartMovie(const char * filename, int flags, int nWidth, int nHeight, float flFrameRate, int nJpegQuality, void * dummy) {
	
	__asm pop eax
	__asm mov [esp+0x10], eax

	__asm mov eax, AFXADDR_GET(csgo_engine_CL_StartMovie)
	__asm call eax

	__asm add esp, 0x10
	__asm ret
}

bool csgo_Audio_StartRecording(const wchar_t * ansiTakeDir)
{
	if (!csgo_Audio_Install())
		return false;

	csgo_Audio_EndRecording();

	g_CAudioXAudio2_RecordAudio_Dir = ansiTakeDir;
	g_csgo_Audio_EngineThreadId = GetCurrentThreadId();
	g_csgo_Audio_Record = true;

	// If we are not already recording with startmovie:
	if ('\0' == *(char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename)) {
		// Make it still init sound related variables without actually creating stuff

		// Work around threading bug in CS:GO code (other wise there can be crashes).
		// Wait for active async sound mix thread to shut down / Signal to it to shut down in the next schedule.
		{
			std::unique_lock<std::mutex> lock(g_SoundRecordingMutex);
			g_SoundRecordingValue = true;
		}
		
		WrpConVarRef host_framerate_cvar("host_framerate");
		float oldHostFrameRateValue = host_framerate_cvar.GetFloat();

		call_CL_StartMovie(
			":afx", // fake filename
			1 << 2, // do wav
			0, 0, 0, 0, nullptr);

		host_framerate_cvar.SetValue(oldHostFrameRateValue);
	}

	return true;
}

void csgo_Audio_EndRecording(void)
{
	if (!g_csgo_Audio_Record)
		return;

	if (0 == strcmp(":afx", (char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename))) {
		// If they are still recording with us, then stop the fake recording:
		*(char*)AFXADDR_GET(csgo_engine_cl_movieinfo_moviename) = '\0';
	}

	if (g_CAudioXAudio2_RecordAudio_File) {
		delete g_CAudioXAudio2_RecordAudio_File;
		g_CAudioXAudio2_RecordAudio_File = nullptr;
	}

	{
		std::unique_lock<std::mutex> lock(g_SoundRecordingMutex);
		g_SoundRecordingValue = false;
	}

	g_csgo_Audio_Record = false;
}
