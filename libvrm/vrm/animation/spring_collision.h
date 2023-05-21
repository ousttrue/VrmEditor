#pragma once
#include "spring_bone.h"
#include "spring_collider.h"

namespace runtimescene {

struct RuntimeSpringCollision
{
  std::vector<std::shared_ptr<libvrm::vrm::SpringColliderGroup>> ColliderGroups;
  int Current = 0;
  void Clear() { Current = 0; }

  RuntimeSpringCollision(
    const std::shared_ptr<libvrm::vrm::SpringBone>& springBone);

  std::optional<DirectX::XMVECTOR> Collide(const DirectX::XMVECTOR& pos,
                                           float radius);
};

}
