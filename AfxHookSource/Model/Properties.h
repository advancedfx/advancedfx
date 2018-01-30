#pragma once

#include "Events.h"

namespace advancedfx {
namespace Model {
	
template <class T>
class CProperty : public CEventSource
{
public:
	CProperty()
	{

	}

	CProperty(T value)
	{
		m_Value = value;
	}

	virtual T Get()
	{
		return m_Value;
	}

	virtual void Set(T value)
	{
		m_Value = value;
		Notify();
	}

protected:
	T m_Value;
};

class CString : public CEventSource
{
	virtual const char * Get()
	{
		return m_Value.c_str();
	}

	virtual void Set(const char * value)
	{
		m_Value = value;
		Notify();
	}

protected:
	std::string m_Value;
};

} // namespace Model {
} // namespace advancedfx {
