#pragma once
#include "boundingbox.h"
#include "humanoid/humanbones.h"
#include <DirectXMath.h>
#include <boneskin/node_state.h>
#include <functional>
#include <gltfjson.h>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace boneskin {
struct BaseMesh;
}

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
  std::vector<std::shared_ptr<Node>> m_roots;
  std::shared_ptr<Node> m_selected;

  std::list<std::function<void(const GltfRoot& scene)>> m_sceneUpdated;
  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::vector<boneskin::NodeState> m_drawables;

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
  std::span<boneskin::NodeState> NodeStates();
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();
  std::shared_ptr<HumanSkeleton> GetHumanSkeleton();
  std::shared_ptr<libvrm::Node> GetSelectedNode() const { return m_selected; }
  void SelectNode(const std::shared_ptr<libvrm::Node>& node);
  bool IsSelected(const std::shared_ptr<libvrm::Node>& node) const;
  std::optional<uint32_t> SelectedIndex() const
  {
    for (uint32_t i = 0; i < m_nodes.size(); ++i) {
      if (m_nodes[i] == m_selected) {
        return i;
      }
    }
    return std::nullopt;
  }

  // Build glTF
  void InitializeGltf();
  uint32_t AddBufferView(uint32_t byteOffset,
                         uint32_t byteSize,
                         uint32_t stride);
  uint32_t AddAccessor(uint32_t bufferViewIndex,
                       uint32_t accessorCount,
                       uint32_t accessorOffset,
                       uint32_t elementType,
                       const char* type);
  uint32_t AddMesh(const std::shared_ptr<boneskin::BaseMesh>& mesh);
  std::shared_ptr<Node> CreateNode(const std::string& name);
};

} // namespace
