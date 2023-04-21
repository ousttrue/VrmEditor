#pragma once
#include "gizmo.h"
#include "node.h"
#include "sprintgcollider.h"
#include "timeline.h"
#include <DirectXMath.h>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace vrm {

class SpringJoint
{
public:
  std::shared_ptr<gltf::Node> Head;
  // 減衰[0~1]
  float DragForce = 0;
  // 剛性。初期姿勢への復元力[0~]
  // 増えれば増えるほど比率が高まる。
  float Stiffiness = 0;

private:
  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

public:
  SpringJoint(const std::shared_ptr<gltf::Node>& head,
              const DirectX::XMFLOAT3& localTailPosition,
              float dragForce,
              float stiffiness);
  void Update(Time time);

  DirectX::XMFLOAT4 WorldPosToLocalRotation(
    const DirectX::XMFLOAT3& nextTail) const;
  void DrawGizmo(IGizmoDrawer* gizmo);
};

struct SpringSolver
{
  std::string Comment;
  Time LastTime = {};

  std::vector<SpringJoint> Joints;
  // for vrm0
  void AddRecursive(const std::shared_ptr<gltf::Node>& node,
                    float dragForce,
                    float stiffiness);
  void Update(Time time);
  void DrawGizmo(IGizmoDrawer* gizmo);
};

// struct Spring
// {
//   std::optional<uint32_t> center;
//   float hitRadius = 0;
//   std::vector<uint32_t> bones;
//   std::vector<uint32_t> colliderGroups;
// };
// inline void
// from_json(const nlohmann::json& j, Spring& spring)
// {
//   spring.stiffiness = j.at("stiffiness");
//   spring.dragForce = j.at("dragForce");
//   if (j.find("bones") != j.end()) {
//     auto& bones = j.at("bones");
//     spring.bones.assign(bones.begin(), bones.end());
//   }
//   if (j.find("colliderGroups") != j.end()) {
//     auto& colliderGroups = j.at("colliderGroups");
//     spring.colliderGroups.assign(colliderGroups.begin(),
//     colliderGroups.end());
//   }
// }
// inline std::ostream&
// operator<<(std::ostream& os, const Spring& spring)
// {
//   os << "<spring";
//   if (spring.comment.size()) {
//     os << " \"" << spring.comment << "\"";
//   }
//   os << " dragForce:" << spring.dragForce;
//   os << " stiffiness:" << spring.stiffiness;
//   if (spring.bones.size()) {
//     os << " bones[";
//     for (int i = 0; i < spring.bones.size(); ++i) {
//       if (i) {
//         os << ",";
//       }
//       os << spring.bones[i];
//     }
//     os << "]";
//   }
//   // collision
//   os << " radius:" << spring.hitRadius;
//   if (auto center = spring.center) {
//     os << " center:" << *center;
//   }
//   if (spring.colliderGroups.size()) {
//     os << " colliderGroups[";
//     for (int i = 0; i < spring.colliderGroups.size(); ++i) {
//       if (i) {
//         os << ",";
//       }
//       os << spring.colliderGroups[i];
//     }
//     os << "]";
//   }
//   os << ">";
//   return os;
// }

}
}
