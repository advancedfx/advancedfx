#pragma once

#include "Tasks/Task.h"
#include "Tasks/Record.h"

namespace advancedfx {
namespace Model {


class CSchedule
{
public:
	CString * GetTitle()
	{
		return &m_Title;
	}

	CString * GetDescription()
	{
		return &m_Description;
	}

private:
	CString m_Title;
	CString m_Description;

};


} // namespace Model {
} // namespace advancedfx {
