#ifndef FUTURE_ADAPTER_H
#define FUTURE_ADAPTER_H

#include <future>
#include <experimental/coroutine>

template <typename... Args>
struct std::experimental::coroutine_traits<std::future<void>, Args...> {
  struct promise_type {
    std::promise<void> p;
    auto get_return_object() { return p.get_future(); }
    std::experimental::suspend_never initial_suspend() { return {}; }
    std::experimental::suspend_never final_suspend() { return {}; }
    void set_exception(std::exception_ptr e) { p.set_exception(std::move(e)); }
    void unhandled_exception() { p.set_exception(std::current_exception()); }
    void return_void() { p.set_value(); }
  };
};

template <typename R, typename... Args>
struct std::experimental::coroutine_traits<std::future<R>, Args...> {
  struct promise_type {
    std::promise<R> p;
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

#endif