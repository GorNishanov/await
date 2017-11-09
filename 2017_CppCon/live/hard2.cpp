// hard2.cpp
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

#include <experimental/net>
#include <experimental/coroutine>
#include <iostream>
#include <vector>
#include "future_adapter.h"
#include "await_adapters.h"

using namespace std::experimental;
using namespace std::experimental::net;

std::future<void> session(io_context& io, ip::tcp::socket s, size_t block_size)
{
  s.set_option(ip::tcp::no_delay(true));
  std::vector<char> buf(block_size);
  for (;;) {
    auto n = co_await async_read_some(s, net::buffer(buf.data(), block_size));
    n = co_await async_write(s, net::buffer(buf.data(), n));
  }
}

std::future<void> server(io_context &io, const ip::tcp::endpoint &endpoint,
                         size_t block_size)
{
  ip::tcp::acceptor acceptor(io, endpoint);
  acceptor.listen();
  for (;;) {
    auto s = co_await async_accept(acceptor);
    session(io, std::move(s), block_size);
  }
}

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
