#pragma once

#include "Model.h"

#include <set>

namespace AfxHookSource {
namespace Model {

template<class Target, class Source>
class CDeletedNotifier : public CClass
{
public:
	typedef void (Target::*NotificationFn_t)(Source * source, CClass * cause);

	CDeletedNotifier(Target * target, NotificationFn_t fn, Source * source)
		: CClass(target)
		, m_Target(target)
		, m_Fn(fn)
	{
		Depend(source);
	}

	virtual EType GetType() const
	{
		return EType_DeletedNotifier;
	}

protected:
	virtual void OnDeleting(CClass * cause)
	{
		if (cause == m_Source)
		{
			(m_Target->*m_Fn)(m_Source, cause);
		}

		CClass::OnDeleting(cause);
	}

private:
	Target * m_Target;
	NotificationFn_t m_Fn;
	Source * m_Source;
};

template<class Source>
class CEventSource : public CClass
{
public:
	class CNotifierBase : public CClass
	{
	public:
		CNotifierBase(CEventSource * parent)
			: CClass(parent)
		{
		}

		virtual void Notify() abstract = 0;
	};

	template<class Target>
	class CNotifier : public CNotifierBase
	{
	public:
		typedef void (Target::*NotificationFn_t)(Source * source);

		CNotifier(CEventSource * parent
			, Target * target, NotificationFn_t fn, Source * source)
			: CNotifierBase(parent)
			, m_Parent(parent)
			, m_Target(target)
			, m_Fn(fn)
		{
			Depend(source);
			Depend(target);
		}

		virtual EType GetType() const
		{
			return EType_EventNotifier;
		}

		virtual void Notify(Source * source)
		{
			(m_Target->*m_Fn)(source);
		}

	protected:

		virtual void OnDeleting(CClass * cause)
		{
			m_Parent->OnNotifierDeleting(this);

			CClass::OnDeleting(cause);
		}

	private:
		CEventSource * m_Parent;
		Target * m_Target;
		NotificationFn_t m_Fn;

	};

	CEventSource(Source * source)
		: CClass(source)
		, m_Source(source)
	{
		m_SinksIterator = m_Sinks.begin();
	}

	virtual EType GetType() const
	{
		return EType_EventSource;
	}

	/// <remarks>You can only have one notification function per notifyee.</remarks>
	void AddNotifier(CNotifierBase * notifier)
	{
		if (m_Sinks.insert(std::pair<CClass *, CEventNotifier *>(new CEventNotifier(notifyee, fn, this))).second)
		{
#ifdef _DEBUG
			throw "advancedfx::Model:CEventSource must have only one notification function per notifyee!";
#endif
		}
	}

	void RemoveNotifier(CNotifierBase * notifier)
	{
		m_Sinks.find(notifyee)->second->Delete();
	}


	void Notify()
	{
		for (m_SinksIterator = m_Sinks.begin(); m_SinksIterator != m_Sinks.end(); )
		{
			(*m_SinksIterator)->Notify();
			if (m_RemovedCurrent)
			{
				m_RemovedCurrent = false;
			}
			else
			{
				++m_SinksIterator;
			}
		}
	}

	Source * GetSource()
	{
		return m_Source;
	}

protected:
	virtual void OnDeleting(CClass * cause)
	{
		while (!m_Sinks.empty())
		{
			(*m_Sinks.begin())->Delete();
		}

		CClass::OnDeleting(cause);
	}

private:
	Source * m_Source;
	std::set<CNotifierBase *> m_Sinks;
	typename std::set<CNotifierBase *>::iterator m_SinksIterator;
	bool m_RemovedCurrent = false;

	void OnNotifierDeleting(CNotifierBase * subject)
	{
		std::set<CNotifierBase *>::iterator it = m_Sinks.find(subject);

		if (it == m_SinksIterator)
		{
			m_RemovedCurrent = true;
			++m_SinksIterator;
		}

		m_Sinks.erase(it);
	}
};


} // namespace Model {
} // namespace AfxHookSource {
