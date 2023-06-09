#pragma once
#include <memory>
#include <span>
#include <stdint.h>

namespace libvrm {

struct GltfRoot;

namespace srht {
void
UpdateScene(const std::shared_ptr<GltfRoot>& scene,
            std::span<const uint8_t> data);

// void
// MakeSkeleton(uint32_t skeletonId,
//              const std::shared_ptr<gltf::Node>& root,
//              std::vector<uint8_t>& out);
//
// void
// MakeFrame(uint32_t skeletonId,
//           const std::shared_ptr<gltf::Node>& root,
//           std::vector<uint8_t>& out);

} // namespace
} // namespace
