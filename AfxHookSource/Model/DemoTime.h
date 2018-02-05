#pragma once

#include "Properties.h"
#include "Model.h"

namespace AfxHookSource {
namespace Model {

class CDemoTime : public CClass
{
public:
	enum EMode
	{
		EMode_Tick,
		EMode_Time
	};

	typedef CIntProperty CTick;
	typedef CFloatProperty CTime;
	typedef CProperty<EMode, EType_DemoTime_Mode> CMode;

	CDemoTime(CClass * memberOf,
		int tick, float time, EMode mode)
		: CClass(memberOf)
		, m_Mode(this, mode)
		, m_Tick(this, tick)
		, m_Time(this, time)
	{

	}

	virtual EType GetType() const
	{
		return EType_DemoTime;
	}

	CTick * GetTick()
	{
		return &m_Tick;
	}

	CTime * GetTime()
	{
		return &m_Time;
	}

	CMode * GetMode()
	{
		return &m_Mode;
	}

private:
	CTick m_Tick;
	CTime m_Time;
	CMode m_Mode;
};

} // namespace Model {
} // namespace AfxHookSource {
