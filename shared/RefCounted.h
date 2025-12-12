#pragma once

#include "TRefCounted.h"

namespace advancedfx {

	template<> class TRefCounted<false>
	{
	public:
		TRefCounted()
			: m_RefCount(0) {
		}

		void AddRef(void) {
			m_RefCount++;
		}

		void Release(void) {
			m_RefCount--;
			if (0 == m_RefCount)
				delete this;
		}

	protected:
		virtual ~TRefCounted() {

		}

	private:
		int m_RefCount;
	};

	typedef TRefCounted<false> CRefCounted;

} // namespace advancedfx {
