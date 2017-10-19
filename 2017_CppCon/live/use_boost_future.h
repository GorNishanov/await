#ifndef CORO_USE_BOOST_FUTURE
# define CORO_USE_BOOST_FUTURE

#include "boost_coro.h"
#include <experimental/net>

struct use_boost_future_t {};
constexpr use_boost_future_t use_boost_future;

template <>
class std::experimental::net::async_result<use_boost_future_t, void()> {
  boost::future<void> fut;
public:
  struct completion_handler_type {
    boost::promise<void> p;
    template <typename Whatever>
    completion_handler_type(Whatever const&) {}
    void operator() () { p.set_value(); }
  };
  using return_type = boost::future<void>;
  explicit async_result(completion_handler_type &h) : fut(h.p.get_future()) {}
  async_result(const async_result &) = delete;
  async_result &operator=(const async_result &) = delete;
  return_type get() {
    return std::move(fut);
  }
};

template <>
class std::experimental::net::async_result<use_boost_future_t, void(std::error_code)> {
  boost::future<void> fut;
public:
  struct completion_handler_type {
    boost::promise<void> p;
    template <typename Whatever>
    completion_handler_type(Whatever const&) {}
    void operator() (std::error_code const& ec) {
      if (ec) p.set_exception(std::system_error(ec));
      else p.set_value();
    }
  };
  using return_type = boost::future<void>;
  explicit async_result(completion_handler_type &h) : fut(h.p.get_future()) {}
  async_result(const async_result &) = delete;
  async_result &operator=(const async_result &) = delete;
  return_type get() {
    return std::move(fut);
  }
};

template <>
class std::experimental::net::async_result<use_boost_future_t, void(std::error_code, size_t)> {
  boost::future<size_t> fut;
public:
  struct completion_handler_type {
    boost::promise<size_t> p;
    template <typename Whatever>
    completion_handler_type(Whatever const&) {}
    void operator() (std::error_code const& ec, size_t n) {
      if (ec) p.set_exception(std::system_error(ec));
      else p.set_value(n);
    }
  };
  using return_type = boost::future<size_t>;
  explicit async_result(completion_handler_type &h) : fut(h.p.get_future()) {}
  async_result(const async_result &) = delete;
  async_result &operator=(const async_result &) = delete;
  return_type get() {
    return std::move(fut);
  }
};

#endif