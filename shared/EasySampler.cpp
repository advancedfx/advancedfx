#include "stdafx.h"

#include "EasySampler.h"

#include <assert.h>
#include <math.h>

//#define DEBUG_EASYSAMPLER

#ifdef DEBUG_EASYSAMPLER
#include <hlsdk.h>
#include <hooks/HookHw.h>
#endif


// EasyByteSamplerImpl /////////////////////////////////////////////////////////

EasyByteSamplerImpl::EasyByteSamplerImpl(
	EasySamplerSettings const & settings)
: EasySamplerBase(settings.FrameDuration_get(), settings.StartTime_get(), settings.Exposure_get())
, m_Settings(settings)
{
	const advancedfx::CImageFormat& imageFormat = settings.ImageFormat_get();
	switch(imageFormat.Format) {
	case advancedfx::ImageFormat::BGR:
	case advancedfx::ImageFormat::BGRA:
	case advancedfx::ImageFormat::A:
	case advancedfx::ImageFormat::RGBA:
		break;
	default:
		throw "AFXERROR: Unsupported image format.";
	}

	int height = imageFormat.Height;
	int width = imageFormat.Width;
	size_t pitch = imageFormat.Pitch;
	size_t packedRowSize = width * imageFormat.GetPixelStride();
	bool twoPoint = EasySamplerSettings::ESM_Trapezoid == settings.Method_get();

	assert(packedRowSize <= pitch);

	m_Frame = new Frame(height * packedRowSize);
}


EasyByteSamplerImpl::~EasyByteSamplerImpl()
{
	delete m_Frame;
}

void EasyByteSamplerImpl::ClearFrame(float frameStrength)
{
	float w = 1.0f -frameStrength;

	ScaleFrame(w);
}


void EasyByteSamplerImpl::Fn_1(void const * sample)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width *  imageFormat.GetPixelStride();
	size_t deltaPitch = imageFormat.Pitch - width;
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

void EasyByteSamplerImpl::Fn_2(void const * sample, float w)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width *  imageFormat.GetPixelStride();
	size_t deltaPitch = imageFormat.Pitch - width;
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

void EasyByteSamplerImpl::Fn_4(void const * sampleA, void const * sampleB, float w)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width *  imageFormat.GetPixelStride();
	size_t deltaPitch = imageFormat.Pitch - width;
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


void EasyByteSamplerImpl::PrintFrame(unsigned char * data)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();

	float w = m_Frame->WhitePoint;

	if(0 == w)
	{
		memset(data, 0, imageFormat.Height * imageFormat.Pitch);
	}
	else
	{
		float * fdata = m_Frame->Data;

		int height = imageFormat.Height;
		int width = imageFormat.Width * imageFormat.GetPixelStride();
		size_t deltaPitch = imageFormat.Pitch - width;

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
}

void EasyByteSamplerImpl::ScaleFrame(float factor)
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

		const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
		memset(m_Frame->Data, 0, imageFormat.Height * imageFormat.Width * imageFormat.GetPixelStride() * sizeof(float));
		return;
	}
	
	float * fdata = m_Frame->Data;

	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width =imageFormat.Width * imageFormat.GetPixelStride();

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

EasyFloatSamplerImpl::EasyFloatSamplerImpl(
	EasySamplerSettings const & settings
)
: EasySamplerBase(settings.FrameDuration_get(), settings.StartTime_get(), settings.Exposure_get())
, m_Settings(settings)
{
	const advancedfx::CImageFormat& imageFormat = settings.ImageFormat_get();
	switch(imageFormat.Format) {
	case advancedfx::ImageFormat::ZFloat:
		break;
	default:
		throw "AFXERROR: Unsupported image format.";
	}

	int height = imageFormat.Height;
	int width = imageFormat.Width;
	size_t pitch = imageFormat.Pitch;
	size_t packedRowSize = width * imageFormat.GetPixelStride();
	bool twoPoint = EasySamplerSettings::ESM_Trapezoid == settings.Method_get();

	assert(packedRowSize <= pitch);

	m_FrameData = new float[height * width];
	m_FrameWhitePoint = 0;
}


EasyFloatSamplerImpl::~EasyFloatSamplerImpl()
{
	delete m_FrameData;
}

void EasyFloatSamplerImpl::ClearFrame(float frameStrength)
{
	float w = 1.0f -frameStrength;

	ScaleFrame(w);
}

void EasyFloatSamplerImpl::Fn_1(void const * sample)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width;
	size_t deltaPitch = imageFormat.Pitch - width *  imageFormat.GetPixelStride();
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

		cdata = (float const *)((unsigned char const *)cdata + deltaPitch);
	}

	m_FrameWhitePoint += 1.0f;
}

void EasyFloatSamplerImpl::Fn_2(void const * sample, float w)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width;
	size_t deltaPitch = imageFormat.Pitch - width *  imageFormat.GetPixelStride();
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

		cdata = (float const *)((unsigned char const *)cdata + deltaPitch);
	}

	m_FrameWhitePoint += w * 1.0f;
}

void EasyFloatSamplerImpl::Fn_4(void const * sampleA, void const * sampleB, float w)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width;
	size_t deltaPitch = imageFormat.Pitch - width *  imageFormat.GetPixelStride();
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

		cdataA = (float const *)((unsigned char const *)cdataA + deltaPitch);
		cdataB = (float const *)((unsigned char const *)cdataB + deltaPitch);
	}

	m_FrameWhitePoint += w * 2.0f * 1.0f;
}


void EasyFloatSamplerImpl::PrintFrame(float * data)
{
	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();

	float w = m_FrameWhitePoint;

	if(0 == w)
	{
		memset(data, 0, imageFormat.Height * imageFormat.Pitch);
	}
	else
	{
		float * fdata = m_FrameData;

		int height = imageFormat.Height;
		int width = imageFormat.Width;
		size_t deltaPitch = imageFormat.Pitch - width *  imageFormat.GetPixelStride();

		w = 1.0f / w;

		for( int iy=0; iy < height; iy++ )
		{
			for( int ix=0; ix < width; ix++ )
			{
				*data = w * *fdata;

				fdata++;
				data++;
			}

			data = (float *)((unsigned char *)data + deltaPitch);
		}
	}
}

void EasyFloatSamplerImpl::ScaleFrame(float factor)
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

		const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
		memset(m_FrameData, 0, imageFormat.Height * imageFormat.Width * sizeof(float));
		return;
	}
	
	float * fdata = m_FrameData;

	const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
	int height = imageFormat.Height;
	int width = imageFormat.Width;

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
	const advancedfx::CImageFormat& imageFormat,
	Method method,
	double frameDuration,
	double startTime,
	double exposure,
	float frameStrength)
: m_ImageFormat(imageFormat)
{
	assert(0 <= imageFormat.Height);
	assert(0 <= imageFormat.Width);

	m_Exposure = exposure;
	m_FrameDuration = frameDuration;
	m_FrameStrength = frameStrength;
	m_Method = method;
	m_StartTime = startTime;
}

EasySamplerSettings::EasySamplerSettings(EasySamplerSettings const & settings)
{
	m_ImageFormat = settings.ImageFormat_get();
	m_Exposure = settings.Exposure_get();
	m_FrameDuration = settings.FrameDuration_get();
	m_FrameStrength = settings.FrameStrength_get();
	m_Method = settings.Method_get();
	m_StartTime = settings.StartTime_get();
}

const advancedfx::CImageFormat& EasySamplerSettings::ImageFormat_get() const {
	return m_ImageFormat;
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
EasySamplerSettings::Method EasySamplerSettings::Method_get() const
{
	return m_Method;
}
double EasySamplerSettings::StartTime_get() const
{
	return m_StartTime;
}
