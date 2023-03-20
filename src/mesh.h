#pragma once
#include <span>
#include <vector>

struct float2 {
  float x;
  float y;
};
struct float3 {
  float x;
  float y;
  float z;
};
struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};
static_assert(sizeof(Vertex) == 32, "sizeof(Vertex)");

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
  uint32_t m_drawCount = 0;
  int indexValueSize() const { return m_indices.size() / m_drawCount; }

  size_t verticesBytes() const { return m_vertices.size() * sizeof(Vertex); }

  void setPosition(std::span<const float3> values) {
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].position = values[i];
    }
  }

  void setNormal(std::span<const float3> values) {
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].normal = values[i];
    }
  }

  void setUv(std::span<const float2> values) {
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].uv = values[i];
    }
  }

  void setIndices(std::span<const uint8_t> values, size_t count) {
    m_indices.assign(values.begin(), values.end());
    m_drawCount = count;
  }
};
