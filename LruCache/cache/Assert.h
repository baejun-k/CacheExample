#ifndef __ASSERT_HEADER___
#define __ASSERT_HEADER___


#include <stdio.h>
#include <stdlib.h>


#define __PRINT(fmt, ...)		fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#if defined(_WIN32) && defined(_DEBUG)
#include <exception>
#define ASSERT(condition, message)	\
	if (!(condition)) \
	{ \
		class AssertException : public std::exception { \
		public: \
			AssertException() \
				: std::exception() \
			{} \
		}; \
		__PRINT("ASSERT: '%s' falied in %s:%d\n  \"%s\"\n", #condition, __FILE__, __LINE__, message); \
		throw AssertException(); \
		abort(); \
	} else int ____dummy = 0
#elif defined(_DEBUG)
#define ASSERT(condition, message)	if (!(condition)) { __PRINT("ASSERT: '%s' falied in %s:%d\n  \"%s\"\n", #condition, __FILE__, __LINE__, message); } else int ____dummy = 0
#else
#define ASSERT(condition, message)	
#endif


#endif // !__ASSERT_HEADER___