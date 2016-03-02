#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;

void *operator new(size_t sz) {
  void *p = malloc(sz);
  printf("alloc %zu => %p\n", sz, p);
  return p;
}
void operator delete(void *p) noexcept {
  printf("delete %p\n", p);
  free(p);
}

struct coro {
  struct promise_type {
    suspend_always initial_suspend() { return {}; }
    auto final_suspend() {
      struct Awaiter {
        promise_type *me;

        bool await_ready() { return false; }
        void await_suspend(coroutine_handle<>) {
          if (me->waiter) {
            puts("has waiter, resuming...");
            me->waiter.resume();
          } else {
            puts("no waiter\n");
          }
        }
        void await_resume() {}
      };
      return Awaiter{this};
    }

    coro get_return_object() {
      return {coroutine_handle<promise_type>::from_promise(*this)};
    }

    void return_void() {}

    coroutine_handle<> waiter;
  };

  bool await_ready() { return false; }
  void await_suspend(coroutine_handle<> h) {
    me.promise().waiter = h;
    me.resume();
  }
  void await_resume() {}

  ~coro() { me.destroy(); }

  coroutine_handle<promise_type> me;
};

void Escape(coro &g) {
  void *p = &g;
  asm volatile("" : : "g"(p) : "memory");
}

extern "C" coro g() {
  printf("g running\n");
  co_await suspend_never{};
}

extern "C" coro f() {
  printf("f started\n");
  co_await g();
  printf("f resumed\n");
}

int main() {
  auto x = f();
  // Escape(x); // uncomment to disable heap elision for x
  x.me.resume();
  printf("post-resume");
}