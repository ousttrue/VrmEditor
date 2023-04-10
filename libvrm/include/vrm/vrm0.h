#pragma once
#include "expression.h"
#include "springbone.h"
#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace gltf {
struct Node;
}

namespace vrm::v0 {

struct Vrm
{
  std::vector<std::shared_ptr<ColliderGroup>> m_colliderGroups;
  std::vector<std::shared_ptr<Spring>> m_springs;
};

} // namespace vrm0
