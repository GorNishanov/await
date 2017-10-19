#ifndef AWAIT_ADAPTERS
# define AWAIT_ADAPTERS

#include <experimental/timer>
#include "handler_allocator.hpp"
#include <algorithm>
#include <experimental/net>
#include <optional>

template <typename AsyncStream, typename BufferSequence>
auto async_write(AsyncStream& s, BufferSequence const& buffers) {
  struct [[nodiscard]] Awaiter {
    AsyncStream& s;
    BufferSequence const& buffers;
    handler_allocator alloc;

    bool await_ready() { return false; }
    size_t await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      async_write(s, buffers,
        make_custom_alloc_handler(alloc,
          [this, coro](auto ec, auto n) mutable {
            this->n = n;
            this->ec = ec;
            coro.resume();
          }));
    }

    size_t n;
    std::error_code ec;
  };
  return Awaiter{s, buffers};
}

template <typename AsyncStream, typename BufferSequence>
auto async_read_some(AsyncStream& s, BufferSequence const& buffers) {
  struct [[nodiscard]] Awaiter {
    AsyncStream& s;
    BufferSequence const& buffers;
    handler_allocator alloc;

    bool await_ready() { return false; }
    size_t await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      s.async_read_some(buffers,
        make_custom_alloc_handler(alloc,
          [this, coro](auto ec, auto n) mutable {
            this->n = n;
            this->ec = ec;
            coro.resume();
          }));
    }

    size_t n;
    std::error_code ec;
  };
  return Awaiter{s, buffers};
}

template <typename AcceptorSocket>
auto async_accept(AcceptorSocket& s) {
  struct [[nodiscard]] Awaiter {
    AcceptorSocket& s;

    bool await_ready() { return false; }
    auto await_resume() {
      if (ec) throw std::system_error(ec);
      return std::move(*result);
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      s.async_accept(
          [this, coro](auto ec, auto result) mutable {
            this->result = std::move(result);
            this->ec = ec;
            coro.resume();
          });
    }

    std::optional<std::experimental::net::ip::tcp::socket> result;
    std::error_code ec;
  };
  return Awaiter{s};
}

template <typename Clock, typename R, typename P>
auto async_wait(std::experimental::net::basic_waitable_timer<Clock> &t,
                 std::chrono::duration<R, P> d) {
  struct Awaiter {
    std::experimental::net::basic_waitable_timer<Clock> &t;
    std::chrono::duration<R, P> d;
    std::error_code ec;
    bool await_ready() { return d.count() == 0; }
    void await_resume() {
      if (ec)
        throw std::system_error(ec);
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      t.expires_after(d);
      t.async_wait([this, coro](auto ec) mutable {this->ec = ec; coro.resume();});
    }
  };
  return Awaiter{ t, d };
}

#endif