#pragma once

// TODO:
//
// In the average case where the full range of a sample is
// used (within a frame), CPU power is wasted, because the same multiplication
// is done twice when interpolating from the next sample.
// (First it's the new one, then the old one and factors
// are about the same, this is a waste).

#include "ImageFormat.h"
#include "TImageBuffer.h"
#include "TGrowingBufferPool.h"

#include <memory.h>

template<bool bThreadSafe> class __declspec(novtable) IFramePrinter abstract
{
public:
	virtual void PrintSampledFrame(advancedfx::TImageBuffer<bThreadSafe> * pImageBuffer) abstract = 0;
};

class __declspec(novtable) ISampleFns abstract
{
public:
	/// <summary>frame += sample</summary>
	virtual void Fn_1(void const *sample) abstract = 0;

	/// <summary>frame += w * sample </summary>
	virtual void Fn_2(void const *sample, float w) abstract = 0;

	/// <summary>frame += w * (sampleA + sampleB)</summary>
	virtual void Fn_4(void const *sampleA, void const *sampleB, float w) abstract = 0;
};


// EasySamplerBase /////////////////////////////////////////////////////////////

/// <summary>
///   Base class, not intended to be used directly.
///   This class supplies implementation of fundamental sampling logic.
/// </summary>
class EasySamplerBase abstract
{
public:

protected:
	/// <param name="exposure">time the shutter is kept open measured in number of frames</param>
	EasySamplerBase(
		double frameDuration,
		double startTime,
		double exposure);
	
	bool CanSkipConstant(double time, double durationPerSample, int numPreviousSamplesRequired);

	/// <summary>
	///   Triggers the sampling logic for an example. If there is anything to be sampled
	///   SubSample will be called one or more times.
	/// </summary>
	void Sample(double time);

	/// <summary>
	///   Can be called mutliple times by Sample(double).<br />
	///   The calle is supposed to approximate a sub-integral
	///   between two samples, using the information supplied.
	/// </summary>
	/// <param name="timeA">Time of last sample.</param>
	/// <param name="timeB">Time of current sample.</param>
	/// <param name="subTimeA">lower integral boundary</param>
	/// <param name="subTimeB">upper integral boundary</param>
	/// <remarks>timeA &lt;= subTimeA &lt; subTimeB &lt;=timeB</remarks>
	virtual void SubSample(
		double timeA,
		double timeB,
		double subTimeA,
		double subTimeB) abstract = 0;

	/// <summary>
	///   Can be called mutliple times by Sample(double).<br />
	///   The calle is supposed to finish (print + clear)
	///   the current frame.
	/// </summary>
	virtual void MakeFrame() abstract = 0;

protected:
	/// <summary>
	///	  Auto 2 (trapezium) / 1 (rectangle) / 0 point sampling by integration.<br />
	///   Selects, optimizes and combines the integration using a given set of base functions.
	/// </summary>
	/// <param name="sampleA">can be 0</param>
	/// <param name="sampleB">can be 0</param>
	static void Integrator_Fn(ISampleFns *fns, void const *sampleA, void const *sampleB, double timeA, double timeB, double subTimeA, double subTimeB);

private:
	double m_FrameDuration;
	double m_LastFrameTime;
	double m_LastSampleTime;
	bool m_ShutterOpen;
	double m_ShutterOpenDuration;
	double m_ShutterTime;
};


// EasySamplerSettings /////////////////////////////////////////////////////////

/// <summary>
///   Encapsulates common sampler settings.
/// </summary>
class EasySamplerSettings
{
public:
	enum Method
	{
		ESM_Rectangle,
		ESM_Trapezoid
	};

	EasySamplerSettings(
		const advancedfx::CImageFormat& imageFormat,
		Method method,
		double frameDuration,
		double startTime,
		double exposure,
		float frameStrength
	);

	EasySamplerSettings(EasySamplerSettings const & settings);

	const advancedfx::CImageFormat& ImageFormat_get() const;

	double Exposure_get() const;
	double FrameDuration_get() const;
	float FrameStrength_get() const;
	Method Method_get() const;
	double StartTime_get() const;

private:
	double m_Exposure;
	double m_FrameDuration;
	float m_FrameStrength;
	Method m_Method;
	double m_StartTime;
	advancedfx::CImageFormat m_ImageFormat;
};


// EasyByteSampler /////////////////////////////////////////////////////////////

class EasyByteSamplerImpl : public EasySamplerBase,
	protected ISampleFns
{
public:
	/// <param name="pitch">bytes of memory to skip for a row</param>
	EasyByteSamplerImpl(
		EasySamplerSettings const & settings
	);

	~EasyByteSamplerImpl();

protected:
	EasySamplerSettings m_Settings;

	void ClearFrame(float frameStrength);

	void PrintFrame(unsigned char * data);

private:

	class Frame
	{
	public:
		Frame(size_t length)
		{
			Data = new float[length];
			WhitePoint = 0;

			memset(Data, 0, sizeof(float) * length);
		}

		~Frame()
		{
			delete Data;
		}

		float * Data;
		float WhitePoint;
	};

	Frame * m_Frame;

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_1(void const * sample) override;

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_2(void const * sample, float w) override;

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_4(void const * sampleA, void const * sampleB, float w) override;

	void ScaleFrame(float factor);
};

template<bool bThreadSafe> class EasyByteSampler
: public EasyByteSamplerImpl
{
public:
	/// <param name="pitch">bytes of memory to skip for a row</param>
	EasyByteSampler(
		EasySamplerSettings const & settings,
		IFramePrinter<bThreadSafe> * framePrinter,
		advancedfx::TGrowingBufferPool<bThreadSafe> * pGrowingBufferPool)
	: EasyByteSamplerImpl(settings)
	, m_FramePrinter(framePrinter)
	, m_pGrowingBufferPool(pGrowingBufferPool)
	{
	}		

	~EasyByteSampler() {
		// ? // PrintFrame();
		if(m_pLastSample) m_pLastSample->Release();
	}

	bool CanSkipConstant(double time, double durationPerSample)
	{
		return EasySamplerBase::CanSkipConstant(time, durationPerSample, m_pLastSample ? 1 : 0);
	}	

	///	<param name="pImageBuffer">NULLPTR is interpreted as the ideal shutter being closed for the deltaTime that passed.</param>
	/// <remarks>A closed shutter and a weight of 0 are not the same, because the integral can be different (depends on the method).</remarks>
	void Sample(class advancedfx::TIImageBuffer<bThreadSafe> * pImageBuffer, double time)
	{
		m_pCurSample = pImageBuffer;

		if(pImageBuffer && *pImageBuffer->GetImageBufferFormat() != m_Settings.ImageFormat_get()) throw "Image buffer format mismatch.";

		EasySamplerBase::Sample(time);

		bool twoPoint = EasySamplerSettings::ESM_Trapezoid == m_Settings.Method_get();

		if(twoPoint && pImageBuffer)
		{
			if(m_pLastSample)  m_pLastSample->Release();
			m_pLastSample = m_pCurSample;
			m_pLastSample->AddRef();
		}
		else {
			if(m_pLastSample) {
				m_pLastSample->Release();
				m_pLastSample = nullptr;
			} 
		}

		m_pCurSample = nullptr;
	}

protected:
	virtual void EasySamplerBase::MakeFrame() override
	{
		PrintFrame();
		ClearFrame(m_Settings.FrameStrength_get());
	}

	virtual void EasySamplerBase::SubSample(
		double timeA,
		double timeB,
		double subTimeA,
		double subTimeB
	) override {
		Integrator_Fn(this,
			m_pLastSample ? m_pLastSample->GetImageBufferData() : nullptr, m_pCurSample->GetImageBufferData(),
			timeA, timeB, subTimeA, subTimeB
		);
	}

private:
	advancedfx::TGrowingBufferPool<bThreadSafe> * m_pGrowingBufferPool;
	advancedfx::TIImageBuffer<bThreadSafe> * m_pCurSample = nullptr;
	advancedfx::TIImageBuffer<bThreadSafe> * m_pLastSample = nullptr;
	IFramePrinter<bThreadSafe> * m_FramePrinter;

	void PrintFrame()
	{
		const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
		advancedfx::TImageBuffer<bThreadSafe> * pImageBuffer = new advancedfx::TImageBuffer<bThreadSafe>(m_pGrowingBufferPool);
		pImageBuffer->AddRef();
		if(pImageBuffer->GrowAlloc(imageFormat)) {
			unsigned char * data = (unsigned char *)pImageBuffer->GetImageBufferData();
			EasyByteSamplerImpl::PrintFrame(data);
		}
		m_FramePrinter->PrintSampledFrame(pImageBuffer);
		pImageBuffer->Release();
	}
};


// EasyFloatSampler ////////////////////////////////////////////////////////////

class EasyFloatSamplerImpl : public EasySamplerBase,
	protected ISampleFns
{
public:
	EasyFloatSamplerImpl(
		EasySamplerSettings const & settings
	);

	~EasyFloatSamplerImpl();

protected:
	EasySamplerSettings m_Settings;
	
	void ClearFrame(float frameStrength);

	void PrintFrame(float * data);

private:
	float * m_FrameData;
	float m_FrameWhitePoint;

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_1(void const * sample);

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_2(void const * sample, float w);

	/// <summary>Implements ISampleFns.</summary>
	virtual void Fn_4(void const * sampleA, void const * sampleB, float w);

	void ScaleFrame(float factor);
};

template<bool bThreadSafe> class EasyFloatSampler
: public EasyFloatSamplerImpl
{
public:
	EasyFloatSampler(
		EasySamplerSettings const & settings,
		IFramePrinter<bThreadSafe> * framePrinter,
		advancedfx::TGrowingBufferPool<bThreadSafe> * pGrowingBufferPool
	)
	: EasyFloatSamplerImpl(settings)
	, m_FramePrinter(framePrinter)
	, m_pGrowingBufferPool(pGrowingBufferPool)
	{
	}	

	~EasyFloatSampler()
	{
		// ? // PrintFrame();

		if(m_pLastSample) m_pLastSample->Release();
	}	

	bool CanSkipConstant(double time, double durationPerSample)
	{
		return EasySamplerBase::CanSkipConstant(time, durationPerSample, m_pLastSample ? 1 : 0);
	}

	///	<param name="pImageBuffer">NULLPTR is interpreted as the ideal shutter being closed for the deltaTime that passed.</param>
	/// <remarks>A closed shutter and a weight of 0 are not the same, because the integral can be different (depends on the method).</remarks>
	void Sample(class advancedfx::TIImageBuffer<bThreadSafe> * pImageBuffer, double time)
	{
		m_pCurSample = pImageBuffer;

		if(pImageBuffer && *pImageBuffer->GetImageBufferFormat() != m_Settings.ImageFormat_get()) throw "Image buffer format mismatch.";

		EasySamplerBase::Sample(time);

		bool twoPoint = EasySamplerSettings::ESM_Trapezoid == m_Settings.Method_get();

		if(twoPoint && pImageBuffer)
		{
			if(m_pLastSample)  m_pLastSample->Release();
			m_pLastSample = m_pCurSample;
			m_pLastSample->AddRef();
		}
		else {
			if(m_pLastSample) {
				m_pLastSample->Release();
				m_pLastSample = nullptr;
			} 
		}

		m_pCurSample = nullptr;
	}	

protected:
	virtual void EasySamplerBase::MakeFrame()
	{
		PrintFrame();
		ClearFrame(m_Settings.FrameStrength_get());
	}

	virtual void EasySamplerBase::SubSample(
		double timeA,
		double timeB,
		double subTimeA,
		double subTimeB)
	{
		Integrator_Fn(this,
			m_pLastSample ? m_pLastSample->GetImageBufferData() : nullptr, m_pCurSample->GetImageBufferData(),
			timeA, timeB, subTimeA, subTimeB
		);
	}

private:
	advancedfx::TGrowingBufferPool<bThreadSafe> * m_pGrowingBufferPool;
	advancedfx::TIImageBuffer<bThreadSafe> * m_pCurSample = nullptr;
	advancedfx::TIImageBuffer<bThreadSafe> * m_pLastSample = nullptr;
	IFramePrinter<bThreadSafe> * m_FramePrinter;

	void PrintFrame()
	{
		const advancedfx::CImageFormat& imageFormat = m_Settings.ImageFormat_get();
		advancedfx::TImageBuffer<bThreadSafe> * pImageBuffer = new advancedfx::TImageBuffer<bThreadSafe>(m_pGrowingBufferPool);
		pImageBuffer->AddRef();
		if(pImageBuffer->GrowAlloc(imageFormat)) {
			float * data = (float *)pImageBuffer->GetImageBufferData();
			EasyFloatSamplerImpl::PrintFrame(data);
		}
		m_FramePrinter->PrintSampledFrame(pImageBuffer);
		pImageBuffer->Release();
	}
};
