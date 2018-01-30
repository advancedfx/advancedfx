#pragma once

#include <set>

#ifndef AFX_MODEL_CHECK_FOR_CYCLES
#ifdef _DEBUG
#define AFX_MODEL_CHECK_FOR_CYCLES 1
#else
#define AFX_MODEL_CHECK_FOR_CYCLES 0
#endif
#endif

namespace AfxHookSource {
namespace Model {
	
typedef const void * ObjectId;

class CClass
{
public:
	enum EType
	{
		EType_Class,
		EType_DeletedNotifier,
		EType_EventNotifier,
		EType_EventSource,
		EType_BoolProperty,
		EType_IntProperty,
		EType_FloatProperty,
		EType_DoubleProperty,
		EType_StringProperty,
		EType_DemoTime,
		EType_DemoTime_Mode,
		EType_Task,
		EType_Record
	};

	/// <param name="parent">Initial dependency, only roots can have nullptr as parent.</param>
	/// <remarks>Circular dependencies must not be created.</remarks>
	CClass(CClass * parent)
	{
		if (parent)
		{
			Depend(parent);
		}
	}

	void Delete()
	{
		Delete(this);
	}

	ObjectId GetId() const
	{
		return this;
	}

	virtual EType GetType() const
	{
		return EType_Class;
	}

	const std::set<CClass *> & GetDependencies() const
	{
		return m_Dependencies;
	}

	const std::set<CClass *> & GetDependents() const
	{
		return m_Dependents;
	}

	/// <remarks>This operation can be computationally expensive (worst case for very unbalanced graphs: graph size - 1).</remarks>
	bool DependsOn(CClass * value) const
	{
		for (std::set<CClass *>::iterator it = m_Dependencies.begin(); it != m_Dependencies.end(); ++it)
		{
			CClass * itValue = *it;

			if (value == itValue || itValue->DependsOn(value))
				return true;
		}

		return false;
	}


protected:
	~CClass()
	{
	}

	void Delete(CClass * cause)
	{
		OnDeleting(cause);

		while (!m_Dependents.empty())
		{
			(*m_Dependents.begin())->Delete(this);
		}

		while (!m_Dependencies.empty())
		{
			Undepend(*m_Dependencies.begin());
		}

		delete this;
	}

	/// <remarks>Circular dependencies must not be created.</remarks>
	void Depend(CClass * value)
	{
#if AFX_MODEL_CHECK_FOR_CYCLES
		if (value->DependsOn(this))
			throw "advancedfx::Model:CClass Dependency graph must not have cycles!";
#endif

		m_Dependencies.insert(value);
		value->AddDependent(this);
	}

	void Undepend(CClass * value)
	{
		value->RemoveDependent(this);
		m_Dependencies.erase(value);
	}

	virtual void OnDeleting(CClass * cause)
	{

	}

private:
	std::set<CClass *> m_Dependents;
	std::set<CClass *> m_Dependencies;

	void AddDependent(CClass * value)
	{
		m_Dependents.insert(value);
	}

	void RemoveDependent(CClass * value)
	{
		m_Dependents.erase(value);
	}
};

} // namespace Model {
} // namespace AfxHookSource {
