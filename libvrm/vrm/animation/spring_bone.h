#pragma once
#include "spring_collider.h"
#include "spring_joint.h"
#include <memory>
#include <vector>

namespace libvrm {
namespace vrm {

struct SpringBone
{
  std::string Comment;
  std::vector<std::shared_ptr<SpringJoint>> Joints;
  std::vector<std::shared_ptr<SpringColliderGroup>> Colliders;

  void AddColliderGroup(
    const std::shared_ptr<SpringColliderGroup>& colliderGroup)
  {
    Colliders.push_back(colliderGroup);
  }

  void AddJoint(const std::shared_ptr<gltf::Node>& head,
                const std::shared_ptr<gltf::Node>& tail,
                const DirectX::XMFLOAT3& localTailPosition,
                float dragForce,
                float stiffiness,
                float radius);

  // for vrm0
  void AddJointRecursive(const std::shared_ptr<gltf::Node>& node,
                         float dragForce,
                         float stiffiness,
                         float radius);
};

}
}
