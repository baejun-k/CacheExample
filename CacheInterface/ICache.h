#ifndef __I_CACHE_H__
#define __I_CACHE_H__


template<typename Ky, typename Ty>
class ICache
{
public:
	ICache() {}
	virtual ~ICache() {}

	virtual bool Exists(Ky const& key) const = 0;
	virtual Ty&  Get(Ky const& key) = 0;
	virtual bool Put(Ky const& key, Ty const& val) = 0;
	virtual void Remove(Ky const& key) = 0;
	virtual void ClearAll() = 0;
};


#endif // !__I_CACHE_H__