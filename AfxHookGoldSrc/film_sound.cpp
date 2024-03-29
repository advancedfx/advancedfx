#include "stdafx.h"

// to understand what we do here please look into the Quake 1 (QW) source by ID Software!

//
// includes:
//

#include <hlsdk.h>

#include "film_sound.h"
#include "hl_addresses.h"

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>


// for debug:
extern cl_enginefuncs_s *pEngfuncs;

//
// defs for Quake 1 (reconstructed form Q1 Source + H-L) structs we use:
//

typedef struct
{
	int left;
	int right;
} portable_samplepair_t;

typedef struct
{
	qboolean		gamealive;
	qboolean		soundalive;
	qboolean		splitbuffer;
	int				channels;
	int				samples;				// mono samples in buffer
	int				submission_chunk;		// don't mix less than this #
	int				samplepos;				// in mono samples
	int				samplebits;
	int				speed;
	unsigned char	*buffer;
} dma_t;


typedef struct
{
	qboolean		gamealive;
	qboolean		soundalive;
	qboolean		splitbuffer;
	int				channels;
	int				samples;				// mono samples in buffer
	int				submission_chunk;		// don't mix less than this #
	int				samplepos;				// in mono samples
	int				samplebits;
	int				Quake_speed;
	int				Valve_speed;			// added by Valve(?)
} dma_HL_t;

typedef void (*S_PaintChannels_t)(int endtime);
typedef void (*S_TransferPaintBuffer_t)(int endtime);

S_PaintChannels_t detoured_S_PaintChannels = NULL;
S_TransferPaintBuffer_t detoured_S_TransferPaintBuffer = NULL;

CFilmSound* g_FilmSound = 0;

int g_Last_paintedtime;
double g_TargetTime, g_CurrentTime;
float g_Volume;

bool g_FilmSound_BlockChannels = false;

bool g_FilmSound_Capturing = false;

enum FilmSound_TimeRounding_e {
	FS_TR_FLOOR,
	FS_TR_CEIL
} g_FilmSound_TimeRounding = FS_TR_FLOOR;

void touring_S_PaintChannels(int endtime)
{
	static volatile dma_HL_t *shm;

	if (g_FilmSound_Capturing)
	{
		double dDeltaTime;
		int deltaTime;

		shm = *(dma_HL_t **)HL_ADDR_GET(shm);

		dDeltaTime = (
			g_TargetTime - g_CurrentTime
		) * (double)shm->Quake_speed;

		// and override
		switch(g_FilmSound_TimeRounding)
		{
		case FS_TR_CEIL:
			deltaTime = (int)ceil(dDeltaTime); // we preffer having too much samples when stopping
			break;
		case FS_TR_FLOOR:
		default:
			deltaTime = (int)floor(dDeltaTime); // we preffer having faster updates and therefore less samples during filming
			break;
		}

		// we cannot go back in time, so stfu:
		if(deltaTime < 0) deltaTime = 0;

		dDeltaTime = (double)deltaTime / (double)shm->Quake_speed;

		*(int *)HL_ADDR_GET(paintedtime) = g_Last_paintedtime;

		// >> Sound painting
		detoured_S_PaintChannels(g_Last_paintedtime +deltaTime);
		// << Sound painting

		g_Last_paintedtime = *(int *)HL_ADDR_GET(paintedtime);

		// update Our class's _CurrentTime:
		g_CurrentTime = g_CurrentTime +dDeltaTime;
	}
	else
	{
		// don't do anything abnormal
		detoured_S_PaintChannels(endtime);
	}
}

void touring_S_TransferPaintBuffer(int endtime)
{
	static volatile dma_HL_t *shm;

	if (g_FilmSound_Capturing)
	{
		// filming

		shm=*(dma_HL_t **)HL_ADDR_GET(shm);
		
		int paintedtime = *(int *)HL_ADDR_GET(paintedtime);

		portable_samplepair_t * paintbuffer = (portable_samplepair_t *)HL_ADDR_GET(paintbuffer);

		int iMyVolume = (int)(g_Volume*256.0f);

		int * snd_p = (int *) paintbuffer;
		int lpaintedtime = paintedtime;

		while (lpaintedtime < endtime)
		{
			for (int i=0;i<2;i++)
			{
				int ilchan;
				int irchan;
				WORD wlchan,wrchan;

				// limiter from Snd_WriteLinearBlastStereo16:
				
				ilchan = (snd_p[0]*iMyVolume)>>8;
				if (ilchan > 0x7fff) wlchan = 0x7fff;
				else if (ilchan < (short)0x8000) wlchan = (short)0x8000;
				else wlchan = ilchan;

				irchan = (snd_p[1]*iMyVolume)>>8;
				if (irchan > 0x7fff) wrchan = 0x7fff;
				else if (irchan < (short)0x8000) wrchan = (short)0x8000;
				else wrchan = irchan;

				g_FilmSound->Snd_Supply(wlchan, wrchan);
				snd_p+=2;
			}

			lpaintedtime++;
		}
	}

	// pass through to sound buffer:
	detoured_S_TransferPaintBuffer(endtime);

}

typedef void sfx_t;
typedef void (*S_StartDynamicSound_t)(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation, unsigned int _unknown_7, int _unknown_8);
S_StartDynamicSound_t detoured_S_StartDynamicSound = nullptr;
void touring_S_StartDynamicSound(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation, unsigned int _unknown_7, int _unknown_8) {

	if(g_FilmSound_BlockChannels) return;
	
	detoured_S_StartDynamicSound(entnum, entchannel, sfx, origin, fvol, attenuation,_unknown_7,_unknown_8);
}

bool InstallHooks()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (AFXADDR_GET(S_Update_)
		&& AFXADDR_GET(S_PaintChannels)
		&& AFXADDR_GET(S_TransferPaintBuffer)
		&& AFXADDR_GET(S_StartDynamicSound)
		&& AFXADDR_GET(shm)
		&& AFXADDR_GET(paintedtime)
		&& AFXADDR_GET(paintbuffer)
	) {
		LONG error = NO_ERROR;

		detoured_S_PaintChannels = (S_PaintChannels_t)AFXADDR_GET(S_PaintChannels);
		detoured_S_TransferPaintBuffer = (S_TransferPaintBuffer_t)AFXADDR_GET(S_TransferPaintBuffer);
		detoured_S_StartDynamicSound = (S_StartDynamicSound_t)AFXADDR_GET(S_StartDynamicSound);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)detoured_S_PaintChannels, touring_S_PaintChannels);
		DetourAttach(&(PVOID&)detoured_S_TransferPaintBuffer, touring_S_TransferPaintBuffer);
		DetourAttach(&(PVOID&)detoured_S_StartDynamicSound, touring_S_StartDynamicSound);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nfilm_sound.cpp: InstallHooks");
		}
	}
	else
		firstResult = false;

	return firstResult;
}

void FilmSound_BlockChannels(bool block) {
	g_FilmSound_BlockChannels = block;
}


// CFilmSound //////////////////////////////////////////////////////////////////

CFilmSound::CFilmSound()
{
	if(g_FilmSound) throw "err";

	g_FilmSound = this;
}

CFilmSound::FilmSoundFile * CFilmSound::_fBeginWave(wchar_t const * fileName, DWORD dwSamplesPerSec)
{
	FilmSoundFile * pfsf = new FilmSoundFile();

	memset(&pfsf->wave_header,0,sizeof(pfsf->wave_header)); // clear header

	pfsf->wave_smaples_written = 0; // clear written samples num

	FILE *pHandle;
	_wfopen_s(&pHandle, fileName, L"wb");

	if (!pHandle)
	{
		delete pfsf;
		return NULL;
	}

	// write temporary header:
	memcpy(pfsf->wave_header.riff_hdr.id,"RIFF",4);
	pfsf->wave_header.riff_hdr.len=0;

	memcpy(pfsf->wave_header.wave_id,"WAVE",4);

	memcpy(pfsf->wave_header.fmt_chunk_hdr.id,"fmt ",4);
	pfsf->wave_header.fmt_chunk_hdr.len = sizeof(pfsf->wave_header.fmt_chunk_pcm);

	pfsf->wave_header.fmt_chunk_pcm.wFormatTag = 0x0001; // Microsoft PCM
	pfsf->wave_header.fmt_chunk_pcm.wChannels = 2;
	pfsf->wave_header.fmt_chunk_pcm.dwSamplesPerSec = dwSamplesPerSec;
	pfsf->wave_header.fmt_chunk_pcm.dwAvgBytesPerSec = 2 * dwSamplesPerSec * (16 / 8);
	pfsf->wave_header.fmt_chunk_pcm.wBlockAlign = 2 * (16 / 8);
	pfsf->wave_header.fmt_chunk_pcm.wBitsPerSample = 16;

	memcpy(pfsf->wave_header.data_chunk_hdr.id,"data",4);
	pfsf->wave_header.data_chunk_hdr.len = 0;

	fwrite(&pfsf->wave_header,sizeof(pfsf->wave_header),1,pHandle);

	pfsf->file_handle = pHandle;

	return pfsf;

}

void CFilmSound::_fWriteWave(FilmSoundFile *pfsf,WORD leftchan,WORD rightchan)
{
	if (!pfsf) return;
	
	WORD chans[2];

	chans[0]=leftchan;
	chans[1]=rightchan;

	fwrite(chans,sizeof(WORD),2,pfsf->file_handle);
	pfsf->wave_smaples_written++;
}

void CFilmSound::_fEndWave(FilmSoundFile* pfsf)
{
	if (!pfsf) return;

	long lfpos = ftell(pfsf->file_handle);
	
	fseek(pfsf->file_handle,0,SEEK_SET);
	
	// we need fo finish the header:
	pfsf->wave_header.riff_hdr.len=lfpos-4;
	pfsf->wave_header.data_chunk_hdr.len= pfsf->wave_smaples_written * (pfsf->wave_header.fmt_chunk_pcm.wBitsPerSample/8) * pfsf->wave_header.fmt_chunk_pcm.wChannels;

	// and write it:
	fwrite(&pfsf->wave_header,sizeof(pfsf->wave_header),1,pfsf->file_handle);

	fclose(pfsf->file_handle);

	delete pfsf;
}

bool CFilmSound::Start(wchar_t const * fileName, double dTargetTime, float fUseVolume, wchar_t const * extraFileName, double extraTime)
{
	InstallHooks(); // make sure hooks are installed

	if (!g_FilmSound_Capturing)
	{
		// only start when idle

		g_FilmSound_TimeRounding = FS_TR_FLOOR;
		
		// init time:
		g_TargetTime = dTargetTime;
		g_CurrentTime = 0.0;
		g_Last_paintedtime = 0;

		// set volume:
		g_Volume = fUseVolume;

		// retrive sound info structure (since we need the samples per second value == shm->Valve_speed):
		volatile dma_HL_t *shm=*(dma_HL_t **)HL_ADDR_GET(shm);

		if(!(_pWaveFile=_fBeginWave(fileName, shm->Valve_speed))) // we use Quake speed since we capture the internal mixer
			return false; // on fail return false

		m_pWaveFileExtra = 0;
		if(0 < extraTime)
		{
			m_ExtraTime = extraTime;

			if(!(m_pWaveFileExtra = _fBeginWave(extraFileName, shm->Valve_speed)))
			{
				_fEndWave(_pWaveFile);
				return false;
			}
		}

		g_FilmSound_Capturing = true; // switch to filming mode

		return true;
	}

	return false;
}

void CFilmSound::AdvanceFrame(double dTargetTime)
{
	// update frame time:
	g_TargetTime = dTargetTime;
}

typedef void (*S_Update_t)(void);

void CFilmSound::Stop()
{
	if(!g_FilmSound_Capturing)
		return;

	// for sound finishing and extra sound make it ceil:
	g_FilmSound_TimeRounding = FS_TR_CEIL;

	//
	// finish the painting:

	touring_S_PaintChannels(*(int *)HL_ADDR_GET(paintedtime));
	_fEndWave(_pWaveFile); // finish the wave file
	_pWaveFile = 0;

	//
	// do any extra painting:

	if(m_pWaveFileExtra)
	{
		_pWaveFile = m_pWaveFileExtra;
		m_pWaveFileExtra = 0;
		g_TargetTime += m_ExtraTime;

		touring_S_PaintChannels(*(int *)HL_ADDR_GET(paintedtime));
		_fEndWave(_pWaveFile); // finish the wave file

		_pWaveFile = 0;
	}

	// done capturing, stop and restore:

	g_FilmSound_Capturing = false;
	
	pEngfuncs->Con_Printf("Sound system finished stopping (almost :).\n");

	//
	// make soundsystem catch up by simulating an overflow:

	*(int *)HL_ADDR_GET(paintedtime) = 0x40000000 + 1;
	((S_Update_t)HL_ADDR_GET(S_Update_))();
}


void CFilmSound::Snd_Supply(WORD leftchan, WORD rightchan) {
	_fWriteWave(_pWaveFile, leftchan, rightchan);
}


