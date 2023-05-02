#pragma once
#include "gizmo.h"
#include "node.h"
#include <DirectXMath.h>
#include <memory>
#include <optional>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace gltf {
struct Node;
}

namespace vrm {

enum class SpringColliderShapeType
{
  Sphere,
  Capsule,
};

struct SpringCollider
{
  std::shared_ptr<gltf::Node> Node;
  SpringColliderShapeType Type = SpringColliderShapeType::Sphere;
  DirectX::XMFLOAT3 Offset = { 0, 0, 0 };
  float Radius = 0;
  DirectX::XMFLOAT3 Tail = { 0, 0, 0 };
  void DrawGizmo(IGizmoDrawer* gizmo);
  DirectX::XMVECTOR Position() const;
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

struct SpringColliderGroup
{
  std::vector<std::shared_ptr<SpringCollider>> Colliders;
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
