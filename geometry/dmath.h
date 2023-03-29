#pragma once
#include <DirectXMath.h>
#include <math.h>

namespace dmath {

inline DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3 &lhs,
                             const DirectX::XMFLOAT3 &rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline DirectX::XMFLOAT3 subtract(const DirectX::XMFLOAT3 &lhs,
                                  const DirectX::XMFLOAT3 &rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline DirectX::XMFLOAT3 multiply(const DirectX::XMFLOAT3 &lhs, float scalar) {
  return {lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

inline DirectX::XMFLOAT3 transform(const DirectX::XMFLOAT3 &lhs,
                                   const DirectX::XMFLOAT4X4 &m) {
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(
      &tmp, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lhs),
                                        DirectX::XMLoadFloat4x4(&m)));
  return tmp;
}

inline DirectX::XMFLOAT3 rotate(const DirectX::XMFLOAT3 &lhs,
                                const DirectX::XMFLOAT4 &r) {
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(&tmp,
                         DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&lhs),
                                                  DirectX::XMLoadFloat4(&r)));
  return tmp;
}

inline DirectX::XMFLOAT4 multiply(const DirectX::XMFLOAT4 &lhs,
                                  const DirectX::XMFLOAT4 &rhs) {
  DirectX::XMFLOAT4 tmp;
  DirectX::XMStoreFloat4(
      &tmp, DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&lhs),
                                          DirectX::XMLoadFloat4(&rhs)));
  return tmp;
}

inline float length(const DirectX::XMFLOAT3 &d) {
  return sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
}

inline float distance(const DirectX::XMFLOAT3 &lhs,
                      const DirectX::XMFLOAT3 &rhs) {
  auto d = subtract(lhs, rhs);
  return length(d);
}

inline DirectX::XMFLOAT3 normalized(const DirectX::XMFLOAT3 &d) {
  auto len = length(d);
  auto f = 1.0f / len;
  return multiply(d, f);
}

inline DirectX::XMFLOAT3 cross(const DirectX::XMFLOAT3 &lhs,
                               const DirectX::XMFLOAT3 &rhs) {
  DirectX::XMFLOAT3 v;
  DirectX::XMStoreFloat3(&v,
                         DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&lhs),
                                                 DirectX::XMLoadFloat3(&rhs)));
  return v;
}

inline float dot(const DirectX::XMFLOAT3 &lhs, const DirectX::XMFLOAT3 &rhs) {
  auto dot = DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&lhs),
                                   DirectX::XMLoadFloat3(&rhs));
  return DirectX::XMVectorGetX(dot);
}

inline DirectX::XMFLOAT4 axisAngle(const DirectX::XMFLOAT3 &axis, float angle) {
  DirectX::XMFLOAT4 q;
  DirectX::XMStoreFloat4(&q, DirectX::XMQuaternionRotationAxis(
                                 DirectX::XMLoadFloat3(&axis), angle));
  return q;
}

inline DirectX::XMFLOAT4 rotate_from_to(DirectX::XMFLOAT3 _lhs,
                                        DirectX::XMFLOAT3 _rhs) {
  auto lhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_lhs));
  auto rhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_rhs));
  auto axis = DirectX::XMVector3Cross(lhs, rhs);
  auto dot = DirectX::XMVector3Dot(lhs, rhs);
  auto angle = acos(DirectX::XMVectorGetX(dot));
  auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
  DirectX::XMFLOAT4 tmp;
  DirectX::XMStoreFloat4(&tmp, q);
  return tmp;
}

} // namespace dmath
