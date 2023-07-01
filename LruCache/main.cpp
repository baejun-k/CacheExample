#include "cache/BruteforceLruCache.h"

using CacheInitCheckPolicy = policy::CheckInitPolicy;
using CacheThreadPolicy = policy::StdMultiThreadPolicy<policy::CheckInitPolicy>;
using TestCache = cache::BruteforceLruCache<int, int, CacheInitCheckPolicy, CacheThreadPolicy>;


int main(int argc, char** argv)
{
	TestCache cache(10);

	int _tmp = -1;
	int a = 6;
	cache.Put(8, 8);
	cache.Put(7, 7);
	cache.Put(9, 9);
	cache.Put(5, 5);
	cache.Get(7, _tmp);
	cache.Put(1, 1);
	cache.Put(2, 2);
	cache.Remove(2);

	return 0;
}