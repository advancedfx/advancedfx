#include "stdafx.h"

#include "AfxGlImage.h"

// AfxGlImage //////////////////////////////////////////////////////////////////

bool AfxGlImage::_CalcSizeComponent (GLenum eGLtype, unsigned char* outSizeComponent, bool* outIsSigned)
{
	// well this is not too nice, replace it if u want:
	// see GL reference for glDrawPixels
	switch(eGLtype)
	{
	case GL_UNSIGNED_BYTE:
		*outIsSigned      = false;
		*outSizeComponent = 1;
		return true;
	case GL_BYTE:
		*outIsSigned      = true;
		*outSizeComponent = 1;
		return true;
//	case GL_BITMAP:
		// I am to lazy to code that now, this would require special treatment determining the bitdepth etc.
//		return false; // not treated yet
	case GL_UNSIGNED_SHORT:
		*outIsSigned      = false;
		*outSizeComponent = 2;
		return true;
	case GL_SHORT:
		*outIsSigned      = true;
		*outSizeComponent = 2;
		return true;
	case GL_UNSIGNED_INT:
		*outIsSigned      = false;
		*outSizeComponent = 4;
		return true;
	case GL_INT:
		*outIsSigned      = true;
		*outSizeComponent = 4;
		return true;
	case GL_FLOAT:
		*outIsSigned      = true;
		*outSizeComponent = 4;
		return true;
	default:
		return false;
	}
}

bool AfxGlImage::_CalcNumComponents (GLenum eGLformat, unsigned char* outNumComponents)
{
	// well this is not too nice, replace it if u want:
	// see GL reference for glReadPixels
	switch(eGLformat)
	{
	case GL_COLOR_INDEX:
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		*outNumComponents = 1;
		return true;
	case GL_BGR_EXT:
	case GL_RGB:
		*outNumComponents = 3;
		return true;
	case GL_BGRA_EXT:
	case GL_RGBA:
		*outNumComponents = 4;
		return true;
	case GL_LUMINANCE_ALPHA:
		*outNumComponents = 2;
		return true;
	default:
		return false;
	}
}

AfxGlImage::AfxGlImage()
{
	m_Format = GL_ZERO;
	m_Height = 0;
	m_Pitch = 0;
	m_Type = GL_ZERO;
	m_Width = 0;
}

GLenum AfxGlImage::GetFormat()
{
	return m_Format;
}

GLsizei AfxGlImage::GetHeight()
{
	return m_Height;
}
	
size_t AfxGlImage::GetPitch()
{
	return m_Pitch;
}

GLenum AfxGlImage::GetType()
{
	return m_Type;
}

GLsizei AfxGlImage::GetWidth()
{
	return m_Width;
}

void * AfxGlImage::GlReadPixels(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height,  GLenum  format,  GLenum  type)
{
	void * newMemory = SetFormat(width, height, format, type);

	if(!newMemory)
		return 0;

	glReadPixels(x, y, width, height, format, type, newMemory);

	return newMemory;
}

void * AfxGlImage::SetFormat(GLsizei  width,  GLsizei  height,  GLenum  format,  GLenum  type)
{
	unsigned char numComponents;

	if (!_CalcNumComponents(format, &numComponents))
		return 0;

	unsigned char sizeComponent;
	bool componentSigned;

	if (!_CalcSizeComponent(type, &sizeComponent, &componentSigned))
		return 0;

	size_t pixelSize = (size_t)numComponents * (size_t)sizeComponent;
	size_t rowSize = width * pixelSize;

	// assuming GL_PIXEL_ALIGNMENT == 4:
	if (rowSize & 0x03)
		// is not divideable by 4 (has remainder)
		rowSize = (1+ (rowSize >> 2))<<2; // fill up to 4

	size_t imageSize = height * rowSize;

	void * newMemory = Realloc(imageSize);

	if(!newMemory)
		return 0;

	m_Format = format;
	m_Height = height;
	m_Pitch = rowSize;
	m_Type = type;
	m_Width = width;

	return newMemory;
}
