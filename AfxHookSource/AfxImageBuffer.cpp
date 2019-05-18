#include "stdafx.h"

#include "AfxImageBuffer.h"

#include "SourceInterfaces.h"

// CAfxImageBufferPool /////////////////////////////////////////////////////////

CAfxImageBufferPool::CAfxImageBufferPool()
{
	for (int i = 0; i < 3; ++i)
	{
		m_Buffers.push(new CAfxImageBuffer(this));
	}

	m_BufferAvailableCondition.notify_one();
}

CAfxImageBufferPool::~CAfxImageBufferPool()
{
	while (!m_Buffers.empty())
	{
		delete m_Buffers.top();
		m_Buffers.pop();
	}
}

CAfxImageBuffer * CAfxImageBufferPool::AquireBuffer(void)
{
	std::unique_lock<std::mutex> lock(m_BuffersMutex);

	m_BufferAvailableCondition.wait(lock, [this]() { return !m_Buffers.empty(); });

	CAfxImageBuffer * result = m_Buffers.top();

	m_Buffers.pop();

	return result;
}

void CAfxImageBufferPool::ImageBuffer_Done(CAfxImageBuffer * buffer)
{
	{
		std::unique_lock<std::mutex> lock(m_BuffersMutex);

		m_Buffers.push(buffer);
	}

	m_BufferAvailableCondition.notify_one();
}

// CAfxImageBuffer /////////////////////////////////////////////////////////////

CAfxImageBuffer::CAfxImageBuffer(CAfxImageBufferPool * pool)
	: Buffer(0)
	, m_BufferBytesAllocated(0)
	, m_Pool(pool)
{
}

CAfxImageBuffer::~CAfxImageBuffer()
{
	free(Buffer);
}

void CAfxImageBuffer::Release(void)
{
	m_Pool->ImageBuffer_Done(this);
}

bool CAfxImageBuffer::AutoRealloc(const CAfxImageFormat & format)
{
	size_t pitch = format.Width;

	switch (format.PixelFormat)
	{
	case CAfxImageFormat::PF_BGR:
		pitch *= 3 * sizeof(char);
		break;
	case CAfxImageFormat::PF_BGRA:
		pitch *= 4 * sizeof(char);
		break;
	case CAfxImageFormat::PF_A:
		pitch *= 1 * sizeof(char);
		break;
	case CAfxImageFormat::PF_ZFloat:
		pitch *= 1 * sizeof(float);
		break;
	default:
		Tier0_Warning("CAfxImageBuffer::AutoRealloc: Unsupported pixelFormat\n");
		return false;
	}

	size_t imageBytes = pitch * format.Height;

	if (!Buffer || m_BufferBytesAllocated < imageBytes)
	{
		Buffer = realloc(Buffer, imageBytes);
		if (Buffer)
		{
			m_BufferBytesAllocated = imageBytes;
		}
	}

	m_BufferBytesAllocated = imageBytes;
	Format.PixelFormat = format.PixelFormat;
	Format.Width = format.Width;
	Format.Height = format.Height;
	Format.Pitch = pitch;
	Format.Bytes = imageBytes;

	return 0 != Buffer;
}

bool CAfxImageBuffer::BgrMergeBlueToRgba(CAfxImageBuffer const * alphaBuffer)
{
	bool ok = alphaBuffer
		&& Format.Width == alphaBuffer->Format.Width
		&& Format.Height == alphaBuffer->Format.Height
		&& Format.PixelFormat == CAfxImageFormat::PF_BGR
		&& Format.PixelFormat == alphaBuffer->Format.PixelFormat
		&& Format.Pitch == alphaBuffer->Format.Pitch
		&& AutoRealloc(CAfxImageFormat(CAfxImageFormat::PF_BGRA, Format.Width, Format.Height))
		;

	if (ok)
	{
		// interleave B as alpha into A:

		for (int y = Format.Height - 1; y >= 0; --y)
		{
			for (int x = Format.Width - 1; x >= 0; --x)
			{
				unsigned char b = ((unsigned char *)Buffer)[y*Format.Pitch + x * 3 + 0];
				unsigned char g = ((unsigned char *)Buffer)[y*Format.Pitch + x * 3 + 1];
				unsigned char r = ((unsigned char *)Buffer)[y*Format.Pitch + x * 3 + 2];
				unsigned char a = ((unsigned char *)alphaBuffer->Buffer)[y*alphaBuffer->Format.Pitch + x * 3 + 0];

				((unsigned char *)Buffer)[y*Format.Pitch + x * 4 + 0] = b;
				((unsigned char *)Buffer)[y*Format.Pitch + x * 4 + 1] = g;
				((unsigned char *)Buffer)[y*Format.Pitch + x * 4 + 2] = r;
				((unsigned char *)Buffer)[y*Format.Pitch + x * 4 + 3] = a;
			}
		}
	}
	else
	{
		Tier0_Warning("CAfxStreams::View_Render: Combining streams failed.\n");
	}

	return ok;
}
