#include "deformed_mesh.h"

namespace boneskin {

void
DeformedMesh::ApplyMorphTarget(
  const BaseMesh& mesh,
  const std::unordered_map<uint32_t, float>& morphMap)
{
  Vertices.assign(mesh.m_vertices.begin(), mesh.m_vertices.end());
  if (morphMap.size()) {
    for (int j = 0; j < mesh.m_morphTargets.size(); ++j) {
      auto& morphtarget = mesh.m_morphTargets[j];
      auto found = morphMap.find(j);
      if (found != morphMap.end()) {
        auto weight = found->second;
        if (weight > 0) {
          for (int i = 0; i < mesh.m_vertices.size(); ++i) {
            // auto v = mesh.m_vertices[i];
            Vertices[i].Position += morphtarget->Vertices[i].position * weight;
          }
        }
      }
    }
  }
}

} // namespace
