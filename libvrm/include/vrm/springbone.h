#pragma once
#include <DirectXMath.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <stdint.h>
#include <vector>

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
