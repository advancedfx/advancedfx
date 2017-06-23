#include "stdafx.h"

#include "Data.h"

using namespace Afx;

Data::Data(void * pointer)
{
	m_Pointer = pointer;
}

Data::Data(size_t size)
{
	m_Pointer = malloc(size);

	if(!m_Pointer)
		throw "Memory allocation failed.";
}

Data::~Data()
{
	if(m_Pointer) free(m_Pointer);
	m_Pointer = 0;
}

IData * Data::Alloc(size_t size)
{
	void * pointer = malloc(size);

	if(!pointer)
		throw "Memory allocation failed.";

	return new Data(pointer);
}


void * Data::Pointer()
{
	return m_Pointer;
}


IRef * Data::Ref()
{
	return this;
}

