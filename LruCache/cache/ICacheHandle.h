#ifndef __INTERFACE_CACHE_HANDLE_H__
#define __INTERFACE_CACHE_HANDLE_H__


namespace cache
{

template<typename Key, typename Type>
class ICacheHandle
{
public:
	virtual ~ICacheHandle() {}

	virtual bool	Exists(Key const& key) const = 0;
	virtual bool	Get(Key const& key, Type& out_data) const = 0;
	virtual bool	Put(Key const& key, Type const& val) = 0;
	virtual bool	Put(Key const& key, Type&& val) = 0;
	virtual void	Remove(Key const& key) = 0;
	virtual void	Clear() = 0;
	virtual size_t	GetMaxSize() const = 0;
	virtual size_t	GetCurSize() const = 0;
};

}


#endif // !__INTERFACE_CACHE_HANDLE_H__