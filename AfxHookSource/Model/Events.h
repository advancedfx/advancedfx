#pragma once

#include "Dependencies.h"

#include <set>

namespace advancedfx {
namespace Model {

class CEventSink : public CDependency
{
public:
	virtual void Notify(void)
	{

	}
};

class CEventSource : public CDependable
{
public:
	CEventSource()
	{
		m_SinksIterator = m_Sinks.begin();
	}

	void Add(CEventSink * sink)
	{
		CDependable::Add(sink);

		m_Sinks.insert(sink);
	}

	void Remove(CEventSink * sink)
	{
		std::set<CEventSink *>::iterator it = m_Sinks.find(sink);
		
		if (it == m_SinksIterator) ++m_SinksIterator;

		m_Sinks.erase(it);

		CDependable::Remove(sink);
	}

protected:
	void Notify()
	{
		for (m_SinksIterator = m_Sinks.begin(); m_SinksIterator != m_Sinks.end(); ++m_SinksIterator)
		{
			(*m_SinksIterator)->Notify();
		}
	}

private:
	std::set<CEventSink *> m_Sinks;
	std::set<CEventSink *>::iterator m_SinksIterator;

};


} // namespace Model {
} // namespace advancedfx {
