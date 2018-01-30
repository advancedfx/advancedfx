#pragma once

#include "../Properties.h"
#include "../Model.h"

namespace AfxHookSource {
namespace Model {
namespace Tasks {

class CRecord;

class CTask
	: public CClass
{
public:
	CTask(CClass * parent,
		const char * title, const char * description)
		: CClass(parent)
		, m_Title(this, title)
		, m_Description(this, description)
	{
	}

	virtual EType GetType() const
	{
		return EType_Task;
	}

	CStringProperty * GetTitle()
	{
		return &m_Title;
	}

	CStringProperty * GetDescription()
	{
		return &m_Description;
	}

private:
	CStringProperty m_Title;
	CStringProperty m_Description;
};


} // namespace Tasks {
} // namespace Model {
} // namespace AfxHookSource {
