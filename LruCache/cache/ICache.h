#ifndef __INTERFACE_CACHE_HANDLE_H__
#define __INTERFACE_CACHE_HANDLE_H__


#include <optional>


template<typename Key, typename Item>
class ICache
{
protected:
	ICache() {}

public:
	virtual ~ICache() {}

	virtual bool Contain(Key const& key) const = 0;
	virtual std::optional<Item> Get(Key const& key) const = 0;
	virtual bool Put(Key const& key, Item const& val) = 0;
	virtual bool Put(Key const& key, Item&& val) = 0;
	virtual void Remove(Key const& key) = 0;
	virtual void Clear() = 0;
	virtual size_t GetMaxSize() const = 0;
	virtual size_t GetCurSize() const = 0;
};


#endif // !__INTERFACE_CACHE_HANDLE_H__