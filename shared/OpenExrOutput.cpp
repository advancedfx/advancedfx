#include "stdafx.h"

#include "OpenExrOutput.h"

#include "StringTools.h"

#undef min
#undef max

#include <ImfChannelList.h>
#include <ImfHeader.h>
#include <ImfFrameBuffer.h>
#include <ImfOutputFile.h>

#include <ImfNamespace.h>
namespace IMF = OPENEXR_IMF_NAMESPACE;


bool WriteFloatZOpenExr(
	wchar_t const * fileName,
	unsigned char const * pData,
	int width,
	int height,
	int xStride,
	int yStride,
	WriteFloatZOpenExrCompression compression,
	bool topDown)
{
	std::string ansiFileName;

	if(!WideStringToUTF8String(fileName, ansiFileName))
		return false;

	try
	{
		IMF::Header header (width, height);
		header.channels().insert ("Z", IMF::Channel (IMF::FLOAT));
		header.compression() = WFZOEC_Zip == compression ? IMF::ZIP_COMPRESSION : IMF:: NO_COMPRESSION;
		if (!topDown) header.lineOrder() = IMF::DECREASING_Y;

		IMF::OutputFile file (ansiFileName.c_str(), header);

		IMF::FrameBuffer frameBuffer;

		frameBuffer.insert ("Z", IMF::Slice (IMF::FLOAT, (char *) pData, xStride, yStride));

		file.setFrameBuffer (frameBuffer);
		file.writePixels (height);
	}
	catch(...)
	{
		return false;
	}

	return true;
}
