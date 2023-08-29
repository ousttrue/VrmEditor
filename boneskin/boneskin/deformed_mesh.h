#pragma once
#include "base_mesh.h"

namespace boneskin {

struct DeformedMesh
{
  // skinning
  std::vector<Vertex> Vertices;

  void ApplyMorphTarget(const BaseMesh& mesh,
                        const std::unordered_map<uint32_t, float>& morphMap);

  DeformedMesh(const std::shared_ptr<BaseMesh>& mesh)
    : Vertices(mesh->m_vertices)
  {
  }
};

}
