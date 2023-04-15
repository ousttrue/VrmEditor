#pragma once
#include "docks/imlogger.h"
#include <list>
#include <memory>
#include <unordered_set>
#include <vrm/humanbone_map.h>
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
class Gui;
struct AssetDir;
class LuaEngine;
class Platform;
class ImLogger;
class Gl3Renderer;
class Cuber;
class UdpReceiver;
struct HumanPoseStream;

class App
{
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::shared_ptr<LuaEngine> m_lua;
  std::list<std::shared_ptr<AssetDir>> m_assets;
  std::shared_ptr<ImLogger> m_logger;

  std::shared_ptr<libvrm::Timeline> m_timeline;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<grapho::OrbitView> m_view;
  std::shared_ptr<Gl3Renderer> m_renderer;

  std::shared_ptr<libvrm::gltf::Scene> m_motion;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<UdpReceiver> m_udp;

  std::shared_ptr<HumanPoseStream> m_pose_stream;

  std::list<std::shared_ptr<libvrm::vrm::HumanBoneMap>> m_humanBoneMapList;

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

  std::shared_ptr<libvrm::vrm::HumanBoneMap> AddHumanBoneMap()
  {
    auto ptr = std::make_shared<libvrm::vrm::HumanBoneMap>();
    m_humanBoneMapList.push_back(ptr);
    return ptr;
  }

  std::shared_ptr<libvrm::vrm::HumanBoneMap> FindHumanBoneMap(
    const libvrm::bvh::Bvh& bvh) const
  {
    for (auto& map : m_humanBoneMapList) {
      if (map->Match(bvh)) {
        return map;
      }
    }
    return {};
  }

  // expose to lua
  const std::shared_ptr<Gui>& GetGui() const { return m_gui; }
  bool LoadPath(const std::filesystem::path& path);
  bool LoadModel(const std::filesystem::path& path);
  void ClearMotion();
  bool LoadMotion(const std::filesystem::path& path);
  void LoadLua(const std::filesystem::path& path);
  bool AddAssetDir(std::string_view name, const std::filesystem::path& path);
};
