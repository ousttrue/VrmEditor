#pragma once
#include "expression.h"
#include "gizmo.h"
#include "humanpose.h"
#include "runtimescene/base_mesh.h"
#include "scenetypes.h"
#include "springbone.h"
#include "springcollider.h"
#include <DirectXMath.h>
#include <chrono>
#include <expected>
#include <filesystem>
#include <functional>
#include <gltfjson/bin.h>
#include <gltfjson/gltf.h>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace libvrm {

namespace vrm {
namespace animation {
struct Animation;
}
}

namespace gltf {
struct Skin;
struct Node;
struct Animation;

using EnterFunc = std::function<bool(const std::shared_ptr<Node>&)>;
using LeaveFunc = std::function<void()>;

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

struct DrawItem
{
  uint32_t Mesh;
  DirectX::XMFLOAT4X4 Matrix;
};

struct GltfRoot
{
  ModelType m_type = ModelType::Gltf;
  std::vector<uint8_t> m_bytes;
  gltfjson::format::Root m_gltf;
  gltfjson::format::Bin m_bin;
  std::string m_title = "scene";
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

  void Traverse(const EnterFunc& enter,
                const LeaveFunc& leave,
                const std::shared_ptr<Node>& node = nullptr);

  std::string m_jsonpath;
  // void TraverseJson(const EnterJson& enter,
  //                   const LeaveJson& leave,
  //                   nlohmann::json* j = nullptr);

  std::shared_ptr<Node> GetBoneNode(vrm::HumanBones bone);

  runtimescene::BoundingBox GetBoundingBox() const;

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

}
}
