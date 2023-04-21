#pragma once
#include <DirectXMath.h>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace vrm {

struct ColliderItem
{
  DirectX::XMFLOAT3 offset;
  float radius = 0;
};
// inline void
// from_json(const nlohmann::json& j, ColliderItem& collider)
// {
//   // collider.node = j.at("node");
//   // auto &colliders = j.at("colliders");
//   // collider.colliders.assign(colliders.begin(), colliders.end());
//   collider.radius = j.at("radius");
// }
// inline std::ostream&
// operator<<(std::ostream& os, const ColliderItem& collider)
// {
//   os << "(";
//   os << collider.radius;
//   os << ")";
//   return os;
// }

struct ColliderGroup
{
  uint32_t node = 0;
  std::vector<ColliderItem> colliders;
};
// inline void
// from_json(const nlohmann::json& j, ColliderGroup& colliderGroup)
// {
//   colliderGroup.node = j.at("node");
//   auto& colliders = j.at("colliders");
//   colliderGroup.colliders.assign(colliders.begin(), colliders.end());
// }
// inline std::ostream&
// operator<<(std::ostream& os, const ColliderGroup& colliderGroup)
// {
//   os << "<colliderGroup";
//   os << " node:" << colliderGroup.node;
//   os << " colliders: [";
//   for (int i = 0; i < colliderGroup.colliders.size(); ++i) {
//     if (i) {
//       os << ",";
//     }
//     os << colliderGroup.colliders[i];
//   }
//   os << "]";
//   os << ">";
//   return os;
// }

}
}
