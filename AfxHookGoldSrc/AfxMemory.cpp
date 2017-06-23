#include "stdafx.h"

#include "AfxMemory.h"

#include <stdlib.h>

// AfxMemory ///////////////////////////////////////////////////////////////////

AfxMemory::AfxMemory()
{
	m_Memory = 0;
	m_NumBytesAllocated = 0;
}

AfxMemory::~AfxMemory()
{
	free(m_Memory);
}

void * AfxMemory::GetMemory()
{
	return m_Memory;
}

void * AfxMemory::Realloc(size_t numBytes)
{
	return Realloc(numBytes, true);
}

void * AfxMemory::Realloc(size_t numBytes, bool dontShrink)
{
	if(dontShrink && numBytes <= m_NumBytesAllocated && m_Memory)
		return m_Memory;

	void * newMemory = realloc(m_Memory, numBytes);

	if(newMemory)
	{
		m_Memory = newMemory;
		m_NumBytesAllocated = numBytes;
	}

	return newMemory;
}
