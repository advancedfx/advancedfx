#pragma once

#include "Task.h"
#include "../Properties.h"
#include "../DemoTime.h"

namespace AfxHookSource {
namespace Model {
namespace Tasks {

class CRecord : public CTask
{
public:
	typedef CStringProperty CDemo;

	CRecord(CClass * memberOf,
		const char * title, const char * description,
		const char * demo,
		int beginTick, float beginTime, CDemoTime::EMode beginMode,
		int endTick, float endTime, CDemoTime::EMode endMode)
		: CTask(memberOf, title, description)
		, m_Demo(this, demo)
		, m_Begin(this, beginTick, beginTime, beginMode)
		, m_End(this, endTick, endTime, endMode)
	{

	}

	virtual EType GetType() const
	{
		return EType_Record;
	}

	CDemo * GetDemo()
	{
		return &m_Demo;
	}

	CDemoTime * GetBegin()
	{
		return &m_Begin;
	}

	CDemoTime * GetEnd()
	{
		return &m_End;
	}

private:
	CDemo m_Demo;
	CDemoTime m_Begin;
	CDemoTime m_End;
};

} // namespace Tasks {
} // namespace Model {
} // namespace AfxHookSource {
