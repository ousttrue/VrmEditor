#pragma once
#include <vrm/springcollider.h>

namespace runtimescene {

struct RuntimeSpringCollision
{
  std::vector<std::shared_ptr<libvrm::vrm::SpringColliderGroup>> ColliderGroups;
  int Current = 0;
  void Clear() { Current = 0; }
  std::optional<DirectX::XMVECTOR> Collide(const DirectX::XMVECTOR& pos,
                                           float radius);
};

}
