#pragma once

#include "AfxImageBuffer.h"
#include "AfxThreadedRefCounted.h"
#include <string>
#include <Windows.h>

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

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer);

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

	virtual bool SupplyVideoData(const CAfxImageBuffer & buffer);

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

	void Close();

	bool HandleOutAndErr();
};
