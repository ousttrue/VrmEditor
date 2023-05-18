#pragma once
// #include <DirectXMath.h>
#include <memory>
#include <span>
#include <stdint.h>
// #include <vector>

namespace libvrm {
namespace gltf {
struct GltfRoot;
}
namespace srht {
void
UpdateScene(const std::shared_ptr<gltf::GltfRoot>& scene,
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
}
}
