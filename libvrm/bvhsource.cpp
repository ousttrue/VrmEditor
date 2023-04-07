#include "vrm/bvhsource.h"
#include "vrm/bvh.h"
#include "vrm/bvhsolver.h"
#include "vrm/scene.h"

namespace bvh {

MotionSource::MotionSource(const std::shared_ptr<gltf::Scene>& scene)
{
  // pose to scene
  PoseCallbacks.push_back(
    [scene](const vrm::HumanPose& pose) { scene->SetHumanPose(pose); });
}

bool
MotionSource::LoadMotion(const std::filesystem::path& path,
                         float scaling,
                         const std::shared_ptr<Timeline>& timeline)
{
  Motion = Bvh::ParseFile(path);
  if (!Motion) {
    return false;
  }

  MotionSolver = std::make_shared<Solver>();
  MotionSolver->Initialize(Motion);

  HumanBoneMap.clear();

  auto track = timeline->AddTrack("bvh", Motion->Duration());
  track->Callbacks.push_back([this](auto time, bool repeat) {
    auto index = Motion->TimeToIndex(time);
    auto frame = Motion->GetFrame(index);
    MotionSolver->ResolveFrame(frame);

    if (HumanBoneMap.empty()) {
      for (auto& node : MotionSolver->Scene->m_nodes) {
        HumanBoneMap.push_back(node->Humanoid->HumanBone);
      }
    }

    // human pose to scene
    auto& hips = MotionSolver->Instances[0];
    SetHumanPose({ .RootPosition = { hips._41, hips._42, hips._43 },
                   .Bones = HumanBoneMap,
                   .Rotations = MotionSolver->LocalRotations });
  });
  return true;
}

}
