#ifndef __LUR_CACHE_H__
#define __LUR_CACHE_H__


#include "ICache.h"
#include <utility>
#include <list>
#include <unordered_map>
#include <assert.h>


template<typename Ky, typename Ty>
class LruCache : public ICache<Ky, Ty>
{
private:
	using Key_t = Ky;
	using Value_t = Ty;
	using Pair_t = std::pair<Key_t, Value_t>;

	unsigned int m_maxNum;
	std::list<Pair_t> m_itemList;
	std::unordered_map<Key_t, decltype(m_itemList.begin())> m_itemMap;

private:
	void DeleteOld()
	{
		while ((0 < m_maxNum) && (m_itemMap.size() >= m_maxNum))
		{
			auto endItor = m_itemList.end();
			endItor--;
			m_itemMap.erase(endItor->first);
			m_itemList.pop_back();
		}
	}

public:
	LruCache()
		: LruCache(200)
	{}
	LruCache(const unsigned int& maxNum)
		: m_maxNum(maxNum)
	{}

	virtual ~LruCache()
	{
		ClearAll();
	}

	virtual bool Exists(Ky const& key) const override
	{
		return (m_itemMap.count(key) > 0);
	}

	virtual Ty& Get(Ky const& key) override
	{
		auto itor = m_itemMap.find(key);
		assert(m_itemMap.end() != itor);
		m_itemList.splice(m_itemList.begin(), m_itemList, itor->second);
		return itor->second->second;
	}

	virtual bool Put(Ky const& key, Ty const& val) override
	{
		if (m_itemMap.size() >= m_maxNum)
		{
			DeleteOld();
		}
		if (m_itemMap.size() >= m_maxNum)
		{
			return false;
		}

		m_itemList.push_front(std::make_pair(key, val));
		m_itemMap.insert(std::make_pair(key, m_itemList.begin()));

		return true;
	}

	virtual void Remove(Ky const& key) override
	{
		auto mapItem = m_itemMap.find(key);
		if (m_itemMap.end() == mapItem)
		{
			return;
		}
		m_itemList.erase(mapItem->second);
		m_itemMap.erase(key);
	}

	virtual void ClearAll() override
	{
		m_itemList.clear();
		m_itemMap.clear();
	}
};


#endif // !__LUR_CACHE_H__