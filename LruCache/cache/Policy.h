#ifndef __POLICY_HEADER__
#define __POLICY_HEADER__


#include "Assert.h"
#include <mutex>


namespace policy
{


class NoCheckInitPolicy
{
public:
	NoCheckInitPolicy() {}
	~NoCheckInitPolicy() {}

	void Init() {}
	bool Check() const { return true; }

	NoCheckInitPolicy& operator=(const NoCheckInitPolicy&) = delete;
	NoCheckInitPolicy& operator=(NoCheckInitPolicy&&) noexcept = delete;
};

class CheckInitPolicy
{ 
public:
	CheckInitPolicy()
		: m_isInit(false)
	{}
	~CheckInitPolicy() {}

	void Init() { m_isInit = true; }
	bool Check() const
	{ 
		ASSERT(true == m_isInit, "Object is not initialized.");
		return m_isInit;
	}

	CheckInitPolicy& operator=(const CheckInitPolicy&) = delete;
	CheckInitPolicy& operator=(CheckInitPolicy&&) noexcept = delete;

private:
	bool m_isInit;
};



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

template<typename CheckPolicy>
struct StdMultiThreadPolicy
{
protected:
	CheckPolicy m_checkPolicy;
	std::mutex m_mutex;

public:
	StdMultiThreadPolicy()
		: m_checkPolicy()
		, m_mutex()
	{
		m_checkPolicy.Init();
	}
	~StdMultiThreadPolicy()
	{
		m_checkPolicy.Check();
	}

	class Lock;
	friend class Lock;

	class Lock
	{
	private:
		StdMultiThreadPolicy& m_policy;

	public:
		Lock() = delete;
		Lock(StdMultiThreadPolicy&&) = delete;
		explicit Lock(StdMultiThreadPolicy& policy)
			: m_policy(policy)
		{
			if (m_policy.m_checkPolicy.Check())
			{
				m_policy.m_mutex.lock();
			}
		}
		~Lock()
		{
			if (m_policy.m_checkPolicy.Check())
			{
				m_policy.m_mutex.unlock();
			}
		}

		Lock& operator=(StdMultiThreadPolicy&&) = delete;
		Lock& operator=(const StdMultiThreadPolicy&) = delete;
	};
};


}


#endif // !__POLICY_HEADER_H__