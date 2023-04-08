#pragma once
#include "vrm/bvh.h"
#include "vrm/bvhframe.h"
#include "vrm/node.h"
#include "vrm/scene.h"
#include "vrm/timeline.h"
#include <memory>

namespace bvh {
void
ResolveFrame(const std::shared_ptr<gltf::Scene>& scene,
             const std::shared_ptr<bvh::Bvh>& bvh,
             Time time);

void
SetBvh(const std::shared_ptr<gltf::Scene>& scene,
       const std::shared_ptr<bvh::Bvh>& bvh,
       std::vector<DirectX::XMFLOAT4X4>& instances);

}
