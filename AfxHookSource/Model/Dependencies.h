#pragma once

#pragma once

#include <set>

namespace advancedfx {
namespace Model {

class CDependency
{
public:
	virtual void Release(void)
	{

	}
};

class CDependable
{
public:
	CDependable()
	{
		m_DependenciesIterator = m_Dependencies.begin();
	}

	~CDependable()
	{
		for (m_DependenciesIterator = m_Dependencies.begin(); m_DependenciesIterator != m_Dependencies.end(); ++m_DependenciesIterator)
		{
			(*m_DependenciesIterator)->Release();
		}
	}

	void Add(CDependency * dependency)
	{
		m_Dependencies.insert(dependency);
	}

	void Remove(CDependency * dependency)
	{
		std::set<CDependency *>::iterator it = m_Dependencies.find(dependency);

		if (it == m_DependenciesIterator) ++m_DependenciesIterator;

		m_Dependencies.erase(it);
	}

private:
	std::set<CDependency *> m_Dependencies;
	std::set<CDependency *>::iterator m_DependenciesIterator;

};

} // namespace Model {
} // namespace advancedfx {
