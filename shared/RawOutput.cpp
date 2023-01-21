#include "stdafx.h"

#include "RawOutput.h"

#include <windows.h>
#include <stdio.h>

int CalcPitch(int width, unsigned char bytePerPixel, int byteAlignment)
{
	if(byteAlignment < 1)
		return 0;

	int pitch = 
		width * (int)bytePerPixel;

	if(0 != pitch % byteAlignment)
		pitch = (1+(pitch / byteAlignment))*byteAlignment;

	return pitch;
}

// see RawOutput.h
bool WriteRawBitmap(
	unsigned char const * pData,
	wchar_t const * fileName,
	unsigned short usWidth,
	unsigned short usHeight,
	unsigned char ucBpp,
	int pitch,
	bool bTopDown
)
{
	if(ucBpp > 24) return false;

	FILE *pFile;
	_wfopen_s(&pFile, fileName, L"wb");
	if (!pFile) return false;

	BITMAPINFOHEADER bmInfoH;
	BITMAPFILEHEADER bmFileH;

	//
	// Construct the BITMAPINFOHEADER:

	bmInfoH.biSize = sizeof(bmInfoH);
	bmInfoH.biWidth = (LONG)usWidth;
	bmInfoH.biHeight = bTopDown ? -(LONG)usHeight : (LONG)usHeight;
	bmInfoH.biPlanes = 1;
	bmInfoH.biBitCount = ucBpp;
	bmInfoH.biCompression = BI_RGB;

	bmInfoH.biSizeImage =
		// biHeight * 4 * ceil( cClrBits / 8 ) * biWidth
		(((bmInfoH.biWidth * ucBpp +31) & ~31)>>3) * (LONG)usHeight; 


	bmInfoH.biXPelsPerMeter = 0; // dunno
	bmInfoH.biYPelsPerMeter = 0; // dunno
	
	bmInfoH.biClrUsed = 0;
	if( ucBpp < 24 ) bmInfoH.biClrUsed = 1 << ucBpp;

	bmInfoH.biClrImportant = 0; // all color indexies important lol

	//
	// construct the BITMAPFILEHEADER:

	bmFileH.bfType = 0x4d42; // 0x42='B', 0x4d = 'M'
	bmFileH.bfSize = bmInfoH.biSize
		+ bmInfoH.biClrUsed * sizeof(RGBQUAD) + bmInfoH.biSizeImage;

    bmFileH.bfReserved1 = 0; 
    bmFileH.bfReserved2 = 0;
 
    bmFileH.bfOffBits =
		(DWORD)sizeof(BITMAPFILEHEADER) + 
        bmInfoH.biSize + bmInfoH.biClrUsed * sizeof (RGBQUAD);

	//
	//	write out headers:

	fwrite(&bmFileH, sizeof(BITMAPFILEHEADER), 1, pFile);
	fwrite(&bmInfoH, sizeof(BITMAPINFOHEADER), 1, pFile);

	//
	//	write out fake pallete if required:

	RGBQUAD rgbquad;
	rgbquad.rgbReserved = 0;
	if( bmInfoH.biClrUsed <= 256)
	{
		// gray fade (okay may have some rounding errors hehe):
		float tmpf = (BYTE)(255.0f / (bmInfoH.biClrUsed-1)); // TODO: check if the BYTE conversion is correct
		for( DWORD cols = 0; cols<bmInfoH.biClrUsed; cols++)
		{
			rgbquad.rgbRed = (BYTE)((float)cols * tmpf);
			rgbquad.rgbGreen = rgbquad.rgbRed;
			rgbquad.rgbBlue = rgbquad.rgbRed;
			fwrite(&rgbquad,sizeof(rgbquad),1,pFile);
		}
	} else {
		// simply encode it into RGB:
		for( DWORD cols = 0; cols<bmInfoH.biClrUsed; cols++)
		{
			rgbquad.rgbRed = (BYTE)(cols & 0xFF0000);
			rgbquad.rgbGreen = (BYTE)(cols & 0x00FF00);
			rgbquad.rgbBlue = (BYTE)(cols & 0x0000FF);
			fwrite(&rgbquad,sizeof(rgbquad),1,pFile);
		}
	}

	//
	//	write out image data:

	LONG realLineSize = bmInfoH.biSizeImage / usHeight;

	if(pitch == realLineSize)
	{
		fwrite(pData, sizeof(unsigned char), bmInfoH.biSizeImage, pFile);
		fclose(pFile);
		return true;
	}
	else if(pitch <  realLineSize)
	{
		int iPaddings = realLineSize-pitch;
		char pad=0x00;

		for( LONG line=0; line<usHeight; line++)
		{
			fwrite(pData, sizeof(unsigned char), pitch, pFile);
			pData += pitch;
			
			for(int i=0;i<iPaddings;i++)
				fwrite(&pad, 1, 1, pFile);
		}

		fclose(pFile);
		return true;
	}

	fclose(pFile);
	return false;
}

// see RawOutput.h
bool WriteRawTarga(
	unsigned char const * pData,
	wchar_t const * fileName,
	unsigned short usWidth,
	unsigned short usHeight,
	unsigned char ucBpp,
	bool bGrayScale,
	int pitch,
	unsigned char ucAlphaBpp,
	bool bTopDown
)
{
	unsigned char ucBppCeilDiv8 =  (ucBpp & 0x07) ? (ucBpp >> 3)+1 : (ucBpp >> 3);
	unsigned char ucGray = (bGrayScale ? 3 : 2);
	unsigned char szTgaheader[12] = { 0, 0, ucGray, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char szHeader[6] = { 
		(unsigned char)(usWidth & 0xFF), (unsigned char)(usWidth >> 8), (unsigned char)(usHeight & 0xFF), (unsigned char)(usHeight >> 8), ucBpp, (unsigned char)(ucAlphaBpp & 0xF) | (bTopDown ? ((unsigned char)1<<5) : 0)};
	FILE *pFile;

	_wfopen_s(&pFile, fileName, L"wb");
	if (NULL != pFile)
	{
		fwrite(szTgaheader, sizeof(unsigned char), 12, pFile);
		fwrite(szHeader, sizeof(unsigned char), 6, pFile);

		if(usWidth * ucBppCeilDiv8 == pitch)
			// already packed
			fwrite(pData, sizeof(unsigned char), usWidth * usHeight * ucBppCeilDiv8, pFile);
		else
		{
			for(unsigned short i = 0; i<usHeight; i++)
			{
				fwrite(pData, sizeof(unsigned char), usWidth * ucBppCeilDiv8, pFile);
				pData += pitch;
			}
		}

		fclose(pFile);

		return true;
	}

	return false;
}
