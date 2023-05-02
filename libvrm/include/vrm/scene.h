#pragma once
#include "expression.h"
#include "gizmo.h"
#include "gltf.h"
#include "humanpose.h"
#include "mesh.h"
#include "scenetypes.h"
#include "springbone.h"
#include "springcollider.h"
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

namespace libvrm {
namespace bvh {
struct Bvh;
struct Joint;
struct Frame;
}
namespace vrm {
namespace animation {
struct Animation;
}
}

namespace gltf {
struct Mesh;
struct MeshInstance;
struct Skin;
struct Node;
struct Animation;
class Image;
struct TextureSampler;
struct Texture;
struct Material;

using EnterFunc = std::function<bool(const std::shared_ptr<Node>&)>;
using LeaveFunc = std::function<void()>;
using EnterJson =
  std::function<bool(nlohmann::json&, std::string_view jsonpath)>;
using LeaveJson = std::function<void()>;

template<typename T>
inline std::optional<size_t>
_IndexOf(std::span<const T> values, const T& target)
{
  for (size_t i = 0; i < values.size(); ++i) {
    if (values[i] == target) {
      return i;
    }
  }
  return {};
}

enum class ModelType
{
  Gltf,
  Vrm0,
  Vrm1,
};

struct Scene
{
  ModelType m_type = ModelType::Gltf;
  std::vector<uint8_t> m_bytes;
  Gltf m_gltf;
  std::string m_title = "scene";
  std::vector<std::shared_ptr<TextureSampler>> m_samplers;
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Texture>> m_textures;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;

  // extensions
  std::shared_ptr<vrm::animation::Animation> m_vrma;
  std::shared_ptr<vrm::Expressions> m_expressions;

  // spring
  std::vector<std::shared_ptr<vrm::SpringCollider>> m_springColliders;
  std::vector<std::shared_ptr<vrm::SpringColliderGroup>> m_springColliderGroups;
  std::vector<std::shared_ptr<vrm::SpringBone>> m_springBones;

  std::unordered_map<vrm::MorphTargetKey, float> m_morphTargetMap;

  std::list<std::function<void(const Scene& scene)>> m_sceneUpdated;

  // humanpose
  std::vector<vrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;
  vrm::HumanPose m_pose;

  Scene();
  ~Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  void Clear()
  {
    m_type = {};
    m_samplers.clear();
    m_images.clear();
    m_textures.clear();
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_roots.clear();
    m_skins.clear();
    m_animations.clear();

    m_springColliders.clear();
    m_springColliderGroups.clear();
    m_springBones.clear();

    m_morphTargetMap.clear();

    m_gltf = {};
    m_vrma = {};
    m_expressions = {};
  }

  std::optional<size_t> IndexOf(
    const std::shared_ptr<TextureSampler>& sampler) const
  {
    return _IndexOf<std::shared_ptr<TextureSampler>>(m_samplers, sampler);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Image>& image) const
  {
    return _IndexOf<std::shared_ptr<Image>>(m_images, image);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Texture>& texture) const
  {
    return _IndexOf<std::shared_ptr<Texture>>(m_textures, texture);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Material>& material) const
  {
    return _IndexOf<std::shared_ptr<Material>>(m_materials, material);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Mesh>& mesh) const
  {
    return _IndexOf<std::shared_ptr<Mesh>>(m_meshes, mesh);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Skin>& skin) const
  {
    return _IndexOf<std::shared_ptr<Skin>>(m_skins, skin);
  }
  std::optional<size_t> IndexOf(const std::shared_ptr<Node>& node) const
  {
    return _IndexOf<std::shared_ptr<Node>>(m_nodes, node);
  }

  void RaiseSceneUpdated()
  {
    for (auto& callback : m_sceneUpdated) {
      callback(*this);
    }
  }

  int GetNodeIndex(const std::shared_ptr<gltf::Node>& node) const
  {
    for (int i = 0; i < m_nodes.size(); ++i) {
      if (m_nodes[i] == node) {
        return i;
      }
    }
    return -1;
  }

  void SyncHierarchy();

  void Traverse(const EnterFunc& enter,
                const LeaveFunc& leave,
                const std::shared_ptr<Node>& node = nullptr);

  std::string m_jsonpath;
  void TraverseJson(const EnterJson& enter,
                    const LeaveJson& leave,
                    nlohmann::json* j = nullptr);

  vrm::HumanPose UpdateHumanPose();
  void SetHumanPose(const vrm::HumanPose& pose);
  std::shared_ptr<Node> GetBoneNode(vrm::HumanBones bone);

  BoundingBox GetBoundingBox() const;

  void InitializeNodes()
  {
    for (auto& root : m_roots) {
      root->CalcWorldMatrix(true);
      for (auto& node : m_nodes) {
        node->InitialTransform = node->Transform;
        node->WorldInitialTransform = node->WorldTransform;
      }
      root->CalcShape();
    }
  }

  void SetInitialPose()
  {
    for (auto& node : m_nodes) {
      node->Transform = node->InitialTransform;
    }
    for (auto& root : m_roots) {
      root->CalcWorldMatrix(true);
    }
    RaiseSceneUpdated();
  }

  void DrawGizmo(IGizmoDrawer* gizmo);
};

} // gltf
}
