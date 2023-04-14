#pragma once
#include <cmath>
#include <stdint.h>

//
// from
// https://github.com/i-saint/Glimmer/blob/master/Source/MeshUtils/muQuat32.h
//
namespace libvrm::quat_packer {

static constexpr float SR2 = 1.41421356237f;
static constexpr float RSR2 = 1.0f / 1.41421356237f;
static constexpr float C = float(0x3ff);
static constexpr float R = 1.0f / float(0x3ff);

inline constexpr uint32_t
pack(float a)
{
  return static_cast<uint32_t>((a * SR2 + 1.0f) * 0.5f * C);
}
inline constexpr float
unpack(uint32_t a)
{
  return ((a * R) * 2.0f - 1.0f) * RSR2;
}
inline constexpr float
square(float a)
{
  return a * a;
}
inline int
dropmax(float a, float b, float c, float d)
{
  if (a > b && a > c && a > d)
    return 0;
  if (b > c && b > d)
    return 1;
  if (c > d)
    return 2;
  return 3;
}
inline float
sign(float v)
{
  return v < float(0.0) ? float(-1.0) : float(1.0);
}

union Packed
{
  uint32_t value;
  struct
  {
    uint32_t x0 : 10;
    uint32_t x1 : 10;
    uint32_t x2 : 10;
    uint32_t drop : 2;
  };
};

inline uint32_t
Pack(float x, float y, float z, float w)
{

  Packed value;

  float a0, a1, a2;
  value.drop = dropmax(square(x), square(y), square(z), square(w));
  if (value.drop == 0) {
    float s = sign(x);
    a0 = y * s;
    a1 = z * s;
    a2 = w * s;
  } else if (value.drop == 1) {
    float s = sign(y);
    a0 = x * s;
    a1 = z * s;
    a2 = w * s;
  } else if (value.drop == 2) {
    float s = sign(z);
    a0 = x * s;
    a1 = y * s;
    a2 = w * s;
  } else {
    float s = sign(w);
    a0 = x * s;
    a1 = y * s;
    a2 = z * s;
  }

  value.x0 = pack(a0);
  value.x1 = pack(a1);
  value.x2 = pack(a2);

  return *(uint32_t*)&value;
}

inline void
Unpack(uint32_t src, float values[4])
{

  auto value = (Packed*)&src;

  const float a0 = unpack(value->x0);
  const float a1 = unpack(value->x1);
  const float a2 = unpack(value->x2);
  const float iss = std::sqrt(1.0f - (square(a0) + square(a1) + square(a2)));

  switch (value->drop) {
    case 0: {
      values[0] = iss;
      values[1] = a0;
      values[2] = a1;
      values[3] = a2;
      break;
    }
    case 1: {
      values[0] = a0;
      values[1] = iss;
      values[2] = a1;
      values[3] = a2;
      break;
    }
    case 2: {
      values[0] = a0;
      values[1] = a1;
      values[2] = iss;
      values[3] = a2;
      break;
    }
    default: {
      values[0] = a0;
      values[1] = a1;
      values[2] = a2;
      values[3] = iss;
      break;
    }
  }
}

}
