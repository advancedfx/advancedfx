#pragma once

#include <mutex>
#include <condition_variable>

class CAfxThreadedRefCounted
{
public:
	//
	// Reference counting:

	virtual int AddRef(bool keepLocked = false)
	{
		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		if (!keepLocked) m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(bool isLocked = false)
	{
		if (!isLocked) m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (1 == result && m_LastRefCondition)
		{
			m_LastRefCondition->notify_one();
		}
		else if (0 == result)
			delete this;

		return result;
	}

	int GetRefCount()
	{
		return m_RefCount;
	}

	void Lock()
	{
		m_RefMutex.lock();
	}

	void Unlock()
	{
		m_RefMutex.unlock();
	}

	void WaitLastRefAndLock()
	{
		std::unique_lock<std::mutex> lock(m_RefMutex);

		if (1 != m_RefCount)
		{
			std::condition_variable * conditionVariable = new std::condition_variable();

			m_LastRefCondition = conditionVariable;

			conditionVariable->wait(lock, [this]() { return 1 == this->m_RefCount; });

			delete conditionVariable;
		}

		lock.release();
	}

protected:
	virtual ~CAfxThreadedRefCounted()
	{
	}

private:
	std::mutex m_RefMutex;
	std::condition_variable  * m_LastRefCondition = nullptr;
	int m_RefCount = 0;
};

class CAfxThreadedRefCountedUniqueLock
{
public:
	CAfxThreadedRefCountedUniqueLock(CAfxThreadedRefCounted * threadedRefCounted)
		: m_ThreadedRefCounted(threadedRefCounted)
	{
		if (m_ThreadedRefCounted)
			m_ThreadedRefCounted->AddRef(true);
	}

	~CAfxThreadedRefCountedUniqueLock()
	{
		if (m_ThreadedRefCounted)
			m_ThreadedRefCounted->Release(true);
	}

private:
	CAfxThreadedRefCounted * m_ThreadedRefCounted;
};
