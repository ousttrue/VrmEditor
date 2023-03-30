#pragma once
#include "humanoid.h"
#include "scenetypes.h"
#include "springbone.h"
#include <DirectXMath.h>
#include <chrono>
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
struct Skin;
struct Node;
struct Animation;
class Image;
class Material;
namespace vrm0 {
struct Vrm;
}
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

using EnterFunc = std::function<bool(Node &)>;
using LeaveFunc = std::function<void()>;
using EnterJson = std::function<bool(nlohmann::json &, const std::string &key)>;
using LeaveJson = std::function<void()>;

struct Scene {
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;
  nlohmann::json m_gltf;
  std::shared_ptr<vrm0::Vrm> m_vrm0;
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
  bool Load(const std::filesystem::path &path);

  void AddIndices(int vertex_offset, Mesh *mesh, struct Glb *glb,
                  int accessor_index, int material_index);

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
