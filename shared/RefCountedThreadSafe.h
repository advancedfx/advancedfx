#pragma once

#include <atomic>

namespace advancedfx {

class CRefCountedThreadSafe
{
public:
	CRefCountedThreadSafe()
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
	virtual ~CRefCountedThreadSafe() {

	}

private:
	std::atomic_int m_RefCount;
};


} // namespace advancedfx {
