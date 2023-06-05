#pragma once
#include "expression.h"
#include "spring_bone.h"
#include "spring_collider.h"
#include "base_mesh.h"
#include <DirectXMath.h>
#include <functional>
#include <gltfjson.h>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace libvrm {

namespace vrm {
namespace animation {
struct Animation;
}
}

struct Node;

using EnterFunc = std::function<bool(const std::shared_ptr<Node>&)>;
using LeaveFunc = std::function<void()>;

enum class ModelType
{
  Gltf,
  Vrm0,
  Vrm1,
};

struct DrawItem
{
  uint32_t Mesh;
  DirectX::XMFLOAT4X4 Matrix;
};

struct GltfRoot
{
  ModelType m_type = ModelType::Gltf;
  std::vector<uint8_t> m_bytes;
  std::shared_ptr<gltfjson::Root> m_gltf;
  gltfjson::Bin m_bin;
  std::string m_title = "scene";
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;

  // extensions
  // std::shared_ptr<vrm::animation::Animation> m_vrma;
  std::shared_ptr<Expressions> m_expressions;

  // spring
  gltfjson::tree::NodePtr m_vrm0Materials;
  std::vector<std::shared_ptr<SpringCollider>> m_springColliders;
  std::vector<std::shared_ptr<SpringColliderGroup>> m_springColliderGroups;
  std::vector<std::shared_ptr<SpringBone>> m_springBones;

  std::unordered_map<MorphTargetKey, float> m_morphTargetMap;

  std::list<std::function<void(const GltfRoot& scene)>> m_sceneUpdated;

  std::vector<DrawItem> m_drawables;
  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;

  GltfRoot();
  ~GltfRoot();
  GltfRoot(const GltfRoot&) = delete;
  GltfRoot& operator=(const GltfRoot&) = delete;

  void Clear()
  {
    m_type = {};
    m_nodes.clear();
    m_roots.clear();

    m_springColliders.clear();
    m_springColliderGroups.clear();
    m_springBones.clear();

    m_morphTargetMap.clear();

    m_gltf = {};
    // m_vrma = {};
    m_expressions = {};
  }

  gltfjson::tree::ArrayValue* Vrm0Materials() const
  {
    if (m_vrm0Materials) {
      return m_vrm0Materials->Array();
    }
    return nullptr;
  }

  void RaiseSceneUpdated()
  {
    for (auto& callback : m_sceneUpdated) {
      callback(*this);
    }
  }

  void Traverse(const EnterFunc& enter,
                const LeaveFunc& leave,
                const std::shared_ptr<Node>& node = nullptr);

  std::shared_ptr<Node> GetBoneNode(HumanBones bone);

  BoundingBox GetBoundingBox() const;

  void InitializeNodes()
  {
    for (auto& root : m_roots) {
      root->CalcWorldInitialMatrix(true);
      root->CalcShape();
    }
  }

  std::span<const DrawItem> Drawables();
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();
};

} // namespace
