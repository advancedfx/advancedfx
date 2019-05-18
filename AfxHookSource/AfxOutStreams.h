#pragma once

#include "AfxImageBuffer.h"
#include "AfxThreadedRefCounted.h"
#include <string>

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
