#pragma once

#include "GrowingBuffer.h"
#include "TGrowingBufferPool.h"

#include <stack>
#include <mutex>

namespace advancedfx {

	template<> class TGrowingBufferPool<true> {
	public:
		~TGrowingBufferPool()
		{
			while (!m_Buffers.empty()) {
				delete m_Buffers.top();
				m_Buffers.pop();
			}
		}

		virtual CGrowingBuffer* AquireBuffer(void) {
			std::unique_lock<std::mutex> lock(m_BuffersMutex);
			if (m_Buffers.empty()) {
				lock.unlock();
				return new CGrowingBuffer();
			}

			CGrowingBuffer* result = m_Buffers.top();
			m_Buffers.pop();
			return result;
		}

		virtual void ReleaseBuffer(CGrowingBuffer* buffer) {
			std::unique_lock<std::mutex> lock(m_BuffersMutex);
			m_Buffers.push(buffer);
		}

	private:
		std::mutex m_BuffersMutex;
		std::stack<CGrowingBuffer*> m_Buffers;
	};

    typedef TGrowingBufferPool<true> CGrowingBufferPoolThreadSafe;
}

