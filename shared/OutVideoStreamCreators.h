#pragma once

#include "RefCountedThreadSafe.h"
#include "AfxImageBuffer.h"
#include "AfxOutStreams.h"

namespace advancedfx {

class COutVideoStreamCreator
	: public advancedfx::CRefCountedThreadSafe
{
public:
	virtual advancedfx::COutVideoStream* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) const = 0;
};

class CClassicRecordingSettingsCreator
	: public advancedfx::COutVideoStreamCreator
{
public:
	CClassicRecordingSettingsCreator(const std::wstring & capturePath, bool bIfZip, bool bFormatBmpAndNotga)
	: m_CapturePath(capturePath)
	, m_bIfZip(bIfZip)
	, m_bFormatBmpAndNotga(bFormatBmpAndNotga) {

	}

	virtual advancedfx::COutVideoStream* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) const override {
		return new advancedfx::COutImageStream(imageFormat, m_CapturePath, m_bIfZip, m_bFormatBmpAndNotga);
	}

private:
	std::wstring m_CapturePath;
	bool m_bIfZip;
	bool m_bFormatBmpAndNotga;
};


class CFfmpegRecordingSettingsCreator
	: public advancedfx::COutVideoStreamCreator
{
public:
	CFfmpegRecordingSettingsCreator(const std::wstring& capturePath, const std::wstring& ffmpegOptions, float frameRate)
		: m_CapturePath(capturePath)
		, m_FfmpegOptions(ffmpegOptions)
		, m_FrameRate(frameRate) {

	}

	virtual advancedfx::COutVideoStream* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) const override {
		return new advancedfx::COutFFMPEGVideoStream(imageFormat, m_CapturePath, m_FfmpegOptions, m_FrameRate);
	}

private:
	std::wstring m_CapturePath;
	std::wstring m_FfmpegOptions;
	float m_FrameRate;
};

class CSamplingRecordingSettingsCreator
	: public advancedfx::COutVideoStreamCreator
{
public:
	CSamplingRecordingSettingsCreator(class advancedfx::COutVideoStreamCreator * outVideoStreamCreator, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength, IImageBufferPool * pImageBufferPool)
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

	virtual advancedfx::COutVideoStream* CreateOutVideoStream(const advancedfx::CImageFormat& imageFormat) const override {
		return new advancedfx::COutSamplingStream(imageFormat, m_OutVideoStreamCreator->CreateOutVideoStream(imageFormat), m_FrameRate, m_Method, m_FrameDuration, m_Exposure, m_FrameStrength, m_pImageBufferPool);
	}

protected:
	~CSamplingRecordingSettingsCreator() {
		m_OutVideoStreamCreator->Release();
	}

private:
	class advancedfx::COutVideoStreamCreator* m_OutVideoStreamCreator;
	float m_FrameRate;
	EasySamplerSettings::Method m_Method;
	double m_FrameDuration;
	double m_Exposure;
	float m_FrameStrength;
    IImageBufferPool * m_pImageBufferPool;
};

} // namespace advancedfx
