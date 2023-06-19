#pragma once
#include <asio.hpp>
#include <functional>

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

  template<typename T>
  void ThreadTask(const std::function<T(void)>& onThred,
                  const std::function<void(T)>& callback)
  {
  }

  void Poll() { m_io.poll_one(); }
};
