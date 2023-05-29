#pragma once
#include "gui.h"
#include <functional>
#include <sstream>
#include <string>
#include <vector>

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
enum class LogLevel
{
  // green
  Info,
  // gray
  Debug,
  // orange
  Wran,
  // red
  Error,
};
struct Msg
{
  LogLevel Level;
  std::string Message;
};

using AddLogFunc = std::function<void(std::string_view)>;
using FlushLog = std::function<void()>;

class ImLogger
{
  std::vector<Msg> Logs;
  bool AutoScroll = true;
  std::string m_home;

public:
  ImLogger();

  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<ImLogger>& logger);

  void Clear();

  LogLevel m_level;
  std::stringstream m_ss;
  std::ostream& Begin(LogLevel level);
  void Push(std::string_view msg) { m_ss << msg; }
  std::string SubStitute(std::string_view src);
  void End();
  void AddLog(LogLevel level, std::string_view msg);
  void Draw();
};

struct LogStream
{
  std::ostream& m_os;
  FlushLog m_flush;

public:
  LogStream(std::ostream& os, const FlushLog& flush)
    : m_os(os)
    , m_flush(flush)
  {
  }
  ~LogStream() { m_flush(); }
  template<typename T>
  LogStream& operator<<(T msg)
  {
    m_os << msg;
    return *this;
  }
};
