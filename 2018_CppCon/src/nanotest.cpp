#include "naive.h"
#include "sm.h"
#include "coro.h"

#include "rng.h"
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <vector>
#include <string_view>

// Test state.
struct State {
  std::vector<int> v;
  std::vector<int> lookups;
  int repeat;

  int streams;
  char const *algo_name;

  State(int ByteCount, int LookupCount, int Repeat) : repeat(Repeat) {
    auto seed1 = 0;

    int count = (ByteCount / sizeof(int));
    v.reserve(count);
    int val = 0;
    for (int i = 0; i < count; ++i)
      v.push_back(i + i);

    lookups.reserve(LookupCount);
    for (auto i : rng<int>(seed1, 0, count + count, LookupCount))
      lookups.push_back(i);

    printf("count: %d lookups: %d repeat %d\n", count, LookupCount, Repeat);
  }

  using hrc_clock = std::chrono::high_resolution_clock;
  hrc_clock::time_point start_time;

  void start(int streams, const char *algo_name) {
    this->streams = streams;
    this->algo_name = algo_name;
    start_time = hrc_clock::now();
  }

  void stop() {
    auto stop_time = hrc_clock::now();
    std::chrono::duration<double, std::nano> elapsed = stop_time - start_time;

    auto divby = log2((double)v.size());
    auto perop = elapsed.count() / divby / lookups.size() / repeat;

    printf("%g ns per lookup/log2(size)\n", perop);
  }
};

// see naive.h
static long testNaive(State &s) {
  size_t found = 0;
  auto beg = s.v.begin();
  auto end = s.v.end();
  for (int key : s.lookups)
    if (naive_binary_search(beg, end, key))
      ++found;
  return found;
}

// see sm.h
static long testSm(State &s) {
  return SmMultiLookup(s.v, s.lookups, s.streams);
}

// see coro.h
long testCoro(State& s){
  return CoroMultiLookup(s.v, s.lookups, s.streams);
}

using TestFn = long (*)(State& s);

int usage(const char* msg = nullptr) {
  puts("");
  if (msg) puts(msg);

  printf("  Usage: nanotest <algo> <size> <streams>\n\n"
          "   <algo>: naive sm coro\n"
          "   <size>: l1 l2 l3 big\n"
          "   <streams>: 1 - whatever\n\n");
  return 1;
}

struct TestParam {
  int SizeInBytes;
  int LookupSize;
  int Repeat;

  long ExpectedResult; // sanity check for bugs
};

using namespace std;

int main(int argc, const char** argv) {
  if (argc != 4)
    return usage();

  TestFn testFn = nullptr;
  if (argv[1] == "coro"sv) testFn = &testCoro;
  else if (argv[1] == "naive"sv) testFn = &testNaive;
  else if (argv[1] == "sm"sv) testFn = &testSm;
  else return usage("invalid algorithm name\n\n");

  TestParam param;
  if (argv[2] == "quick"sv)    param = TestParam{ 16*1024, 1024, 1,            505};
  else if (argv[2] == "l1"sv)  param = TestParam{ 16*1024, 1024, 10000,        5050000};
  else if (argv[2] == "l2"sv)  param = TestParam{200*1024, 1024*1024, 50,      26225050};
  else if (argv[2] == "l3"sv)  param = TestParam{6*1024*1024, 1024*1024, 50,   26215900};
  else if (argv[2] == "big"sv) param = TestParam{256*1024*1024, 1024*1024, 5,  2624940};
  else return usage("invalid size\n\n");

  auto streams = atoi(argv[3]);
  if (streams < 1)
    return usage("invalid stream count");

  State s(param.SizeInBytes, param.LookupSize, param.Repeat);

  s.start(streams, argv[1]);

  long sum = 0;
  int repeat = s.repeat;
  while (repeat-- > 0) {
    auto result = (*testFn)(s);
    sum += result;
  }
  s.stop();
  printf("sum %ld\n", sum);
  if (sum != param.ExpectedResult) {
    printf("!!!! BUG, expected %ld\n", param.ExpectedResult);
    return 1;
  }
}
