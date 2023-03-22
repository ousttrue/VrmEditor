#pragma once
#include <ostream>

struct float2 {
  float x;
  float y;
};
struct float3 {
  float x;
  float y;
  float z;
};
inline std::ostream &operator<<(std::ostream &os, const float3 &v) {
  os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
  return os;
}

struct float4 {
  float x;
  float y;
  float z;
  float w;
};
struct ushort4 {
  uint16_t joint0;
  uint16_t joint1;
  uint16_t joint2;
  uint16_t joint3;
};

struct quaternion {
  float x;
  float y;
  float z;
  float w;
};
inline std::ostream &operator<<(std::ostream &os, const quaternion &v) {
  os << "{" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "}";
  return os;
}

struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};
static_assert(sizeof(Vertex) == 32, "sizeof(Vertex)");

struct JointBinding {
  ushort4 joints;
  float4 weights;
};
