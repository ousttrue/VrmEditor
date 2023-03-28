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
class Timeline;
class AssetDir;
class LuaEngine;
class Platform;

class App {
  std::shared_ptr<LuaEngine> lua_;
  std::shared_ptr<Scene> scene_;
  std::list<std::shared_ptr<AssetDir>> assets_;

  std::shared_ptr<Timeline> timeline_;

  std::shared_ptr<Bvh> motion_;
  std::shared_ptr<BvhSolver> motionSolver_;
  std::vector<vrm::HumanBones> humanBoneMap_ = {
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

  std::shared_ptr<Platform> platform_;
  std::shared_ptr<Gui> gui_;

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
