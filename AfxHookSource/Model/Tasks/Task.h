#pragma once

#include "../Properties.h"

namespace advancedfx {
namespace Model {
namespace Tasks {

class CRecord;

class CTask
{
public:
	enum EType
	{
		EType_Task,
		EType_Record
	};

	virtual EType GetType() const
	{
		return EType_Task;
	}

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


} // namespace Tasks {
} // namespace Model {
} // namespace advancedfx {
