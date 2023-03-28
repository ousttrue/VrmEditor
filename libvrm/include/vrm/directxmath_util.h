#pragma once
#include <DirectXMath.h>
#include <math.h>

namespace dmath {
// inline DirectX::XMVECTOR loadFloat3(float x, float y, float z) {
//   DirectX::XMFLOAT3 v{x, y, z};
//   return DirectX::XMFLOAT3(DirectX::XMLoadFloat3(&v));
// }

inline DirectX::XMVECTOR load(const DirectX::XMFLOAT3 &v) {
  return DirectX::XMLoadFloat3(&v);
}

inline DirectX::XMFLOAT3 store(DirectX::XMVECTOR &v) {
  DirectX::XMFLOAT3 store;
  DirectX::XMStoreFloat3(&store, v);
  return store;
}

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

inline float distance(const DirectX::XMFLOAT3 &lhs,
                      const DirectX::XMFLOAT3 &rhs) {
  auto d = subtract(lhs, rhs);
  return sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
}

inline DirectX::XMFLOAT3 normalize(const DirectX::XMFLOAT3 &d) {
  auto len = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
  auto f = 1.0f / len;
  return multiply(d, f);
}

inline DirectX::XMFLOAT4 rotate_from_to(DirectX::XMFLOAT3 _lhs,
                                        DirectX::XMFLOAT3 _rhs) {
  _lhs = normalize(_lhs);
  auto lhs = DirectX::XMLoadFloat3(&_lhs);
  _rhs = normalize(_rhs);
  auto rhs = DirectX::XMLoadFloat3(&_rhs);
  auto axis = DirectX::XMVector3Cross(lhs, rhs);
  auto theta = DirectX::XMVector3Dot(lhs, rhs);
  auto angle = acos(DirectX::XMVectorGetX(theta));
  auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
  DirectX::XMFLOAT4 tmp;
  DirectX::XMStoreFloat4(&tmp, q);
  return tmp;
}

} // namespace dmath
