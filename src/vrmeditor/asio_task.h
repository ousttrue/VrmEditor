#pragma once
#include <asio.hpp>
#include <functional>
#include <thread>

using Task = std::function<void()>;

class AsioTask
{
  asio::io_context m_io;

  AsioTask() {}

public:
  ~AsioTask() {}
  AsioTask(const AsioTask&) = delete;
  AsioTask& operator=(const AsioTask&) = delete;
  static AsioTask& Instance()
  {
    static AsioTask s_instance;
    return s_instance;
  }
  void PostTask(const Task& task) { m_io.post(task); }

  // https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp20/operations/callback_wrapper.cpp
  template<typename T>
  struct ThreadTask
  {
    template<typename Callback>
    static void Execute(const std::function<T()>& task, Callback cb)
    {
      std::thread([task, cb = std::move(cb)]() mutable {
        std::move(cb)(task());
      }).detach();
    }

    template<asio::completion_token_for<void(T)> CompletionToken>
    static auto AsyncThredTask(const std::function<T()>& task,
                               CompletionToken&& token)
    {
      auto init = [task](asio::completion_handler_for<void(T)> auto handler) {
        auto work = asio::make_work_guard(handler);

        Execute(task,
                [handler = std::move(handler),
                 work = std::move(work)](T result) mutable {
                  // Get the handler's associated allocator. If the handler
                  // does not specify an allocator, use the recycling
                  // allocator as the default.
                  auto alloc = asio::get_associated_allocator(
                    handler, asio::recycling_allocator<void>());

                  // Dispatch the completion handler through the handler's
                  // associated executor, using the handler's associated
                  // allocator.
                  asio::dispatch(
                    work.get_executor(),
                    asio::bind_allocator(
                      alloc, [handler = std::move(handler), result]() mutable {
                        std::move(handler)(result);
                      }));
                });
      };

      asio::async_initiate<CompletionToken, void(T)>(init, token);
    }
  };

  template<typename T>
  void PostThreadTask(const std::function<T(void)>& task,
                      const std::function<void(T)>& callback)
  {
    ThreadTask<T>::AsyncThredTask(task, asio::bind_executor(m_io, callback));
  }

  void Poll() { m_io.poll_one(); }

  asio::io_context& Executor() { return m_io; }
};
