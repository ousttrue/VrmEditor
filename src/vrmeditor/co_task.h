#pragma once
#include <coroutine>
#include <exception>

template<typename T>
struct CoFuture
{
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<>) {}
  T await_resume();
};

struct Co
{
  struct promise_type
  {
    Co get_return_object();
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
  };
};
