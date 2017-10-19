//
// myserver.cpp
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
#include <iostream>
#include <vector>

using namespace std::experimental;
using namespace std::experimental::net;

class session {
public:
  session(io_context &io, ip::tcp::socket s, size_t block_size)
      : io_context_(io), socket_(std::move(s)), block_size_(block_size),
        buf_(block_size), read_data_length_(0)
  {}

  void start() {
    std::error_code set_option_err;
    ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay, set_option_err);
    if (!set_option_err) {
      socket_.async_read_some( buffer(buf_.data(), block_size_),
        [this](auto ec, auto n) { handle_read(ec, n); });
      return;
    }

    net::post(io_context_, [this] { destroy(this); });
  }

  void handle_read(const std::error_code &err, size_t length) {
    if (!err) {
      read_data_length_ = length;
      async_write(socket_, buffer(buf_.data(), read_data_length_),
        [this](auto ec, auto) { handle_write(ec); });
      return;
    }

    net::post(io_context_, [this] { destroy(this); });
  }

  void handle_write(const std::error_code &err) {
    if (!err) {
      socket_.async_read_some(buffer(buf_.data(), block_size_),
        [this](auto ec, auto n) { handle_read(ec, n); });
      return;
    }

    net::post(io_context_, [this] { destroy(this); });
  }

  static void destroy(session *s) { delete s; }

private:
  net::io_context &io_context_;
  net::ip::tcp::socket socket_;
  size_t block_size_;
  std::vector<char> buf_;
  size_t read_data_length_;
};

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
      session *new_session = new session(io_context_, std::move(s), block_size_);
      new_session->start();
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
