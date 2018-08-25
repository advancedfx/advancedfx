#pragma once

#include "AfxMemory.h"
#include <GL/GL.h>


class AfxGlImage : public AfxMemory
{
public:
	AfxGlImage();

	GLenum GetFormat();

	GLsizei GetHeight();
	
	/// <returns>Size of a row in bytes.</returns>
	size_t GetPitch();

	GLenum GetType();

	GLsizei GetWidth();

	/// <returns>0 upon error, otherwise pointer to memory.</returns>
	void * GlReadPixels(GLint  x,  GLint  y,  GLsizei  width,  GLsizei  height,  GLenum  format,  GLenum  type);

	/// <returns>0 upon error, otherwise pointer to memory if format was set successfully.</returns>
	/// <remarks>Data (Memory) is not converted, semantics of Realloc apply. Conversion must be applied beforehand or afterwards as is appropiate.</remarks>
	void * SetFormat(GLsizei  width,  GLsizei  height,  GLenum  format,  GLenum  type);

private:
	GLenum m_Format;
	GLsizei m_Height;
	size_t m_Pitch;
	GLenum m_Type;
	GLsizei m_Width;

	static bool _CalcSizeComponent (GLenum eGLtype, unsigned char* outSizeComponent, bool* outIsSigned);	// returns the size of one component in bytes if result is true, false if unknown, also used by constructor
	static bool _CalcNumComponents (GLenum eGLformat, unsigned char* outNumComponents);	// returns false if unknown otherwise true and the number of components per pixel
};
