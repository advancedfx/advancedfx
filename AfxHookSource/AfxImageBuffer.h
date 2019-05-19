#pragma once

#include <string>
#include <stack>
#include <mutex>
#include <condition_variable>

struct CAfxImageFormat
{
	enum PixelFormat_e
	{
		PF_Invalid,
		PF_BGR,
		PF_BGRA,
		PF_A,
		PF_ZFloat
	};

	PixelFormat_e PixelFormat;
	int Width;
	int Height;
	size_t Pitch;
	size_t Bytes;

	CAfxImageFormat()
		: PixelFormat(PF_Invalid)
	{

	}

	CAfxImageFormat(PixelFormat_e pixelFormat, int width, int height)
		: PixelFormat(pixelFormat)
		, Width(width)
		, Height(height)
	{

	}

	bool operator==(const CAfxImageFormat & other) const
	{
		return PixelFormat == other.PixelFormat
			&& Width == other.Width
			&& Height == other.Height
			&& Pitch == other.Pitch
			&& Bytes == other.Bytes;
	}
};

class CAfxImageBuffer;

class CAfxImageBufferPool
{
public:
	CAfxImageBufferPool();

	/// <remarks>Must not be called until all buffers are done.</remarks>
	~CAfxImageBufferPool();

	CAfxImageBuffer * AquireBuffer(void);

	void ImageBuffer_Done(CAfxImageBuffer * buffer);

private:
	std::stack<CAfxImageBuffer *> m_Buffers;
	std::mutex m_BuffersMutex;
	std::condition_variable m_BufferAvailableCondition;
};

class CAfxImageBuffer
{
public:
	CAfxImageFormat Format;

	void * Buffer;

	CAfxImageBuffer(CAfxImageBufferPool * pool);
	~CAfxImageBuffer();

	/// <summary>Releases the buffer back to the pool (it may not be used anymore until being aquired from the pool again).</summary>
	void Release(void);

	bool AutoRealloc(const CAfxImageFormat & format);

	/// <summary>
	/// Resizes and merges this BGR format buffer with the blue component of
	/// an buffer of same Width, Height and ImagePitch.
	/// On success the resulting format is BGRA
	/// </summary>
	bool BgrMergeBlueToRgba(CAfxImageBuffer const * alphaBuffer);

private:
	size_t m_BufferBytesAllocated;
	CAfxImageBufferPool * m_Pool;
};

