#pragma once
#include <DirectXMath.h>
#include <ostream>

inline std::ostream&
operator<<(std::ostream& os, const DirectX::XMFLOAT3& v)
{
  os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
  return os;
}

namespace dmath {

inline DirectX::XMFLOAT3
add(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline DirectX::XMFLOAT3
subtract(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline DirectX::XMFLOAT3
multiply(const DirectX::XMFLOAT3& lhs, float scalar)
{
  return { lhs.x * scalar, lhs.y * scalar, lhs.z * scalar };
}

inline DirectX::XMFLOAT3
transform(const DirectX::XMFLOAT3& lhs, const DirectX::XMMATRIX& m)
{
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(
    &tmp, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lhs), m));
  return tmp;
}

inline DirectX::XMFLOAT3
transform(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT4X4& m)
{
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(
    &tmp,
    DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lhs),
                                DirectX::XMLoadFloat4x4(&m)));
  return tmp;
}

inline DirectX::XMFLOAT3
rotate(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT4& r)
{
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(&tmp,
                         DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&lhs),
                                                  DirectX::XMLoadFloat4(&r)));
  return tmp;
}

inline DirectX::XMFLOAT4
multiply(const DirectX::XMFLOAT4& lhs, const DirectX::XMFLOAT4& rhs)
{
  DirectX::XMFLOAT4 tmp;
  DirectX::XMStoreFloat4(
    &tmp,
    DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&lhs),
                                  DirectX::XMLoadFloat4(&rhs)));
  return tmp;
}

inline float
length(const DirectX::XMFLOAT3& d)
{
  return DirectX::XMVectorGetX(
    DirectX::XMVector3Length(DirectX::XMLoadFloat3(&d)));
}

inline float
distance(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  auto d = subtract(lhs, rhs);
  return length(d);
}

inline DirectX::XMFLOAT3
normalized(const DirectX::XMFLOAT3& d)
{
  auto len = length(d);
  auto f = 1.0f / len;
  return multiply(d, f);
}

inline DirectX::XMFLOAT3
cross(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  DirectX::XMFLOAT3 v;
  DirectX::XMStoreFloat3(&v,
                         DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&lhs),
                                                 DirectX::XMLoadFloat3(&rhs)));
  return v;
}

inline float
dot(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
  auto dot = DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&lhs),
                                   DirectX::XMLoadFloat3(&rhs));
  return DirectX::XMVectorGetX(dot);
}

inline DirectX::XMFLOAT4
axisAngle(const DirectX::XMFLOAT3& axis, float angle)
{
  DirectX::XMFLOAT4 q;
  DirectX::XMStoreFloat4(
    &q, DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&axis), angle));
  return q;
}

// inline DirectX::XMVECTOR
// rotate_from_to(DirectX::XMFLOAT3 _lhs, DirectX::XMFLOAT3 _rhs)
// {
//   auto lhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_lhs));
//   auto rhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_rhs));
//   auto axis = DirectX::XMVector3Cross(lhs, rhs);
//   auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(lhs, rhs));
//   if (fabs(1 - dot) < 1e-4) {
//     return { 0, 0, 0, 1 };
//   }
//   auto angle = acos(dot);
//   return DirectX::XMQuaternionRotationAxis(axis, angle);
// }

inline DirectX::XMVECTOR
rotate_from_to(DirectX::XMVECTOR lhs, DirectX::XMVECTOR rhs)
{
  lhs = DirectX::XMVector3Normalize(lhs);
  rhs = DirectX::XMVector3Normalize(rhs);
  auto axis = DirectX::XMVector3Cross(lhs, rhs);
  auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(lhs, rhs));
  if (fabs(1 - fabs(dot)) < 1e-4) {
    return { 0, 0, 0, 1 };
  }
  auto angle = acos(dot);
  return DirectX::XMQuaternionRotationAxis(axis, angle);
}

inline DirectX::XMVECTOR
rotate_from_to(DirectX::XMFLOAT3 _lhs, DirectX::XMFLOAT3 _rhs)
{
  return rotate_from_to(DirectX::XMLoadFloat3(&_lhs),
                        DirectX::XMLoadFloat3(&_rhs));
}

inline DirectX::XMFLOAT4X4
trs(const DirectX::XMFLOAT3& translation,
    const DirectX::XMFLOAT4& rotation,
    const DirectX::XMFLOAT3& scale)
{
  auto t =
    DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
  auto r =
    DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
  auto s = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, s * r * t);
  return m;
}

} // namespace dmath
