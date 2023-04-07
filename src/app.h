#pragma once
#include "docks/imlogger.h"
#include <list>
#include <memory>
#include <vrm/humanbones.h>

namespace gltf {
struct Scene;
}
namespace bvh {
struct Bvh;
}
class Gui;
struct Timeline;
struct AssetDir;
class LuaEngine;
class Platform;
class OrbitView;
class ImLogger;
class Gl3Renderer;
class Cuber;

class App
{
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::shared_ptr<LuaEngine> m_lua;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<ImLogger> m_logger;

  std::shared_ptr<Timeline> m_timeline;
  std::shared_ptr<gltf::Scene> m_scene;
  std::shared_ptr<OrbitView> m_view;
  std::shared_ptr<Gl3Renderer> m_renderer;

  std::shared_ptr<gltf::Scene> m_motion;
  std::shared_ptr<Cuber> m_cuber;

  struct HumanBoneMap
  {
    std::unordered_map<std::string, vrm::HumanBones> NameBoneMap;
    void Add(std::string_view joint_name, std::string_view bone);
  };
  std::list<std::shared_ptr<HumanBoneMap>> m_humanBoneMapList;

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
  LogStream Log(LogLevel level);

  const std::shared_ptr<LuaEngine>& Lua() const { return m_lua; }
  int Run();
  bool WriteScene(const std::filesystem::path& path);
  void ClearScene();

  std::shared_ptr<HumanBoneMap> FindHumanBoneMap(const bvh::Bvh& bvh) const;

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path);
  bool LoadModel(const std::filesystem::path& path);
  bool LoadMotion(const std::filesystem::path& path, float scaling = 1.0f);
  void LoadLua(const std::filesystem::path& path);
  bool AddAssetDir(std::string_view name, const std::filesystem::path& path);
  std::shared_ptr<HumanBoneMap> AddHumanBoneMap();
};
