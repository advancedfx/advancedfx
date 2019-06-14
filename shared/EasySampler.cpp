#include "stdafx.h"

#include "EasySampler.h"

#include <assert.h>
#include <math.h>

//#define DEBUG_EASYSAMPLER

#ifdef DEBUG_EASYSAMPLER
#include <hlsdk.h>
#include <hooks/HookHw.h>
#endif


// EasyByteSampler /////////////////////////////////////////////////////////////

EasyByteSampler::EasyByteSampler(
	EasySamplerSettings const & settings,
	int pitch,
	IFramePrinter * framePrinter
)
: EasySamplerBase(settings.FrameDuration_get(), settings.StartTime_get(), settings.Exposure_get())
, m_Settings(settings)
{
	int height = settings.Height_get();
	int width = settings.Width_get();
	bool twoPoint = EasySamplerSettings::ESM_Trapezoid == settings.Method_get();

	assert(width <= pitch);

	m_Pitch = pitch;
	m_Frame = new Frame(height * width);
	m_FramePrinter = framePrinter;
	m_HasLastSample = false;
	m_LastSample = twoPoint ? new unsigned char[height * pitch] : 0;
	m_PrintMem = new unsigned char[height * pitch];
}


EasyByteSampler::~EasyByteSampler()
{
	// ? // PrintFrame();

	delete m_PrintMem;
	delete m_LastSample;
	delete m_Frame;
}

bool EasyByteSampler::CanSkipConstant(double time, double durationPerSample)
{
	return EasySamplerBase::CanSkipConstant(time, durationPerSample, m_LastSample ? 1 : 0);
}


void EasyByteSampler::ClearFrame(float frameStrength)
{
	float w = 1.0f -frameStrength;

	ScaleFrame(w);
}


void EasyByteSampler::Fn_1(void const * sample)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	int deltaPitch = m_Pitch -width;
	float *fdata = m_Frame->Data;
	unsigned char const * cdata = (unsigned char const *)sample;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + *cdata;
			
			fdata++;
			cdata++;
		}
		cdata += deltaPitch;
	}

	m_Frame->WhitePoint += 255.0f;
}

void EasyByteSampler::Fn_2(void const * sample, float w)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	int deltaPitch = m_Pitch -width;
	float *fdata = m_Frame->Data;
	unsigned char const * cdata = (unsigned char const *)sample;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + w * *cdata;
			
			fdata++;
			cdata++;
		}
		cdata += deltaPitch;
	}

	m_Frame->WhitePoint += w * 255.0f;
}

void EasyByteSampler::Fn_4(void const * sampleA, void const * sampleB, float w)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	int deltaPitch = m_Pitch -width;
	float *fdata = m_Frame->Data;
	unsigned char const * cdataA = (unsigned char const *)sampleA;
	unsigned char const * cdataB = (unsigned char const *)sampleB;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + w * ((unsigned int)*cdataA +(unsigned int)*cdataB);
			
			fdata++;
			cdataA++;
			cdataB++;
		}
		cdataA += deltaPitch;
		cdataB += deltaPitch;
	}

	m_Frame->WhitePoint += w * 2.0f * 255.0f;
}


void EasyByteSampler::PrintFrame()
{
	float w = m_Frame->WhitePoint;

	unsigned char * data = m_PrintMem;

	if(0 == w)
	{
		memset(data, 0, m_Settings.Height_get() * m_Pitch * sizeof(unsigned char));
	}
	else
	{
		float * fdata = m_Frame->Data;

		int height = m_Settings.Height_get();
		int width = m_Settings.Width_get();
		int deltaPitch = m_Pitch - width;

		w = 255.0f / w;

		for( int iy=0; iy < height; iy++ )
		{
			for( int ix=0; ix < width; ix++ )
			{
				*data = (unsigned char)(w * *fdata);

				fdata++;
				data++;
			}

			data += deltaPitch;
		}
	}

	m_FramePrinter->Print(m_PrintMem);
}


void EasyByteSampler::Sample(unsigned char const * data, double time)
{
	m_CurSample = data;

	EasySamplerBase::Sample(time);

	if(m_LastSample && data)
	{
		memcpy(m_LastSample, data, m_Settings.Height_get() * m_Pitch * sizeof(unsigned char));
		m_HasLastSample = true;
	}
	else
		m_HasLastSample = false;
}


void EasyByteSampler::ScaleFrame(float factor)
{
	float w = m_Frame->WhitePoint;

	if(w * factor == w)
	{
		// No change.
		return;
	}
	
	if(0 == w * factor)
	{
		// Zero.
		m_Frame->WhitePoint = 0;
		memset(m_Frame->Data, 0, m_Settings.Height_get() * m_Settings.Width_get() * sizeof(float));
		return;
	}
	
	float * fdata = m_Frame->Data;

	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();

	m_Frame->WhitePoint *= factor;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ )
		{
			*fdata = factor * *fdata;
			fdata++;
		}
	}
}




// EasyFloatSampler ////////////////////////////////////////////////////////////

EasyFloatSampler::EasyFloatSampler(
	EasySamplerSettings const & settings,
	IFloatFramePrinter * framePrinter
)
: EasySamplerBase(settings.FrameDuration_get(), settings.StartTime_get(), settings.Exposure_get())
, m_Settings(settings)
{
	int height = settings.Height_get();
	int width = settings.Width_get();
	bool twoPoint = EasySamplerSettings::ESM_Trapezoid == settings.Method_get();

	m_FrameData = new float[height * width];
	m_FramePrinter = framePrinter;
	m_FrameWhitePoint = 0;
	m_HasLastSample = false;
	m_LastSample = twoPoint ? new float[height * width] : 0;
	m_PrintMem = new float[height * width];
}


EasyFloatSampler::~EasyFloatSampler()
{
	// ? // PrintFrame();

	delete m_PrintMem;
	delete m_LastSample;
	delete m_FrameData;
}

bool EasyFloatSampler::CanSkipConstant(double time, double durationPerSample)
{
	return EasySamplerBase::CanSkipConstant(time, durationPerSample, m_LastSample ? 1 : 0);
}


void EasyFloatSampler::ClearFrame(float frameStrength)
{
	float w = 1.0f -frameStrength;

	ScaleFrame(w);
}


void EasyFloatSampler::Fn_1(void const * sample)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	float *fdata = m_FrameData;
	float const * cdata = (float const *)sample;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + *cdata;
			
			fdata++;
			cdata++;
		}
	}

	m_FrameWhitePoint += 1.0f;
}

void EasyFloatSampler::Fn_2(void const * sample, float w)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	float *fdata = m_FrameData;
	float const * cdata = (float const *)sample;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + w * *cdata;
			
			fdata++;
			cdata++;
		}
	}

	m_FrameWhitePoint += w * 1.0f;
}

void EasyFloatSampler::Fn_4(void const * sampleA, void const * sampleB, float w)
{
	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();
	float *fdata = m_FrameData;
	float const * cdataA = (float const *)sampleA;
	float const * cdataB = (float const *)sampleB;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ ) 
		{
			*fdata = *fdata + w * (*cdataA + *cdataB);
			
			fdata++;
			cdataA++;
			cdataB++;
		}
	}

	m_FrameWhitePoint += w * 2.0f * 1.0f;
}


void EasyFloatSampler::PrintFrame()
{
	float w = m_FrameWhitePoint;
	float * data = m_PrintMem;

	if(0 == w)
	{
		memset(data, 0, m_Settings.Height_get() * m_Settings.Width_get() * sizeof(float));
	}
	else
	{
		float * fdata = m_FrameData;

		int height = m_Settings.Height_get();
		int width = m_Settings.Width_get();

		w = 1.0f / w;

		for( int iy=0; iy < height; iy++ )
		{
			for( int ix=0; ix < width; ix++ )
			{
				*data = w * *fdata;

				fdata++;
				data++;
			}
		}
	}

	m_FramePrinter->Print(m_PrintMem);
}


void EasyFloatSampler::Sample(float const * data, double time)
{
	m_CurSample = data;

	EasySamplerBase::Sample(time);

	if(m_LastSample && data)
	{
		memcpy(m_LastSample, data, m_Settings.Height_get() * m_Settings.Width_get() * sizeof(float));
		m_HasLastSample = true;
	}
	else
		m_HasLastSample = false;
}


void EasyFloatSampler::ScaleFrame(float factor)
{
	float w = m_FrameWhitePoint;

	if(w * factor == w)
	{
		// No change.
		return;
	}
	
	if(0 == w * factor)
	{
		// Zero.
		m_FrameWhitePoint = 0;
		memset(m_FrameData, 0, m_Settings.Height_get() * m_Settings.Width_get() * sizeof(float));
		return;
	}
	
	float * fdata = m_FrameData;

	int height = m_Settings.Height_get();
	int width = m_Settings.Width_get();

	m_FrameWhitePoint *= factor;

	for( int iy=0; iy < height; iy++ )
	{
		for( int ix=0; ix < width; ix++ )
		{
			*fdata = factor * *fdata;
			fdata++;
		}
	}
}


// EasySamplerBase /////////////////////////////////////////////////////////////


EasySamplerBase::EasySamplerBase(
	double frameDuration,
	double startTime,
	double exposure)
{
	m_FrameDuration = frameDuration;
	m_LastFrameTime = startTime;
	m_LastSampleTime = startTime;
	m_ShutterOpen = 0.0 < exposure;
	m_ShutterOpenDuration = frameDuration * (exposure < 0 ? 0 : (exposure > 1 ? 1 : exposure));
	m_ShutterTime = m_LastFrameTime;
}


bool EasySamplerBase::CanSkipConstant(double time, double durationPerSample, int numPreviousSamplesRequired)
{
	// skipping is only allowed when enough samples are guaranteed to be captured to allow interpolation before the shutter is opened.

	double prepareTime = numPreviousSamplesRequired * durationPerSample;
	double timeLeft = m_FrameDuration -(time -m_LastFrameTime);

	return
		!m_ShutterOpen && prepareTime < timeLeft
	;
}


void EasySamplerBase::Integrator_Fn(ISampleFns *fns, void const * sampleA, void const *sampleB, double timeA, double timeB, double subTimeA, double subTimeB)
{
	double weightA;
	double weightB;

	{
		// Calculate weigths:
		double dAB = timeB -timeA;
		double w1 = (subTimeB -subTimeA) / 2.0;
		double w2 = dAB ? (subTimeA +subTimeB -2.0 * timeA) / dAB : 0.0;
		weightA = w1 * (2 -w2);
		weightB = w1 * w2;
	}

#ifdef DEBUG_EASYSAMPLER
	pEngfuncs->Con_Printf(" (wA=%f, SA:%s, wB=%f, SB:%s)", weightA, sampleA ? "Y" : "N", weightB, sampleB? "Y" : "N");
#endif


	//
	// optimize / combine the function:

	if(0 == weightA)
	{
		weightA = weightB;
		weightB = 0;
		sampleA = sampleB;
		sampleB = 0;
	}

	if(0 == weightA)
	{
		// zero weights.

		return;  // done;
	}

	assert(0 != weightA);

	if(0 == sampleA)
	{
		// switch to 1 point sampling.

		sampleA = sampleB;
		sampleB = 0;

		weightA = weightA + weightB;
		weightB = 0;
	}

	if(0 == sampleA)
	{
		// 0 point sampling.

		return; // done.
	}

	assert(0 != weightA);
	assert(0 != sampleA);

	if(0 == sampleB || 0 == weightB)
	{
		if(1 == weightA)
		{
			fns->Fn_1(sampleA);
		}
		else
		{
			fns->Fn_2(sampleA, (float)weightA);
		}
	}
	else
	{
		// 2 points.

		assert(0 != weightB && 0 != sampleB);

		if(weightA == weightB)
		{
			if(1 == weightA)
			{
				fns->Fn_1(sampleA);
				fns->Fn_1(sampleB);
			}
			else
			{
				fns->Fn_4(sampleA, sampleB, (float)weightA);
			}
		}
		else
		{
			if(1 == weightB)
			{
				void const *tS = sampleA;
				double tW = weightA;

				sampleA = sampleB;
				sampleB = tS;

				weightA = weightB;
				weightB = tW;
			}

			if(1 == weightA)
			{
				fns->Fn_1(sampleA);
				fns->Fn_2(sampleB, (float)weightB);
			}
			else
			{
				fns->Fn_2(sampleA, (float)weightA);
				fns->Fn_2(sampleB, (float)weightB);
			}
		}
	}
}


void EasySamplerBase::Sample(double time)
{	
	double subMin = m_LastSampleTime;

#ifdef DEBUG_EASYSAMPLER
	pEngfuncs->Con_Printf("Sample: [%f, %f]\n", m_LastSampleTime, time);
#endif

	while(subMin < time)
	{		
		double subMax = time;

		double shutterEvent = m_ShutterTime+ (m_ShutterOpen ? m_ShutterOpenDuration : m_FrameDuration);
		double frameEnd = m_LastFrameTime +m_FrameDuration;

		// apply restrictions:

		if(subMin < frameEnd && frameEnd <= subMax)
		{
			subMax = frameEnd;
		}

		if(subMin < shutterEvent && shutterEvent <= subMax)
		{
			subMax = shutterEvent;
		}

		// sub sample restricted interval:

#ifdef DEBUG_EASYSAMPLER
		pEngfuncs->Con_Printf("\tSub interval: [%f, %f] shutter_%s", subMin, subMax, m_ShutterOpen ? "open" : "closed");
#endif

		if(m_ShutterOpen)
		{
			SubSample(
				m_LastSampleTime, time,
				subMin, subMax
			);
		}

		// process active restrictions:

		if(subMin < frameEnd && frameEnd <= subMax)
		{
#ifdef DEBUG_EASYSAMPLER
			pEngfuncs->Con_Printf(" frame_end");
#endif

			MakeFrame();
			m_LastFrameTime = subMax;
		}

		if(subMin < shutterEvent && shutterEvent <= subMax)
		{
#ifdef DEBUG_EASYSAMPLER
			pEngfuncs->Con_Printf(" shutter_event");
#endif
			if(0.0f < m_ShutterOpenDuration && m_ShutterOpenDuration < m_FrameDuration)
			{
				m_ShutterOpen = !m_ShutterOpen;

				if(m_ShutterOpen)
					m_ShutterTime = subMax;
			}
		}

#ifdef DEBUG_EASYSAMPLER
		pEngfuncs->Con_Printf("\n");
#endif

		subMin = subMax;
	}

	m_LastSampleTime = time;
}


// EasySamplerSettings /////////////////////////////////////////////////////////

EasySamplerSettings::EasySamplerSettings(
	int width, 
	int height,
	Method method,
	double frameDuration,
	double startTime,
	double exposure,
	float frameStrength
)
{
	assert(0 <= height);
	assert(0 <= width);

	m_Exposure = exposure;
	m_FrameDuration = frameDuration;
	m_FrameStrength = frameStrength;
	m_Height = height;
	m_Method = method;
	m_StartTime = startTime;
	m_Width = width;	
}

EasySamplerSettings::EasySamplerSettings(EasySamplerSettings const & settings)
{
	m_Exposure = settings.Exposure_get();
	m_FrameDuration = settings.FrameDuration_get();
	m_FrameStrength = settings.FrameStrength_get();
	m_Height = settings.Height_get();
	m_Method = settings.Method_get();
	m_StartTime = settings.StartTime_get();
	m_Width = settings.Width_get();
}

double EasySamplerSettings::Exposure_get() const
{
	return m_Exposure;	
}
double EasySamplerSettings::FrameDuration_get() const
{
	return m_FrameDuration;
}
float EasySamplerSettings::FrameStrength_get() const
{
	return m_FrameStrength;
}
int EasySamplerSettings::Height_get() const
{
	return m_Height;	
}
EasySamplerSettings::Method EasySamplerSettings::Method_get() const
{
	return m_Method;
}
double EasySamplerSettings::StartTime_get() const
{
	return m_StartTime;
}
int EasySamplerSettings::Width_get() const
{
	return m_Width;
}
