#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <ostream>
#include <span>
#include <string_view>

inline std::ostream &operator<<(std::ostream &os,
                                const DirectX::XMFLOAT3 &offset) {
  os << "[" << offset.x << ", " << offset.y << ", " << offset.z << "]";
  return os;
}

struct BvhTransform {
  DirectX::XMFLOAT4 Rotation;
  DirectX::XMFLOAT3 Translation;
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
  DirectX::XMFLOAT3 init;
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
