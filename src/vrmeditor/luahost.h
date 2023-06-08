#pragma once
#include <expected>
#include <filesystem>
#include <string>

class LuaEngine
{
  struct LuaEngineImpl* m_impl = nullptr;

  LuaEngine();

public:
  ~LuaEngine();
  LuaEngine(const LuaEngine&) = delete;
  LuaEngine& operator=(const LuaEngine&) = delete;
  static LuaEngine& Instance()
  {
    static LuaEngine s_instance;
    return s_instance;
  }
  // lua_State* state() { return L_; }
  std::expected<bool, std::string> Eval(std::string_view script);
  void DoFile(const std::filesystem::path& path);
};
