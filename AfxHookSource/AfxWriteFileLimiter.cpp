#include "stdafx.h"

#include "AfxWriteFileLimiter.h"

const int CAfxWriteFileLimiter::m_MaxInScope = 2;
std::mutex CAfxWriteFileLimiter::m_WriteFileMutex;
std::condition_variable CAfxWriteFileLimiter::m_WriteFileCondition;
int CAfxWriteFileLimiter::m_WriteFileCount = 0;


void CAfxWriteFileLimiter::EnterScope(void)
{
	std::unique_lock<std::mutex> lock(m_WriteFileMutex);

	m_WriteFileCondition.wait(lock, []() { return m_WriteFileCount < m_MaxInScope; });

	++m_WriteFileCount;
}

void CAfxWriteFileLimiter::ExitScope(void)
{
	{
		std::unique_lock<std::mutex> lock(m_WriteFileMutex);
		--m_WriteFileCount;
	}

	m_WriteFileCondition.notify_one();
}

CAfxWriteFileLimiterScope::CAfxWriteFileLimiterScope()
{
	CAfxWriteFileLimiter::EnterScope();
}

CAfxWriteFileLimiterScope::~CAfxWriteFileLimiterScope()
{
	CAfxWriteFileLimiter::ExitScope();
}