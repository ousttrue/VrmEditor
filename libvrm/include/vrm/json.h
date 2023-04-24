#pragma once
#include <nlohmann/json.hpp>

namespace libvrm {
namespace gltf {

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

}
}
