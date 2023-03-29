#pragma once
#include "vrm/node.h"
#include <DirectXMath.h>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <vector>

struct Node;

namespace vrm {

class SpringJoint {
public:
  std::shared_ptr<Node> Head;
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
  SpringJoint(const std::shared_ptr<Node> &head,
              const DirectX::XMFLOAT3 &localTailPosition, float dragForce,
              float stiffiness);
  void Update();

  DirectX::XMFLOAT4
  WorldPosToLocalRotation(const DirectX::XMFLOAT3 &nextTail) const;
  void DrawGizmo();
};

class SpringSolver {

  std::vector<SpringJoint> m_joints;

public:
  void Clear() { m_joints.clear(); }
  void Add(const std::shared_ptr<Node> &node, float dragForce,
           float stiffiness);
  void Update();
};

} // namespace vrm

namespace vrm0 {

struct Spring {
  std::string comment;
  float stiffiness = 0;
  float gravityPower = 0;
  DirectX::XMFLOAT3 gravityDir = {};
  float dragForce = 0;
  std::optional<uint32_t> center;
  float hitRadius = 0;
  std::vector<uint32_t> bones;
  std::vector<uint32_t> colliderGroups;
};
inline void from_json(const nlohmann::json &j, Spring &spring) {
  spring.stiffiness = j.at("stiffiness");
  spring.dragForce = j.at("dragForce");
  if (j.find("bones") != j.end()) {
    auto &bones = j.at("bones");
    spring.bones.assign(bones.begin(), bones.end());
  }
  if (j.find("colliderGroups") != j.end()) {
    auto &colliderGroups = j.at("colliderGroups");
    spring.colliderGroups.assign(colliderGroups.begin(), colliderGroups.end());
  }
}
inline std::ostream &operator<<(std::ostream &os, const Spring &spring) {
  os << "<spring";
  if (spring.comment.size()) {
    os << " \"" << spring.comment << "\"";
  }
  os << " dragForce:" << spring.dragForce;
  os << " stiffiness:" << spring.stiffiness;
  if (spring.bones.size()) {
    os << " bones[";
    for (int i = 0; i < spring.bones.size(); ++i) {
      if (i) {
        os << ",";
      }
      os << spring.bones[i];
    }
    os << "]";
  }
  // collision
  os << " radius:" << spring.hitRadius;
  if (auto center = spring.center) {
    os << " center:" << *center;
  }
  if (spring.colliderGroups.size()) {
    os << " colliderGroups[";
    for (int i = 0; i < spring.colliderGroups.size(); ++i) {
      if (i) {
        os << ",";
      }
      os << spring.colliderGroups[i];
    }
    os << "]";
  }
  os << ">";
  return os;
}

struct ColliderItem {
  DirectX::XMFLOAT3 offset;
  float radius = 0;
};
inline void from_json(const nlohmann::json &j, ColliderItem &collider) {
  // collider.node = j.at("node");
  // auto &colliders = j.at("colliders");
  // collider.colliders.assign(colliders.begin(), colliders.end());
  collider.radius = j.at("radius");
}
inline std::ostream &operator<<(std::ostream &os,
                                const ColliderItem &collider) {
  os << "(";
  os << collider.radius;
  os << ")";
  return os;
}

struct ColliderGroup {
  uint32_t node = 0;
  std::vector<ColliderItem> colliders;
};
inline void from_json(const nlohmann::json &j, ColliderGroup &colliderGroup) {
  colliderGroup.node = j.at("node");
  auto &colliders = j.at("colliders");
  colliderGroup.colliders.assign(colliders.begin(), colliders.end());
}
inline std::ostream &operator<<(std::ostream &os,
                                const ColliderGroup &colliderGroup) {
  os << "<colliderGroup";
  os << " node:" << colliderGroup.node;
  os << " colliders: [";
  for (int i = 0; i < colliderGroup.colliders.size(); ++i) {
    if (i) {
      os << ",";
    }
    os << colliderGroup.colliders[i];
  }
  os << "]";
  os << ">";
  return os;
}

} // namespace vrm0
