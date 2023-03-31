#pragma once
#include "gltf.h"
#include "humanoid.h"
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

struct Camera;
struct Mesh;
struct MeshInstance;
struct Skin;
struct Node;
struct Animation;
class Image;
class Material;
namespace vrm0 {
struct Vrm;
}
using RenderFunc = std::function<void(const Camera &, const Mesh &,
                                      const MeshInstance &, const float[16])>;

using EnterFunc = std::function<bool(Node &)>;
using LeaveFunc = std::function<void()>;
using EnterJson = std::function<bool(nlohmann::json &, const std::string &key)>;
using LeaveJson = std::function<void()>;

struct Scene {
  Gltf m_gltf;
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;
  std::shared_ptr<vrm0::Vrm> m_vrm0;

  // runtime
  std::shared_ptr<vrm::SpringSolver> m_spring;

  void Clear() {
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
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  std::expected<void, std::string> Load(const std::filesystem::path &path);
  std::expected<void, std::string> Load(const std::filesystem::path &path,
                                        std::span<const uint8_t> json_chunk,
                                        std::span<const uint8_t> bin_chunk);

  void AddIndices(int vertex_offset, Mesh *mesh, int accessor_index,
                  const std::shared_ptr<Material> &material);

  void SyncHierarchy();

  void Render(Time time, const Camera &camera, const RenderFunc &render);
  void Traverse(const EnterFunc &enter, const LeaveFunc &leave,
                Node *node = nullptr);
  void TraverseJson(const EnterJson &enter, const LeaveJson &leave,
                    nlohmann::json *j = nullptr, std::string_view key = {});
  void SetHumanPose(std::span<const vrm::HumanBones> humanMap,
                    const DirectX::XMFLOAT3 &rootPosition,
                    std::span<const DirectX::XMFLOAT4> rotations);
  std::shared_ptr<Node> GetBoneNode(vrm::HumanBones bone);

  std::vector<uint8_t> ToGlb() const;
};
