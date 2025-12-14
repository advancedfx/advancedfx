#pragma once

#include "RefCountedThreadSafe.h"
#include "GrowingBufferPoolThreadSafe.h"
#include "ImageBufferThreadSafe.h"
#include "AfxOutStreams.h"

namespace advancedfx {

class COutVideoStreamCreator
	: public CRefCountedThreadSafe
{
public:
	virtual TIOutVideoStream<true>* CreateOutVideoStream(const CImageFormat& imageFormat) const = 0;
};

class CClassicRecordingSettingsCreator
	: public COutVideoStreamCreator
{
public:
	CClassicRecordingSettingsCreator(const std::wstring & capturePath, bool bIfZip, bool bFormatBmpAndNotga)
	: m_CapturePath(capturePath)
	, m_bIfZip(bIfZip)
	, m_bFormatBmpAndNotga(bFormatBmpAndNotga) {

	}

	virtual TIOutVideoStream<true>* CreateOutVideoStream(const CImageFormat& imageFormat) const override {
		return new COutImageStream<true>(imageFormat, m_CapturePath, m_bIfZip, m_bFormatBmpAndNotga);
	}

private:
	std::wstring m_CapturePath;
	bool m_bIfZip;
	bool m_bFormatBmpAndNotga;
};


class CFfmpegRecordingSettingsCreator
	: public COutVideoStreamCreator
{
public:
	CFfmpegRecordingSettingsCreator(const std::wstring& capturePath, const std::wstring& ffmpegOptions, float frameRate)
		: m_CapturePath(capturePath)
		, m_FfmpegOptions(ffmpegOptions)
		, m_FrameRate(frameRate) {

	}

	virtual TIOutVideoStream<true>* CreateOutVideoStream(const CImageFormat& imageFormat) const override {
		return new COutFFMPEGVideoStream<true>(imageFormat, m_CapturePath, m_FfmpegOptions, m_FrameRate);
	}

private:
	std::wstring m_CapturePath;
	std::wstring m_FfmpegOptions;
	float m_FrameRate;
};

class CSamplingRecordingSettingsCreator
	: public COutVideoStreamCreator
{
public:
	CSamplingRecordingSettingsCreator(class COutVideoStreamCreator * outVideoStreamCreator, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength, CGrowingBufferPoolThreadSafe * pImageBufferPool)
		: m_OutVideoStreamCreator(outVideoStreamCreator)
		, m_FrameRate(frameRate)
		, m_Method(method)
		, m_FrameDuration(frameDuration)
		, m_Exposure(exposure)
		, m_FrameStrength(frameStrength)
        , m_pImageBufferPool(pImageBufferPool)
	{
		outVideoStreamCreator->AddRef();
	}

	virtual TIOutVideoStream<true>* CreateOutVideoStream(const CImageFormat& imageFormat) const override {
		return new COutSamplingStream<true>(imageFormat, m_OutVideoStreamCreator->CreateOutVideoStream(imageFormat), m_FrameRate, m_Method, m_FrameDuration, m_Exposure, m_FrameStrength, m_pImageBufferPool);
	}

protected:
	~CSamplingRecordingSettingsCreator() {
		m_OutVideoStreamCreator->Release();
	}

private:
	class COutVideoStreamCreator* m_OutVideoStreamCreator;
	float m_FrameRate;
	EasySamplerSettings::Method m_Method;
	double m_FrameDuration;
	double m_Exposure;
	float m_FrameStrength;
    CGrowingBufferPoolThreadSafe * m_pImageBufferPool;
};

} // namespace advancedfx
