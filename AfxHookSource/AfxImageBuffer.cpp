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

	bool available = !m_Buffers.empty();

	if (available)
	{
		lock.unlock();
		m_BufferAvailableCondition.notify_one();
	}

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
	: Buffer(nullptr)
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
	if (!Buffer || m_BufferBytesAllocated < format.Bytes)
	{
		Buffer = realloc(Buffer, format.Bytes);
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
		Format = CAfxImageFormat();
	}

	return 0 != Buffer;
}
