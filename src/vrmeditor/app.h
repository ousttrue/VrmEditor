#pragma once
#include "docks/imlogger.h"
#include <list>
#include <memory>
#include <unordered_set>
#include <vrm/gltfroot.h>
#include <vrm/humanoid/humanbones.h>

class FileWatcher;
class HierarchyGui;

namespace libvrm {
namespace bvh {
struct Bvh;
}

struct RuntimeScene;
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
class Gl3RendererGui;
}

class Gui;
struct AssetDir;
class LuaEngine;
class Platform;
struct JsonGui;

class GltfJsonGui;

class App
{
  std::filesystem::path m_ini;
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<HierarchyGui> m_hierarchy;

  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<struct SceneNodeSelection> m_selection;
  std::shared_ptr<grapho::OrbitView> m_staticView;
  std::shared_ptr<grapho::OrbitView> m_runtimeView;
  std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::RenderingEnv> m_env;

  std::shared_ptr<JsonGui> m_json;
  std::shared_ptr<glr::Gl3RendererGui> m_gl3gui;

  App();

public:
  ~App();
  App(const App&) = delete;
  App& operator=(const App&) = delete;
  static App& Instance()
  {
    static App s_instance;
    return s_instance;
  }

  void ProjectMode();

  std::shared_ptr<libvrm::RuntimeScene> SetScene(
    const std::shared_ptr<libvrm::GltfRoot>& scene);
  void LoadImGuiIni(std::string_view ini);
  void LoadImNodesIni(std::string_view ini);
  void SetWindowSize(int width, int height, bool maximize);
  void SaveState();

  int Run();
  bool WriteScene(const std::filesystem::path& path);

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path);
  bool LoadModel(const std::filesystem::path& path);
  bool LoadPbr(const std::filesystem::path& hdr);
  void LoadLua(const std::filesystem::path& path);
  bool AddAssetDir(std::string_view name, const std::filesystem::path& path);
  void ShowDock(std::string_view name, bool visible);
  void SetShaderDir(const std::filesystem::path& path);
  void OnFileUpdated(const std::filesystem::path& path);
  void SetShaderChunkDir(const std::filesystem::path& path);
};
