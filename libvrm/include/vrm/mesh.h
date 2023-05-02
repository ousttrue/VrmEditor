#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <assert.h>
#include <memory>
#include <span>
#include <vector>

namespace libvrm::gltf {

struct Material;
struct Primitive
{
  uint32_t DrawCount;
  std::shared_ptr<Material> Material;
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
  std::string Name;
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
      .DrawCount = static_cast<uint32_t>(values.size()),
      .Material = material,
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

} // namespace gltf
