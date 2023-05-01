#pragma once
#include "springjoint.h"
#include "vrm/springcollider.h"
#include <memory>
#include <vector>

namespace libvrm {
namespace vrm {

struct SpringSolver
{
  std::string Comment;

  std::vector<SpringJoint> Joints;
  std::shared_ptr<SpringCollision> Collision;

  SpringSolver();
  void AddColliderGroup(
    const std::shared_ptr<SpringColliderGroup>& colliderGroup);
  void Add(const std::shared_ptr<gltf::Node>& head,
           const std::shared_ptr<gltf::Node>& tail,
           const DirectX::XMFLOAT3& localTailPosition,
           float dragForce,
           float stiffiness,
           float radius);
  // for vrm0
  void AddRecursive(const std::shared_ptr<gltf::Node>& node,
                    float dragForce,
                    float stiffiness,
                    float radius);
};

}
}
