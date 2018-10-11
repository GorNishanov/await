#pragma once

#include <xmmintrin.h>
#include <experimental/coroutine>

///// --- INFRASTRUCTURE CODE BEGIN ---- ////

struct scheduler_queue {
  static constexpr const int N = 256;
  using coro_handle = std::experimental::coroutine_handle<>;

  uint32_t head = 0;
  uint32_t tail = 0;
  coro_handle arr[N];

  void push_back(coro_handle h) {
    arr[head] = h;
    head = (head + 1) % N;
  }

  coro_handle pop_front() {
    auto result = arr[tail];
    tail = (tail + 1) % N;
    return result;
  }
  auto try_pop_front() { return head != tail ? pop_front() : coro_handle{}; }

  void run() {
    while (auto h = try_pop_front())
      h.resume();
  }
};

inline scheduler_queue scheduler;

// prefetch Awaitable
template <typename T> struct prefetch_Awaitable {
  T &value;

  prefetch_Awaitable(T &value) : value(value) {}

  bool await_ready() { return false; }
  T &await_resume() { return value; }
  template <typename Handle> auto await_suspend(Handle h) {
    _mm_prefetch(reinterpret_cast<char const *>(std::addressof(value)),
                 _MM_HINT_NTA);
    auto &q = scheduler;
    q.push_back(h);
    return q.pop_front();
  }
};

template <typename T> auto prefetch(T &value) {
  return prefetch_Awaitable<T>{value};
}

// Simple thread caching allocator.
struct tcalloc {
  struct header {
    header *next;
    size_t size;
  };
  header *root = nullptr;
  size_t last_size_allocated = 0;
  size_t total = 0;
  size_t alloc_count = 0;

  ~tcalloc() {
    auto current = root;
    while (current) {
      auto next = current->next;
      ::free(current);
      current = next;
    }
  }

  void *alloc(size_t sz) {
    if (root && root->size <= sz) {
      void *mem = root;
      root = root->next;
      return mem;
    }
    ++alloc_count;
    total += sz;
    last_size_allocated = sz;

    return malloc(sz);
  }

  void stats() {
    printf("allocs %zu total %zu sz %zu\n", alloc_count, total, last_size_allocated);
  }

  void free(void *p, size_t sz) {
    auto new_entry = static_cast<header *>(p);
    new_entry->size = sz;
    new_entry->next = root;
    root = new_entry;
  }
};

inline tcalloc allocator;


struct throttler;

struct root_task {
  struct promise_type;
  using HDL = std::experimental::coroutine_handle<promise_type>;

  struct promise_type {
    throttler *owner = nullptr;

    void *operator new(size_t sz) { return allocator.alloc(sz); }
    void operator delete(void *p, size_t sz) { allocator.free(p, sz); }

    root_task get_return_object() { return root_task{*this}; }
    std::experimental::suspend_always initial_suspend() { return {}; }
    void return_void();
    void unhandled_exception() noexcept { std::terminate(); }
    std::experimental::suspend_never final_suspend() { return {}; }
  };

  // TODO: this can be done via a wrapper coroutine
  auto set_owner(throttler *owner) {
    auto result = h;
    h.promise().owner = owner;
    h = nullptr;
    return result;
  }

  ~root_task() {
    if (h)
      h.destroy();
  }

  root_task(root_task&& rhs) : h(rhs.h) { rhs.h = nullptr; }
  root_task(root_task const&) = delete;

private:
  root_task(promise_type &p) : h(HDL::from_promise(p)) {}

  HDL h;
};

struct throttler {
  unsigned limit;

  explicit throttler(unsigned limit) : limit(limit) {}

  void on_task_done() { ++limit; }

  void spawn(root_task t) {
    if (limit == 0)
      scheduler.pop_front().resume();

    auto h = t.set_owner(this);
    scheduler.push_back(h);
    --limit;
  }

  void run() {
    scheduler.run();
  }

  ~throttler() { run(); }
};

void root_task::promise_type::return_void() { owner->on_task_done(); }

///// --- INFRASTRUCTURE CODE END ---- ////
