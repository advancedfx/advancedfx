#pragma once

#include <gl/gl.h>

#include "Image.h"

namespace Afx {


// GlImage /////////////////////////////////////////////////////////////////////

class GlImage : public Data,
	public ICapture,
	public IImageX8
{
public:
	GlImage(int x, int y, int width, int height, GLenum format, GLenum type);

	//
	// Interface implementations:

	virtual void ICapture::Capture();
	virtual IRef * ICapture::Ref() { return this; }

	virtual IData * IImageX8::Data() { return this; }
	virtual int IImageX8::Height() { return m_Height; }
	virtual IRef * IImageX8::Ref() { return this; }
	virtual int IImageX8::Pitch() { return m_Pitch; }
	virtual int IImageX8::Width() { return m_WidthX8; }

protected:

private:
	GLenum m_Format;
	int m_Height;
	int m_Pitch;
	GLenum m_Type;
	int m_Width;
	int m_WidthX8;
	int m_X;
	int m_Y;
};



} // namespace Afx {
