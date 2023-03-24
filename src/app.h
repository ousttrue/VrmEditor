#pragma once
#include "camera.h"
#include <filesystem>
#include <functional>
#include <list>
#include <lua.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

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
class Gui;
using AssetEnter =
    std::function<bool(const std::filesystem::path &path, uint64_t id)>;
using AssetLeave = std::function<void()>;
class AssetDir {
  std::string name_;
  std::filesystem::path root_;

  std::unordered_map<std::filesystem::path, uint64_t> idMap_;
  uint64_t nextId_ = 1;

public:
  AssetDir(std::string_view name, std::string_view path);
  const std::string &name() const { return name_; }
  void traverse(const AssetEnter &enter, const AssetLeave &leave,
                const std::filesystem::path &path = {});
};

class App {
  LuaEngine lua_;
  std::shared_ptr<Gui> gui_;
  std::shared_ptr<Scene> scene_;
  std::list<std::shared_ptr<AssetDir>> assets_;
  Camera camera{};

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
  // lua API
  bool load(const std::filesystem::path &path);
  bool addAssetDir(std::string_view name, const std::string &path);

private:
  void cameraViewDock();
  void jsonDock();
  void sceneDock();
  void timelineDock();
  void assetsDock();
};
