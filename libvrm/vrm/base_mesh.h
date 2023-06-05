#pragma once
#include <DirectXMath.h>
#include <assert.h>
#include <gltfjson.h>
#include <memory>
#include <span>
#include <vector>

namespace libvrm {

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

struct ushort4
{
  uint16_t X;
  uint16_t Y;
  uint16_t Z;
  uint16_t W;
};

struct byte4
{
  uint8_t X;
  uint8_t Y;
  uint8_t Z;
  uint8_t W;
};

struct BoundingBox
{
  DirectX::XMFLOAT3 Min{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
  };
  DirectX::XMFLOAT3 Max{
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
  };

  void Extend(const DirectX::XMFLOAT3& p)
  {
    Min.x = std::min(Min.x, p.x);
    Min.y = std::min(Min.y, p.y);
    Min.z = std::min(Min.z, p.z);
    Max.x = std::max(Max.x, p.x);
    Max.y = std::max(Max.y, p.y);
    Max.z = std::max(Max.z, p.z);
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

  // T=byte4 or ushort4
  template<typename T>
  void setBoneSkinning(uint32_t offset,
                       std::span<const T> joints,
                       std::span<const DirectX::XMFLOAT4> weights)
  {
    assert(offset + joints.size() == m_vertices.size());
    assert(offset + weights.size() == m_vertices.size());
    m_bindings.resize(m_vertices.size());
    for (size_t i = 0; i < joints.size(); ++i) {
      auto& dst = m_bindings[offset + i];
      auto& src = joints[i];
      dst.Joints.X = src.X;
      dst.Joints.Y = src.Y;
      dst.Joints.Z = src.Z;
      dst.Joints.W = src.W;
      dst.Weights = weights[i];
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
