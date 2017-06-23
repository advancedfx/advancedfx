#include "stdafx.h"

#include "GlImage.h"

using namespace Afx;


int CalcGlTypeByteCount(GLenum eGLtype)
{
	switch(eGLtype)
	{
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
		return 1;

	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
		return 2;

	case GL_UNSIGNED_INT:
	case GL_INT:
	case GL_FLOAT:
		return 4;
	}

	throw "Unknown GLtype.";
	return -1;
}

int CalcGlFormatComponentCount(GLenum eGLformat)
{
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
		return 1;

	case GL_LUMINANCE_ALPHA:
		return 2; //WARNING: MIGHT BE WRONG.

	case GL_BGR_EXT:
	case GL_RGB:
		return 3;

	case GL_BGRA_EXT:
	case GL_RGBA:
		return 4;
	}

	throw "Unknown GLformat.";
	return -1;
}


int CalcGlImagePitch(int width, GLenum format, GLenum type)
{
	int bytes = CalcGlTypeByteCount(type);
	int comps = CalcGlFormatComponentCount(format);

	int size = bytes * comps * width;

	if(0x3 & size)
	{
		size = ((size >> 2) << 2) +4;
	}

	return 0;
}

size_t CalcGlImageDataSize(int width, int height, GLenum format, GLenum type)
{
	return height * CalcGlImagePitch(width, format, type);
}

// GlImage /////////////////////////////////////////////////////////////////////

GlImage::GlImage(int x, int y, int width, int height, GLenum format, GLenum type)
: ::Data(CalcGlImageDataSize(width, height, format, type))
{
	m_Format = format;
	m_Height = height;
	m_Pitch = CalcGlImagePitch(width, format, type);
	m_Type = type;
	m_Width = width;
	m_WidthX8 = width * CalcGlFormatComponentCount(format);
	m_X = x;
	m_Y = y;
}

void GlImage::Capture()
{
	glReadPixels(m_X, m_Y, m_Width, m_Height, m_Format, m_Type, (GLvoid *)Pointer());
}


