#pragma once
#include "bvhframe.h"
#include <chrono>
#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <span>
#include <vector>

namespace libvrm {
namespace bvh {
struct Joint
{
  std::string name;
  uint16_t index;
  std::optional<uint16_t> parent;
  DirectX::XMFLOAT3 localOffset;
  DirectX::XMFLOAT3 worldOffset;
  Channels channels;
};

inline std::ostream&
operator<<(std::ostream& os, const Joint& joint)
{
  os << joint.name << ": " << joint.worldOffset << " " << joint.channels;
  return os;
}

struct Bvh
{
  std::vector<Joint> joints;
  std::vector<Joint> endsites;
  Time frame_time = {};
  std::vector<float> frames;
  uint32_t frame_channel_count = 0;
  float max_height = 0;
  Bvh();
  ~Bvh();
  std::expected<bool, std::string> Parse(std::string_view src);
  static std::expected<std::shared_ptr<Bvh>, std::string> FromFile(
    const std::filesystem::path& path);
  uint32_t FrameCount() const { return frames.size() / frame_channel_count; }
  const Joint* GetParent(int parent) const
  {
    for (auto& joint : joints) {
      if (joint.parent == parent) {
        return &joint;
      }
    }
    return nullptr;
  }
  int TimeToIndex(Time time) const
  {
    auto div = time / frame_time;
    auto index = (int)div;
    if (index >= FrameCount()) {
      index = index % FrameCount();
    }
    return index;
  }
  Frame GetFrame(int index) const
  {
    auto begin = frames.data() + index * frame_channel_count;
    return {
      .index = index,
      .time = frame_time * index,
      .values = { begin, begin + frame_channel_count },
    };
  }
  float GuessScaling() const
  {
    // guess bvh scale
    float scalingFactor = 1.0f;
    if (max_height < 2) {
      // maybe meter scale. do nothing
    } else if (max_height < 200) {
      // maybe cm scale
      scalingFactor = 0.01f;
    }
    return scalingFactor;
  }
  std::chrono::milliseconds Duration() const
  {
    int channel_count = 0;
    for (auto& joint : joints) {
      channel_count += joint.channels.size();
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      frame_time * (frames.size() / channel_count));
  }
};

inline std::ostream&
operator<<(std::ostream& os, const Bvh& bvh)
{
  // int channel_count = 0;
  // for (auto &joint : bvh.joints) {
  //   channel_count += joint.channels.size();
  // }

  os << "<BVH: "
     << bvh.joints.size()
     // << " joints: " << (bvh.frames.size() / channel_count) //
     // << " frames/" << bvh.frame_time                       //
     // << " max_height: " << bvh.max_height                  //
     << std::endl;
  for (auto joint : bvh.joints) {
    os << "  " << joint << std::endl;
  }
  os << ">" << std::endl;

  return os;
}
}
}
