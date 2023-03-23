#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <assert.h>
#include <memory>
#include <span>
#include <vector>

class Material;
struct Primitive {
  uint32_t offset;
  uint32_t drawCount;
  std::shared_ptr<Material> material;
};

struct Mesh {
  uint32_t id;
  Mesh() {
    static uint32_t s_id = 0;
    id = ++s_id;
  }
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  std::vector<Vertex> m_vertices;
  std::vector<uint8_t> m_indices;
  std::vector<Primitive> m_primitives;
  uint32_t m_indexValueSize = 0;
  // skinning
  std::vector<JointBinding> m_bindings;
  std::vector<Vertex> m_updated;

  size_t verticesBytes() const { return m_vertices.size() * sizeof(Vertex); }

  size_t addPosition(std::span<const float3> values) {
    auto offset = m_vertices.size();
    m_vertices.resize(offset + values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].position = values[i];
    }
    return offset;
  }

  void setNormal(uint32_t offset, std::span<const float3> values) {
    assert(offset + values.size() == m_vertices.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].normal = values[i];
    }
  }

  void setUv(uint32_t offset, std::span<const float2> values) {
    assert(offset + values.size() == m_vertices.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].uv = values[i];
    }
  }

  void setBoneSkinning(uint32_t offset, std::span<const ushort4> joints,
                       std::span<const float4> weights) {
    assert(offset + joints.size() == m_vertices.size());
    assert(offset + weights.size() == m_vertices.size());
    m_bindings.resize(m_vertices.size());
    for (size_t i = 0; i < joints.size(); ++i) {
      m_bindings[offset + i] = {
          .joints = joints[i],
          .weights = weights[i],
      };
    }
  }

  void addSubmesh(uint32_t offset, std::span<const uint8_t> values,
                  uint32_t count, std::shared_ptr<Material> material) {
    auto indexOffset = m_indices.size();
    m_indices.resize(indexOffset + values.size());
    std::copy(values.begin(), values.end(), m_indices.data() + indexOffset);

    m_primitives.push_back({
        .offset = static_cast<uint32_t>(indexOffset),
        .drawCount = count,
        .material = material,
    });

    auto indexValueSize = values.size() / count;
    if (m_indexValueSize == 0) {
      m_indexValueSize = indexValueSize;
    } else {
      assert(indexValueSize == m_indexValueSize);
    }
  }

  void add(float3 *dst, const float3 &src, float w,
           const DirectX::XMFLOAT4X4 &m) {
    if (w > 0) {
      auto pos = DirectX::XMLoadFloat3((DirectX::XMFLOAT3 *)&src);
      auto newPos =
          DirectX::XMVector3Transform(pos, DirectX::XMLoadFloat4x4(&m));
      float3 store;
      DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&store, newPos);
      *dst += (store * w);
    }
  }

  void skinning(std::span<DirectX::XMFLOAT4X4> skinningMatrices) {
    m_updated.assign(m_vertices.begin(), m_vertices.end());
    for (int i = 0; i < m_vertices.size(); ++i) {
      auto src = m_vertices[i];
      auto &dst = m_updated[i];
      dst.position = {0, 0, 0};
      dst.normal = {0, 0, 0};
      auto binding = m_bindings[i];
      if (auto w = binding.weights.x)
        add(&dst.position, src.position, w, skinningMatrices[binding.joints.x]);
      if (auto w = binding.weights.y)
        add(&dst.position, src.position, w, skinningMatrices[binding.joints.y]);
      if (auto w = binding.weights.z)
        add(&dst.position, src.position, w, skinningMatrices[binding.joints.z]);
      if (auto w = binding.weights.w)
        add(&dst.position, src.position, w, skinningMatrices[binding.joints.w]);
    }
  }
};
