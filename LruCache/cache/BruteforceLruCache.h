#ifndef __BRUTEFORCE_LRU_CACHE_HEADER__
#define __BRUTEFORCE_LRU_CACHE_HEADER__


#include "ICacheHandle.h"
#include "Policy.h"
#include <list>
#include <unordered_map>
#include <iostream>


namespace cache
{


template<typename Key, typename Item>
using BruteforceLruCacheListItemType = std::pair<const Key, Item>;

template<typename Key, typename Item>
using BruteforceLruCacheMapItemType = std::pair<const Key, typename std::list<std::pair<const Key, Item>>::iterator>;



template<typename Key, typename Item>
struct DefaultBruteforceLruCacheItemSizeCalc
{
	size_t operator() (const Key& key, const Item& item) const 
	{ 
		return 1; 
	}
};

template<typename Key, typename Item>
struct DefaultBruteforceLruCacheItemMemSizeCalc
{
	size_t operator() (const Key& key, const Item& item) const 
	{ 
		return sizeof(BruteforceLruCacheListItemType<Key, Item>) + sizeof(BruteforceLruCacheMapItemType<Key, Item>);
	}
};



template<
	typename Key, 
	typename Item,
	typename CheckPolicy, 
	typename ThreadPolicy,
	typename ItemSizeCalc = DefaultBruteforceLruCacheItemSizeCalc<Key, Item>,
	typename ListAlloc  = std::allocator<cache::BruteforceLruCacheListItemType<Key, Item>>,
	typename MapAlloc   = std::allocator<cache::BruteforceLruCacheMapItemType<Key, Item>>,
	typename KeyHasher  = std::hash<Key>,
	typename KeyEqualTo = std::equal_to<Key>
>
class BruteforceLruCache : public ICacheHandle<typename Key, typename Item>
{
public:
	using BaseType = ICacheHandle<Key, Item>;
	using List     = std::list<BruteforceLruCacheListItemType<Key, Item>, ListAlloc>;
	using ListIter = typename List::iterator;
	using Map	   = std::unordered_map<Key, ListIter, KeyHasher, KeyEqualTo, MapAlloc>;
	using MapIter  = typename Map::iterator;
	using Lock	   = typename ThreadPolicy::Lock;

public:
	BruteforceLruCache(size_t maxSize)
		: BaseType()
		, m_checkPolicy()
		, m_threadPolicy()
		, m_list()
		, m_map()
		, m_maxSize(maxSize)
		, m_curSize(0)
		, m_itemSizeCalc()
	{
		m_checkPolicy.Init();
	}
	~BruteforceLruCache()
	{
		m_checkPolicy.Check();
	}

public:
	virtual bool	Exists(Key const& key) const override;
	virtual bool	Get(Key const& key, Item& out_data) const override;
	virtual bool	Put(Key const& key, Item const& val) override;
	virtual bool	Put(Key const& key, Item&& val) override;
	virtual void	Remove(Key const& key) override;
	virtual void	Clear() override;
	virtual size_t	GetMaxSize() const override;
	virtual size_t	GetCurSize() const override;

private:
	CheckPolicy			 m_checkPolicy;
	mutable ThreadPolicy m_threadPolicy;
	mutable List		 m_list;
	mutable Map			 m_map;
	size_t				 m_maxSize;
	size_t				 m_curSize;
	ItemSizeCalc		 m_itemSizeCalc;
};




template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline bool BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Exists(Key const& key) const
{
	if (!m_checkPolicy.Check())
	{
		return false;
	}

	Lock _lock(m_threadPolicy);

	return (m_map.end() != m_map.find(key));
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline bool BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Get(Key const& key, Item& out_data) const
{
	if (!m_checkPolicy.Check())
	{
		return false;
	}

	Lock _lock(m_threadPolicy);

	MapIter mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return false;
	}

	ListIter listIter = mapIter->second;
	out_data = listIter->second;
	m_list.splice(m_list.begin(), m_list, listIter);
	return true;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline bool BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Put(Key const& key, Item const& val)
{
	if (!m_checkPolicy.Check())
	{
		return false;
	}

	size_t itemSize = m_itemSizeCalc(key, val);

	Lock _lock(m_threadPolicy);

	if (m_maxSize < (m_curSize + itemSize))
	{
		return false;
	}

	MapIter mapIter = m_map.find(key);
	if (m_map.end() != mapIter)
	{
		return false;
	}

	m_list.push_front(std::make_pair(key, val));
	std::pair<MapIter, bool> result = m_map.insert(std::make_pair(key, m_list.begin()));
	if (false == result.second)
	{
		m_list.pop_front();
		return false;
	}

	m_curSize += itemSize;
	return true;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline bool BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Put(Key const& key, Item&& val)
{
	if (!m_checkPolicy.Check())
	{
		return false;
	}

	size_t itemSize = m_itemSizeCalc(key, val);

	Lock _lock(m_threadPolicy);

	if (m_maxSize < (m_curSize + itemSize))
	{
		return false;
	}

	MapIter mapIter = m_map.find(key);
	if (m_map.end() != mapIter)
	{
		return false;
	}

	m_list.push_front(std::make_pair(key, std::move(val)));
	std::pair<MapIter, bool> result = m_map.insert(std::make_pair(key, m_list.begin()));
	if (false == result.second)
	{
		m_list.pop_front();
		return false;
	}

	m_curSize += itemSize;
	return true;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline void BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Remove(Key const& key)
{
	if (!m_checkPolicy.Check())
	{
		return;
	}

	Lock _lock(m_threadPolicy);

	MapIter mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return;
	}

	ListIter listIter = mapIter->second;
	const Item& item = listIter->second;
	size_t itemSize = m_itemSizeCalc(key, item);
	m_list.erase(listIter);
	m_map.erase(key);
	m_curSize -= itemSize;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline void BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::Clear()
{
	if (!m_checkPolicy.Check())
	{
		return;
	}

	Lock _lock(m_threadPolicy);

	m_list.clear();
	m_map.clear();
	m_curSize = 0;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline size_t BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::GetMaxSize() const
{
	if (!m_checkPolicy.Check())
	{
		return 0;
	}
	return m_maxSize;
}

template<typename Key, typename Item, typename CheckPolicy, typename ThreadPolicy, typename ItemSizeCalc, typename ListAlloc, typename MapAlloc, typename KeyHasher, typename KeyEqualTo>
inline size_t BruteforceLruCache<Key, Item, CheckPolicy, ThreadPolicy, ItemSizeCalc, ListAlloc, MapAlloc, KeyHasher, KeyEqualTo>::GetCurSize() const
{
	if (!m_checkPolicy.Check())
	{
		return 0;
	}
	return m_curSize;
}


}


#endif // !__BRUTEFORCE_LRU_CACHE_HEADER__