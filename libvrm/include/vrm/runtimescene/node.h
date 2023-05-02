#pragma once
#include <vrm/mesh.h>

namespace runtimescene {

struct MeshInstance
{
  // morph targets
  std::vector<float> weights;
  // skinning
  std::vector<libvrm::Vertex> m_updated;

  MeshInstance(const std::shared_ptr<libvrm::gltf::Mesh>& mesh)
    : weights(mesh->m_morphTargets.size())
    , m_updated(mesh->m_vertices)
  {
  }

  void applySkinning(DirectX::XMFLOAT3* dst,
                     const DirectX::XMFLOAT3& src,
                     float w,
                     const DirectX::XMFLOAT4X4& m)
  {
    if (w > 0) {
      auto pos = DirectX::XMLoadFloat3(&src);
      auto newPos =
        DirectX::XMVector3Transform(pos, DirectX::XMLoadFloat4x4(&m));
      DirectX::XMFLOAT3 store;
      DirectX::XMStoreFloat3(&store, newPos);
      *dst += (store * w);
    }
  }

  void applyMorphTargetAndSkinning(
    const libvrm::gltf::Mesh& mesh,
    std::span<DirectX::XMFLOAT4X4> skinningMatrices)
  {
    // clear & apply morph target
    m_updated.clear();
    for (int i = 0; i < mesh.m_vertices.size(); ++i) {
      auto v = mesh.m_vertices[i];
      for (int j = 0; j < weights.size(); ++j) {
        auto& morphtarget = mesh.m_morphTargets[j];
        if (weights[j]) {
          v.Position += morphtarget->Vertices[i].position * weights[j];
        }
      }
      m_updated.push_back(v);
    }

    // calc skinning
    if (skinningMatrices.size()) {
      for (int i = 0; i < mesh.m_vertices.size(); ++i) {
        auto src = m_updated[i];
        auto& dst = m_updated[i];
        dst.Position = { 0, 0, 0 };
        dst.Normal = { 0, 0, 0 };
        auto binding = mesh.m_bindings[i];
        if (auto w = binding.Weights.x)
          applySkinning(
            &dst.Position, src.Position, w, skinningMatrices[binding.Joints.X]);
        if (auto w = binding.Weights.y)
          applySkinning(
            &dst.Position, src.Position, w, skinningMatrices[binding.Joints.Y]);
        if (auto w = binding.Weights.z)
          applySkinning(
            &dst.Position, src.Position, w, skinningMatrices[binding.Joints.Z]);
        if (auto w = binding.Weights.w)
          applySkinning(
            &dst.Position, src.Position, w, skinningMatrices[binding.Joints.W]);
      }
    }
  }
};

struct RuntimeNode
{
  std::shared_ptr<MeshInstance> MeshInstance;
};

}
