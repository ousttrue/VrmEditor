#pragma once
#include <filesystem>
#include <string>
#include <expected>

class LuaEngine
{
  struct LuaEngineImpl* m_impl = nullptr;

public:
  LuaEngine();
  ~LuaEngine();
  // lua_State* state() { return L_; }
  std::expected<bool, std::string> Eval(std::string_view script);
  std::expected<bool, std::string> DoFile(const std::filesystem::path& path);
};
