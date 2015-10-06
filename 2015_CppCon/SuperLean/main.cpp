#include "stdafx.h"
#include "main.h"

// uncomment one
//#define WHAT_TO_RUN traditional
//#define WHAT_TO_RUN awaitable
#define WHAT_TO_RUN improved

int main()
{
	auto nReaders = int(1);
	bool sync = 0;
	int64_t bytes = 1'000'000'000;

	printf("%I64dM ", bytes / 1'000'000);

	WHAT_TO_RUN::run(nReaders, bytes, sync);
}
