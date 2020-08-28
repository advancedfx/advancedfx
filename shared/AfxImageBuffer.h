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

class CImageFormat
{
public:
	ImageFormat Format;
	int Width;
	int Height;
	size_t Pitch;
	size_t Bytes;

	CImageFormat()
		: Format(ImageFormat::Unkown)
		, Width(0)
		, Height(0)
		, Pitch(0)
	{
		Calc();
	}

	CImageFormat(ImageFormat format, int width, int height)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(0)
	{
		Calc();
	}


	CImageFormat(ImageFormat format, int width, int height, size_t pitch)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(pitch)
	{
		Calc();
	}

	bool operator==(const CImageFormat & other) const
	{
		return Format == other.Format
			&& Width == other.Width
			&& Height == other.Height
			&& Pitch == other.Pitch
			&& Bytes == other.Bytes;
	}

private:
	void Calc()
	{
		if (Pitch == 0)
		{
			Pitch = Width;
		}

		switch (Format)
		{
		case ImageFormat::BGR:
			Pitch *= 3 * sizeof(char);
			break;
		case ImageFormat::BGRA:
			Pitch *= 4 * sizeof(char);
			break;
		case ImageFormat::A:
			Pitch *= 1 * sizeof(char);
			break;
		case ImageFormat::ZFloat:
			Pitch *= 1 * sizeof(float);
			break;
		default:
			Pitch = 0;
			break;
		}

		Bytes = Height * Pitch;
	}
};


class CImageBuffer
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

private:
	size_t m_BufferBytesAllocated;
};


class CImageBufferPool
{
public:
	~CImageBufferPool()
	{
		while (!m_Buffers.empty()) {
			delete m_Buffers.top();
			m_Buffers.pop();
		}
	}

	CImageBuffer* AquireBuffer(void) {
		if (m_Buffers.empty()) return new CImageBuffer();

		CImageBuffer* result = m_Buffers.top();
		m_Buffers.pop();
		return result;
	}

	void ReleaseBuffer(CImageBuffer* buffer) {
		m_Buffers.push(buffer);
	}

private:
	std::stack<CImageBuffer*> m_Buffers;
};


} // namespace advancedfx {
