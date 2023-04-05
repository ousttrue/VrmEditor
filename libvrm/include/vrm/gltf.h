#pragma once
#include "base64.h"
#include "directory.h"
#include "gltf_buffer.h"
#include <algorithm>
#include <expected>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>
#include <stdint.h>
#include <string_view>
#include <unordered_map>

inline bool
has(const nlohmann::json& obj, std::string_view key)
{
  if (!obj.is_object()) {
    return {};
  }
  if (obj.find(key) == obj.end()) {
    return {};
  }
  return true;
}

namespace gltf {
const auto VERTEX_JOINT = "JOINTS_0";
const auto VERTEX_WEIGHT = "WEIGHTS_0";
const auto VERTEX_POSITION = "POSITION";
const auto VERTEX_NORMAL = "NORMAL";
const auto VERTEX_UV = "TEXCOORD_0";

struct Gltf
{
  std::shared_ptr<Directory> Dir;
  nlohmann::json Json;
  std::span<const uint8_t> Bin;

  std::expected<std::span<const uint8_t>, std::string> buffer_view(
    int buffer_view_index) const
  {
    auto buffer_view = Json.at("bufferViews").at(buffer_view_index);
    // std::cout << buffer_view << std::endl;

    int buffer_index = buffer_view.at("buffer");
    auto buffer = Json.at("buffers").at(buffer_index);
    if (has(buffer, "uri")) {
      // external file
      std::string_view uri = buffer.at("uri");
      if (auto buffer = Dir->GetBuffer(uri)) {
        return buffer->subspan(buffer_view.value("byteOffset", 0),
                               buffer_view.at("byteLength"));
      } else {
        return buffer;
      }
    } else {
      // glb
      return Bin.subspan(buffer_view.value("byteOffset", 0),
                         buffer_view.at("byteLength"));
    }
  }

  template<typename S, typename T>
  std::expected<bool, std::string> SetSparseValue(std::span<const S> indices,
                                                  uint32_t buffer_view_index,
                                                  std::span<T> dst) const
  {
    if (auto span = buffer_view(buffer_view_index)) {
      assert(indices.size() == span->size() / sizeof(T));
      auto p = (const T*)span->data();
      for (int i = 0; i < indices.size(); ++i) {
        dst[i] = p[indices[i]];
      }
      return true;
    } else {
      return std::unexpected{ span.error() };
    }
  }

  mutable std::vector<uint8_t> m_sparseBuffer;
  template<typename T>
  std::expected<std::span<const T>, std::string> accessor(
    int accessor_index) const
  {
    auto accessor = Json.at("accessors").at(accessor_index);
    // std::cout << accessor << std::endl;
    assert(*item_size(accessor) == sizeof(T));
    int count = accessor.at("count");
    if (has(accessor, "sparse")) {
      auto sparse = accessor.at("sparse");
      m_sparseBuffer.resize(count * sizeof(T));
      auto begin = (T*)m_sparseBuffer.data();
      auto sparse_span = std::span<T>(begin, begin + count);
      if (has(accessor, "bufferView")) {
        // non zero sparse
        return std::unexpected{ "non zero sparse not implemented" };
      } else {
        // zero fill
        T zero = {};
        std::fill(sparse_span.begin(), sparse_span.end(), zero);
      }
      int sparse_count = sparse.at("count");
      auto sparse_indices = sparse.at("indices");
      auto sparse_values = sparse.at("values");
      switch ((gltf::ComponentType)sparse_indices.at("componentType")) {
        case gltf::ComponentType::UNSIGNED_BYTE:
          if (auto sparse_indices_bytes =
                buffer_view(sparse_indices.at("bufferView"))) {
            auto begin = (const uint8_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, sparse_values.at("bufferView"), sparse_span)) {
              return sparse_span;
            } else {
              return std::unexpected{ ok.error() };
            }
          } else {
            return std::unexpected{ sparse_indices_bytes.error() };
          }
        case gltf::ComponentType::UNSIGNED_SHORT:
          if (auto sparse_indices_bytes =
                buffer_view(sparse_indices.at("bufferView"))) {
            auto begin = (const uint16_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, sparse_values.at("bufferView"), sparse_span)) {
              return sparse_span;
            } else {
              return std::unexpected{ ok.error() };
            }
          } else {
            return std::unexpected{ sparse_indices_bytes.error() };
          }
        case gltf::ComponentType::UNSIGNED_INT:
          if (auto sparse_indices_bytes =
                buffer_view(sparse_indices.at("bufferView"))) {
            auto begin = (const uint32_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, sparse_values.at("bufferView"), sparse_span)) {

              return sparse_span;
            } else {
              return std::unexpected{ ok.error() };
            }
          } else {
            return std::unexpected{ sparse_indices_bytes.error() };
          }
        default:
          return std::unexpected{ "sparse.indices: unknown" };
      }
      throw std::runtime_error("not implemented");
    } else if (has(accessor, "bufferView")) {
      if (auto span = buffer_view(accessor["bufferView"])) {
        int offset = accessor.value("byteOffset", 0);
        return std::span<const T>((const T*)(span->data() + offset), count);

      } else {
        return std::unexpected{ span.error() };
      }
    } else {
      return std::unexpected{ "sparse nor bufferView" };
    }
  }
};
}
