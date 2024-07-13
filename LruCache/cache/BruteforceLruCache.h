#ifndef __BRUTEFORCE_LRU_CACHE_HEADER__
#define __BRUTEFORCE_LRU_CACHE_HEADER__


#include "ICache.h"
#include "Policy.h"
#include <list>
#include <unordered_map>
#include <functional>


namespace cache
{
namespace lru
{
namespace bruteforce
{


template<typename Key, typename Item>
using ListItemType = std::pair<const Key, Item>;

template<typename Key, typename Item>
using MapItemType = std::pair<const Key, typename std::list<ListItemType<Key, Item>>::iterator>;

template<typename Key, typename Item>
struct DefaultItemCalc
{
	size_t operator() (const Key& key, const Item& item) const
	{
		return 1;
	}
};

template<
	typename Key,
	typename Item,
	typename KeyHasher = std::hash<Key>,
	typename KeyEqualTo = std::equal_to<Key>,
	typename ThreadPolicy = policy::SingleThreadPolicy,
	template<typename> class Alloc = std::allocator
>
class CacheImpl : public ICache<typename Key, typename Item>
{
public:
	using Base_t = ICache<Key, Item>;
	using List_t = std::list<ListItemType<Key, Item>, Alloc<ListItemType<Key, Item>>>;
	using ListIter_t = typename List_t::iterator;
	using Map_t = std::unordered_map<Key, ListIter_t, KeyHasher, KeyEqualTo, Alloc<MapItemType<Key, Item>>>;
	using MapIter_t = typename Map_t::iterator;
	using Lock_t = typename ThreadPolicy::Lock;

public:
	CacheImpl(size_t maxSize, std::function<size_t(const Key&, const Item&)> itemSzCalc = DefaultItemCalc<Key, Item>{});
	virtual ~CacheImpl();

public:
	virtual bool Contain(Key const& key) const override;
	virtual std::optional<Item> Get(Key const& key) const override;
	virtual bool Put(Key const& key, Item const& val) override;
	virtual bool Put(Key const& key, Item&& val) override;
	virtual void Remove(Key const& key) override;
	virtual void Clear() override;
	virtual size_t GetMaxSize() const override;
	virtual size_t GetCurSize() const override;

private:
	void RemoveWithoutLock(Key const& key);

private:
	mutable ThreadPolicy m_threadPolicy;
	mutable List_t m_list;
	mutable Map_t m_map;
	size_t m_curSize;
	const size_t m_itemMaxSize;
	std::function<size_t(const Key&, const Item&)> m_itemSizeCalc;
};





template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::CacheImpl(size_t maxSize, std::function<size_t(const Key&, const Item&)> itemSzCalc)
	: Base_t()
	, m_threadPolicy()
	, m_list()
	, m_map()
	, m_curSize(0u)
	, m_itemMaxSize(maxSize)
	, m_itemSizeCalc(itemSzCalc)
{}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::~CacheImpl()
{
	Lock_t _lock(m_threadPolicy);
	this->Clear();
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Contain(Key const& key) const
{
	Lock_t _lock(m_threadPolicy);

	bool result = (m_map.cend() != m_map.find(key));
	return result;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline std::optional<Item> CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Get(Key const& key) const
{
	Lock_t _lock(m_threadPolicy);

	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return std::nullopt;
	}

	ListIter_t listIter = mapIter->second;
	m_list.splice(m_list.begin(), m_list, listIter);
	return listIter->second;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Put(Key const& key, Item const& val)
{
	const size_t itemSize = m_itemSizeCalc(key, val);

	Lock_t _lock(m_threadPolicy);

	this->RemoveWithoutLock(key);

	while (m_itemMaxSize < (m_curSize + itemSize))
	{
		if (m_list.empty())
		{
			break;
		}
		const Key& _backKey = m_list.back().first;
		this->RemoveWithoutLock(_backKey);
	}

	if (m_itemMaxSize < (m_curSize + itemSize))
	{
		return false;
	}

	m_list.push_front(std::move(std::make_pair(key, val)));
	std::pair<MapIter_t, bool> result = m_map.insert(std::make_pair(key, m_list.begin()));
	if (false == result.second)
	{
		m_list.pop_front();
		return false;
	}

	m_curSize += itemSize;
	return true;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Put(Key const& key, Item&& val)
{
	const size_t itemSize = m_itemSizeCalc(key, val);

	Lock_t _lock(m_threadPolicy);

	this->RemoveWithoutLock(key);

	while (m_itemMaxSize < (m_curSize + itemSize))
	{
		if (m_list.empty())
		{
			break;
		}
		const Key& _backKey = m_list.back().first;
		this->RemoveWithoutLock(_backKey);
	}

	if (m_itemMaxSize < (m_curSize + itemSize))
	{
		return false;
	}

	m_list.push_front(std::move(std::make_pair(key, std::move(val))));
	std::pair<MapIter_t, bool> result = m_map.insert(std::make_pair(key, m_list.begin()));
	if (false == result.second)
	{
		m_list.pop_front();
		return false;
	}

	m_curSize += itemSize;
	return true;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline void CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Remove(Key const& key)
{
	Lock_t _lock(m_threadPolicy);
	
	this->RemoveWithoutLock(key);
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline void CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Clear()
{
	Lock_t _lock(m_threadPolicy);

	m_map.clear();
	m_list.clear();
	m_curSize = 0u;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline size_t CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::GetMaxSize() const
{
	return m_itemMaxSize;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline size_t CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::GetCurSize() const
{
	return m_curSize;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline void CacheImpl<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::RemoveWithoutLock(Key const& key)
{
	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return;
	}

	ListIter_t listIter = mapIter->second;
	const Item& item = listIter->second;
	const size_t itemSize = m_itemSizeCalc(key, item);
	m_map.erase(mapIter);
	m_list.erase(listIter);
	m_curSize -= itemSize;
}


}	// bruteforce
}	// lru
}	// !cache


#endif // !__BRUTEFORCE_LRU_CACHE_HEADER__