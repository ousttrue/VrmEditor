#pragma once
#include "humanoid/humanbones.h"
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

enum class ModelType
{
  Gltf,
  Vrm0,
  Vrm1,
  VrmA,
};

struct DrawItem
{
  DirectX::XMFLOAT4X4 Matrix;
  std::unordered_map<uint32_t, float> MorphMap;
};

struct BoundingBox
{
  DirectX::XMFLOAT3 Min{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
  };
  DirectX::XMFLOAT3 Max{
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
  };

  void Extend(const DirectX::XMFLOAT3& p)
  {
    Min.x = std::min(Min.x, p.x);
    Min.y = std::min(Min.y, p.y);
    Min.z = std::min(Min.z, p.z);
    Max.x = std::max(Max.x, p.x);
    Max.y = std::max(Max.y, p.y);
    Max.z = std::max(Max.z, p.z);
  }
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
  std::list<std::function<void(const GltfRoot& scene)>> m_sceneUpdated;
  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::vector<DrawItem> m_drawables;

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

  std::shared_ptr<Node> GetBoneNode(HumanBones bone);
  BoundingBox GetBoundingBox() const;
  void InitializeNodes();
  std::span<DrawItem> Drawables();
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();
};

} // namespace
