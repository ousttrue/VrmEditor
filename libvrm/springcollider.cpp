#include "vrm/springcollider.h"
#include "vrm/gizmo.h"

namespace libvrm {
namespace vrm {

void
SpringCollider::DrawGizmo(IGizmoDrawer* gizmo)
{
  DirectX::XMFLOAT3 offset;
  DirectX::XMStoreFloat3(
    &offset, Node->WorldTransformPoint(DirectX::XMLoadFloat3(&Offset)));
  switch (Type) {
    case SpringColliderShapeType::Sphere:
      gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      break;

    case SpringColliderShapeType::Capsule: {
      DirectX::XMFLOAT3 tail;
      DirectX::XMStoreFloat3(
        &tail, Node->WorldTransformPoint(DirectX::XMLoadFloat3(&Tail)));

      // gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      // gizmo->DrawSphere(tail, Radius, { 0, 1, 1, 1 });
      gizmo->DrawCapsule(offset, tail, Radius, { 0, 1, 1, 1 });
      break;
    }
  }
}

DirectX::XMVECTOR
SpringCollider::Position() const
{
  return Node->WorldTransformPoint(DirectX::XMLoadFloat3(&Offset));
}

}
}
