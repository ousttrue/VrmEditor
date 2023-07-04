#pragma once
#include "types.h"
#include <DirectXMath.h>
#include <assert.h>
#include <gltfjson.h>
#include <memory>
#include <span>
#include <vector>

namespace boneskin {

inline DirectX::XMFLOAT3&
operator+=(DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.z += rhs.z;
  return lhs;
}
inline DirectX::XMFLOAT3
operator+(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}
inline DirectX::XMFLOAT3
operator-(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}
inline DirectX::XMFLOAT3
operator*(const DirectX::XMFLOAT3& lhs, float rhs)
{
  return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

struct Primitive
{
  uint32_t DrawCount;
  std::optional<uint32_t> Material;
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

struct Vertex
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT3 Normal;
  DirectX::XMFLOAT2 Uv;
};
static_assert(sizeof(Vertex) == 32, "sizeof(Vertex)");

struct JointBinding
{
  ushort4 Joints;
  DirectX::XMFLOAT4 Weights;
};

struct BaseMesh
{
  uint32_t id;
  std::u8string Name;
  BaseMesh()
  {
    static uint32_t s_id = 0;
    id = ++s_id;
  }
  BaseMesh(const BaseMesh&) = delete;
  BaseMesh& operator=(const BaseMesh&) = delete;

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

  size_t AddPosition(const gltfjson::MemoryBlock& block)
  {
    auto offset = m_vertices.size();
    if (block.Stride == 12) {
      m_vertices.resize(m_vertices.size() + block.ItemCount);
      auto p = block.Span.data();
      for (size_t i = 0; i < block.ItemCount; ++i, p += block.Stride) {
        m_vertices[offset + i].Position = *((const DirectX::XMFLOAT3*)p);
      }
    } else {
      assert(false);
    }
    return offset;
  }

  void SetNormal(uint32_t offset, const gltfjson::MemoryBlock& block)
  {
    assert(offset + block.ItemCount == m_vertices.size());
    auto p = block.Span.data();
    for (size_t i = 0; i < block.ItemCount; ++i, p += block.Stride) {
      m_vertices[offset + i].Normal = *((const DirectX::XMFLOAT3*)p);
    }
  }

  void SetUv(uint32_t offset, const gltfjson::MemoryBlock& block)
  {
    assert(offset + block.ItemCount == m_vertices.size());
    auto p = block.Span.data();
    for (size_t i = 0; i < block.ItemCount; ++i, p += block.Stride) {
      m_vertices[offset + i].Uv = *((const DirectX::XMFLOAT2*)p);
    }
  }

  std::shared_ptr<MorphTarget> getOrCreateMorphTarget(int index)
  {
    while (index >= m_morphTargets.size()) {
      m_morphTargets.push_back(std::make_shared<MorphTarget>());
    }
    return m_morphTargets[index];
  }

  // T=byte4 or ushort4
  template<typename T>
  void SetBoneSkinning(uint32_t offset,
                       const gltfjson::MemoryBlock& joints,
                       const gltfjson::MemoryBlock& weights)
  {
    assert(offset + joints.ItemCount == m_vertices.size());
    assert(offset + weights.ItemCount == m_vertices.size());
    m_bindings.resize(m_vertices.size());
    auto pJ = joints.Span.data();
    auto pW = weights.Span.data();
    for (size_t i = 0; i < joints.ItemCount;
         ++i, pJ += joints.Stride, pW += weights.Stride) {
      auto& dst = m_bindings[offset + i];
      auto& src = (*(const T*)pJ);
      dst.Joints.X = src.X;
      dst.Joints.Y = src.Y;
      dst.Joints.Z = src.Z;
      dst.Joints.W = src.W;
      dst.Weights = (*(const DirectX::XMFLOAT4*)pW);
    }
  }

  template<typename T>
  void addSubmesh(uint32_t offset,
                  std::span<const T> values,
                  std::optional<uint32_t> material)
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

}
