#include "stdafx.h"

#include "Image.h"

using namespace Afx;


// ImageX16FromImageX8 /////////////////////////////////////////////////////////

class ImageX16FromImageX8 : public Ref,
	public IImageX16
{
public:
	static bool Compatible(IImageX8 * image)
	{
		return 0 == image->Width() % 2;
	}

	ImageX16FromImageX8(IImageX8 * image)
	{
		image->Ref()->AddRef();

		m_ImageX8 = image;
		m_WidthX16 = image->Width() / 2;
	}

	virtual IData * IImageX16::Data() { return m_ImageX8->Data(); }
	virtual int IImageX16::Height() { return m_ImageX8->Height(); }
	virtual int IImageX16::Pitch() { return m_ImageX8->Pitch(); }
	virtual IRef * IImageX16::Ref() { return m_ImageX8->Ref(); }
	virtual int IImageX16::Width() { return m_WidthX16; }

protected:
	virtual ~ImageX16FromImageX8()
	{
		m_ImageX8->Ref()->Release();
	}

private:
	IImageX8 * m_ImageX8;
	int m_WidthX16;
};


// ImageX24FromImageX8 /////////////////////////////////////////////////////////

class ImageX24FromImageX8 : public Ref,
	public IImageX24
{
public:
	static bool Compatible(IImageX8 * image)
	{
		return 0 == image->Width() % 3;
	}

	ImageX24FromImageX8(IImageX8 * image)
	{
		image->Ref()->AddRef();

		m_ImageX8 = image;
		m_WidthX24 = image->Width() / 3;
	}

	virtual IData * IImageX24::Data() { return m_ImageX8->Data(); }
	virtual int IImageX24::Height() { return m_ImageX8->Height(); }
	virtual int IImageX24::Pitch() { return m_ImageX8->Pitch(); }
	virtual IRef * IImageX24::Ref() { return m_ImageX8->Ref(); }
	virtual int IImageX24::Width() { return m_WidthX24; }

protected:
	virtual ~ImageX24FromImageX8()
	{
		m_ImageX8->Ref()->Release();
	}

private:
	IImageX8 * m_ImageX8;
	int m_WidthX24;
};


// ImageX32FromImageX8 /////////////////////////////////////////////////////////

class ImageX32FromImageX8 : public Ref,
	public IImageX32
{
public:
	static bool Compatible(IImageX8 * image)
	{
		return 0 == image->Width() % 4;
	}

	ImageX32FromImageX8(IImageX8 * image)
	{
		image->Ref()->AddRef();

		m_ImageX8 = image;
		m_WidthX32 = image->Width() / 4;
	}

	virtual IData * IImageX32::Data() { return m_ImageX8->Data(); }
	virtual int IImageX32::Height() { return m_ImageX8->Height(); }
	virtual int IImageX32::Pitch() { return m_ImageX8->Pitch(); }
	virtual IRef * IImageX32::Ref() { return m_ImageX8->Ref(); }
	virtual int IImageX32::Width() { return m_WidthX32; }

protected:
	virtual ~ImageX32FromImageX8()
	{
		m_ImageX8->Ref()->Release();
	}

private:
	IImageX8 * m_ImageX8;
	int m_WidthX32;
};


// ImageXSFromImageX8 /////////////////////////////////////////////////////////

class ImageXSFromImageX8 : public Ref,
	public IImageXS
{
public:
	static bool Compatible(IImageX8 * image)
	{
		return 0 == image->Width() % 4;
	}

	ImageXSFromImageX8(IImageX8 * image)
	{
		image->Ref()->AddRef();

		m_ImageX8 = image;
		m_WidthXS = image->Width() / 4;
	}

	virtual IData * IImageXS::Data() { return m_ImageX8->Data(); }
	virtual int IImageXS::Height() { return m_ImageX8->Height(); }
	virtual int IImageXS::Pitch() { return m_ImageX8->Pitch(); }
	virtual IRef * IImageXS::Ref() { return m_ImageX8->Ref(); }
	virtual int IImageXS::Width() { return m_WidthXS; }

protected:
	virtual ~ImageXSFromImageX8()
	{
		m_ImageX8->Ref()->Release();
	}

private:
	IImageX8 * m_ImageX8;
	int m_WidthXS;
};


// ImageWrapper ////////////////////////////////////////////////////////////////

IImageX16 * ImageWrapper::ToX16(IImageX8 * image)
{
	if(!ImageX16FromImageX8::Compatible(image))
		return 0;

	return new ImageX16FromImageX8(image);
}


IImageX24 * ImageWrapper::ToX24(IImageX8 * image)
{
	if(!ImageX24FromImageX8::Compatible(image))
		return 0;

	return new ImageX24FromImageX8(image);
}

IImageX32 * ImageWrapper::ToX32(IImageX8 * image)
{
	if(!ImageX32FromImageX8::Compatible(image))
		return 0;

	return new ImageX32FromImageX8(image);
}

IImageXS * ImageWrapper::ToXS(IImageX8 * image)
{
	if(!ImageXSFromImageX8::Compatible(image))
		return 0;

	return new ImageXSFromImageX8(image);
}
