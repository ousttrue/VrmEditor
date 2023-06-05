#pragma once
#include "../timeline.h"
#include <DirectXMath.h>
#include <chrono>
#include <ostream>
#include <span>
#include <string_view>

namespace libvrm::bvh {
inline std::ostream&
operator<<(std::ostream& os, const DirectX::XMFLOAT3& offset)
{
  os << "[" << offset.x << ", " << offset.y << ", " << offset.z << "]";
  return os;
}

struct Transform
{
  DirectX::XMFLOAT4 Rotation;
  DirectX::XMFLOAT3 Translation;
};

enum class ChannelTypes
{
  None,
  Xposition,
  Yposition,
  Zposition,
  Xrotation,
  Yrotation,
  Zrotation,
};

inline std::string_view
to_str(ChannelTypes channelType)
{
  switch (channelType) {
    case ChannelTypes::None:
      return "None";
    case ChannelTypes::Xposition:
      return "Xp";
    case ChannelTypes::Yposition:
      return "Yp";
    case ChannelTypes::Zposition:
      return "Zp";
    case ChannelTypes::Xrotation:
      return "Xr";
    case ChannelTypes::Yrotation:
      return "Yr";
    case ChannelTypes::Zrotation:
      return "Zr";
    default:
      throw std::runtime_error("unknown");
  }
}

struct Channels
{
  DirectX::XMFLOAT3 init;
  size_t startIndex;
  ChannelTypes types[6] = {};
  ChannelTypes operator[](size_t index) const { return types[index]; }
  ChannelTypes& operator[](size_t index) { return types[index]; }
  size_t size() const
  {
    size_t i = 0;
    for (; i < 6; ++i) {
      if (types[i] == ChannelTypes::None) {
        break;
      }
    }
    return i;
  }
};

inline std::ostream&
operator<<(std::ostream& os, const Channels channels)
{
  for (int i = 0; i < 6; ++i) {
    if (channels[i] == ChannelTypes::None) {
      break;
    }
    if (i) {
      os << ", ";
    }
    os << to_str(channels[i]);
  }
  return os;
}

struct Frame
{
  int index;
  Time time;
  std::span<const float> values;

  Transform Resolve(const Channels& channels) const;
};
}
