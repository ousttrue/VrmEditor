#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <ostream>
#include <span>

struct BvhOffset {
  float x;
  float y;
  float z;

  BvhOffset &operator+=(const BvhOffset &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
};
inline std::ostream &operator<<(std::ostream &os, const BvhOffset &offset) {
  os << "[" << offset.x << ", " << offset.y << ", " << offset.z << "]";
  return os;
}

///
/// Mat3 for bvh rotation
///
/// cos sin
///-sin cos
///
///        [0, 1, 2]
/// [x,y,z][3, 4, 5] => [0x + 3y + 6z][1x + 4y + 7z][2x + 5y + 8z]
///        [6, 7, 8]
///
struct BvhMat3 {
  float _0 = 1;
  float _1 = 0;
  float _2 = 0;
  float _3 = 0;
  float _4 = 1;
  float _5 = 0;
  float _6 = 0;
  float _7 = 0;
  float _8 = 1;
  static BvhMat3 RotateXDegrees(float degree);
  static BvhMat3 RotateYDegrees(float degree);
  static BvhMat3 RotateZDegrees(float degree);
  BvhMat3 operator*(const BvhMat3 &rhs);
  BvhMat3 Transpose() const {
    return {
        _0, _3, _6, //
        _1, _4, _7, //
        _2, _5, _8  //
    };
  }
};

struct BvhTransform {
  BvhMat3 Rotation;
  BvhOffset Translation;
};

enum class BvhChannelTypes {
  None,
  Xposition,
  Yposition,
  Zposition,
  Xrotation,
  Yrotation,
  Zrotation,
};

inline std::string_view to_str(BvhChannelTypes channelType) {
  switch (channelType) {
  case BvhChannelTypes::None:
    return "None";
  case BvhChannelTypes::Xposition:
    return "Xp";
  case BvhChannelTypes::Yposition:
    return "Yp";
  case BvhChannelTypes::Zposition:
    return "Zp";
  case BvhChannelTypes::Xrotation:
    return "Xr";
  case BvhChannelTypes::Yrotation:
    return "Yr";
  case BvhChannelTypes::Zrotation:
    return "Zr";
  default:
    throw std::runtime_error("unknown");
  }
}

struct BvhChannels {
  BvhOffset init;
  size_t startIndex;
  BvhChannelTypes types[6] = {};
  BvhChannelTypes operator[](size_t index) const { return types[index]; }
  BvhChannelTypes &operator[](size_t index) { return types[index]; }
  size_t size() const {
    size_t i = 0;
    for (; i < 6; ++i) {
      if (types[i] == BvhChannelTypes::None) {
        break;
      }
    }
    return i;
  }
};

inline std::ostream &operator<<(std::ostream &os, const BvhChannels channels) {
  for (int i = 0; i < 6; ++i) {
    if (channels[i] == BvhChannelTypes::None) {
      break;
    }
    if (i) {
      os << ", ";
    }
    os << to_str(channels[i]);
  }
  return os;
}

using BvhTime = std::chrono::duration<float, std::ratio<1, 1>>;

struct BvhFrame {
  int index;
  BvhTime time;
  std::span<const float> values;

  BvhTransform Resolve(const BvhChannels &channels) const;
};
