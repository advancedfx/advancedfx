#pragma once

namespace advancedfx {


class CRefCounted
{
public:
	CRefCounted()
		: m_RefCount(0) {
	}

	void AddRef(void) {
		++m_RefCount;
	}

	void Release(void) {
		--m_RefCount;
		if (0 == m_RefCount)
			delete this;
	}

protected:
	virtual ~CRefCounted() {

	}

private:
	int m_RefCount;
};


} // namespace advancedfx {
