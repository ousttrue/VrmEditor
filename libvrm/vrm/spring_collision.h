#pragma once
#include "spring_bone.h"
#include "spring_collider.h"
#include <optional>

namespace libvrm {

struct RuntimeSpringCollision
{
  std::vector<std::shared_ptr<SpringColliderGroup>> ColliderGroups;
  int Current = 0;
  void Clear() { Current = 0; }

  RuntimeSpringCollision(const std::shared_ptr<SpringBone>& springBone);

  std::optional<DirectX::XMVECTOR> Collide(const DirectX::XMVECTOR& pos,
                                           float radius);
};

} // namespace
