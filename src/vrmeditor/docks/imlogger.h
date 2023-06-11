#pragma once
#include <plog/Log.h>
#include <regex>
#include <vector>

class ImLogger
{
  struct Msg
  {
    plog::Severity Level;
    plog::util::nstring Message;
  };
  std::vector<Msg> Logs;
  bool AutoScroll = true;
  std::regex m_home;
  ImLogger();

public:
  ImLogger(const ImLogger&) = delete;
  ImLogger& operator=(const ImLogger&) = delete;
  static ImLogger& Instance()
  {
    static ImLogger s_instance;
    return s_instance;
  }

  void Clear();

  plog::Severity m_level;
  void AddLog(plog::Severity level, const plog::util::nstring& msg);
  void Draw();
};
