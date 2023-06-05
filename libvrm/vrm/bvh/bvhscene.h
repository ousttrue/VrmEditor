#pragma once
#include "../animation/runtime_node.h"
#include "../animation/runtime_scene.h"
#include "../animation/timeline.h"
#include "../gltfroot.h"
#include "../node.h"
#include "bvh.h"
#include "bvhframe.h"
#include "humanbone_map.h"
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
InitializeSceneFromBvh(const std::shared_ptr<GltfRoot>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<HumanBoneMap>& map = {});

}
}
