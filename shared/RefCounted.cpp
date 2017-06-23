#include "stdafx.h"

#include "RefCounted.h"

namespace Afx {

// CRefCounted /////////////////////////////////////////////////////////////////

CRefCounted::CRefCounted()
: m_RefCount(0)
{
}

CRefCounted::~CRefCounted()
{
}

void CRefCounted::AddRef(void)
{
	++m_RefCount;
}

void CRefCounted::Release(void)
{
	--m_RefCount;
	if(0 == m_RefCount)
		delete this;
}

} // namespace Afx {
