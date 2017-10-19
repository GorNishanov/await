//
// handler_allocator.cpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HANDLER_ALLOCATOR_HPP
#define HANDLER_ALLOCATOR_HPP

#include <experimental/net>
#include <array>

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
struct handler_allocator
{
  void* allocate(std::size_t size)
  {
    if (!in_use_ && size <= storage_.size())
    {
      in_use_ = true;
      return storage_.data();
    }

    return nullptr;
  }

  bool deallocate(void* pointer)
  {
    if (pointer == storage_.data())
    {
      in_use_ = false;
      return true;
    }
    return false;
  }

private:
  // Storage space used for handler-based custom memory allocation.
  alignas(double) std::array<char, 1024> storage_;

  // Whether the handler-based custom allocation storage has been used.
  bool in_use_ = false;
};

template <class T> struct SimpleAllocator {
  using value_type = T;
  SimpleAllocator(handler_allocator& my_alloc) : my_alloc(my_alloc) {}
  template <class U>
  SimpleAllocator(const SimpleAllocator<U> &other) : my_alloc(other.my_alloc) {}

  T *allocate(std::size_t n) {
    size_t bytes = n * sizeof(T);
    if (bytes / sizeof(T) != n)
      throw std::bad_alloc();

    void* result = my_alloc.allocate(bytes);
    if (!result)
      result = operator new(bytes);
    return static_cast<T*>(result);
  }
  void deallocate(T *p, std::size_t n) {
    if (my_alloc.deallocate(p))
      return;
    operator delete(p, n);
  }

  handler_allocator& my_alloc;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler
{
public:
  custom_alloc_handler(handler_allocator& a, Handler h)
    : allocator_(a),
      handler_(h)
  {
  }

  template <typename... Args>
  void operator()(Args&&... args) {
    handler_(std::forward<Args>(args)...);
  }

  using allocator_type = SimpleAllocator<char>;

  auto get_allocator() const { return SimpleAllocator<char>{allocator_}; }

private:
  handler_allocator& allocator_;
  Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
    handler_allocator& a, Handler h)
{
  return custom_alloc_handler<Handler>(a, h);
}

#endif // HANDLER_ALLOCATOR_HPP
