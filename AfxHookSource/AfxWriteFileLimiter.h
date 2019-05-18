#pragma once

#include <mutex>
#include <condition_variable>

class CAfxWriteFileLimiter
{
public:
	static void EnterScope(void);

	static void ExitScope(void);

private:
	static const int m_MaxInScope;
	static std::mutex m_WriteFileMutex;
	static std::condition_variable m_WriteFileCondition;
	static int m_WriteFileCount;
};

class CAfxWriteFileLimiterScope
{
public:
	CAfxWriteFileLimiterScope();
	~CAfxWriteFileLimiterScope();
};