#pragma once

#include "Events.h"

namespace AfxHookSource {
namespace Model {
	
template<typename T, CClass::EType ET>
class CProperty : public CClass
{
public:
	CProperty(CClass * memberOf)
		: CClass(memberOf)
		, m_SetEventSource(this, this)
	{
	}

	CProperty(CClass * memberOf, T value)
		: CClass(memberOf)
		, m_SetEventSource(this, this)
	{
		m_Value = value;
	}

	virtual EType GetType() const
	{
		return ET;
	}

	CEventSource<CProperty> & GetSetEventSource()
	{
		return m_SetEventSource;
	}

	virtual T Get()
	{
		return m_Value;
	}

	virtual void Set(T value)
	{
		m_Value = value;
		m_SetEventSource.Notify();
	}

protected:
	T m_Value;

private:
	CEventSource<CProperty> m_SetEventSource;
};

typedef CProperty<bool, CClass::EType_BoolProperty> CBoolProperty;
typedef CProperty<int, CClass::EType_IntProperty> CIntProperty;
typedef CProperty<float, CClass::EType_FloatProperty> CFloatProperty;
typedef CProperty<double, CClass::EType_DoubleProperty> CDoubleProperty;


class CStringProperty : public CClass
{
public:
	CStringProperty(CClass * memberOf)
		: CClass(memberOf)
		, m_SetEventSource(this, this)
	{
	}

	CStringProperty(CClass * memberOf, char const * value)
		: CClass(memberOf)
		, m_SetEventSource(this, this)
	{
		m_Value = value;
	}

	virtual EType GetType() const
	{
		return EType_StringProperty;
	}
	
	CEventSource<CStringProperty> & GetSetEventSource()
	{
		return m_SetEventSource;
	}
	

	virtual const char * Get()
	{
		return m_Value.c_str();
	}

	virtual void Set(const char * value)
	{
		m_Value = value;
		m_SetEventSource.Notify();
	}

protected:
	std::string m_Value;

private:
	CEventSource<CStringProperty> m_SetEventSource;
};

} // namespace Model {
} // namespace advancedfx {
