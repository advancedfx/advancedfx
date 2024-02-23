#pragma once

#include "AfxRefCounted.h"
#include "AfxImageBuffer.h"
#include "EasySampler.h"
#include <string>
#include <list>
#include <Windows.h>

namespace advancedfx {


class COutStream : public CRefCounted
{
public:
	enum Type {
		Type_Audio,
		Type_Video,
		Type_AudioVideo
	};

	Type GetMediaType() const
	{
		return m_Type;
	}

protected:
	COutStream(Type mediaType)
		: m_Type(mediaType)
	{
	}

private:
	Type m_Type;
};

class COutAudioStream : public COutStream
{
public:
	unsigned int GetChannels() const
	{
		return m_Channles;
	}

	virtual bool SupplyAudioData(unsigned int channels, unsigned int samples, const float* data) = 0;

protected:
	COutAudioStream(unsigned int channels)
		: COutStream(Type_Audio)
		, m_Channles(channels)
	{

	}

	unsigned int m_Channles;
};

class COutVideoStream : public COutStream
{
public:
	const CImageFormat& GetImageFormat() const
	{
		return m_ImageFormat;
	}

	virtual bool SupplyImageFrame(double secondsSinceLastFrame, const unsigned char* pBuffer) = 0;

	bool SupplyVideoData(const CImageBuffer& buffer) {
		if (!(buffer.Format == m_ImageFormat))
			return false;
		return SupplyImageFrame(-1, static_cast<const unsigned char*>(buffer.Buffer));
	}

	bool SupplyImageBuffer(const IImageBuffer* buffer) {
		if (!(nullptr != buffer && *buffer->GetImageBufferFormat() == m_ImageFormat))
			return false;
		return SupplyImageFrame(-1, static_cast<const unsigned char*>(buffer->GetImageBufferData()));
	}

protected:
	COutVideoStream(const CImageFormat& imageFormat)
		: COutStream(Type_Audio)
		, m_ImageFormat(imageFormat)
	{

	}

	const CImageFormat m_ImageFormat;
};

class COutImageStream : public COutVideoStream
{
public:
	COutImageStream(const CImageFormat& imageFormat, const std::wstring& path, bool ifZip, bool ifBmpNotTga)
		: COutVideoStream(imageFormat)
		, m_Path(path)
		, m_IfZip(ifZip)
		, m_IfBmpNotTga(ifBmpNotTga)
	{

	}

	virtual bool SupplyImageFrame(double secondsSinceLastFrame, const unsigned char* pBuffer) override;

private:
	std::wstring m_Path;
	bool m_IfZip;
	bool m_IfBmpNotTga;

	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath = false;

	size_t m_FrameNumber = 0;

	bool CreateCapturePath(const char* fileExtension, std::wstring& outPath);
};

class COutFFMPEGVideoStream : public COutVideoStream
{
public:
	COutFFMPEGVideoStream(const CImageFormat& imageFormat, const std::wstring& path, const std::wstring& ffmpegOptions, float frameRate);

	virtual bool SupplyImageFrame(double secondsSinceLastFrame, const unsigned char* pBuffer) override;

protected:
	virtual ~COutFFMPEGVideoStream() override;

private:
	PROCESS_INFORMATION m_ProcessInfo;
	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath = false;
	BOOL m_Okay = FALSE;
	HANDLE m_hChildStd_IN_Rd = NULL;
	HANDLE m_hChildStd_IN_Wr = NULL;
	HANDLE m_hChildStd_OUT_Rd = NULL;
	HANDLE m_hChildStd_OUT_Wr = NULL;
	HANDLE m_hChildStd_ERR_Rd = NULL;
	HANDLE m_hChildStd_ERR_Wr = NULL;
	OVERLAPPED m_OverlappedStdin = {};
	//OVERLAPPED m_OverlappedStdout = {};
	//OVERLAPPED m_OverlappedStderr = {};

	void Close();

	bool HandleOutAndErr(DWORD processWaitTimeOut = 0);
};


// TODO: The sampler could work in parallel on the image.
// TODO:
// - optimize shutter to allow skipping (capturing of) frames.
// - think about error propagation, though not entirely applicable.
// - RGBA smapling might not be accurate, since it doesn't take alpha into account?
class COutSamplingStream : public COutVideoStream
	, public IFramePrinter
	, public IFloatFramePrinter
{
public:
	COutSamplingStream(const CImageFormat& imageFormat, COutVideoStream* outVideoStream, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength, IImageBufferPool* imageBufferPool);

	virtual bool SupplyImageFrame(double secondsSinceLastFrame, const unsigned char* pBuffer) override;

	virtual void Print(unsigned char const* data) override;
	virtual void Print(float const* data) override;

protected:
	virtual ~COutSamplingStream() override;

private:
	union {
		EasyByteSampler* Byte;
		EasyFloatSampler* Float;
	} m_EasySampler;
	COutVideoStream* m_OutVideoStream;
	double m_Time;
	double m_InputFrameDuration;
	IImageBufferPool* m_ImageBufferPool;
};


// TODO: The out streams could work in parallel on the image.
class COutMultiVideoStream : public COutVideoStream
{
public:
	COutMultiVideoStream(const CImageFormat& imageFormat, std::list<COutVideoStream*>&& outStreams)
		: COutVideoStream(imageFormat)
		, m_OutStreams(outStreams)
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (COutVideoStream* stream = *it) stream->AddRef();
		}
	}

	virtual bool SupplyImageFrame(double secondsSinceLastFrame, const unsigned char* pBuffer) override
	{
		bool okay = true;

		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (COutVideoStream* stream = *it)
			{
				if (!stream->SupplyImageFrame(secondsSinceLastFrame, pBuffer)) okay = false;
			}
		}

		return okay;
	}

protected:
	virtual ~COutMultiVideoStream() override
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (COutVideoStream* stream = *it) stream->Release();
		}
	}

private:
	std::list<COutVideoStream*> m_OutStreams;
};


} // namespace advancedfx {
