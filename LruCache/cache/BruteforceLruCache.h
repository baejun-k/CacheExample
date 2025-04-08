#ifndef __BRUTEFORCE_LRU_CACHE_HEADER__
#define __BRUTEFORCE_LRU_CACHE_HEADER__


#include "ICache.h"
#include "Policy.h"
#include <list>
#include <unordered_map>
#include <functional>
#include <assert.h>


namespace cache
{
namespace lru
{
namespace bruteforce
{

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
class Cache : public ICache<typename Key, typename Item>
{
public:
	using Base_t = ICache<Key, Item>;
	using ListItem_t = std::pair<const Key, Item>;
	using List_t = std::list<ListItem_t, Alloc<ListItem_t>>;
	using ListIter_t = typename List_t::iterator;
	using MapItem_t = std::pair<const Key, ListIter_t>;
	using Map_t = std::unordered_map<Key, ListIter_t, KeyHasher, KeyEqualTo, Alloc<MapItem_t>>;
	using MapIter_t = typename Map_t::iterator;
	using Lock_t = typename ThreadPolicy::Lock;
	using SizeCalculator_t = std::function<size_t(const Key&, const Item&)>;

public:
	Cache(const size_t& maxSize);
	Cache(const size_t& maxSize, SizeCalculator_t sizeCalculator);
	virtual ~Cache();

	size_t GetCapacity() const override;
	size_t GetSize() const override;
	bool IsContained(Key const& key) const override;
	std::optional<std::reference_wrapper<Item>> GetItem(Key const& key) const override;

	bool Insert(Key const& key, Item&& item) override;
	bool Insert(Key const& key, Item& item) override;
	void Remove(Key const& key) override;
	void ClearAll() override;

private:
	bool RemoveWithoutLock(Key const& key);

private:
	mutable ThreadPolicy m_threadPolicy;
	mutable List_t m_list;
	mutable Map_t m_map;
	const size_t m_maxSize;
	size_t m_curSize;
	SizeCalculator_t m_sizeCalculator;
};





template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Cache(const size_t& maxSize)
	: Cache(maxSize, DefaultItemCalc<Key, Item>{})
{}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Cache(const size_t& maxSize, SizeCalculator_t sizeCalculator)
	: ICache<Key, Item>()
	, m_threadPolicy()
	, m_list()
	, m_map()
	, m_maxSize(maxSize)
	, m_curSize(0)
	, m_sizeCalculator(sizeCalculator)
{}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::~Cache()
{}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline size_t Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::GetCapacity() const
{
	return m_maxSize;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline size_t Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::GetSize() const
{
	return m_curSize;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::IsContained(Key const& key) const
{
	Lock_t _lock(m_threadPolicy);
	bool result = (m_map.cend() != m_map.find(key));
	return result;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline std::optional<std::reference_wrapper<Item>> Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::GetItem(Key const& key) const
{
	Lock_t _lock(m_threadPolicy);
	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return std::nullopt;
	}

	ListIter_t listIter = mapIter->second;
	m_list.splice(m_list.begin(), m_list, listIter);
	std::reference_wrapper<Item> result = std::ref(listIter->second);
	return std::make_optional(result);
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Insert(Key const& key, Item&& item)
{
	const size_t newItemSize = m_sizeCalculator(key, item);

	Lock_t _lock(m_threadPolicy);
	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		while ((m_curSize + newItemSize) > m_maxSize)
		{
			if (m_list.empty())
			{
				return false;
			}
			ListItem_t& listItem = m_list.back();
			const Key& key = listItem.first;
			if (false == RemoveWithoutLock(key))
			{
				return false;
			}
		}
		m_list.emplace_front(std::make_pair(key, std::move(item)));
		ListIter_t listIter = m_list.begin();
		if (listIter->first != key)
		{
			return false;
		}

		std::pair<MapIter_t, bool> result = m_map.insert(std::make_pair(key, listIter));
		if (false == result.second)
		{
			m_list.pop_front();
			return false;
		}

		m_curSize += newItemSize;
	}
	else
	{
		ListIter_t listIter = mapIter->second;
		const size_t oldItemSize = m_sizeCalculator(key, listIter->second);
		m_curSize -= oldItemSize;
		m_curSize += newItemSize;
		listIter->second = std::move(item);
	}
	return true;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Insert(Key const& key, Item& item)
{
	const size_t newItemSize = m_sizeCalculator(key, item);

	Lock_t _lock(m_threadPolicy);
	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		while ((m_curSize + newItemSize) > m_maxSize)
		{
			if (m_list.empty())
			{
				return false;
			}
			ListItem_t& listItem = m_list.back();
			const Key& key = listItem.first;
			if (false == RemoveWithoutLock(key))
			{
				return false;
			}
		}
		m_list.emplace_front(std::make_pair(key, item));
		ListIter_t listIter = m_list.begin();
		if (listIter->first != key)
		{
			return false;
		}

		std::pair<MapIter_t, bool> result = m_map.insert(std::make_pair(key, listIter));
		if (false == result.second)
		{
			m_list.pop_front();
			return false;
		}

		m_curSize += newItemSize;
	}
	else
	{
		ListIter_t listIter = mapIter->second;
		const size_t oldItemSize = m_sizeCalculator(key, listIter->second);
		m_curSize -= oldItemSize;
		m_curSize += newItemSize;
		listIter->second = item;
	}

	return true;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline void Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::Remove(Key const& key)
{
	Lock_t _lock(m_threadPolicy);
	RemoveWithoutLock(key);
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline void Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::ClearAll()
{
	Lock_t _lock(m_threadPolicy);
	m_list.clear();
	m_map.clear();
	m_curSize = 0;
}

template<typename Key, typename Item, typename KeyHasher, typename KeyEqualTo, typename ThreadPolicy, template<typename> class Alloc>
inline bool Cache<Key, Item, KeyHasher, KeyEqualTo, ThreadPolicy, Alloc>::RemoveWithoutLock(Key const& key)
{
	MapIter_t mapIter = m_map.find(key);
	if (m_map.end() == mapIter)
	{
		return false;
	}

	ListIter_t listIter = mapIter->second;
	const Item& item = listIter->second;
	const size_t itemSize = m_sizeCalculator(key, item);
	m_map.erase(mapIter);
	m_list.erase(listIter);
	assert(m_curSize >= itemSize);
	if (m_curSize < itemSize)
	{
		m_curSize = 0;
	}
	else
	{
		m_curSize -= itemSize;
	}
	return true;
}


}	// !bruteforce
}	// !lru
}	// !cache


#endif // !__BRUTEFORCE_LRU_CACHE_HEADER__