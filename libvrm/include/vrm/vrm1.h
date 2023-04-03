#pragma once
#include "vrm0.h"

namespace vrm::v1 {

struct Vrm
{
  std::vector<std::shared_ptr<vrm::v0::Expression>> m_expressions;
  std::vector<std::shared_ptr<vrm::v0::ColliderGroup>> m_colliderGroups;
  std::vector<std::shared_ptr<vrm::v0::Spring>> m_springs;
  std::unordered_map<vrm::v0::MorphTargetKey, float> m_morphTargetMap;
};

}
