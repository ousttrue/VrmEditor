#pragma once
#include <DirectXMath.h>
#include <limits>
#include <nlohmann/json.hpp>
#include <ostream>

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

namespace DirectX {
inline void
to_json(nlohmann::json& j, const XMFLOAT3& v)
{
  j = nlohmann::json{ v.x, v.y, v.z };
}
inline void
from_json(const nlohmann::json& j, XMFLOAT3& v)
{
  v.x = j[0];
  v.y = j[1];
  v.z = j[2];
}
inline void
to_json(nlohmann::json& j, const XMFLOAT4& v)
{
  j = nlohmann::json{ v.x, v.y, v.z, v.w };
}
inline void
from_json(const nlohmann::json& j, XMFLOAT4& v)
{
  v.x = j[0];
  v.y = j[1];
  v.z = j[2];
  v.w = j[3];
}

} // namespace DirectX

struct ushort4
{
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t w;
};

struct Vertex
{
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT3 normal;
  DirectX::XMFLOAT2 uv;
};
static_assert(sizeof(Vertex) == 32, "sizeof(Vertex)");

struct JointBinding
{
  ushort4 joints;
  DirectX::XMFLOAT4 weights;
};

struct BoundingBox
{
  DirectX::XMFLOAT3 Min{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
  };
  DirectX::XMFLOAT3 Max{
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
  };

  void Extend(const DirectX::XMFLOAT3& p)
  {
    Min.x = std::min(Min.x, p.x);
    Min.y = std::min(Min.y, p.y);
    Min.z = std::min(Min.z, p.z);
    Max.x = std::max(Max.x, p.x);
    Max.y = std::max(Max.y, p.y);
    Max.z = std::max(Max.z, p.z);
  }
};
