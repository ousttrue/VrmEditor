#pragma once
#include "vrm/bvh.h"
#include "vrm/bvhframe.h"
#include "vrm/humanbone_map.h"
#include "vrm/node.h"
#include "vrm/runtimescene/node.h"
#include "vrm/runtimescene/scene.h"
#include "vrm/gltfroot.h"
#include "vrm/timeline.h"
#include <memory>

namespace libvrm {
namespace bvh {

void
UpdateSceneFromBvhFrame(
  const std::shared_ptr<runtimescene::RuntimeScene>& scene,
  std::shared_ptr<runtimescene::RuntimeNode>& node,
  const std::shared_ptr<bvh::Bvh>& bvh,
  const bvh::Frame& frame,
  float scaling);

void
UpdateSceneFromBvhFrame(
  const std::shared_ptr<runtimescene::RuntimeScene>& scene,
  const std::shared_ptr<bvh::Bvh>& bvh,
  Time time);

void
InitializeSceneFromBvh(const std::shared_ptr<gltf::GltfRoot>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<vrm::HumanBoneMap>& map = {});

}
}
