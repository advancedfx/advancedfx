#pragma once

// Temporary raw and dirty raw output functions
// to allow getting the existing code more structured and readable
// and also to reduce code duplication.
//
// Raw means dumb, no checks etc..

#include <windows.h>
#include <winbase.h>


int CalcPitch(int width, unsigned char bytePerPixel, int byteAlignment);

//	WriteRawTarga
//
//	Writes out prepared data to an tga file:
//	This function assumes that the given
//	parameters are correct, no additional consistency checks
//	are performed.
//
//	Params: (not checked for consistency)
//	pData - pointer on Data in the format of (B,G,R,[A]),(B,G,R,[A]),....
//		from bottom-left to top-right
//	pszFileName - [path] and filename of the output file
//	usWidth - Width in pixels
//	usHeight - Height in pixels
//	ucBpp - BitDepth
//	bGrayScale - if this image is GrayScale or color
/// <param name="ucAlphaBpp">Number of alpha bits (0 - 15).</param>
bool WriteRawTarga(
	unsigned char const * pData, wchar_t const * fileName,
	unsigned short usWidth, unsigned short usHeight,
	unsigned char ucBpp, bool bGrayScale,
	int pitch,
	unsigned char ucAlphaBpp = 0,
	bool bTopDown = false
);

//	WriteRawBitmap
//
//	pData - bottom-left -> top-right, 4 Byte (32Bit DWORD) alligned
//	ucBpp <= 24
//	pitch // number of bytes in a row
bool WriteRawBitmap(
	unsigned char const* pData,
	wchar_t const* fileName,
	unsigned short usWidth,
	unsigned short usHeight,
	unsigned char ucBpp,
	int pitch,
	bool bTopDown = false
);
