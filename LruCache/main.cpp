#include "LruCache.h"
#include <iostream>
#include <memory>


int main(int argc, char** argv)
{
	std::unique_ptr<ICache<int, double>> pCache = std::make_unique<LruCache<int, double>>(3);

	pCache->Put(1, 1.0);
	pCache->Put(2, 2.0);
	pCache->Put(3, 3.0);
	pCache->Put(4, 4.0);
	pCache->Put(5, 5.0);

	for (int k = 0; k < 7; ++k)
	{
		std::cout << k << " : ";
		if (pCache->Exists(k))
		{
			if (3 == k)
			{
				double& v = pCache->Get(k);
				v = 6.0;
			}
			std::cout << pCache->Get(k) << std::endl;
		}
		else
		{
			std::cout << "not found" << std::endl;
		}
	}
	std::cout << std::endl;
	
	pCache->Remove(3);
	for (int k = 0; k < 7; ++k)
	{
		std::cout << k << " : ";
		if (pCache->Exists(k))
		{
			std::cout << pCache->Get(k) << std::endl;
		}
		else
		{
			std::cout << "not found" << std::endl;
		}
	}

	return 0;
}