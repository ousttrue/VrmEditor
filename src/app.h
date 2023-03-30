#pragma once
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <vrm/humanoid.h>

struct Scene;
struct Bvh;
class BvhSolver;
class Gui;
struct Timeline;
class AssetDir;
class LuaEngine;
class Platform;

class App {
  std::shared_ptr<Platform> m_platform;
  std::shared_ptr<Gui> m_gui;
  std::shared_ptr<LuaEngine> m_lua;
  std::list<std::shared_ptr<AssetDir>> m_assets;

  std::shared_ptr<Timeline> m_timeline;
  std::shared_ptr<Scene> m_scene;

  std::shared_ptr<Bvh> m_motion;
  std::shared_ptr<BvhSolver> m_motionSolver;
  std::vector<vrm::HumanBones> m_humanBoneMap = {
      vrm::HumanBones::hips,          vrm::HumanBones::spine,
      vrm::HumanBones::chest,         vrm::HumanBones::neck,
      vrm::HumanBones::head,          vrm::HumanBones::leftShoulder,
      vrm::HumanBones::leftUpperArm,  vrm::HumanBones::leftLowerArm,
      vrm::HumanBones::leftHand,      vrm::HumanBones::rightShoulder,
      vrm::HumanBones::rightUpperArm, vrm::HumanBones::rightLowerArm,
      vrm::HumanBones::rightHand,     vrm::HumanBones::leftUpperLeg,
      vrm::HumanBones::leftLowerLeg,  vrm::HumanBones::leftFoot,
      vrm::HumanBones::leftToe,       vrm::HumanBones::rightUpperLeg,
      vrm::HumanBones::rightLowerLeg, vrm::HumanBones::rightFoot,
      vrm::HumanBones::rightToe,
  };

  App();

public:
  ~App();
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  static App &instance() {
    static App s_instance;
    return s_instance;
  }
  const std::shared_ptr<LuaEngine> &lua() const { return m_lua; }
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
