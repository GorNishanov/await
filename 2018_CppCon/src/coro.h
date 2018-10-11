#pragma once
#include <vector>
#include <stdio.h>

#include "coro_infra.h"

template <typename Iterator, typename Found, typename NotFound>
root_task CoroBinarySearch(Iterator first, Iterator last, int val,
                          Found on_found, NotFound on_not_found) {
  auto len = last - first;
  while (len > 0) {
    auto half = len / 2;
    auto middle = first + half;
    auto x = co_await prefetch(*middle);
    if (x < val) {
      first = middle;
      ++first;
      len = len - half - 1;
    } else
      len = half;
    if (x == val)
      co_return on_found(middle);
  }
  on_not_found();
}

long CoroMultiLookup(
  std::vector<int> const& v, std::vector<int> const& lookups, int streams) {

  size_t found_count = 0;
  size_t not_found_count = 0;

  throttler t(streams);

  for (auto key: lookups)
    t.spawn(CoroBinarySearch(v.begin(), v.end(), key,
      [&](auto) { ++found_count; }, [&] { ++not_found_count; }));

  t.run();

  if (found_count + not_found_count != lookups.size())
    printf("BUG: found %zu, not-found: %zu total %zu\n", found_count,
           not_found_count, found_count + not_found_count);

  return found_count;
}
