#pragma once
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace libvrm {
struct Directory;

namespace gltf {
struct Scene;

std::expected<std::shared_ptr<Scene>, std::string>
LoadPath(const std::filesystem::path& path);

std::expected<bool, std::string>
LoadBytes(const std::shared_ptr<Scene>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<Directory>& dir = nullptr);

}
}
