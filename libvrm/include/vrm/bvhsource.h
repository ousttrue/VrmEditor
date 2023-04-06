#pragma once
#include "vrm/humanbones.h"
#include "vrm/humanpose.h"
#include "vrm/timeline.h"
#include <filesystem>
#include <functional>
#include <memory>

namespace gltf {
struct Scene;
}
namespace bvh {

struct Bvh;
class Solver;

using OnPose = std::function<void(const vrm::HumanPose&)>;

struct MotionSource
{
  std::shared_ptr<Bvh> Motion;
  std::shared_ptr<Solver> MotionSolver;
  std::vector<vrm::HumanBones> HumanBoneMap = {
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
  std::list<OnPose> PoseCallbacks;

  MotionSource(const std::shared_ptr<gltf::Scene>& scene);

  void SetHumanPose(const vrm::HumanPose& pose)
  {
    for (auto& callback : PoseCallbacks) {
      callback(pose);
    }
  }

  bool LoadMotion(const std::filesystem::path& path,
                  float scaling,
                  const std::shared_ptr<Timeline>& timeline);
};
}
