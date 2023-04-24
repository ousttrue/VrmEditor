#pragma once
#include "springjoint.h"
#include <vector>

namespace libvrm {
namespace vrm {

struct SpringSolver
{
  std::string Comment;

  std::vector<SpringJoint> Joints;
  void Add(const std::shared_ptr<gltf::Node>& head,
           const DirectX::XMFLOAT3& tail,
           float dragForce,
           float stiffiness);
  // for vrm0
  void AddRecursive(const std::shared_ptr<gltf::Node>& node,
                    float dragForce,
                    float stiffiness);
  void Update(Time deltaForSimulation);
  void DrawGizmo(IGizmoDrawer* gizmo);
};

}
}
