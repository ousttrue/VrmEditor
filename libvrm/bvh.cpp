#include <vrm/Bvh.h>
#include "ReadAllBytes.h"
#include <assert.h>
#include <cctype>
#include <charconv>
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdlib.h>

template <typename T> std::optional<T> to_num(const std::string &src);

template <> std::optional<int> to_num<int>(const std::string &src) {
  return std::stoi(src);
}

template <> std::optional<float> to_num<float>(const std::string &src) {
  return std::stof(src);
}

using It = std::string_view::iterator;
struct Result {
  It end;
  It next;
};
using Delimiter = std::function<std::optional<Result>(It, It)>;

class Tokenizer {
  std::string_view m_data;
  std::string_view::iterator m_pos;

public:
  Tokenizer(std::string_view data) : m_data(data) { m_pos = m_data.begin(); }

  std::optional<std::string_view> token(const Delimiter &delimiter) {
    auto begin = m_pos;

    auto end = begin;

    for (; end != m_data.end(); ++end) {
      if (auto found = delimiter(end, m_data.end())) {
        auto [tail, next] = *found;
        m_pos = next;
        return std::string_view(begin, tail);
      }
    }

    return {};
  }

  bool expect(std::string_view expected, const Delimiter &delimiter) {
    if (auto line = token(delimiter)) {
      if (*line == expected) {
        return true;
      }
    }
    return false;
  }

  template <typename T> std::optional<T> number(const Delimiter &delimiter) {
    auto n = token(delimiter);
    if (!n) {
      return {};
    }
    if (auto value = to_num<T>(std::string(n->begin(), n->end()))) {
      return *value;
    } else {
      return {};
    }
  }
};

static std::optional<Result> is_space(It it, It end) {
  if (!std::isspace(*it)) {
    return {};
  }
  auto tail = it;
  ++it;
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return Result{tail, it};
}

static std::optional<Result> get_eol(It it, It end) {
  if (*it != '\n') {
    return {};
  }
  auto tail = it;
  ++it;
  return Result{tail, it};
}

static std::optional<Result> get_name(It it, It end) {
  if (*it != '\n') {
    return {};
  }
  auto tail = it;
  ++it;
  // head space
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return Result{tail, it};
}

struct BvhImpl {
  Tokenizer token_;
  std::vector<BvhJoint> &joints_;
  std::vector<BvhJoint> &endsites_;
  std::vector<float> &frames_;
  uint32_t frame_count_ = 0;
  BvhTime frame_time_ = {};
  uint32_t channel_count_ = 0;
  float max_height_ = 0;

  BvhImpl(std::vector<BvhJoint> &joints, std::vector<BvhJoint> &endsites,
          std::vector<float> &frames, std::string_view src)
      : token_(src), joints_(joints), endsites_(endsites), frames_(frames) {}

  std::vector<int> stack_;

  bool Parse() {
    if (!token_.expect("HIERARCHY", is_space)) {
      return false;
    }

    if (!ParseJoint()) {
      return false;
    }

    if (!token_.expect("Frames:", is_space)) {
      return false;
    }
    auto frames = token_.number<int>(is_space);
    if (!frames) {
      return false;
    }
    frame_count_ = *frames;

    if (!token_.expect("Frame", is_space)) {
      return false;
    }
    if (!token_.expect("Time:", is_space)) {
      return false;
    }
    auto frameTime = token_.number<float>(is_space);
    if (!frameTime) {
      return false;
    }
    frame_time_ = BvhTime(*frameTime);

    // each frame
    channel_count_ = 0;
    for (auto &joint : joints_) {
      channel_count_ += joint.channels.size();
    }
    frames_.reserve(frame_count_ * channel_count_);
    for (int i = 0; i < frame_count_; ++i) {
      auto line = token_.token(get_eol);
      if (!line) {
        return false;
      }

      Tokenizer line_token(*line);
      for (int j = 0; j < channel_count_; ++j) {
        if (auto value = line_token.number<float>(is_space)) {
          frames_.push_back(*value);
        } else {
          return false;
        }
      }
    }
    assert(frames_.size() == frame_count_ * channel_count_);

    return true;
  }

private:
  bool ParseJoint() {
    while (true) {
      auto token = token_.token(is_space);
      if (!token) {
        return false;
      }

      if (*token == "ROOT" || *token == "JOINT") {
        // name
        // {
        // OFFSET x y z
        // CHANNELS 6
        // X {
        // }
        
        // }
        auto name = token_.token(get_name);
        if (!name) {
          return false;
        }

        // for (size_t i = 0; i < stack_.size(); ++i) {
        //   std::cout << "  ";
        // }
        // std::cout << *name << std::endl;

        if (!token_.expect("{", is_space)) {
          return false;
        }

        auto index = joints_.size();
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        auto channels = ParseChannels();
        if (!channels) {
          return false;
        }
        channels->init = *offset;
        channels->startIndex = joints_.empty()
                                   ? 0
                                   : joints_.back().channels.startIndex +
                                         joints_.back().channels.size();

        auto parentIndex = stack_.empty() ? -1 : stack_.back();
        // auto parent = stack_.empty() ? nullptr : &joints_[parentIndex];
        joints_.push_back(BvhJoint{
            .name = {name->begin(), name->end()},
            .index = static_cast<uint16_t>(index),
            .parent = static_cast<uint16_t>(parentIndex),
            .localOffset = *offset,
            .worldOffset = *offset,
            .channels = *channels,
        });
        if (stack_.size()) {
          auto &parent = joints_[stack_.back()];
          joints_.back().worldOffset.x += parent.worldOffset.x;
          joints_.back().worldOffset.y += parent.worldOffset.y;
          joints_.back().worldOffset.z += parent.worldOffset.z;
        }

        max_height_ = std::max(max_height_, joints_.back().worldOffset.y);

        stack_.push_back(index);

        ParseJoint();

      } else if (*token == "End") {
        // End Site
        // {
        // OFFSET x y z
        // }
        if (!token_.expect("Site", get_name)) {
          return false;
        }

        if (!token_.expect("{", is_space)) {
          return false;
        }
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        endsites_.push_back(BvhJoint{
            .name = "End Site",
            .parent =
                static_cast<uint16_t>(stack_.empty() ? -1 : stack_.back()),
            .localOffset = *offset,
        });

        if (!token_.expect("}", is_space)) {
          return false;
        }
      } else if (*token == "}") {
        stack_.pop_back();
        return true;
      } else if (*token == "MOTION") {
        return true;
      } else {
        throw std::runtime_error("unknown");
      }
    }

    throw std::runtime_error("not reach here");
  }

  std::optional<DirectX::XMFLOAT3> ParseOffset() {
    if (!token_.expect("OFFSET", is_space)) {
      return {};
    }
    auto x = token_.number<float>(is_space);
    if (!x) {
      return {};
    }
    auto y = token_.number<float>(is_space);
    if (!y) {
      return {};
    }
    auto z = token_.number<float>(is_space);
    if (!z) {
      return {};
    }

    return DirectX::XMFLOAT3{*x, *y, *z};
  }

  std::optional<BvhChannels> ParseChannels() {
    if (!token_.expect("CHANNELS", is_space)) {
      return {};
    }

    auto n = token_.number<int>(is_space);
    if (!n) {
      return {};
    }
    auto channel_count = *n;
    auto channels = BvhChannels{};
    for (int i = 0; i < channel_count; ++i) {
      if (auto channel = token_.token(is_space)) {
        if (*channel == "Xposition") {
          channels[i] = BvhChannelTypes::Xposition;
        } else if (*channel == "Yposition") {
          channels[i] = BvhChannelTypes::Yposition;
        } else if (*channel == "Zposition") {
          channels[i] = BvhChannelTypes::Zposition;
        } else if (*channel == "Xrotation") {
          channels[i] = BvhChannelTypes::Xrotation;
        } else if (*channel == "Yrotation") {
          channels[i] = BvhChannelTypes::Yrotation;
        } else if (*channel == "Zrotation") {
          channels[i] = BvhChannelTypes::Zrotation;
        } else {
          throw std::runtime_error("unknown");
        }
      }
    }
    return channels;
  }
};

Bvh::Bvh() {}
Bvh::~Bvh() {}
bool Bvh::Parse(std::string_view src) {
  BvhImpl parser(joints, endsites, frames, src);
  if (!parser.Parse()) {
    return false;
  }
  frame_time = parser.frame_time_;
  frame_channel_count = parser.channel_count_;
  max_height = parser.max_height_;
  return true;
}

std::shared_ptr<Bvh> Bvh::ParseFile(const std::filesystem::path &file) {
  auto bytes = ReadAllBytes<char>(file);
  if (bytes.empty()) {
    return {};
  }
  std::cout << "load: " << file << " " << bytes.size() << "bytes" << std::endl;

  auto bvh = std::make_shared<Bvh>();
  if (!bvh->Parse({bytes.begin(), bytes.end()})) {
    return {};
  }

  std::cout << *bvh << std::endl;
  return bvh;
}
