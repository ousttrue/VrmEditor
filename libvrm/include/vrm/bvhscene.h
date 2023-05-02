#pragma once
#include "vrm/bvh.h"
#include "vrm/bvhframe.h"
#include "vrm/humanbone_map.h"
#include "vrm/node.h"
#include "vrm/scene.h"
#include "vrm/timeline.h"
#include "vrm/runtimescene/node.h"
#include <memory>

namespace libvrm::bvh {
void
UpdateSceneFromBvhFrame(const std::shared_ptr<gltf::Scene>& scene,
                        std::shared_ptr<gltf::Node>& node,
                        const std::shared_ptr<bvh::Bvh>& bvh,
                        const bvh::Frame& frame,
                        float scaling);

void
UpdateSceneFromBvhFrame(const std::shared_ptr<gltf::Scene>& scene,
                        const std::shared_ptr<bvh::Bvh>& bvh,
                        Time time);

void
InitializeSceneFromBvh(const std::shared_ptr<gltf::Scene>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<vrm::HumanBoneMap>& map = {});

}
