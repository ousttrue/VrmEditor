#pragma once
#include "../runtime_node.h"
#include "../runtime_scene.h"
#include "../timeline.h"
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
  const std::shared_ptr<RuntimeScene>& scene,
  std::shared_ptr<RuntimeNode>& node,
  const std::shared_ptr<bvh::Bvh>& bvh,
  const bvh::Frame& frame,
  float scaling);

void
UpdateSceneFromBvhFrame(
  const std::shared_ptr<RuntimeScene>& scene,
  const std::shared_ptr<bvh::Bvh>& bvh,
  Time time);

void
InitializeSceneFromBvh(const std::shared_ptr<GltfRoot>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<HumanBoneMap>& map = {});

}
}
