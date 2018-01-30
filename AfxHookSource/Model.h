#pragma once

#include <set>

namespace advancedfx {
namespace Model {
	
typedef const void * ObjectId;

class CClass
{
public:
	enum EType
	{
		EType_Class = 0,
	};

	CClass(CClass * owner)
		: m_Owner(owner)
	{

	}

	ObjectId GetId() const
	{
		return this;
	}

	virtual EType GetType() const
	{
		return EType_Class;
	}

	CClass * GetOwner()
	{
		return m_Owner;
	}

private:
	CClass * m_Owner;
	std::set<CClass * > m_Owned;


};


} // namespace Model {
} // namespace advancedfx {
