#pragma once

enum WriteFloatZOpenExrCompression
{
	WFZOEC_None,
	WFZOEC_Zip
};

bool WriteFloatZOpenExr(
	wchar_t const * fileName,
	unsigned char const * pData,
	int width,
	int height,
	int xStride,
	int yStride,
	WriteFloatZOpenExrCompression compression,
	bool topDown = true);
