#pragma once
#include "boundingbox.h"
#include "humanoid/humanbones.h"
#include "node_state.h"
#include <DirectXMath.h>
#include <functional>
#include <gltfjson.h>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace libvrm {

struct Node;
struct HumanSkeleton;

enum class ModelType
{
  Gltf,
  Vrm0,
  Vrm1,
  VrmA,
};

struct GltfRoot
{
  ModelType m_type = ModelType::Gltf;
  std::string m_title = "scene";

  std::vector<uint8_t> m_bytes;
  std::shared_ptr<gltfjson::Root> m_gltf;
  gltfjson::Bin m_bin;

  std::vector<std::shared_ptr<Node>> m_nodes;
  std::shared_ptr<Node> m_selected;
  std::vector<std::shared_ptr<Node>> m_roots;

  std::list<std::function<void(const GltfRoot& scene)>> m_sceneUpdated;
  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::vector<NodeState> m_drawables;

  GltfRoot();
  ~GltfRoot();
  GltfRoot(const GltfRoot&) = delete;
  GltfRoot& operator=(const GltfRoot&) = delete;

  void Clear()
  {
    m_type = {};
    m_nodes.clear();
    m_roots.clear();
    m_gltf = {};
  }

  void RaiseSceneUpdated()
  {
    for (auto& callback : m_sceneUpdated) {
      callback(*this);
    }
  }

  std::tuple<std::shared_ptr<Node>, uint32_t> GetBoneNode(HumanBones bone);
  BoundingBox GetBoundingBox() const;
  void InitializeNodes();
  std::span<NodeState> NodeStates();
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();
  std::shared_ptr<HumanSkeleton> GetHumanSkeleton();
};

} // namespace
