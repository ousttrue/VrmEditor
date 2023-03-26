#pragma once
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <string_view>

struct Scene;
struct Bvh;
class Gui;
class Timeline;
class AssetDir;
class LuaEngine;

class App {
  std::shared_ptr<LuaEngine> lua_;
  std::shared_ptr<Gui> gui_;
  std::shared_ptr<Scene> scene_;
  std::list<std::shared_ptr<AssetDir>> assets_;
  std::shared_ptr<Bvh> motion_;
  std::shared_ptr<Timeline> timeline_;

  App();

public:
  ~App();
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  static App &instance() {
    static App s_instance;
    return s_instance;
  }
  const std::shared_ptr<LuaEngine> &lua() const { return lua_; }
  int run();
  // lua API
  void clear_scene();
  bool load_model(const std::filesystem::path &path);
  bool load_motion(const std::filesystem::path &path, float scaling = 1.0f);
  void load_lua(const std::filesystem::path &path);
  bool addAssetDir(std::string_view name, const std::string &path);

private:
  void jsonDock();
  void sceneDock();
  void timelineDock();
  void assetsDock();
};
