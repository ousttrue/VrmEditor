#pragma once
#include <lua.hpp>
#include <memory>
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

struct Scene;
class App {
  LuaEngine lua_;
  std::shared_ptr<Scene> scene_;

  App();

public:
  ~App();
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  static App &instance() {
    static App s_instance;
    return s_instance;
  }
  lua_State *lua();
  int run(int argc, char **argv);
  bool load(const std::string &path);
};
