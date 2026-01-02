#pragma once

#include <malloc.h>

namespace advancedfx {

	class CGrowingBuffer {
	public:
		CGrowingBuffer()
		: m_pBuffer(nullptr)
		, m_BufferBytesAllocated(0) {

		}

        ~CGrowingBuffer() {
            free(m_pBuffer);
        }		

        bool GrowAlloc(size_t minSizeBytes) {
            if (nullptr == m_pBuffer || m_BufferBytesAllocated < minSizeBytes)
            {
                void * newBuffer = realloc(m_pBuffer, minSizeBytes);
                if (nullptr == newBuffer) {
                    m_BufferBytesAllocated = 0;
                    return false;
                }
                m_pBuffer = newBuffer;
                m_BufferBytesAllocated = minSizeBytes;
            }

			return true;
        }
        
        const void * GetBuffer() const {
            return m_pBuffer;
        }

        void * GetBuffer() {
            return m_pBuffer;
        }

        size_t GetBytesAllocated() const {
            return m_BufferBytesAllocated;
        }

	private:
		void * m_pBuffer;
		size_t m_BufferBytesAllocated;
	};
}
