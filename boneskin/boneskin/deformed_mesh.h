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
};

}
