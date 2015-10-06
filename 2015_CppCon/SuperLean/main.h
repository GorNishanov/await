#pragma once
#include <cstdint>

namespace improved
{
	void run(int nReaders, uint64_t bytes, bool sync);
}

namespace traditional
{
	void run(int nReaders, uint64_t bytes, bool sync);
}

namespace awaitable
{
	void run(int nReaders, uint64_t bytes, bool sync);
}

