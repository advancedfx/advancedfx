#pragma once

#include "GrowingBuffer.h"
#include "ImageFormat.h"
#include "TRefCounted.h"
#include "TGrowingBufferPool.h"

namespace advancedfx {

	template<bool bThreadSafe> class TIImageBuffer abstract {
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;

		virtual const CImageFormat * GetImageBufferFormat() const = 0;
		virtual const void * GetImageBufferData() const = 0;
	};

	template<bool bThreadSafe> class TImageBuffer : public TRefCounted<bThreadSafe>
	, public TIImageBuffer<bThreadSafe>
	{
	public:
		/**
		* @param pGrowingBufferPool must not be nullptr
		*/
		TImageBuffer(TGrowingBufferPool<bThreadSafe>* pGrowingBufferPool)
			: m_pPool(pGrowingBufferPool)
			, m_pBuffer(pGrowingBufferPool->AquireBuffer()) {
		}

		virtual void AddRef() override {
			TRefCounted<bThreadSafe>::AddRef();
		}

		virtual void Release() override {
			TRefCounted<bThreadSafe>::Release();
		}

		bool GrowAlloc(const CImageFormat& format) {
			if (m_pBuffer->GrowAlloc(format.Bytes))
			{
				m_Format = format;
				return true;
			}

			m_Format = CImageFormat();
			return false;
		}

		virtual const CImageFormat * GetImageBufferFormat() const {
			return &m_Format;
		}
		virtual const void * GetImageBufferData() const {
			return m_pBuffer->GetBuffer();
		}

		virtual void * GetImageBufferData() {
			return m_pBuffer->GetBuffer();
		}
	protected:
		~TImageBuffer() {
			m_pPool->ReleaseBuffer(m_pBuffer);
		}

	private:
		CImageFormat m_Format;
		CGrowingBuffer * m_pBuffer;
		TGrowingBufferPool<bThreadSafe>* m_pPool;
		size_t m_BufferBytesAllocated;
	};	
}
