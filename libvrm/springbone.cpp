#include "vrm/springbone.h"
#include "vrm/gizmo.h"
#include "vrm/scenetypes.h"
#include "vrm/springjoint.h"
#include <iostream>

namespace libvrm::vrm {

SpringSolver::SpringSolver()
  : Collision(new SpringCollision)
{
}

void
SpringSolver::AddColliderGroup(
  const std::shared_ptr<SpringColliderGroup>& colliderGroup)
{
  for (auto& collider : colliderGroup->Colliders) {
    Collision->Colliders.push_back(collider);
  }
}

void
SpringSolver::Add(const std::shared_ptr<gltf::Node>& head,
                  const DirectX::XMFLOAT3& tail,
                  float dragForce,
                  float stiffiness,
                  float radius)
{
  head->ShapeColor = { 0.5f, 0.5f, 1.0f, 1 };
  Joints.push_back(SpringJoint(head, tail, dragForce, stiffiness, radius));
}

void
SpringSolver::AddRecursive(const std::shared_ptr<gltf::Node>& node,
                           float dragForce,
                           float stiffiness,
                           float radius)
{
  if (node->Children.size()) {
    for (auto& child : node->Children) {
      Add(node, child->Transform.Translation, dragForce, stiffiness, radius);
      break;
    }
  } else {
    auto delta = node->WorldTransform.Translation - node->ParentWorldPosition();
    auto childPosition = DirectX::XMVectorAdd(
      DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
      DirectX::XMVectorScale(
        DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&delta)), 0.07f));

    DirectX::XMFLOAT3 localTailPosition;
    DirectX::XMStoreFloat3(
      &localTailPosition,
      DirectX::XMVector3Transform(
        childPosition, DirectX::XMMatrixInverse(nullptr, node->WorldMatrix())));

    Add(node, localTailPosition, dragForce, stiffiness, radius);
  }

  for (auto& child : node->Children) {
    AddRecursive(child, dragForce, stiffiness, radius);
  }
}

void
SpringSolver::Update(Time delta)
{
  bool doUpdate = delta.count() > 0;
  if (!doUpdate) {
    return;
  }

  for (auto& joint : Joints) {
    Collision->Clear();
    joint.Update(delta, Collision.get());
  }
}

void
SpringSolver::DrawGizmo(IGizmoDrawer* gizmo)
{
  for (auto& joint : Joints) {
    joint.DrawGizmo(gizmo);
  }
}

}
