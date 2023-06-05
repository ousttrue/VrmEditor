#pragma once
#include "base_mesh.h"

namespace runtimescene {

struct DeformedMesh
{
  // morph targets
  std::vector<float> Weights;
  // skinning
  std::vector<Vertex> Vertices;

  DeformedMesh(const std::shared_ptr<BaseMesh>& mesh)
    : Weights(mesh->m_morphTargets.size())
    , Vertices(mesh->m_vertices)
  {
  }

  void applySkinning(Vertex* dst,
                     const Vertex& src,
                     float w,
                     std::span<DirectX::XMFLOAT4X4> matrices,
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

  void applyMorphTargetAndSkinning(
    const BaseMesh& mesh,
    std::span<DirectX::XMFLOAT4X4> skinningMatrices)
  {
    // clear & apply morph target
    Vertices.clear();
    for (int i = 0; i < mesh.m_vertices.size(); ++i) {
      auto v = mesh.m_vertices[i];
      for (int j = 0; j < Weights.size(); ++j) {
        auto& morphtarget = mesh.m_morphTargets[j];
        if (Weights[j]) {
          v.Position += morphtarget->Vertices[i].position * Weights[j];
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
          applySkinning(&dst, src, w, skinningMatrices, binding.Joints.X);
        if (auto w = binding.Weights.y)
          applySkinning(&dst, src, w, skinningMatrices, binding.Joints.Y);
        if (auto w = binding.Weights.z)
          applySkinning(&dst, src, w, skinningMatrices, binding.Joints.Z);
        if (auto w = binding.Weights.w)
          applySkinning(&dst, src, w, skinningMatrices, binding.Joints.W);
      }
    }
  }
};

}
