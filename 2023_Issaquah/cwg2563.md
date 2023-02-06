## 2563. Initialization of coroutine result object

Q: https://cplusplus.github.io/CWG/issues/2563.html

Three common patterns with coroutines:

1. async coroutines (tasks - the result is know in the future)
2. generators (lazily produced sequence of values, synchronously generated when pull the value).
3. short-circuiting synchronous coroutines: expected/optional, etc (used for unwrapping the value and error propagation)

```c++
// example for case 3:

expected<int> bar()
expected<int> bar2()

expected<int> foo()
{
    int val = co_await bar() + co_await bar2();
    co_return val;
}
```

For these three common patterns:

1. get_return_object returns the same object that coroutine returns/coroutine immediately suspends - sequencing of conversion to return-type/execution of body/initial-suspend not significant
2. same as case 1
3. get_return_object returns a proxy, coroutine body executes, conversion to return type happens when coroutine return the caller. Need coroutine body to execute before initialising the return-value since coroutine body produces the result.

To answer the issues raised by the core:

Q: _whether get_return_object() is invoked inside or outside of the try-block shown in paragraph 5 (see issue 2562),_

A: Outside. Exceptions from get_return_object propagates to the caller of the coroutine.

Coroutine state implicitly destroyed during unwind.

Q: _whether the prvalue result object may be initialized later (e.g. ~~before~~ after the first actual suspension),_

A: Yes it can! This is the intent when get_return_object type != return type of the coroutine

Necessary for supporting std::expected coroutine use-case.

Q: _if the initialization does occur later, by what mechanism the prvalue result of get_return_object is forwarded to that initialization._

A: (see below)

Case 1/2. Same type of get_return_object and coroutine return type.

Constructed in the return slot.

Case 3. Different types:

Step 1. Constructed temporary object prior to initial suspend initialized with a call to get_return_object()

Step 2. when coroutine needs to to return to the caller and needs to construct return value for the coroutine
it is initialized with expiring value of the temporary obtained in step 1.

If creation of coroutine return object or a call to get_return_object fails, the exception is propagated to the caller.

## Appendix

```c++
template<typename T, typename E, typename... Args>
struct std::coroutine_traits<expected<T, E>, Args...> {
  struct promise_type {
    std::variant<std::monostate, T, E> result;

    struct expected_proxy {
      coroutine_handle<promise_type> coro;

      operator expected<T, E>() {
        if (coro.promise().result.index() == 0) {
          return expected<T, E>{std::in_place, std::get<0>(std::move(coro.promise().result))};
        } else {
          return expected<T, E>{std::unexpect, std::get<1>(std::move(coro.promise().result))};
        }
      }
    };

    expected_proxy get_return_object() {
      return expected_proxy{coroutine_handle<promise_type>::from_promise(*this)};
    }

    template<typename U>
    void return_value(U&& value) {
      result.emplace<2>(std::forward<U>(value));
    }
    ...
};
```
