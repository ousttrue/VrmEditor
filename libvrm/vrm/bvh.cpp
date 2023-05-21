#include <assert.h>
#include <cctype>
#include <charconv>
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <vrm/bvh.h>
#include <vrm/fileutil.h>

template<typename T>
std::optional<T>
to_num(const std::string& src);

template<>
std::optional<int>
to_num<int>(const std::string& src)
{
  return std::stoi(src);
}

template<>
std::optional<float>
to_num<float>(const std::string& src)
{
  return std::stof(src);
}

using It = std::string_view::iterator;
struct Result
{
  It end;
  It next;
};
using Delimiter = std::function<std::optional<Result>(It, It)>;

class Tokenizer
{
  std::string_view m_data;
  std::string_view::iterator m_pos;

public:
  Tokenizer(std::string_view data)
    : m_data(data)
  {
    m_pos = m_data.begin();
  }

  std::optional<std::string_view> token(const Delimiter& delimiter)
  {
    auto begin = m_pos;

    auto end = begin;

    for (; end != m_data.end(); ++end) {
      if (auto found = delimiter(end, m_data.end())) {
        auto [tail, next] = *found;
        m_pos = next;
        return std::string_view(begin, tail);
      }
    }

    return std::string_view(begin, end);
  }

  bool expect(std::string_view expected, const Delimiter& delimiter)
  {
    if (auto line = token(delimiter)) {
      if (*line == expected) {
        return true;
      }
    }
    return false;
  }

  template<typename T>
  std::optional<T> number(const Delimiter& delimiter)
  {
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

static std::optional<Result>
is_space(It it, It end)
{
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
  return Result{ tail, it };
}

static std::optional<Result>
get_eol(It it, It end)
{
  if (*it != '\n') {
    return {};
  }
  auto tail = it;
  ++it;
  return Result{ tail, it };
}

static std::optional<Result>
get_name(It it, It end)
{
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
  return Result{ tail, it };
}

namespace libvrm::bvh {
struct BvhImpl
{
  Tokenizer token_;
  std::vector<Joint>& joints_;
  std::vector<Joint>& endsites_;
  std::vector<float>& frames_;
  uint32_t frame_count_ = 0;
  Time frame_time_ = {};
  uint32_t channel_count_ = 0;
  float max_height_ = 0;

  BvhImpl(std::vector<Joint>& joints,
          std::vector<Joint>& endsites,
          std::vector<float>& frames,
          std::string_view src)
    : token_(src)
    , joints_(joints)
    , endsites_(endsites)
    , frames_(frames)
  {
  }

  std::vector<int> stack_;

  std::expected<bool, std::string> Parse()
  {
    if (!token_.expect("HIERARCHY", is_space)) {
      return std::unexpected{ "format: no HIERARCHY" };
    }

    if (auto parsed = ParseJoint()) {
    } else {
      return parsed;
    }

    if (!token_.expect("Frames:", is_space)) {
      return std::unexpected{ "format: no Frames:" };
    }
    auto frames = token_.number<int>(is_space);
    if (!frames) {
      return std::unexpected{ "format: no frame number" };
    }
    frame_count_ = *frames;

    if (!token_.expect("Frame", is_space)) {
      return std::unexpected{ "format: Frame" };
    }
    if (!token_.expect("Time:", is_space)) {
      return std::unexpected{ "format: Time" };
    }
    auto frameTime = token_.number<float>(is_space);
    if (!frameTime) {
      return std::unexpected{ "format: no frame time" };
    }
    frame_time_ = Time(*frameTime);

    // each frame
    channel_count_ = 0;
    for (auto& joint : joints_) {
      channel_count_ += joint.channels.size();
    }
    frames_.reserve(frame_count_ * channel_count_);
    for (int i = 0; i < frame_count_; ++i) {
      auto line = token_.token(get_eol);
      if (!line) {
        return std::unexpected{ "format: no line" };
      }

      Tokenizer line_token(*line);
      for (int j = 0; j < channel_count_; ++j) {
        if (auto value = line_token.number<float>(is_space)) {
          frames_.push_back(*value);
        } else {
          std::stringstream ss;
          ss << "format:" << i << ",(" << j << "/" << channel_count_
             << "): no line value";
          return std::unexpected{ ss.str() };
        }
      }
    }

    size_t frame_channel_count = frame_count_ * channel_count_;
    if (frames_.size() != frame_channel_count) {
      std::stringstream ss;
      ss << "format: invalid count:" << frames_.size() << "!=" << frame_count_
         << "x" << channel_count_ << "=" << frame_count_ * channel_count_;
      return std::unexpected{ ss.str() };
    }

    return true;
  }

private:
  std::expected<bool, std::string> ParseJoint()
  {
    while (true) {
      auto token = token_.token(is_space);
      if (!token) {
        return std::unexpected{ "format: no token" };
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
          return std::unexpected{ "format: no name" };
        }

        // for (size_t i = 0; i < stack_.size(); ++i) {
        //   std::cout << "  ";
        // }
        // std::cout << *name << std::endl;

        if (!token_.expect("{", is_space)) {
          return std::unexpected{ "format: {" };
        }

        auto index = joints_.size();
        auto offset = ParseOffset();
        if (!offset) {
          return std::unexpected{ "format: offset" };
        }
        auto channels = ParseChannels();
        if (!channels) {
          return std::unexpected{ "format: channels" };
        }
        channels->init = *offset;
        channels->startIndex = joints_.empty()
                                 ? 0
                                 : joints_.back().channels.startIndex +
                                     joints_.back().channels.size();

        joints_.push_back(Joint{
          .name = { name->begin(), name->end() },
          .index = static_cast<uint16_t>(index),
          .localOffset = *offset,
          .worldOffset = *offset,
          .channels = *channels,
        });
        if (stack_.size()) {
          joints_.back().parent = stack_.back();
        }
        if (stack_.size()) {
          auto& parent = joints_[stack_.back()];
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
          return std::unexpected{ "format: no Site" };
        }

        if (!token_.expect("{", is_space)) {
          return std::unexpected{ "format: {" };
        }
        auto offset = ParseOffset();
        if (!offset) {
          return std::unexpected{ "format: offset" };
        }
        endsites_.push_back(Joint{
          .name = "End Site",
          .parent = static_cast<uint16_t>(stack_.empty() ? -1 : stack_.back()),
          .localOffset = *offset,
        });

        if (!token_.expect("}", is_space)) {
          return std::unexpected{ "}" };
        }
      } else if (*token == "}") {
        stack_.pop_back();
        return true;
      } else if (*token == "MOTION") {
        return true;
      } else {
        return std::unexpected{ "format: unknown" };
      }
    }

    return std::unexpected("not reach here");
  }

  std::optional<DirectX::XMFLOAT3> ParseOffset()
  {
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

    return DirectX::XMFLOAT3{ *x, *y, *z };
  }

  std::optional<Channels> ParseChannels()
  {
    if (!token_.expect("CHANNELS", is_space)) {
      return {};
    }

    auto n = token_.number<int>(is_space);
    if (!n) {
      return {};
    }
    auto channel_count = *n;
    auto channels = Channels{};
    for (int i = 0; i < channel_count; ++i) {
      if (auto channel = token_.token(is_space)) {
        if (*channel == "Xposition") {
          channels[i] = ChannelTypes::Xposition;
        } else if (*channel == "Yposition") {
          channels[i] = ChannelTypes::Yposition;
        } else if (*channel == "Zposition") {
          channels[i] = ChannelTypes::Zposition;
        } else if (*channel == "Xrotation") {
          channels[i] = ChannelTypes::Xrotation;
        } else if (*channel == "Yrotation") {
          channels[i] = ChannelTypes::Yrotation;
        } else if (*channel == "Zrotation") {
          channels[i] = ChannelTypes::Zrotation;
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

std::expected<std::shared_ptr<Bvh>, std::string>
Bvh::FromFile(const std::filesystem::path& path)
{
  auto bytes = fileutil::ReadAllBytes(path);
  if (bytes.empty()) {
    return std::unexpected{ std::string("fail to read: " + path.string()) };
  }

  auto ptr = std::make_shared<Bvh>();
  if (auto result = ptr->Parse({
        (const char*)bytes.data(),
        (const char*)(bytes.data() + bytes.size()),
      })) {
    return ptr;
  } else {
    return std::unexpected{ result.error() };
  }
}

std::expected<bool, std::string>
Bvh::Parse(std::string_view src)
{
  BvhImpl parser(joints, endsites, frames, src);
  auto result = parser.Parse();
  if (result) {
    frame_time = parser.frame_time_;
    frame_channel_count = parser.channel_count_;
    max_height = parser.max_height_;
  }
  return result;
}

}
