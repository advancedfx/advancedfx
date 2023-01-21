#pragma once

#include <stack>
#include <malloc.h>

namespace advancedfx {


enum class ImageFormat : unsigned int {
	Unkown = 0,
	BGR = 1,
	BGRA = 2,
	A = 3,
	ZFloat = 4
};

enum class ImageOrigin : unsigned int {
	Unknown = 0,
	TopLeft = 1,
	BottomLeft = 2
};

class CImageFormat
{
public:
	ImageFormat Format;
	int Width;
	int Height;
	size_t Pitch;
	size_t Bytes;
	ImageOrigin Origin;

	CImageFormat()
		: Format(ImageFormat::Unkown)
		, Width(0)
		, Height(0)
		, Pitch(0)
		, Origin(ImageOrigin::Unknown)
	{
		Calc();
	}

	CImageFormat(ImageFormat format, int width, int height)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(0)
		, Origin(ImageOrigin::TopLeft)
	{
		Calc();
	}


	CImageFormat(ImageFormat format, int width, int height, size_t pitch)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(pitch)
		, Origin(ImageOrigin::TopLeft)
	{
		Calc();
	}

	bool operator==(const CImageFormat & other) const
	{
		return Format == other.Format
			&& Width == other.Width
			&& Height == other.Height
			&& Pitch == other.Pitch
			&& Bytes == other.Bytes
			&& Origin == other.Origin;
	}

	void SetOrigin(ImageOrigin origin) {
		Origin = origin;
	}

	size_t GetLineStride() const {
		return Pitch;
	}

	size_t GetPixelStride() const {
		switch (Format)
		{
		case ImageFormat::BGR:
			return 3 * sizeof(unsigned char);
		case ImageFormat::BGRA:
			return 4 * sizeof(unsigned char);
		case ImageFormat::A:
			return 1 * sizeof(unsigned char);
		case ImageFormat::ZFloat:
			return 1 * sizeof(float);
		}

		return 0;	
	}

private:
	void Calc()
	{
		if (Pitch == 0)
		{
			Pitch = Width * GetPixelStride();
		}

		Bytes = Height * Pitch;
	}
};


class IImageBuffer abstract {
public:
	virtual const CImageFormat * GetImageBufferFormat() const = 0;
	virtual const void * GetImageBufferData() const = 0;
};

class CImageBuffer
	: public IImageBuffer
{
public:
	CImageFormat Format;
	void* Buffer;

	CImageBuffer()
		: Buffer(nullptr)
		, m_BufferBytesAllocated(0) {
	}

	~CImageBuffer() {
		free(Buffer);
	}

	bool AutoRealloc(const CImageFormat& format) {
		if (!Buffer || m_BufferBytesAllocated < format.Bytes)
		{
			void * newBuffer = realloc(Buffer, format.Bytes);
			if (!newBuffer) return false;
			Buffer = newBuffer;
			if (Buffer)
			{
				m_BufferBytesAllocated = format.Bytes;
			}
		}

		if (0 != Buffer)
		{
			Format = format;
		}
		else
		{
			Format = CImageFormat();
		}

		return 0 != Buffer;
	}

	virtual const CImageFormat * GetImageBufferFormat() const {
		return &Format;
	}
	virtual const void * GetImageBufferData() const {
		return Buffer;
	}

private:
	size_t m_BufferBytesAllocated;
};


class IImageBufferPool {
public:
	virtual class CImageBuffer* AquireBuffer(void) = 0;
	virtual void ReleaseBuffer(class CImageBuffer* buffer) = 0;
};

class CImageBufferPool
	: public IImageBufferPool
{
public:
	~CImageBufferPool()
	{
		while (!m_Buffers.empty()) {
			delete m_Buffers.top();
			m_Buffers.pop();
		}
	}

	virtual CImageBuffer* AquireBuffer(void) {
		if (m_Buffers.empty()) return new CImageBuffer();

		CImageBuffer* result = m_Buffers.top();
		m_Buffers.pop();
		return result;
	}

	virtual void ReleaseBuffer(CImageBuffer* buffer) {
		m_Buffers.push(buffer);
	}

private:
	std::stack<CImageBuffer*> m_Buffers;
};


} // namespace advancedfx {
