#pragma once

#include "Tasks/Task.h"
#include "Tasks/Record.h"
#include "Properties.h"
#include "Events.h"

namespace AfxHookSource {
namespace Model {


class CSchedule
	: public CClass
{
public:
	CSchedule(CClass * memberOf,
		const char * title, const char * description)
		: CClass(memberOf)
		, m_Title(this, title)
		, m_Description(this, description)
	{

	}

	CStringProperty * GetTitle()
	{
		return &m_Title;
	}

	CStringProperty * GetDescription()
	{
		return &m_Description;
	}

	const std::set<Tasks::CTask *> & GetTasks()
	{
		return m_Tasks;
	}

	Tasks::CRecord * AddRecord(
		const char * title, const char * description,
		const char * demo,
		int beginTick, float beginTime, CDemoTime::EMode beginMode,
		int endTick, float endTime, CDemoTime::EMode endMode)
	{
		Tasks::CRecord * record = new Tasks::CRecord(this,
			title, description,
			demo,
			beginTick, beginTime, beginMode,
			endTick, endTime, endMode);

		m_Tasks.insert(record);

		new CDeletedNotifier<CSchedule,Tasks::CTask>(this, &CSchedule::OnTaskDeleting, record);

		return record;
	}

private:
	CStringProperty m_Title;
	CStringProperty m_Description;
	std::set<Tasks::CTask *> m_Tasks;

	void OnTaskDeleting(Tasks::CTask * source, CClass * cause)
	{
		m_Tasks.erase(source);
	}
};


} // namespace Model {
} // namespace AfxHookSource {
