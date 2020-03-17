#pragma once

#include <string>
#include <stack>
#include <mutex>
#include <condition_variable>

class CAfxImageFormat
{
public:
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


	CAfxImageFormat(PixelFormat_e pixelFormat, int width, int height, size_t pitch)
		: PixelFormat(pixelFormat)
		, Width(width)
		, Height(height)
		, Pitch(pitch)
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

private:
	void Calc()
	{
		if (Pitch == 0)
		{
			Pitch = Width;
			
			switch (PixelFormat)
			{
			case CAfxImageFormat::PF_BGR:
				Pitch *= 3 * sizeof(char);
				break;
			case CAfxImageFormat::PF_BGRA:
				Pitch *= 4 * sizeof(char);
				break;
			case CAfxImageFormat::PF_A:
				Pitch *= 1 * sizeof(char);
				break;
			case CAfxImageFormat::PF_ZFloat:
				Pitch *= 1 * sizeof(float);
				break;
			default:
				//Tier0_Warning("CAfxImageFormat::Calc: Unsupported pixelFormat\n");
				Pitch = 0;
				break;
			}
		}

		Bytes = Height * Pitch;
	}
};


class CAfxImageBuffer;

class CAfxImageBufferPool
{
public:
	CAfxImageBufferPool();

	/// <remarks>Must not be called until all buffers are done.</remarks>
	~CAfxImageBufferPool();

	CAfxImageBuffer* AquireBuffer(void);

	void ImageBuffer_Done(CAfxImageBuffer* buffer);

private:
	std::stack<CAfxImageBuffer*> m_Buffers;
	std::mutex m_BuffersMutex;
	std::condition_variable m_BufferAvailableCondition;
};

class CAfxImageBuffer
{
public:
	CAfxImageFormat Format;
	void* Buffer;

	CAfxImageBuffer(CAfxImageBufferPool* pool);
	~CAfxImageBuffer();

	/// <summary>Releases the buffer back to the pool (it may not be used anymore until being aquired from the pool again).</summary>
	void Release(void);

	bool AutoRealloc(const CAfxImageFormat& format);

private:
	size_t m_BufferBytesAllocated;
	CAfxImageBufferPool* m_Pool;
};
