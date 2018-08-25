#pragma once

#include <stddef.h>

class AfxMemory
{
public:
	AfxMemory();
	~AfxMemory();

	void * GetMemory();

	/// <summary>Same as Realloc(numBytes, true).</summary>
	void * Realloc(size_t numBytes);

	/// <returns>0 if realloc failed, otherwise pointer to the memory.</returns>
	/// <remarks>Even if realloc fails, the memory previously allocated stays valid.</remarks>
	void * Realloc(size_t numBytes, bool dontShrink);

private:
	void * m_Memory;
	size_t m_NumBytesAllocated;
};
