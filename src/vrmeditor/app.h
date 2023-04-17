#pragma once
#include "docks/imlogger.h"
#include <list>
#include <memory>
#include <unordered_set>
#include <vrm/humanbones.h>

namespace libvrm {
namespace gltf {
struct Scene;
}
namespace bvh {
struct Bvh;
}
struct Timeline;
}

namespace grapho {
class OrbitView;
}

namespace humanpose{
struct HumanPoseStream;
}

class Gui;
struct AssetDir;
class LuaEngine;
class Platform;
class ImLogger;
class Gl3Renderer;

class App
{
  std::filesystem::path m_ini;
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::shared_ptr<LuaEngine> m_lua;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<ImLogger> m_logger;

  // root timeline
  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<grapho::OrbitView> m_view;
  std::shared_ptr<Gl3Renderer> m_renderer;

  App();

public:
  std::shared_ptr<humanpose::HumanPoseStream> PoseStream;
  ~App();
  App(const App&) = delete;
  App& operator=(const App&) = delete;
  static App& Instance()
  {
    static App s_instance;
    return s_instance;
  }
  LogStream Log(LogLevel level);
  void LoadImGuiIni(std::string_view ini);
  void LoadImNodesIni(std::string_view ini);
  void SetWindowSize(int width, int height, bool maximize);
  void SaveState();

  const std::shared_ptr<LuaEngine>& Lua() const { return m_lua; }
  int Run();
  bool WriteScene(const std::filesystem::path& path);
  void ClearScene();

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path);
  bool LoadModel(const std::filesystem::path& path);

  void LoadLua(const std::filesystem::path& path);
  bool AddAssetDir(std::string_view name, const std::filesystem::path& path);
};
