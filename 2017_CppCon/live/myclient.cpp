//
// myclient.cpp
// ~~~~~~~~~~
//
// Slightly modified version of
// https://github.com/chriskohlhoff/asio/blob/master/asio/src/tests/performance/client.cpp
// that has the following copyright:
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <experimental/net>
#include <experimental/timer>
#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <mutex>
#include "handler_allocator.hpp"
#include <atomic>

using namespace std::experimental;

#ifdef COUNT_ALLOCS
std::atomic<int> allocs;

void* operator new(size_t n) {
  allocs.fetch_add(1, std::memory_order::memory_order_relaxed);
  return malloc(n);
}
void operator delete(void* p) noexcept {
  free(p);
}
#endif

class stats
{
public:
  stats(int timeout)
    : mutex_(),
      timeout_(timeout),
      total_bytes_written_(0),
      total_bytes_read_(0)
  {
  }

  void add(size_t bytes_written, size_t bytes_read)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    total_bytes_written_ += bytes_written;
    total_bytes_read_ += bytes_read;
  }

  void print()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << scale(total_bytes_written_) << " Mbytes written per second\n";
    std::cout << scale(total_bytes_read_) << " Mbytes read per second\n";
  }

  double scale(double bytes) const
  {
    return bytes / timeout_ / 1024 / 1024;
  }

private:
  std::mutex mutex_;
  const int timeout_;
  size_t total_bytes_written_;
  size_t total_bytes_read_;
};

class session
{
public:
  session(net::io_context& ioc, size_t block_size, stats& s)
    : strand_(ioc.get_executor()),
      socket_(ioc),
      block_size_(block_size),
      read_data_(new char[block_size]),
      read_data_length_(0),
      write_data_(new char[block_size]),
      unwritten_count_(0),
      bytes_written_(0),
      bytes_read_(0),
      stats_(s)
  {
    for (size_t i = 0; i < block_size_; ++i)
      write_data_[i] = static_cast<char>(i % 128);
  }

  ~session()
  {
    stats_.add(bytes_written_, bytes_read_);

    delete[] read_data_;
    delete[] write_data_;
  }

  void start(net::ip::tcp::resolver::results_type endpoints)
  {
    net::async_connect(socket_, endpoints,
        net::bind_executor(strand_,
          [this] (auto ec, auto) { handle_connect(ec); }));
  }

  void stop()
  {
    net::post(strand_, [this] { close_socket(); });
  }

private:
  void handle_connect(const std::error_code& err)
  {
    if (!err)
    {
      std::error_code set_option_err;
      net::ip::tcp::no_delay no_delay(true);
      socket_.set_option(no_delay, set_option_err);
      if (!set_option_err)
      {
        ++unwritten_count_;
        async_write(socket_, net::buffer(write_data_, block_size_),
            net::bind_executor(strand_,
              //make_custom_alloc_handler(write_allocator_,
                [this](auto ec, auto n) { handle_write(ec, n); }));
        socket_.async_read_some(net::buffer(read_data_, block_size_),
            net::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                [this](auto ec, auto n) { handle_read(ec, n); })));
        }
    }
  }

  void handle_read(const std::error_code& err, size_t length)
  {
    if (!err)
    {
      bytes_read_ += length;

      read_data_length_ = length;
      ++unwritten_count_;
      if (unwritten_count_ == 1)
      {
        std::swap(read_data_, write_data_);
        async_write(socket_, net::buffer(write_data_, read_data_length_),
            net::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                [this](auto ec, auto n) { handle_write(ec, n); })));
        socket_.async_read_some(net::buffer(read_data_, block_size_),
            net::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                [this](auto ec, auto n) { handle_read(ec, n); })));
      }
    }
  }

  void handle_write(const std::error_code& err, size_t length)
  {
    if (!err && length > 0)
    {
      bytes_written_ += length;

      --unwritten_count_;
      if (unwritten_count_ == 1)
      {
        std::swap(read_data_, write_data_);
        async_write(socket_, net::buffer(write_data_, read_data_length_),
            net::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                [this](auto ec, auto n) { handle_write(ec, n); })));
        socket_.async_read_some(net::buffer(read_data_, block_size_),
            net::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                [this](auto ec, auto n) { handle_read(ec, n); })));
      }
    }
  }

  void close_socket()
  {
    socket_.close();
  }

private:
  net::strand<net::io_context::executor_type> strand_;
  net::ip::tcp::socket socket_;
  size_t block_size_;
  char* read_data_;
  size_t read_data_length_;
  char* write_data_;
  int unwritten_count_;
  size_t bytes_written_;
  size_t bytes_read_;
  stats& stats_;
  handler_allocator read_allocator_;
  handler_allocator write_allocator_;
};

class client
{
public:
  client(net::io_context& ioc,
      const net::ip::tcp::resolver::results_type endpoints,
      size_t block_size, size_t session_count, int timeout)
    : io_context_(ioc),
      stop_timer_(ioc),
      sessions_(),
      stats_(timeout)
  {
    stop_timer_.expires_after(std::chrono::seconds(timeout));
    stop_timer_.async_wait([this](auto){ handle_timeout(); });

    for (size_t i = 0; i < session_count; ++i)
    {
      session* new_session = new session(io_context_, block_size, stats_);
      new_session->start(endpoints);
      sessions_.push_back(new_session);
    }
  }

  ~client()
  {
    while (!sessions_.empty())
    {
      delete sessions_.front();
      sessions_.pop_front();
    }

    stats_.print();
  }

  void handle_timeout()
  {
    for (auto *session: sessions_)
      session->stop();
  }

private:
  net::io_context& io_context_;
  net::system_timer stop_timer_;
  std::list<session*> sessions_;
  stats stats_;
};

int main(int argc, char const* argv[])
{
  try
  {
    char const **args = argv;
    if (argc != 7)
    {
      static const char* defargs[] = {"myserver", "127.0.0.1", "8888", "4", "128", "14", "3"};
      args = defargs;
      //std::cerr << "Usage: client <host> <port> <threads> <blocksize> ";
      //std::cerr << "<sessions> <time>\n";
      //return 1;
    }
    printf("myclient %s %s %s %s %s %s\n", args[1], args[2], args[3], args[4], args[5], args[6]);

    using namespace std; // For atoi.
    const char* host = args[1];
    const char* port = args[2];
    int thread_count = atoi(args[3]);
    size_t block_size = atoi(args[4]);
    size_t session_count = atoi(args[5]);
    int timeout = atoi(args[6]);

    net::io_context ioc;

    net::ip::tcp::resolver r(ioc);
    net::ip::tcp::resolver::results_type endpoints =
      r.resolve(host, port);

    client c(ioc, endpoints, block_size, session_count, timeout);

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
#ifdef COUNT_ALLOCS
    std::cout << "allocs: " << allocs.load() << "\n";
#endif
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
