#pragma once
#include <lua.hpp>
#include <string>

class LuaEngine {
  lua_State *L_ = nullptr;

public:
  LuaEngine();
  ~LuaEngine();
  lua_State *state() { return L_; }
  void eval(const std::string &script);
  void dofile(const std::string &path);
};
