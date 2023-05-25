#pragma once
#include "docks/imlogger.h"
#include <list>
#include <memory>
#include <unordered_set>
#include <vrm/gltf.h>
#include <vrm/humanoid/humanbones.h>

class FileWatcher;

namespace libvrm {
namespace bvh {
struct Bvh;
}
struct Timeline;
}

namespace grapho {
struct OrbitView;
namespace gl3 {
class Texture;
}
}

namespace gltfjson {
namespace format {
struct Root;
}
}

namespace humanpose {
struct HumanPoseStream;
}

namespace glr {
struct ViewSettings;
struct RenderingEnv;
}

namespace runtimescene {
struct RuntimeScene;
}

class Gui;
struct AssetDir;
class LuaEngine;
class Platform;
class ImLogger;
class Gl3Renderer;
struct JsonGui;

class GltfJsonGui;

class App
{
  std::shared_ptr<FileWatcher> m_watcher;
  std::filesystem::path m_shaderDir;

  std::filesystem::path m_ini;
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::shared_ptr<LuaEngine> m_lua;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<ImLogger> m_logger;

  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<runtimescene::RuntimeScene> m_runtime;
  std::shared_ptr<struct SceneNodeSelection> m_selection;
  std::shared_ptr<grapho::OrbitView> m_staticView;
  std::shared_ptr<grapho::OrbitView> m_runtimeView;
  std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::RenderingEnv> m_env;

  std::shared_ptr<GltfJsonGui> m_gltfjson;
  std::shared_ptr<JsonGui> m_json;

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

  std::shared_ptr<runtimescene::RuntimeScene> SetScene(
    const std::shared_ptr<libvrm::gltf::GltfRoot>& scene);
  LogStream Log(LogLevel level);
  void LoadImGuiIni(std::string_view ini);
  void LoadImNodesIni(std::string_view ini);
  void SetWindowSize(int width, int height, bool maximize);
  void SaveState();

  const std::shared_ptr<LuaEngine>& Lua() const { return m_lua; }
  int Run();
  bool WriteScene(const std::filesystem::path& path);

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path);
  bool LoadModel(const std::filesystem::path& path);
  void LoadPbr(const std::filesystem::path& hdr);
  void LoadLua(const std::filesystem::path& path);
  bool AddAssetDir(std::string_view name, const std::filesystem::path& path);
  void ShowDock(std::string_view name, bool visible);
  void SetShaderDir(const std::filesystem::path& path);
  void OnFileUpdated(const std::filesystem::path& path);
};
