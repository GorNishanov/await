#include "use_boost_future.h"
#include <stdio.h>
#include <experimental/executor>

using namespace std::experimental::net;

boost::future<void> repost(io_context &io, int n) {
  auto ex = io.get_executor();
  while (n-- > 0)
    co_await post(ex, use_boost_future);
}

int main() {
  io_context io;
  repost(io, 1'000'000);
  io.run();
};
