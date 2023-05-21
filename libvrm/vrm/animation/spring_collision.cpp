#include "spring_collision.h"

namespace runtimescene {

RuntimeSpringCollision::RuntimeSpringCollision(
  const std::shared_ptr<libvrm::vrm::SpringBone>& springBone)
{
  for (auto& group : springBone->Colliders) {
    for (auto& collider : group->Colliders) {
      // ColliderGroups.push_back
    }
  }
}

std::optional<DirectX::XMVECTOR>
RuntimeSpringCollision::Collide(const DirectX::XMVECTOR& springTail,
                                float radius)
{
  // for (; Current < Colliders.size(); ++Current) {
  //   auto collider = Colliders[Current];
  //   auto r = radius + collider->Radius;
  //   auto colliderPos = collider->Position();
  //   auto d = DirectX::XMVectorSubtract(springTail, colliderPos);
  //   if (DirectX::XMVectorGetX(DirectX::XMVector3Length(d)) <= (r * r)) {
  //     // ヒット。Colliderの半径方向に押し出す
  //     auto normal = DirectX::XMVector3Normalize(d);
  //     return DirectX::XMVectorAdd(
  //       colliderPos, DirectX::XMVectorScale(normal, radius +
  //       collider->Radius));
  //   }
  // }
  return {};
}

}
