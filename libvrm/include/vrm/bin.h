#pragma once
#include "base64.h"
#include "directory.h"
#include <algorithm>
#include <expected>
#include <filesystem>
// #include <gltfjson/binary_writer.h>
#include <gltfjson/gltf.h>
#include <iostream>
#include <span>
#include <stdint.h>
#include <string_view>
#include <unordered_map>

namespace libvrm::gltf {
const auto VERTEX_JOINT = "JOINTS_0";
const auto VERTEX_WEIGHT = "WEIGHTS_0";
const auto VERTEX_POSITION = "POSITION";
const auto VERTEX_NORMAL = "NORMAL";
const auto VERTEX_UV = "TEXCOORD_0";

struct Gltf
{
  std::shared_ptr<Directory> Dir;
  gltfjson::format::Root m_gltf;
  std::span<const uint8_t> Bin;

  std::expected<std::span<const uint8_t>, std::string> buffer_view(
    int buffer_view_index) const
  {
    auto buffer_view = m_gltf.BufferViews[buffer_view_index];
    // std::cout << buffer_view << std::endl;

    int buffer_index = *buffer_view.Buffer;
    auto buffer = m_gltf.Buffers[buffer_index];
    if (buffer.Uri.size()) {
      // external file
      if (auto bytes = Dir->GetBuffer(buffer.Uri)) {
        return bytes->subspan(buffer_view.ByteOffset, buffer_view.ByteLength);
      } else {
        return bytes;
      }
    } else {
      // glb
      return Bin.subspan(buffer_view.ByteOffset, buffer_view.ByteLength);
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
    auto accessor = m_gltf.Accessors[accessor_index];
    // std::cout << accessor << std::endl;
    // assert(*item_size(accessor) == sizeof(T));
    int count = accessor.Count;
    if (auto sparse = accessor.Sparse) {
      m_sparseBuffer.resize(count * sizeof(T));
      auto begin = (T*)m_sparseBuffer.data();
      auto sparse_span = std::span<T>(begin, begin + count);
      if (accessor.BufferView) {
        // non zero sparse
        return std::unexpected{ "non zero sparse not implemented" };
      } else {
        // zero fill
        T zero = {};
        std::fill(sparse_span.begin(), sparse_span.end(), zero);
      }
      int sparse_count = sparse->Count;
      auto sparse_indices = sparse->Indices;
      auto sparse_values = sparse->Values;
      switch (sparse_indices.ComponentType) {
        case gltfjson::format::ComponentTypes::UNSIGNED_BYTE:
          if (auto sparse_indices_bytes =
                buffer_view(*sparse_indices.BufferView)) {
            auto begin = (const uint8_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, *sparse_values.BufferView, sparse_span)) {
              return sparse_span;
            } else {
              return std::unexpected{ ok.error() };
            }
          } else {
            return std::unexpected{ sparse_indices_bytes.error() };
          }
        case gltfjson::format::ComponentTypes::UNSIGNED_SHORT:
          if (auto sparse_indices_bytes =
                buffer_view(*sparse_indices.BufferView)) {
            auto begin = (const uint16_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, *sparse_values.BufferView, sparse_span)) {
              return sparse_span;
            } else {
              return std::unexpected{ ok.error() };
            }
          } else {
            return std::unexpected{ sparse_indices_bytes.error() };
          }
        case gltfjson::format::ComponentTypes::UNSIGNED_INT:
          if (auto sparse_indices_bytes =
                buffer_view(*sparse_indices.BufferView)) {
            auto begin = (const uint32_t*)sparse_indices_bytes->data();
            auto indices_span = std::span(begin, begin + sparse_count);
            if (auto ok = SetSparseValue(
                  indices_span, *sparse_values.BufferView, sparse_span)) {

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
    } else if (auto bufferView = accessor.BufferView) {
      if (auto span = buffer_view(*bufferView)) {
        int offset = accessor.ByteOffset;
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
