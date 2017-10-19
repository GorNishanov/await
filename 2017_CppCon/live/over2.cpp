#include "use_boost_future.h"
#include <stdio.h>
#include <experimental/executor>

using namespace std::experimental::net;
using namespace std::experimental;

template <typename Executor>
auto post(Executor& ex) {
  struct Awaitable {
    Executor& ex;

    bool await_ready() { return false; }
    void await_resume() {}
    void await_suspend(coroutine_handle<> h) {
      post(ex, [h]() mutable {h.resume();});
    }
  };
  return Awaitable{ex};
}

boost::future<void> repost(io_context &io, int n) {
  auto ex = io.get_executor();
  while (n-- > 0)
    co_await post(ex);
}

int main() {
  io_context io;
  repost(io, 1'000'000);
  io.run();
};
