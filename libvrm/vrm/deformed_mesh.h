#pragma once
#include "base_mesh.h"

namespace libvrm {

struct DeformedMesh
{
  // skinning
  std::vector<Vertex> Vertices;

  DeformedMesh(const std::shared_ptr<BaseMesh>& mesh)
    : Vertices(mesh->m_vertices)
  {
  }

  void ApplySkinning(Vertex* dst,
                     const Vertex& src,
                     float w,
                     std::span<const DirectX::XMFLOAT4X4> matrices,
                     uint16_t matrixIndex)
  {
    if (w > 0) {
      if (matrixIndex < matrices.size()) {
        DirectX::XMFLOAT3 store;
        {
          auto pos = DirectX::XMLoadFloat3(&src.Position);
          auto newPos = DirectX::XMVector3Transform(
            pos, DirectX::XMLoadFloat4x4(&matrices[matrixIndex]));
          DirectX::XMStoreFloat3(&store, newPos);
          dst->Position += (store * w);
        }
        {
          auto normal = DirectX::XMLoadFloat3(&src.Normal);
          auto newNormal = DirectX::XMVector3Transform(
            normal, DirectX::XMLoadFloat4x4(&matrices[matrixIndex]));
          DirectX::XMStoreFloat3(&store, newNormal);
          dst->Normal += (store * w);
        }
      } else {
        // error
      }
    }
  }

  void ApplyMorphTargetAndSkinning(
    const BaseMesh& mesh,
    const std::unordered_map<uint32_t, float>& morphMap,
    std::span<const DirectX::XMFLOAT4X4> skinningMatrices)
  {
    // clear & apply morph target
    Vertices.clear();
    for (int i = 0; i < mesh.m_vertices.size(); ++i) {
      auto v = mesh.m_vertices[i];
      for (int j = 0; j < mesh.m_morphTargets.size(); ++j) {
        auto& morphtarget = mesh.m_morphTargets[j];
        auto found = morphMap.find(j);
        if (found != morphMap.end()) {
          v.Position += morphtarget->Vertices[i].position * found->second;
        }
      }
      Vertices.push_back(v);
    }

    // calc skinning
    if (skinningMatrices.size()) {
      for (int i = 0; i < mesh.m_vertices.size(); ++i) {
        auto src = Vertices[i];
        auto& dst = Vertices[i];
        dst.Position = { 0, 0, 0 };
        dst.Normal = { 0, 0, 0 };
        auto binding = mesh.m_bindings[i];
        if (auto w = binding.Weights.x)
          ApplySkinning(&dst, src, w, skinningMatrices, binding.Joints.X);
        if (auto w = binding.Weights.y)
          ApplySkinning(&dst, src, w, skinningMatrices, binding.Joints.Y);
        if (auto w = binding.Weights.z)
          ApplySkinning(&dst, src, w, skinningMatrices, binding.Joints.Z);
        if (auto w = binding.Weights.w)
          ApplySkinning(&dst, src, w, skinningMatrices, binding.Joints.W);
      }
    }
  }
};

}
