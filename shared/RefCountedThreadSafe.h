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
		m_RefCount--;
		if (0 == m_RefCount)
			delete this;
	}

protected:
	virtual ~CRefCountedThreadSafe() {

	}

private:
	std::atomic_int m_RefCount;
};


} // namespace advancedfx {
