#include <iostream>
#include "cache/ICache.h"
#include "cache/BruteforceLruCache.h"


using LruCache_t = cache::lru::bruteforce::Cache<int, int>;
using CacheHandle_t = ICache<int, int>;
using ReadOnlyCacheHandle_t = IReadOnlyCache<int, int>;


int main(int argc, char** argv)
{
	std::unique_ptr<LruCache_t> pCache = std::make_unique<LruCache_t>(4);
	CacheHandle_t* pCacheHandle = pCache.get();
	ReadOnlyCacheHandle_t* pReadOnlyCacheHandle = pCache.get();

	pCacheHandle->Insert(8, 8);
	pCacheHandle->Insert(7, std::move(7));
	pCacheHandle->Insert(9, 9);
	pCacheHandle->Insert(5, 5);
	auto result0 = pCacheHandle->GetItem(7);
	auto result1 = pCacheHandle->GetItem(1);
	std::cout << (pReadOnlyCacheHandle->IsContained(7)) << " : "
		<< (pCacheHandle->GetItem(7) == pReadOnlyCacheHandle->GetItem(7)) << std::endl;
	pCacheHandle->Insert(1, 1);
	auto result2 = pCacheHandle->GetItem(1);
	std::cout << (pReadOnlyCacheHandle->IsContained(8)) << " : "
		<< (pCacheHandle->GetItem(8) == pReadOnlyCacheHandle->GetItem(8)) << std::endl;
	pCacheHandle->Insert(1, 11);
	auto result3 = pCacheHandle->GetItem(1);
	pCacheHandle->Insert(2, 2);
	pCacheHandle->Remove(2);

	pCacheHandle->ClearAll();
	pCacheHandle->Insert(8, 8);
	pCacheHandle->Insert(7, std::move(7));
	pCacheHandle->Insert(9, 9);
	pCacheHandle->Insert(5, 5);
	result0 = pCacheHandle->GetItem(7);
	result1 = pCacheHandle->GetItem(1);
	pCacheHandle->Insert(1, 1);
	result2 = pCacheHandle->GetItem(1);
	pCacheHandle->Insert(1, 11);
	result3 = pCacheHandle->GetItem(1);
	pCacheHandle->Insert(2, 2);
	pCacheHandle->Remove(2);


	return 0;
}