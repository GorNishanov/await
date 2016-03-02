#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <utility>

void *operator new(size_t sz) {
  void *p = malloc(sz);
  printf("alloc %zu => %p\n", sz, p);
  return p;
}
void operator delete(void *p) noexcept {
  printf("delete %p\n", p);
  free(p);
}

template <typename _Ty> struct generator {
  struct promise_type {
    _Ty current_value;
    std::suspend_always yield_value(_Ty value) {
      this->current_value = std::move(value);
      return {};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() { return {}; }
    generator get_return_object() { return generator{this}; };
  };

  struct iterator {
    std::coroutine_handle<promise_type> _Coro;
    bool _Done;

    iterator(std::coroutine_handle<promise_type> Coro, bool Done)
        : _Coro(Coro), _Done(Done) {}

    iterator &operator++() {
      _Coro.resume();
      _Done = _Coro.done();
      return *this;
    }

    bool operator==(iterator const &_Right) const {
      return _Done == _Right._Done;
    }

    bool operator!=(iterator const &_Right) const { return !(*this == _Right); }

    _Ty const &operator*() const { return _Coro.promise().current_value; }

    _Ty const *operator->() const { return &(operator*()); }
  };

  iterator begin() {
    p.resume();
    return {p, p.done()};
  }

  iterator end() { return {p, true}; }

  generator(generator &&rhs) : p(rhs.p) { rhs.p = nullptr; }

  ~generator() {
    if (p)
      p.destroy();
  }

private:
  explicit generator(promise_type *p)
      : p(std::coroutine_handle<promise_type>::from_promise(*p)) {}

  std::coroutine_handle<promise_type> p;
};

void Escape(generator<int> &g) {
  void *p = &g;
  asm volatile("" : : "g"(p) : "memory");
}

generator<int> f() {
  int N = 200;
  for (int i = 3; i < N; i *= 2)
    co_yield i;
}

int main() {
  auto g = f();
  // Escape(g); // uncomment to disable heap elision for g
  for (auto v : g)
    printf("%d\n", v);
}
