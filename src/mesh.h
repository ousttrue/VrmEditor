#pragma once
#include "scenetypes.h"
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
};
