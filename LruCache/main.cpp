#include <iostream>
#include "cache/ICache.h"
#include "cache/BruteforceLruCache.h"


//using CacheInitCheckPolicy = policy::CheckInitPolicy;
//using CacheThreadPolicy = policy::StdMultiThreadPolicy<policy::CheckInitPolicy>;
//using TestCache = cache::BruteforceLruCache<int, int, CacheInitCheckPolicy, CacheThreadPolicy>;


int main(int argc, char** argv)
{
	std::unique_ptr<ICache<int, int>> pCache =
		std::unique_ptr<ICache<int, int>>(new cache::lru::bruteforce::CacheImpl<int, int>(4));

	pCache->Put(8, 8);
	pCache->Put(7, std::move(7));
	pCache->Put(9, 9);
	pCache->Put(5, 5);
	auto result0 = pCache->Get(7);
	auto result1 = pCache->Get(1);
	pCache->Put(1, 1);
	auto result2 = pCache->Get(1);
	pCache->Put(1, 11);
	auto result3 = pCache->Get(1);
	pCache->Put(2, 2);
	pCache->Remove(2);

	pCache->Clear();
	pCache->Put(8, 8);
	pCache->Put(7, std::move(7));
	pCache->Put(9, 9);
	pCache->Put(5, 5);
	result0 = pCache->Get(7);
	result1 = pCache->Get(1);
	pCache->Put(1, 1);
	result2 = pCache->Get(1);
	pCache->Put(1, 11);
	result3 = pCache->Get(1);
	pCache->Put(2, 2);
	pCache->Remove(2);


	return 0;
}