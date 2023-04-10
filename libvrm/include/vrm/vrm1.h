#pragma once
#include "expression.h"
#include "vrm0.h"

namespace vrm::v1 {

struct Vrm
{
  std::vector<std::shared_ptr<vrm::Expression>> m_expressions;
  std::vector<std::shared_ptr<vrm::v0::ColliderGroup>> m_colliderGroups;
  std::vector<std::shared_ptr<vrm::v0::Spring>> m_springs;
  std::unordered_map<vrm::MorphTargetKey, float> m_morphTargetMap;
};

}
