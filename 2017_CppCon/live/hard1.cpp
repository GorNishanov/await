#include <experimental/net>
#include <iostream>
#include <vector>
#include "future_adapter.h"
#include "use_boost_future.h"

using namespace std;
using namespace std::experimental;
using namespace std::experimental::net;

template <typename Socket, typename BufferSeq>
auto async_read_some(Socket& s, BufferSeq const& b) {
  struct Awaitable {
    Socket& s; BufferSeq const& b;
    std::error_code ec;
    size_t n;
    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) {
      s.async_read_some(b, [this, h](auto ec, auto n) mutable {
        this->ec = ec;
        this->n = n;
        h.resume();
      });
    }
    auto await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
  };
  return Awaitable{s, b};
}

template <typename Socket, typename BufferSeq>
auto async_write(Socket& s, BufferSeq const& b) {
  struct Awaitable {
    Socket& s; BufferSeq const& b;
    std::error_code ec;
    size_t n;
    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) {
      async_write(s, b, [this, h](auto ec, auto n) mutable {
        this->ec = ec;
        this->n = n;
        h.resume();
      });
    }
    auto await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
  };
  return Awaitable{s, b};
}

future<void> session(ip::tcp::socket s, size_t block_size) {
  vector<char> buf(block_size);
  s.set_option(ip::tcp::no_delay(true));
  for (;;) {
    auto n = co_await async_read_some(s, buffer(buf.data(), block_size));
    n = co_await async_write(s, buffer(buf.data(), n));
  }
}

class server {
public:
  server(io_context &io, const ip::tcp::endpoint &endpoint, size_t block_size)
      : io_context_(io), acceptor_(io, endpoint), block_size_(block_size)
  {
    acceptor_.listen();

    start_accept();
  }

  void start_accept()
  {
    acceptor_.async_accept(
        [this](auto ec, auto s) { handle_accept(ec, std::move(s)); });
  }

  void handle_accept(std::error_code err, ip::tcp::socket s)
  {
    if (!err) {
      session(std::move(s), block_size_);
    }
    start_accept();
  }

private:
  io_context &io_context_;
  ip::tcp::acceptor acceptor_;
  size_t block_size_;
};

int main(int argc, const char *argv[]) {
  try {
    char const **args = argv;
    if (argc != 5) {
      static const char* defargs[] = {"myserver", "127.0.0.1", "8888", "4", "128"};
      args = defargs;
    }
    printf("%s %s %s %s %s\n", argv[0], args[1], args[2], args[3], args[4]);

    using namespace std; // For atoi.
    net::ip::address address = net::ip::make_address(args[1]);
    short port = atoi(args[2]);
    int thread_count = atoi(args[3]);
    size_t block_size = atoi(args[4]);

    io_context io;

    auto s = server(io, ip::tcp::endpoint(address, port), block_size);

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    while (--thread_count > 0) {
      threads.emplace_back([&io] { io.run(); });
    }

    io.run();

    while (!threads.empty()) {
      threads.back().join();
      threads.pop_back();
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}











#if 0
//
// hard1.cpp
// ~~~~~~~~~~~~
//
// Slightly modified version of
// https://github.com/chriskohlhoff/asio/blob/master/asio/src/tests/performance/server.cpp
// that has the following copyright:
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "future_adapter.h"
#include "use_boost_future.h"
#include <experimental/net>
#include <iostream>
#include <vector>

using namespace std::experimental;
using namespace std::experimental::net;

template <typename Socket, typename BufferSeq>
auto async_read_some(Socket& s, BufferSeq const& b) {
  struct Awaiter {
    Socket& s; BufferSeq const& b;

    std::error_code ec;
    size_t n;

    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) {
      s.async_read_some(b,
        [h, this](auto ec, auto n) mutable {
          this->ec = ec;
          this->n = n;
          h.resume();
        });
    }
    auto await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
  };
  return Awaiter{s, b};
}

template <typename Socket, typename BufferSeq>
auto async_write(Socket& s, BufferSeq const& b) {
  struct Awaiter {
    Socket& s; BufferSeq const& b;

    std::error_code ec;
    size_t n;

    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) {
      async_write(s, b,
        [h, this](auto ec, auto n) mutable {
          this->ec = ec;
          this->n = n;
          h.resume();
        });
    }
    auto await_resume() {
      if (ec) throw std::system_error(ec);
      return n;
    }
  };
  return Awaiter{s, b};
}

std::future<void> session(ip::tcp::socket s, size_t block_size)
{
  std::vector<char> buf(block_size);
  ip::tcp::no_delay no_delay(true);
  s.set_option(no_delay);
  for(;;) {
    auto n = co_await async_read_some(s, net::buffer(buf.data(), block_size));
    n = co_await async_write(s, net::buffer(buf.data(), n));
  }
}

class server {
public:
  server(net::io_context &ioc, const net::ip::tcp::endpoint &endpoint,
         size_t block_size)
      : acceptor_(ioc, endpoint), block_size_(block_size)
  {
    acceptor_.listen();

    start_accept();
  }

  void start_accept()
  {
    acceptor_.async_accept(
        [this](auto ec, auto s) { handle_accept(ec, std::move(s)); });
  }

  void handle_accept(std::error_code err, net::ip::tcp::socket s)
  {
    if (!err) {
      session(std::move(s), block_size_);
    }
    start_accept();
  }

private:
  net::ip::tcp::acceptor acceptor_;
  size_t block_size_;
};

int main(int argc, const char *argv[]) {
  try {
    char const **args = argv;
    if (argc != 5) {
      static const char* defargs[] = {"myserver", "127.0.0.1", "8888", "4", "128"};
      args = defargs;
    }
    printf("myserver %s %s %s %s\n", args[1], args[2], args[3], args[4]);

    using namespace std; // For atoi.
    net::ip::address address = net::ip::make_address(args[1]);
    short port = atoi(args[2]);
    int thread_count = atoi(args[3]);
    size_t block_size = atoi(args[4]);

    net::io_context ioc;

    auto s = server(ioc, net::ip::tcp::endpoint(address, port), block_size);

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    while (--thread_count > 0) {
      threads.emplace_back([&ioc] { ioc.run(); });
    }

    ioc.run();

    while (!threads.empty()) {
      threads.back().join();
      threads.pop_back();
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
#endif