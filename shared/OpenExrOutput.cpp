#include "stdafx.h"

#include "OpenExrOutput.h"

#include "StringTools.h"

#include <ImfNamespace.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;

using namespace IMF;

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
		Header header (width, height);
		header.channels().insert ("Z", Channel (IMF::FLOAT));
		header.compression() = WFZOEC_Zip == compression ? ZIP_COMPRESSION : NO_COMPRESSION;
		if (!topDown) header.lineOrder() = DECREASING_Y;

		OutputFile file (ansiFileName.c_str(), header);

		FrameBuffer frameBuffer;

		frameBuffer.insert ("Z", Slice (IMF::FLOAT, (char *) pData, xStride, yStride));

		file.setFrameBuffer (frameBuffer);
		file.writePixels (height);
	}
	catch(...)
	{
		return false;
	}

	return true;
}
