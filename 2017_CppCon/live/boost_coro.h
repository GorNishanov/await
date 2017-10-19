#ifndef CORO_BOOST_FUTURE
#define CORO_BOOST_FUTURE

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION // Enables future::then
#define BOOST_THREAD_PROVIDES_EXECUTORS
#include <boost/thread.hpp>
#include <experimental/coroutine>
#include <stdio.h>

template <typename... Args>
struct std::experimental::coroutine_traits<boost::future<void>, Args...> {
  struct promise_type {
    boost::promise<void> p;
    auto get_return_object() { return p.get_future(); }
    std::experimental::suspend_never initial_suspend() { return {}; }
    std::experimental::suspend_never final_suspend() { return {}; }
    void set_exception(std::exception_ptr e) { p.set_exception(std::move(e)); }
    void unhandled_exception() { p.set_exception(std::current_exception()); }
    void return_void() { p.set_value(); }
  };
};

template <typename R, typename... Args>
struct std::experimental::coroutine_traits<boost::future<R>, Args...> {
  struct promise_type {
    boost::promise<R> p;
    auto get_return_object() { return p.get_future(); }
    std::experimental::suspend_never initial_suspend() { return {}; }
    std::experimental::suspend_never final_suspend() { return {}; }
    void set_exception(std::exception_ptr e) { p.set_exception(std::move(e)); }
    void unhandled_exception() { p.set_exception(std::current_exception()); }
    template <typename U> void return_value(U &&u) {
      p.set_value(std::forward<U>(u));
    }
  };
};

struct Myexec {
  template <typename F> void submit(F &&f) { f(); }
  bool try_executing_one() { return true; }
  bool closed() { return false; }
  void close() {}
};

template <typename R> auto operator co_await(boost::future<R> &&f) {
  struct Awaiter {
    boost::future<R> &&input;
    boost::future<R> output;
    Myexec ex;
    bool await_ready() {
      if (input.is_ready()) {
        output = std::move(input);
        return true;
      }
      return false;
    }
    auto await_resume() { return output.get(); }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      input.then(ex, [this, coro](auto result_future) mutable {
        this->output = std::move(result_future);
        coro.resume();
      });
    }
  };
  return Awaiter{static_cast<boost::future<R> &&>(f)};
}

#endif