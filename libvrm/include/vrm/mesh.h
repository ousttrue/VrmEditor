#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <assert.h>
#include <memory>
#include <span>
#include <vector>

class Material;
struct Primitive {
  uint32_t drawCount;
  std::shared_ptr<Material> material;
};

struct MorphVertex {
  float3 position;
};

struct MorphTarget {
  std::string name;
  std::vector<MorphVertex> m_vertices;
  float weight = 0;

  size_t addPosition(std::span<const float3> values) {
    auto offset = m_vertices.size();
    m_vertices.resize(offset + values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[offset + i].position = values[i];
    }
    return offset;
  }
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
  std::vector<uint32_t> m_indices;
  std::vector<Primitive> m_primitives;
  // morphtarget
  std::vector<std::shared_ptr<MorphTarget>> m_morphTargets;
  // skinning
  std::vector<JointBinding> m_bindings;
  std::vector<Vertex> m_updated;

  size_t verticesBytes() const {
    return m_vertices.size() * sizeof(m_vertices[0]);
  }
  size_t indicesBytes() const {
    return m_indices.size() * sizeof(m_indices[0]);
  }

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

  std::shared_ptr<MorphTarget> getOrCreateMorphTarget(int index) {
    while (index >= m_morphTargets.size()) {
      m_morphTargets.push_back(std::make_shared<MorphTarget>());
    }
    return m_morphTargets[index];
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

  template <typename T>
  void addSubmesh(uint32_t offset, std::span<const T> values,
                  std::shared_ptr<Material> material) {
    m_indices.reserve(m_indices.size() + values.size());
    for (auto value : values) {
      m_indices.push_back(offset + value);
    }

    m_primitives.push_back({
        .drawCount = static_cast<uint32_t>(values.size()),
        .material = material,
    });
  }

  void applySkinning(float3 *dst, const float3 &src, float w,
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

  void applyMorphTargetAndSkinning(std::span<DirectX::XMFLOAT4X4> skinningMatrices) {
    // clear & apply morph target
    m_updated.clear();
    for (int i = 0; i < m_vertices.size(); ++i) {
      auto v = m_vertices[i];
      for (auto &morphtarget : m_morphTargets) {
        if (morphtarget->weight) {
          v.position +=
              morphtarget->m_vertices[i].position * morphtarget->weight;
        }
      }
      m_updated.push_back(v);
    }

    // calc skinning
    for (int i = 0; i < m_vertices.size(); ++i) {
      auto src = m_updated[i];
      auto &dst = m_updated[i];
      dst.position = {0, 0, 0};
      dst.normal = {0, 0, 0};
      auto binding = m_bindings[i];
      if (auto w = binding.weights.x)
        applySkinning(&dst.position, src.position, w,
                      skinningMatrices[binding.joints.x]);
      if (auto w = binding.weights.y)
        applySkinning(&dst.position, src.position, w,
                      skinningMatrices[binding.joints.y]);
      if (auto w = binding.weights.z)
        applySkinning(&dst.position, src.position, w,
                      skinningMatrices[binding.joints.z]);
      if (auto w = binding.weights.w)
        applySkinning(&dst.position, src.position, w,
                      skinningMatrices[binding.joints.w]);
    }
  }
};