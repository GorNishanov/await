#pragma once

namespace std {
template <typename R, typename...> struct coroutine_traits {
  using promise_type = typename R::promise_type;
};

template <typename Promise = void> struct coroutine_handle;

template <> struct coroutine_handle<void> {
  static coroutine_handle from_address(void *addr) {
    coroutine_handle me;
    me.ptr = addr;
    return me;
  }
  void operator()() { resume(); }
  void *address() const { return ptr; }
  void resume() const { __builtin_coro_resume(ptr); }
  void destroy() const { __builtin_coro_destroy(ptr); }
  bool done() const { return __builtin_coro_done(ptr); }
  coroutine_handle &operator=(decltype(nullptr)) {
    ptr = nullptr;
    return *this;
  }
  coroutine_handle() : ptr(nullptr) {}
  explicit operator bool() const { return ptr; }

protected:
  void *ptr;
};

template <typename Promise> struct coroutine_handle : coroutine_handle<> {
  using coroutine_handle<>::operator=;

  static coroutine_handle from_address(void *addr) {
    coroutine_handle me;
    me.ptr = addr;
    return me;
  }

  Promise &promise() const {
    return *reinterpret_cast<Promise *>(__builtin_coro_promise((Promise *)ptr));
  }
  static coroutine_handle from_promise(Promise &promise) {
    coroutine_handle p;
    p.ptr = __builtin_coro_from_promise(&promise);
    return p;
  }
};

template <typename _PromiseT>
bool operator==(coroutine_handle<_PromiseT> const &_Left,
                coroutine_handle<_PromiseT> const &_Right) noexcept {
  return _Left.address() == _Right.address();
}

template <typename _PromiseT>
bool operator!=(coroutine_handle<_PromiseT> const &_Left,
                coroutine_handle<_PromiseT> const &_Right) noexcept {
  return !(_Left == _Right);
}

struct suspend_always {
  bool await_ready() { return false; }
  void await_suspend(coroutine_handle<>) {}
  void await_resume() {}
};
struct suspend_never {
  bool await_ready() { return true; }
  void await_suspend(coroutine_handle<>) {}
  void await_resume() {}
};
}

// FIXME: generate in the FE?
template <typename Awaitable, typename Promise> void _Ramp(void *a, void *b) {
  static_cast<Awaitable *>(a)
      ->await_suspend(std::coroutine_handle<Promise>::from_address(b));
}
