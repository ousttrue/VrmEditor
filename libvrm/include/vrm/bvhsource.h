#pragma once
#include "vrm/humanbones.h"
#include "vrm/humanpose.h"
#include "vrm/timeline.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>

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
  std::vector<vrm::HumanBones> HumanBoneMap;
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
