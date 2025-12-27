#include "stdafx.h"

#include "mdt_media.h"

#include <algorithm>

//#define MDT_DEBUG
#ifdef MDT_DEBUG
	#include <stdio.h> // _snprintf
	#include <malloc.h> // _msize
#endif

CMdt_Media_RAWGLPIC::Error CMdt_Media_RAWGLPIC::DoGlReadPixels(int iXofs, int iYofs, const advancedfx::CImageFormat & imageFormat, advancedfx::CImageBuffer * pImageBuffer)
{
#ifdef MDT_DEBUG
	static char sztmp[2000];
#endif
	if(
		imageFormat.Origin != advancedfx::ImageOrigin::TopLeft
		|| imageFormat.Pitch % 4 != 0
	)
		return Error::ImageFormat;

	GLenum eGLformat;
	GLenum eGLtype;

	switch(imageFormat.Format){
	case advancedfx::ImageFormat::BGR:
		eGLformat = GL_BGR_EXT;
		eGLtype = GL_UNSIGNED_BYTE;
		break;
	case advancedfx::ImageFormat::BGRA:
		eGLformat = GL_BGRA_EXT;
		eGLtype = GL_UNSIGNED_BYTE;
		break;
	case advancedfx::ImageFormat::A:
		eGLformat = GL_ALPHA;
		eGLtype = GL_UNSIGNED_BYTE;
		break;
	case advancedfx::ImageFormat::ZFloat:
		eGLformat = GL_DEPTH_COMPONENT;
		eGLtype = GL_FLOAT;
		break;
	case advancedfx::ImageFormat::RGBA:
		eGLformat = GL_RGBA;
		eGLtype = GL_UNSIGNED_BYTE;
		break;
	default:
		return Error::ImageFormat;		
	}

	if(nullptr == pImageBuffer || !pImageBuffer->GrowAlloc(imageFormat))
		return Error::Memory;

	unsigned char* pBuffer = static_cast<unsigned char*>(pImageBuffer->GetImageBufferData());

#ifdef MDT_DEBUG
	_snprintf(sztmp,1000,"_uiSize: %u Bytes\niXofs: %i\niYofs: %i\n_iWidth: %i\n_iHeight: %i\n_eGLformat: 0x%08x\n_eGLtype: 0x%08x\n_pBuffer:  0x%08x (%u Bytes)",imageFormat.Bytes,iXofs,iYofs,imageFormat.Width,imageFormat.Height,eGLformat,eGLtype,pBuffer,_msize(_pBuffer));
	MessageBox(0,sztmp,"DEBUG",MB_OK);
	memset(pBuffer,0x0FF,_uiSize);
#endif

	glReadPixels(iXofs, iYofs, imageFormat.Width, imageFormat.Height, eGLformat, eGLtype, pBuffer);

#ifdef MDT_DEBUG
	unsigned char* tBuffer=pBuffer+imageFormat.Bytes-1;
	unsigned int tcnt=0;
	GLint t_rowl,t_skipp,t_skiprows,t_align;

	glGetIntegerv(GL_PACK_ROW_LENGTH,&t_rowl);
	glGetIntegerv(GL_PACK_SKIP_PIXELS,&t_skipp);
	glGetIntegerv(GL_PACK_SKIP_ROWS,&t_skiprows);
	glGetIntegerv(GL_PACK_ALIGNMENT,&t_align);

	while (pBuffer<=tBuffer && *tBuffer == 0x0FF)
	{
		tBuffer--;
		tcnt++;
	}
	_snprintf(sztmp,2000,"GL_PACK_ROW_LENGTH:%i\nGL_PACK_SKIP_PIXELS: %i\nGL_PACK_SKIP_ROWS: %i\nGL_PACK_ALIGNMENT: %i\n\niXofs: %i\niYofs: %i\n_iWidth: %i\n_iHeight: %i\n_eGLformat: 0x%08x\n_eGLtype: 0x%08x\n_pBuffer:  0x%08x (%u Bytes)\n\nExpected: %u Bytes\nAllocated: %u Bytes\nglReadPixels used: %u Bytes (minimum, might be more)",t_rowl,t_skipp,t_skiprows,t_align,iXofs,iYofs,imageFormat.Width,imageFormat.Height,eGLformat,eGLtype,pBuffer,_msize(_pBuffer),imageFormat.Width*imageFormat.GetPixelStride()*imageFormat.Height,imageFormat.Bytes,imageFormat.Bytes-tcnt);
	MessageBox(0,sztmp,"glReadPixels DEBUG",MB_OK);
#endif

	return Error::None;
}
