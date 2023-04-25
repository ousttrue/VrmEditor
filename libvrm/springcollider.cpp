#include "vrm/springcollider.h"
#include "vrm/gizmo.h"

namespace libvrm {
namespace vrm {

void
SpringCollider::DrawGizmo(IGizmoDrawer* gizmo)
{
  auto node = Node.lock();
  if (!node) {
    return;
  }

  DirectX::XMFLOAT3 offset;
  DirectX::XMStoreFloat3(
    &offset, node->WorldTransformPoint(DirectX::XMLoadFloat3(&Offset)));
  switch (Type) {
    case SpringColliderShapeType::Sphere:
      gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      break;

    case SpringColliderShapeType::Capsule: {
      DirectX::XMFLOAT3 tail;
      DirectX::XMStoreFloat3(
        &tail, node->WorldTransformPoint(DirectX::XMLoadFloat3(&Tail)));

      // gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      // gizmo->DrawSphere(tail, Radius, { 0, 1, 1, 1 });
      gizmo->DrawCapsule(offset, tail, Radius, { 0, 1, 1, 1 });
      break;
    }
  }
}

}
}
