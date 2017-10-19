#include <experimental/coroutine>
#include <stdio.h>
#include <future>

using namespace std::experimental;

template <typename R, typename... Args>
struct std::experimental::coroutine_traits<std::future<R>, Args...> {
  struct promise_type {
    std::promise<R> p;
    suspend_never initial_suspend() { return {}; }
    suspend_never final_suspend() { return {}; }
    void return_value(int v) {
      p.set_value(v);
    }
    auto get_return_object() { return p.get_future(); }
    void unhandled_exception() { p.set_exception(std::current_exception()); }
  };
};

std::future<int> f() {
  puts("Hello");
  co_return 42;
}

int main() {
  printf("%d\n", f().get());
}





























#if 0 // Start
#include <experimental/coroutine>
#include <stdio.h>

namespace stdx = std::experimental;
using namespace stdx;

int main() {
  puts("Hello");
  return 42;
}
#endif

#if 0 // End Result
#include <experimental/coroutine>
#include <stdio.h>
#include <future>

namespace stdx = std::experimental;
using namespace stdx;


template <typename R, typename... Args>
struct stdx::coroutine_traits<std::future<R>, Args...> {
  struct promise_type {
    std::promise<R> p;
    suspend_never initial_suspend() { return {}; }
    suspend_never final_suspend() { return {}; }
    void return_value(int v) {
      p.set_value(v);
    }
    auto get_return_object() { return p.get_future(); }
    void unhandled_exception() {
      p.set_exception(std::current_exception());
    }
  };
};

/// HORRIBLE. DO NOT DO THIS!!!

template <typename R>
auto operator co_await(std::future<R>&& f) {
  struct Awaiter {
    std::future<R>&& f;

    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) {
      std::thread([this, h] () mutable { f.wait(); h.resume(); }).detach();
    }
    auto await_resume() { return f.get(); }
  };
  return Awaiter{std::forward<std::future<R>>(f)};
}

std::future<int> f() {
  puts("Hello");
  co_return 42;
}

std::future<int> g() {
  co_return 1 + co_await f();
}

int main() {
  printf("%d\n", g().get());
}
#endif