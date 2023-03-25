#pragma once
#include <expected>
#include <nlohmann/json.hpp>
#include <optional>
#include <span>
#include <stdint.h>
#include <tuple>

using json = nlohmann::json;

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

inline std::expected<size_t, std::string> item_size(const json &accessor) {
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

struct Glb {
  json gltf;
  std::span<const uint8_t> bin;

  std::span<const uint8_t> buffer_view(int buffer_view_index) const {
    auto buffer_view = gltf["bufferViews"][buffer_view_index];
    // std::cout << buffer_view << std::endl;
    return bin.subspan(buffer_view["byteOffset"], buffer_view["byteLength"]);
  }

  template <typename T> std::span<const T> accessor(int accessor_index) const {
    auto accessor = gltf["accessors"][accessor_index];
    // std::cout << accessor << std::endl;
    assert(*item_size(accessor) == sizeof(T));
    auto span = buffer_view(accessor["bufferView"]);

    int offset = accessor.value("byteOffset", 0);
    return std::span<const T>((const T *)(span.data() + offset),
                              accessor.at("count"));
  }

  static std::optional<Glb> parse(std::span<const uint8_t> bytes);
};
