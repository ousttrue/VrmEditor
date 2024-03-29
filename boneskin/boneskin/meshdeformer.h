#pragma once
#include "base_mesh.h"
#include "deformed_mesh.h"
#include "node_state.h"
#include "skin.h"
#include <DirectXMath.h>
#include <memory>
#include <span>
#include <stdint.h>

namespace boneskin {

struct NodeMesh
{
  uint32_t NodeIndex;
  uint32_t MeshIndex;
  DirectX::XMFLOAT4X4 Matrix;
};

class MeshDeformer
{
  std::unordered_map<uint32_t, std::shared_ptr<BaseMesh>> m_baseMap;
  std::unordered_map<uint32_t, std::shared_ptr<DeformedMesh>> m_deformMap;
  std::unordered_map<uint32_t, std::shared_ptr<Skin>> m_skinMap;
  std::vector<NodeMesh> m_meshNodes;

public:
  MeshDeformer();
  MeshDeformer(const MeshDeformer&) = delete;
  MeshDeformer& operator=(const MeshDeformer&) = delete;

  void Release()
  {
    m_baseMap.clear();
    m_deformMap.clear();
    m_skinMap.clear();
  }

  void PushBaseMesh(const std::shared_ptr<BaseMesh>& mesh)
  {
    m_baseMap.insert({ (uint32_t)m_baseMap.size(), mesh });
  }

  std::shared_ptr<BaseMesh> GetOrCreateBaseMesh(const gltfjson::Root& root,
                                                const gltfjson::Bin& bin,
                                                std::optional<uint32_t> mesh);

  std::shared_ptr<DeformedMesh> GetOrCreateDeformedMesh(
    uint32_t mesh,
    const std::shared_ptr<BaseMesh>& baseMesh);

  std::shared_ptr<Skin> GetOrCreaeSkin(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       std::optional<uint32_t> skinId);

  std::span<const NodeMesh> ProcessSkin(const gltfjson::Root& root,
                                        const gltfjson::Bin& bin,
                                        std::span<const NodeState> drawables);
};

} // namespace
