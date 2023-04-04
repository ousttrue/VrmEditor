#pragma once
#include "gltf.h"
#include "humanoid.h"
#include "humanpose.h"
#include "scenetypes.h"
#include "springbone.h"
#include <DirectXMath.h>
#include <chrono>
#include <expected>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace vrm {
namespace v0 {
struct Vrm;
}
namespace v1 {
struct Vrm;
}
}

namespace gltf {
struct Mesh;
struct MeshInstance;
struct Skin;
struct Node;
struct Animation;
class Image;
class Material;
using RenderFunc =
  std::function<void(const Mesh&, const MeshInstance&, const float[16])>;

using EnterFunc = std::function<bool(Node&)>;
using LeaveFunc = std::function<void()>;
using EnterJson = std::function<bool(nlohmann::json&, const std::string& key)>;
using LeaveJson = std::function<void()>;

struct Scene
{
  Gltf m_gltf;
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;

  vrm::Humanoid m_humanoid;
  std::shared_ptr<vrm::v0::Vrm> m_vrm0;
  std::shared_ptr<vrm::v1::Vrm> m_vrm1;

  // runtime
  std::shared_ptr<vrm::SpringSolver> m_spring;

  void Clear()
  {
    m_vrm0 = nullptr;
    m_images.clear();
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_roots.clear();
    m_skins.clear();
    m_animations.clear();
    m_gltf = {};
  }

  Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  std::expected<bool, std::string> LoadPath(const std::filesystem::path& path);
  std::expected<bool, std::string> Load(
    std::span<const uint8_t> json_chunk,
    std::span<const uint8_t> bin_chunk = {},
    const std::shared_ptr<Directory>& dir = nullptr);

  std::expected<bool, std::string> AddIndices(
    int vertex_offset,
    Mesh* mesh,
    const nlohmann::json& prim,
    const std::shared_ptr<Material>& material);

  void SyncHierarchy();

  void Render(Time time, const RenderFunc& render);
  void Traverse(const EnterFunc& enter,
                const LeaveFunc& leave,
                Node* node = nullptr);
  void TraverseJson(const EnterJson& enter,
                    const LeaveJson& leave,
                    nlohmann::json* j = nullptr,
                    std::string_view key = {});
  void SetHumanPose(const vrm::HumanPose& pose);
  std::shared_ptr<Node> GetBoneNode(vrm::HumanBones bone);

  std::vector<uint8_t> ToGlb() const;

  BoundingBox GetBoundingBox() const;

private:
  std::expected<bool, std::string> Parse();
  std::expected<std::shared_ptr<Image>, std::string> ParseImage(
    int i,
    const nlohmann::json& image);
  std::expected<std::shared_ptr<Material>, std::string> ParseMaterial(
    int i,
    const nlohmann::json& material);
  std::expected<std::shared_ptr<Mesh>, std::string> ParseMesh(
    int i,
    const nlohmann::json& mesh);
  std::expected<std::shared_ptr<Skin>, std::string> ParseSkin(
    int i,
    const nlohmann::json& skin);
  std::expected<std::shared_ptr<Node>, std::string> ParseNode(
    int i,
    const nlohmann::json& node);
  std::expected<std::shared_ptr<Animation>, std::string> ParseAnimation(
    int i,
    const nlohmann::json& animation);
  std::expected<std::shared_ptr<vrm::v0::Vrm>, std::string> ParseVrm0();
  std::expected<std::shared_ptr<vrm::v1::Vrm>, std::string> ParseVrm1();
};
}
