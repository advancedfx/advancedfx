#pragma once

#include "Ref.h"

namespace Afx {

struct __declspec(novtable) IData abstract
{
	/// <returns> pointer to memory </returns>
	virtual void * Pointer() abstract = 0;

	virtual IRef * Ref() abstract = 0;
};

class Data : public Ref,
	public IData
{
public:
	/// <summary> Takes ownership of pointed memory. </summary>
	/// <remarks> The memory is freed when the Data
	///  Object is destructed. </remarks>
	Data(void * pointer);

	Data(size_t size);

	static IData * Alloc(size_t size);

	//
	// Interface implementations:

	virtual void * IData::Pointer();

	virtual IRef * IData::Ref();

protected:
	virtual ~Data();

private:
	void * m_Pointer;
};

} // namespace Afx {
