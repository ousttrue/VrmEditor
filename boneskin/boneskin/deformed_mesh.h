#pragma once
#include "base_mesh.h"

namespace boneskin {

struct DeformedMesh
{
  // skinning
  std::vector<Vertex> Vertices;

  DeformedMesh(const std::shared_ptr<BaseMesh>& mesh)
    : Vertices(mesh->m_vertices)
  {
  }

  void ApplyMorphTarget(const BaseMesh& mesh,
                        const std::unordered_map<uint32_t, float>& morphMap);

  void ApplySkinning(std::span<const JointBinding> bindings,
                     std::span<const DirectX::XMFLOAT4X4> skinningMatrices);

};

} // namespace
