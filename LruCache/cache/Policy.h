#ifndef __POLICY_HEADER__
#define __POLICY_HEADER__


#include "Assert.h"
#include <mutex>


namespace policy
{


class SingleThreadPolicy
{
public:
	SingleThreadPolicy() {}
	~SingleThreadPolicy() {}

	class Lock;
	friend class Lock;

	class Lock
	{
	public:
		Lock() = delete;
		Lock(SingleThreadPolicy&&) = delete;
		explicit Lock(SingleThreadPolicy& obj) {}
		~Lock() {}

		Lock& operator=(Lock&&) = delete;
		Lock& operator=(const Lock&) = delete;
	};
};


struct StdMultiThreadPolicy
{
private:
	bool m_isInit;
	std::mutex m_mutex;

public:
	StdMultiThreadPolicy()
		: m_isInit(false)
		, m_mutex()
	{
		m_isInit = true;
	}
	~StdMultiThreadPolicy()
	{
		ASSERT(true == m_isInit, "MultiThreadPolicy is not initialized.");
	}

	class Lock;
	friend class Lock;

	class Lock
	{
	private:
		StdMultiThreadPolicy& m_policy;
		bool m_isLocked;

	public:
		Lock() = delete;
		Lock(StdMultiThreadPolicy&&) = delete;
		explicit Lock(StdMultiThreadPolicy& policy)
			: m_policy(policy)
			, m_isLocked(false)
		{
			ASSERT(true == m_policy.m_isInit, "MultiThreadPolicy is not initialized.");
			if (false == m_policy.m_isInit)
			{
				return;
			}
			m_policy.m_mutex.lock();
			m_isLocked = true;
		}
		~Lock()
		{
			ASSERT(true == m_policy.m_isInit, "MultiThreadPolicy is not initialized.");
			if (false == m_policy.m_isInit)
			{
				return;
			}
			else if (false == m_isLocked)
			{
				return;
			}
			m_policy.m_mutex.unlock();
			m_isLocked = false;
		}

		Lock& operator=(StdMultiThreadPolicy&&) = delete;
		Lock& operator=(const StdMultiThreadPolicy&) = delete;
	};
};


}


#endif // !__POLICY_HEADER_H__