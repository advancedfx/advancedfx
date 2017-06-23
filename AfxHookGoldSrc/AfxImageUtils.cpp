#include "stdafx.h"

#include "AfxImageUtils.h"

#include "filming.h"
#include <shared/RawOutput.h>


// AfxImageUtils ///////////////////////////////////////////////////////////////

bool AfxImageUtils::DebugFloatDepthBuffer(AfxGlImage * image)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	::DebugDepthBuffer(
		(GLfloat *)image->GetMemory(),
		image->GetWidth() * image->GetHeight()
	);

	return true;
}

bool AfxImageUtils::FloatDepthBufferToByteBuffer(AfxGlImage * image)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	// compact so smaller format:

	GLfloatArrayToXByteArray(
		(GLfloat *)image->GetMemory(),
		image->GetWidth(),
		image->GetHeight(),
		1
	);

	// set new format:

	if(!image->SetFormat(
		image->GetWidth(),
		image->GetHeight(),
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE
	))
		return false;

	return true;
}

bool AfxImageUtils::InverseFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	::InverseFloatDepthBuffer(
		(GLfloat *)image->GetMemory(),
		image->GetWidth() * image->GetHeight(),
		zNear,
		zFar
	);

	return true;
}

bool AfxImageUtils::IsEasyFloatDepthBuffer(AfxGlImage * image)
{
	return
		GL_DEPTH_COMPONENT == image->GetFormat()
		&& GL_FLOAT == image->GetType()
		&& image->GetWidth() * sizeof(GLfloat) == image->GetPitch()
	;

	return true;
}

bool AfxImageUtils::LinearizeFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	::LinearizeFloatDepthBuffer(
		(GLfloat *)image->GetMemory(),
		image->GetWidth() * image->GetHeight(),
		zNear,
		zFar
	);

	return true;
}

bool AfxImageUtils::LogarithmizeFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	::LogarithmizeDepthBuffer(
		(GLfloat *)image->GetMemory(),
		image->GetWidth() * image->GetHeight(),
		zNear,
		zFar
	);

	return true;
}

bool AfxImageUtils::SliceFloatDepthBuffer(AfxGlImage * image, GLdouble sliceLo, GLdouble sliceHi)
{
	if(!IsEasyFloatDepthBuffer(image))
		return false;

	SliceDepthBuffer(
		(GLfloat *)image->GetMemory(),
		image->GetWidth() * image->GetHeight(),
		(GLfloat)sliceLo,
		(GLfloat)sliceHi
	);

	return true;
}

bool AfxImageUtils::WriteBitmap(AfxGlImage * image, wchar_t const * fileName)
{
	GLsizei width = image->GetWidth();
	GLsizei height = image->GetHeight();
	size_t pitch = image->GetPitch();
	GLenum format = image->GetFormat();
	GLenum type = image->GetType();

	unsigned char sizeComponent;
	unsigned char numComponents;

	switch(type)
	{
	case GL_UNSIGNED_BYTE:
		sizeComponent = 1;
		break;
	default:
		return false;
	}

	switch(format)
	{
	case GL_DEPTH_COMPONENT:
	case GL_ALPHA:
	case GL_LUMINANCE:
		// currently all those are treated as grey.
		numComponents = 1;
		break;
	case GL_BGR_EXT:
		numComponents = 3;
		break;
	default:
		return false;
	}

	return ::WriteRawBitmap(
		(unsigned char *)image->GetMemory(),
		fileName,
		width,
		height,
		(sizeComponent * numComponents) << 3,
		pitch
	);
}
