#include "spring_bone.h"
#include "gizmo.h"
#include "spring_joint.h"
#include <iostream>

inline DirectX::XMFLOAT3&
operator+=(DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.z += rhs.z;
  return lhs;
}
inline DirectX::XMFLOAT3
operator+(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}
inline DirectX::XMFLOAT3
operator-(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}
inline DirectX::XMFLOAT3
operator*(const DirectX::XMFLOAT3& lhs, float rhs)
{
  return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}
// inline std::ostream &operator<<(std::ostream &os, const DirectX::XMFLOAT3 &v)
// {
//   os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
//   return os;
// }
inline std::ostream&
operator<<(std::ostream& os, const DirectX::XMFLOAT4& v)
{
  os << "{" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "}";
  return os;
}

// // namespace DirectX {
// // inline void
// // to_json(nlohmann::json& j, const XMFLOAT3& v)
// // {
// //   j = nlohmann::json{ v.x, v.y, v.z };
// // }
// // inline void
// // from_json(const nlohmann::json& j, XMFLOAT3& v)
// // {
// //   v.x = j[0];
// //   v.y = j[1];
// //   v.z = j[2];
// // }
// // inline void
// // to_json(nlohmann::json& j, const XMFLOAT4& v)
// // {
// //   j = nlohmann::json{ v.x, v.y, v.z, v.w };
// // }
// // inline void
// // from_json(const nlohmann::json& j, XMFLOAT4& v)
// // {
// //   v.x = j[0];
// //   v.y = j[1];
// //   v.z = j[2];
// //   v.w = j[3];
// // }
// //
// // } // namespace DirectX
//
// namespace libvrm {
//
//
//
// }

namespace libvrm {

void
SpringBone::AddJoint(const std::shared_ptr<Node>& head,
                     const std::shared_ptr<Node>& tail,
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
SpringBone::AddJointRecursive(const std::shared_ptr<Node>& node,
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

} // namespace
