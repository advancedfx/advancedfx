#include "stdafx.h"

#include "Ref.h"

#include <assert.h>

using namespace Afx;

#ifdef AFX_DEBUG_REF
	unsigned int Ref::m_GlobalRefCount = 0;
#endif

Ref::Ref()
{
	m_RefCount = 0;
}

Ref::~Ref()
{
	assert( 0 == m_RefCount );
}

void Ref::AddRef()
{
    m_RefCount++;

#ifdef AFX_DEBUG_REF
	m_GlobalRefCount++;
#endif
}

#ifdef AFX_DEBUG_REF
unsigned int Ref::DEBUG_GetGlobalRefCount (void)
{
	return m_GlobalRefCount;
}
#endif

void Ref::Release()
{
#ifdef AFX_DEBUG_REF
	m_GlobalRefCount--;
#endif

	m_RefCount--;
	assert( 0 <= m_RefCount );

    if (0 == m_RefCount)
    {
        delete this;
    }
}

void Ref::TouchRef (void)
{
	TouchRef(this);
}

void Ref::TouchRef (IRef * ref)
{
	if(ref)
	{
		ref->AddRef();
		ref->Release();
	}
}
