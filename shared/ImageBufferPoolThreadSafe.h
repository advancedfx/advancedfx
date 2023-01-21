#pragma once

#include "AfxImageBuffer.h"

#include <stack>
#include <mutex>

namespace advancedfx {

	class CImageBufferPoolThreadSafe
		: public IImageBufferPool
	{
	public:
		~CImageBufferPoolThreadSafe()
		{
			while (!m_Buffers.empty()) {
				delete m_Buffers.top();
				m_Buffers.pop();
			}
		}

		virtual CImageBuffer* AquireBuffer(void) {
			std::unique_lock<std::mutex> lock(m_BuffersMutex);
			if (m_Buffers.empty()) {
				lock.unlock();
				return new CImageBuffer();
			}

			CImageBuffer* result = m_Buffers.top();
			m_Buffers.pop();
			return result;
		}

		virtual void ReleaseBuffer(CImageBuffer* buffer) {
			std::unique_lock<std::mutex> lock(m_BuffersMutex);
			m_Buffers.push(buffer);
		}

	private:
		std::mutex m_BuffersMutex;
		std::stack<CImageBuffer*> m_Buffers;
	};

} // namespace advancedfx {
