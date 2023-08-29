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

static void
SkinningVertex(Vertex* dst,
               // const Vertex& src,
               const DirectX::XMVECTOR& pos,
               const DirectX::XMVECTOR& normal,
               float w,
               std::span<const DirectX::XMFLOAT4X4> matrices,
               uint16_t matrixIndex)
{
  if (w > 0) {
    if (matrixIndex < matrices.size()) {
      DirectX::XMFLOAT3 store;
      {
        auto newPos = DirectX::XMVector3Transform(
          pos, DirectX::XMLoadFloat4x4(&matrices[matrixIndex]));
        DirectX::XMStoreFloat3(&store, newPos);
        dst->Position += (store * w);
      }
      {
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

void
DeformedMesh::ApplySkinning(
  std::span<const JointBinding> bindings,
  std::span<const DirectX::XMFLOAT4X4> skinningMatrices)
{
  if (skinningMatrices.size()) {
    for (int i = 0; i < Vertices.size(); ++i) {
      auto src = Vertices[i];
      auto pos = DirectX::XMLoadFloat3(&src.Position);
      auto normal = DirectX::XMLoadFloat3(&src.Normal);
      auto& dst = Vertices[i];
      dst.Position = { 0, 0, 0 };
      dst.Normal = { 0, 0, 0 };
      auto binding = bindings[i];
      if (auto w = binding.Weights.x)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.X);
      if (auto w = binding.Weights.y)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.Y);
      if (auto w = binding.Weights.z)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.Z);
      if (auto w = binding.Weights.w)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.W);
    }
  }
}
} // namespace
