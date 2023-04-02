#pragma once
#include "base64.h"
#include "directory.h"
#include <expected>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <span>
#include <stdint.h>
#include <string_view>
#include <unordered_map>

const auto VERTEX_JOINT = "JOINTS_0";
const auto VERTEX_WEIGHT = "WEIGHTS_0";
const auto VERTEX_POSITION = "POSITION";
const auto VERTEX_NORMAL = "NORMAL";
const auto VERTEX_UV = "TEXCOORD_0";

inline bool has(const nlohmann::json &obj, std::string_view key) {
  if (!obj.is_object()) {
    return false;
  }
  return obj.find(key) != obj.end();
}

enum class ComponentType {
  BYTE = 5120,
  UNSIGNED_BYTE = 5121,
  SHORT = 5122,
  UNSIGNED_SHORT = 5123,
  UNSIGNED_INT = 5125,
  FLOAT = 5126,
};

inline std::expected<size_t, std::string>
component_size(ComponentType component_type) {
  switch (component_type) {
  case ComponentType::BYTE:
    return 1;
  case ComponentType::UNSIGNED_BYTE:
    return 1;
  case ComponentType::SHORT:
    return 2;
  case ComponentType::UNSIGNED_SHORT:
    return 2;
  case ComponentType::UNSIGNED_INT:
    return 4;
  case ComponentType::FLOAT:
    return 4;
  default:
    return std::unexpected{"invalid component type"};
  }
}

inline std::expected<size_t, std::string> type_size(std::string_view type) {
  if (type == "SCALAR") {
    return 1;
  } else if (type == "VEC2") {
    return 2;
  } else if (type == "VEC3") {
    return 3;
  } else if (type == "VEC4") {
    return 4;
  } else if (type == "MAT2") {
    return 4;
  } else if (type == "MAT3") {
    return 9;
  } else if (type == "MAT4") {
    return 16;
  } else {
    return std::unexpected{std::string(type)};
  }
}

inline std::expected<size_t, std::string>
item_size(const nlohmann::json &accessor) {
  if (auto cs = component_size(accessor["componentType"])) {
    if (auto ts = type_size(accessor["type"])) {
      return *cs * *ts;
    } else {
      return ts;
    }
  } else {
    throw cs;
  }
}

struct Gltf {

  std::shared_ptr<Directory> m_dir;

  nlohmann::json json;
  std::span<const uint8_t> bin;

  std::expected<std::span<const uint8_t>, std::string>
  buffer_view(int buffer_view_index) const {
    auto buffer_view = json["bufferViews"][buffer_view_index];
    // std::cout << buffer_view << std::endl;

    int buffer_index = buffer_view.at("buffer");
    auto buffer = json["buffers"][buffer_index];
    if (buffer.find("uri") != buffer.end()) {
      std::string_view uri = buffer.at("uri");
      if (auto buffer = m_dir->GetBuffer(uri)) {
        return buffer->subspan(buffer_view.value("byteOffset", 0),
                               buffer_view.at("byteLength"));
      } else {
        return buffer;
      }
    } else {
      // glb
      return bin.subspan(buffer_view.value("byteOffset", 0),
                         buffer_view.at("byteLength"));
    }
  }

  template <typename T>
  std::expected<std::span<const T>, std::string>
  accessor(int accessor_index) const {
    auto accessor = json["accessors"][accessor_index];
    // std::cout << accessor << std::endl;
    assert(*item_size(accessor) == sizeof(T));
    if (auto span = buffer_view(accessor["bufferView"])) {
      int offset = accessor.value("byteOffset", 0);
      return std::span<const T>((const T *)(span->data() + offset),
                                accessor.at("count"));

    } else {
      return std::unexpected{span.error()};
    }
  }
};