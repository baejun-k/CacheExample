#ifndef __INTERFACE_CACHE_HANDLE_H__
#define __INTERFACE_CACHE_HANDLE_H__


#include <optional>


template<typename TYPE>
using __IsCopyOrMoveConstructible_t = std::enable_if_t<
	(std::is_copy_constructible_v<TYPE> ||
	 std::is_move_constructible_v<TYPE>),
	bool>;


template<typename Key, typename Item, __IsCopyOrMoveConstructible_t<Item> = true>
class IReadOnlyCache
{
protected:
	IReadOnlyCache() {}
	virtual ~IReadOnlyCache() {}

public:
	virtual size_t GetCapacity() const = 0;
	virtual size_t GetSize() const = 0;
	virtual bool IsContained(Key const& key) const = 0;
	virtual std::optional<std::reference_wrapper<Item>> GetItem(Key const& key) const = 0;
};


template<typename Key, typename Item, __IsCopyOrMoveConstructible_t<Item> = true>
class ICache : public IReadOnlyCache<Key, Item>
{
protected:
	ICache() : IReadOnlyCache<Key, Item>() {}

public:
	virtual ~ICache() {}

	virtual bool Insert(Key const& key, Item&& item) = 0;
	virtual bool Insert(Key const& key, Item& item) = 0;
	virtual void Remove(Key const& key) = 0;
	virtual void ClearAll() = 0;

};


#endif // !__INTERFACE_CACHE_HANDLE_H__