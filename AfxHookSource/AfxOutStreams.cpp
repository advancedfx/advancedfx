#include "stdafx.h"

#include "AfxOutStreams.h"
#include "AfxWriteFileLimiter.h"

#include <shared/RawOutput.h>
#include <shared/OpenExrOutput.h>
#include <shared/StringTools.h>
#include <shared/FileTools.h>

#include <prop/AfxHookSource/SourceInterfaces.h>

#include <string>
#include <sstream>
#include <iomanip>

bool CAfxOutImageStream::SupplyVideoData(const CAfxImageBuffer & buffer)
{
	CAfxWriteFileLimiterScope writeFileLimiterScope;

	std::wstring path;

	if (CAfxImageFormat::PF_ZFloat == buffer.Format.PixelFormat)
	{
		return CreateCapturePath(".exr", path) && WriteFloatZOpenExr(
			path.c_str(),
			(unsigned char*)buffer.Buffer,
			buffer.Format.Width,
			buffer.Format.Height,
			sizeof(float),
			buffer.Format.Pitch,
			m_IfZip ? WFZOEC_Zip : WFZOEC_None
		);
	}

	if (CAfxImageFormat::PF_A == buffer.Format.PixelFormat)
	{
		return m_IfBmpNotTga
			? CreateCapturePath(".bmp", path) && WriteRawBitmap((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 8, buffer.Format.Pitch)
			: CreateCapturePath(".tga", path) && WriteRawTarga((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 8, true, buffer.Format.Pitch, 0)
			;
	}

	bool isBgra = CAfxImageFormat::PF_BGRA == buffer.Format.PixelFormat;

	return m_IfBmpNotTga && !isBgra
		? CreateCapturePath(".bmp", path) && WriteRawBitmap((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, 24, buffer.Format.Pitch)
		: CreateCapturePath(".tga", path) && WriteRawTarga((unsigned char*)buffer.Buffer, path.c_str(), buffer.Format.Width, buffer.Format.Height, isBgra ? 32 : 24, false, buffer.Format.Pitch, isBgra ? 8 : 0)
		;
}


bool CAfxOutImageStream::CreateCapturePath(const char * fileExtension, std::wstring &outPath)
{
	if (!m_TriedCreatePath)
	{
		m_TriedCreatePath = true;

		bool dirCreated = CreatePath(m_Path.c_str(), m_Path);
		if (dirCreated)
		{
			m_SucceededCreatePath = true;
		}
		else
		{
			std::string ansiString;
			if (!WideStringToUTF8String(m_Path.c_str(), ansiString)) ansiString = "[n/a]";

			Tier0_Warning("ERROR: could not create \"%s\"\n", ansiString.c_str());
		}
	}

	if (!m_SucceededCreatePath)
		return false;

	std::wostringstream os;
	os << m_Path << L"\\" << std::setfill(L'0') << std::setw(5) << m_FrameNumber << std::setw(0) << fileExtension;

	outPath = os.str();

	++m_FrameNumber;

	return true;
}