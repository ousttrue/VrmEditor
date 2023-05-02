#include "vrm/springbone.h"
#include "vrm/gizmo.h"
#include "vrm/scenetypes.h"
#include "vrm/springjoint.h"
#include <iostream>

namespace libvrm::vrm {

void
SpringBone::AddJoint(const std::shared_ptr<gltf::Node>& head,
                     const std::shared_ptr<gltf::Node>& tail,
                     const DirectX::XMFLOAT3& localTailPosition,
                     float dragForce,
                     float stiffiness,
                     float radius)
{
  // head->ShapeColor = { 0.5f, 0.5f, 1.0f, 1 };
  Joints.push_back(std::make_shared<SpringJoint>(
    head, tail, localTailPosition, dragForce, stiffiness, radius));
}

void
SpringBone::AddJointRecursive(const std::shared_ptr<gltf::Node>& node,
                              float dragForce,
                              float stiffiness,
                              float radius)
{
  if (node->Children.size()) {
    for (auto& child : node->Children) {
      AddJoint(node,
               child,
               child->InitialTransform.Translation,
               dragForce,
               stiffiness,
               radius);
      break;
    }
  } else {
    auto delta = node->WorldInitialTransform.Translation -
                 node->ParentWorldInitialPosition();
    auto childPosition = DirectX::XMVectorAdd(
      DirectX::XMLoadFloat3(&node->WorldInitialTransform.Translation),
      DirectX::XMVectorScale(
        DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&delta)), 0.07f));

    DirectX::XMFLOAT3 localTailPosition;
    DirectX::XMStoreFloat3(
      &localTailPosition,
      DirectX::XMVector3Transform(
        childPosition,
        DirectX::XMMatrixInverse(nullptr, node->WorldInitialMatrix())));

    AddJoint(node, nullptr, localTailPosition, dragForce, stiffiness, radius);
  }

  for (auto& child : node->Children) {
    AddJointRecursive(child, dragForce, stiffiness, radius);
  }
}

}
