#pragma once
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace gltfjson {
struct Directory;
}

namespace libvrm {

struct GltfRoot;

std::shared_ptr<GltfRoot>
LoadPath(const std::filesystem::path& path);

bool
LoadBytes(const std::shared_ptr<GltfRoot>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<gltfjson::Directory>& dir = nullptr);

std::shared_ptr<GltfRoot>
LoadGltf(const std::string& json);

} // namespace
