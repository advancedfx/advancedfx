#pragma once

#include "Task.h"
#include "../Properties.h"
#include "../Events.h"

namespace advancedfx {
namespace Model {
namespace Tasks {


class CRecord : public CTask
{
public:
	class CTimePoint
	{
	public:
		enum EMode
		{
			EMode_Tick,
			EMode_Time
		};

		class CTick : public CProperty<int>
		{
		public:

		};

		class CTime : public CProperty<double>
		{
		public:

		};

		class CMode : public CProperty<EMode>
		{
		public:

		};

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

	class CDemo : public CString
	{
	public:

	};

	virtual EType GetType() const
	{
		return EType_Record;
	}

	CDemo * GetDemo()
	{
		return &m_Demo;
	}

	CTimePoint * GetBegin()
	{
		return &m_Begin;
	}

	CTimePoint * GetEnd()
	{
		return &m_End;
	}

private:
	CDemo m_Demo;
	CTimePoint m_Begin;
	CTimePoint m_End;
};

} // namespace Tasks {
} // namespace Model {
} // namespace advancedfx {
