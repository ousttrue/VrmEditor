#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <assert.h>
#include <memory>
#include <span>
#include <vector>

namespace libvrm::gltf {

class Material;
struct Primitive
{
  uint32_t drawCount;
  std::shared_ptr<Material> material;
};

struct MorphVertex
{
  DirectX::XMFLOAT3 position;
};

struct MorphTarget
{
  std::string Name;
  std::vector<MorphVertex> Vertices;

  size_t addPosition(std::span<const DirectX::XMFLOAT3> values)
  {
    auto offset = Vertices.size();
    Vertices.resize(offset + values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      Vertices[offset + i].position = values[i];
    }
    return offset;
  }
};

struct Mesh
{
  uint32_t id;
  Mesh()
  {
    static uint32_t s_id = 0;
    id = ++s_id;
  }
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::vector<Primitive> m_primitives;
  // skinning
  std::vector<JointBinding> m_bindings;
  // morphtarget
  std::vector<std::shared_ptr<MorphTarget>> m_morphTargets;

  size_t verticesBytes() const
  {
    return m_vertices.size() * sizeof(m_vertices[0]);
  }
  size_t indicesBytes() const
  {
    return m_indices.size() * sizeof(m_indices[0]);
  }

  size_t addPosition(std::span<const DirectX::XMFLOAT3> values)
  {
    auto offset = m_vertices.size();
    m_vertices.resize(offset + values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].Position = values[i];
    }
    return offset;
  }

  void setNormal(uint32_t offset, std::span<const DirectX::XMFLOAT3> values)
  {
    assert(offset + values.size() == m_vertices.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].Normal = values[i];
    }
  }

  void setUv(uint32_t offset, std::span<const DirectX::XMFLOAT2> values)
  {
    assert(offset + values.size() == m_vertices.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].Uv = values[i];
    }
  }

  std::shared_ptr<MorphTarget> getOrCreateMorphTarget(int index)
  {
    while (index >= m_morphTargets.size()) {
      m_morphTargets.push_back(std::make_shared<MorphTarget>());
    }
    return m_morphTargets[index];
  }

  void setBoneSkinning(uint32_t offset,
                       std::span<const ushort4> joints,
                       std::span<const DirectX::XMFLOAT4> weights)
  {
    assert(offset + joints.size() == m_vertices.size());
    assert(offset + weights.size() == m_vertices.size());
    m_bindings.resize(m_vertices.size());
    for (size_t i = 0; i < joints.size(); ++i) {
      m_bindings[offset + i] = {
        .Joints = joints[i],
        .Weights = weights[i],
      };
    }
  }

  template<typename T>
  void addSubmesh(uint32_t offset,
                  std::span<const T> values,
                  std::shared_ptr<Material> material)
  {
    m_indices.reserve(m_indices.size() + values.size());
    for (auto value : values) {
      m_indices.push_back(offset + value);
    }

    m_primitives.push_back({
      .drawCount = static_cast<uint32_t>(values.size()),
      .material = material,
    });
  }

  BoundingBox GetBoundingBox() const
  {
    BoundingBox bb;
    for (auto v : m_vertices) {
      bb.Extend(v.Position);
    }
    return bb;
  }
};

struct MeshInstance
{
  // morph targets
  std::vector<float> weights;
  // skinning
  std::vector<Vertex> m_updated;

  MeshInstance(const std::shared_ptr<Mesh>& mesh)
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
    const Mesh& mesh,
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

} // namespace gltf
