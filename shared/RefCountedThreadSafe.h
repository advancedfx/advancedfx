#pragma once

#include "TRefCounted.h"
#include <atomic>

namespace advancedfx {
	
	template<> class TRefCounted<true>
	{
	public:
		TRefCounted()
			: m_RefCount(0) {
		}

		void AddRef(void) {
			m_RefCount++;
		}

		void Release(void) {
			if(1 == std::atomic_fetch_sub_explicit(&m_RefCount, 1, std::memory_order_relaxed))
				delete this;
		}

	protected:
		virtual ~TRefCounted() {

		}

		int GetRefCount() const {
			return m_RefCount;
		}

	private:
		std::atomic_int m_RefCount;
	};

	typedef TRefCounted<true> CRefCountedThreadSafe;
}
