#pragma once
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace gltfjson {
struct Directory;
}

namespace libvrm {

struct GltfRoot;

std::expected<std::shared_ptr<GltfRoot>, std::string>
LoadPath(const std::filesystem::path& path);

std::expected<bool, std::string>
LoadBytes(const std::shared_ptr<GltfRoot>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<gltfjson::Directory>& dir = nullptr);

std::expected<std::shared_ptr<GltfRoot>, std::string>
LoadGltf(const std::string& json);

} // namespace
