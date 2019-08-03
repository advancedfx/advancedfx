#pragma once

#include "AfxThreadedRefCounted.h"
#include "AfxImageBuffer.h"
#include <shared/EasySampler.h>
#include <string>
#include <Windows.h>

#include <list>

class CAfxOutStream : public CAfxThreadedRefCounted
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
	CAfxOutStream(Type mediaType)
	{
	}

private:
	Type m_Type;
};

class CAfxOutAudioStream : public CAfxOutStream
{
public:
	unsigned int GetChannels() const
	{
		return m_Channles;
	}

	virtual bool SupplyAudioData(unsigned int channels, unsigned int samples, const float * data) = 0;

protected:
	CAfxOutAudioStream(unsigned int channels)
		: CAfxOutStream(Type_Audio)
		, m_Channles(channels)
	{

	}

	unsigned int m_Channles;
};

class CAfxOutVideoStream : public CAfxOutStream
{
public:
	const CAfxImageFormat & GetImageFormat() const
	{
		return m_ImageFormat;
	}

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer) = 0;

protected:
	CAfxOutVideoStream(const CAfxImageFormat & imageFormat)
		: CAfxOutStream(Type_Audio)
		, m_ImageFormat(imageFormat)
	{

	}

	const CAfxImageFormat & m_ImageFormat;
};

class CAfxOutImageStream : public CAfxOutVideoStream
{
public:
	CAfxOutImageStream(const CAfxImageFormat & imageFormat, const std::wstring & path, bool ifZip, bool ifBmpNotTga)
		: CAfxOutVideoStream(imageFormat)
		, m_Path(path)
		, m_IfZip(ifZip)
		, m_IfBmpNotTga(ifBmpNotTga)
	{

	}

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer) override;

private:
	std::wstring m_Path;
	bool m_IfZip;
	bool m_IfBmpNotTga;

	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath;

	size_t m_FrameNumber = 0;

	bool CreateCapturePath(const char * fileExtension, std::wstring &outPath);
};

class CAfxOutFFMPEGVideoStream : public CAfxOutVideoStream
{
public:
	CAfxOutFFMPEGVideoStream(const CAfxImageFormat & imageFormat, const std::wstring & path, const std::wstring & ffmpegOptions, float frameRate);

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer) override;

protected:
	virtual ~CAfxOutFFMPEGVideoStream() override;

private:
	PROCESS_INFORMATION m_ProcessInfo;
	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath;
	BOOL m_Okay = FALSE;
	HANDLE m_hChildStd_IN_Rd = NULL;
	HANDLE m_hChildStd_IN_Wr = NULL;
	HANDLE m_hChildStd_OUT_Rd = NULL;
	HANDLE m_hChildStd_OUT_Wr = NULL;
	HANDLE m_hChildStd_ERR_Rd = NULL;
	HANDLE m_hChildStd_ERR_Wr = NULL;
	OVERLAPPED m_OverlappedWrite = {};

	void Close();

	bool HandleOutAndErr();
};


// TODO:
// - optimize shutter to allow skipping (capturing of) frames.
// - think about error propagation, though not entirely applicable.
// - RGBA smapling might not be accurate, since it doesn't take alpha into account?
class CAfxOutSamplingStream : public CAfxOutVideoStream
	, public IFramePrinter
	, public IFloatFramePrinter
{
public:
	CAfxOutSamplingStream(const CAfxImageFormat & imageFormat, CAfxOutVideoStream * outVideoStream, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength);

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer) override;

	virtual void Print(unsigned char const * data) override;
	virtual void Print(float const * data) override;

protected:
	virtual ~CAfxOutSamplingStream() override;

private:
	union {
		EasyByteSampler * Byte;
		EasyFloatSampler * Float;
	} m_EasySampler;
	CAfxOutVideoStream * m_OutVideoStream;
	double m_Time;

	double m_InputFrameDuration;
};

class CAfxOutMultiVideoStream : public CAfxOutVideoStream
{
public:
	CAfxOutMultiVideoStream(const CAfxImageFormat & imageFormat, std::list<CAfxOutVideoStream *> && outStreams)
		: CAfxOutVideoStream(imageFormat)
		, m_OutStreams(outStreams)
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (CAfxOutVideoStream * stream = *it) stream->AddRef();
		}
	}

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer) override
	{
		bool okay = true;

		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (CAfxOutVideoStream * stream = *it)
			{
				if (!stream->SupplyVideoData(buffer)) okay = false;
			}
		}

		return okay;
	}

protected:
	virtual ~CAfxOutMultiVideoStream() override
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (CAfxOutVideoStream * stream = *it) stream->Release();
		}
	}

private:
	std::list<CAfxOutVideoStream *> m_OutStreams;
};