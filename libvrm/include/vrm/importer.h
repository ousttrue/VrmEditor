#pragma once
#include "directory.h"
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace libvrm {
namespace gltf {

struct Scene;

std::expected<std::shared_ptr<Scene>, std::string>
LoadPath(const std::filesystem::path& path);

std::expected<bool, std::string>
LoadBytes(const std::shared_ptr<Scene>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<Directory>& dir = nullptr);

// std::expected<bool, std::string> Load(
//   std::span<const uint8_t> json_chunk,
//   std::span<const uint8_t> bin_chunk = {},
//   const std::shared_ptr<Directory>& dir = nullptr);
//
// std::expected<bool, std::string> AddIndices(
//   int vertex_offset,
//   Mesh* mesh,
//   const nlohmann::json& prim,
//   const std::shared_ptr<Material>& material);
//
// std::expected<bool, std::string> Parse();
// std::expected<std::shared_ptr<TextureSampler>, std::string>
// ParseTextureSampler(int i, const nlohmann::json& sampler);
// std::expected<std::shared_ptr<Image>, std::string> ParseImage(
//   int i,
//   const nlohmann::json& image);
// std::expected<std::shared_ptr<Texture>, std::string> ParseTexture(
//   int i,
//   const nlohmann::json& texture);
// std::expected<std::shared_ptr<Material>, std::string> ParseMaterial(
//   int i,
//   const nlohmann::json& material);
// std::expected<std::shared_ptr<Mesh>, std::string> ParseMesh(
//   int i,
//   const nlohmann::json& mesh);
// std::expected<std::shared_ptr<Skin>, std::string> ParseSkin(
//   int i,
//   const nlohmann::json& skin);
// std::expected<std::shared_ptr<Node>, std::string> ParseNode(
//   int i,
//   const nlohmann::json& node);
// std::expected<std::shared_ptr<Animation>, std::string> ParseAnimation(
//   int i,
//   const nlohmann::json& animation);
// std::expected<bool, std::string> ParseVrm0();
// std::expected<bool, std::string> ParseVrm1();

}
}
