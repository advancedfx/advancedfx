#pragma once

#include "GrowingBuffer.h"
#include "TGrowingBufferPool.h"

#include <stack>

namespace advancedfx {   

    template<> class TGrowingBufferPool<false> {
    public:
        ~TGrowingBufferPool()
        {
            while (!m_Buffers.empty()) {
                delete m_Buffers.top();
                m_Buffers.pop();
            }
        }

        virtual CGrowingBuffer* AquireBuffer(void) {
            if (m_Buffers.empty()) return new CGrowingBuffer();

            CGrowingBuffer* result = m_Buffers.top();
            m_Buffers.pop();
            return result;
        }

        virtual void ReleaseBuffer(CGrowingBuffer* buffer) {
            m_Buffers.push(buffer);
        }

    private:
        std::stack<CGrowingBuffer*> m_Buffers;
    };

    typedef TGrowingBufferPool<false> CGrowingBufferPool;
}
