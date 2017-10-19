#include <stdio.h>
#include "future_adapter.h"
#include "await_adapters.h"

#include <experimental/timer>
#include <system_error>
#include <iostream>

using namespace std::chrono;
using namespace std::experimental::net;

template <typename Clock, typename R, typename P>
auto better_async_wait(std::experimental::net::basic_waitable_timer<Clock> &t,
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

    struct Callback {
      Awaiter *me;
      std::experimental::coroutine_handle<> coro;

      void operator() (std::error_code ec) {
        me->ec = ec;
        auto tmp = coro;
        coro = {};
        tmp.resume();
      }

      Callback(Awaiter *me, std::experimental::coroutine_handle<> coro) :
        me(me), coro(coro)
      {}
      Callback(Callback const&) = delete;
      Callback(Callback && other) : me(other.me), coro(other.coro) {
        other.coro = nullptr;
      }

      ~Callback() {
        if (coro) {
          me->ec = error::operation_aborted;
          coro.resume();
        }
      }
    };

    void await_suspend(std::experimental::coroutine_handle<> coro) {
      t.expires_after(d);
      t.async_wait(Callback{this, coro});
    }
  };
  return Awaiter{ t, d };
}


std::future<void> noisy_clock(system_timer &timer) {
  try {
    for (;;) {
      co_await better_async_wait(timer, 1s);
      puts("tick");
      co_await better_async_wait(timer, 1s);
      puts("tock");
    }
  }
  catch (std::exception const& e) {
    std::cout << "caught: " << e.what() << "\n";
  }
}

int main() {
  {
    io_context io;
    system_timer timer(io);
    auto f = noisy_clock(timer);
    system_timer fast_timer(io, 1s);
    fast_timer.async_wait([&](auto){io.stop();});
    io.run();
    puts("done");
  }
  puts("io_context destroyed");
}

























#if 0
#include <stdio.h>
#include "future_adapter.h"
#include "await_adapters.h"

#include <experimental/timer>
#include <system_error>
#include <iostream>

using namespace std::chrono;
using namespace std::experimental::net;

std::future<void> noisy_clock(system_timer &timer) {
  try {
    for (;;) {
      co_await async_wait(timer, 1s);
      puts("tick");
      co_await async_wait(timer, 1s);
      puts("tock");
    }
  }
  catch (std::exception const& e) {
    std::cout << "caught: " << e.what() << "\n";
  }
}

int main() {
  {
    io_context io;
    system_timer timer(io);
    auto f = noisy_clock(timer);
    io.run();
    puts("done");
  }
  puts("io_context destroyed");
}
#endif


#if 0
#include <stdio.h>
#include "future_adapter.h"
#include "await_adapters.h"

#include <experimental/timer>
#include <system_error>
#include <iostream>

using namespace std::chrono;
using namespace std::experimental::net;

template <typename Clock, typename R, typename P>
auto better_async_wait(std::experimental::net::basic_waitable_timer<Clock> &t,
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

    struct Callback {
      Awaiter *me;
      std::experimental::coroutine_handle<> coro;

      void operator() (std::error_code ec) {
        me->ec = ec;
        auto tmp = coro;
        coro = {};
        tmp.resume();
      }

      Callback(Awaiter *me, std::experimental::coroutine_handle<> coro) :
        me(me), coro(coro)
      {}
      Callback(Callback const&) = delete;
      Callback(Callback && other) : me(other.me), coro(other.coro) {
        other.coro = nullptr;
      }

      ~Callback() {
        if (coro) {
          me->ec = error::operation_aborted;
          coro.resume();
        }
      }
    };

    void await_suspend(std::experimental::coroutine_handle<> coro) {
      t.expires_after(d);
      t.async_wait(Callback{this, coro});
    }
  };
  return Awaiter{ t, d };
}

std::future<void> noisy_clock(system_timer &timer) {
  try {
    for (;;) {
      co_await better_async_wait(timer, 1s);
      puts("tick");
      co_await better_async_wait(timer, 1s);
      puts("tock");
    }
  }
  catch (std::exception const& e) {
    std::cout << "caught: " << e.what() << "\n";
  }
}

int main() {
  {
    io_context io;
    system_timer timer(io);
    auto f = noisy_clock(timer);
    system_timer fast_timer(io, 3s);
    fast_timer.async_wait([&](auto){ io.stop();});

    io.run();
    puts("done");
  }
  puts("io_context destroyed");
}

#endif