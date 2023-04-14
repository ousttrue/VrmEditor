#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <span>
#include <string_view>
#include <vrm/bvh.h>
#include <vrm/bvhframe.h>

namespace asio {
class io_context;
} // namespace asio

class Animation
{
  struct AnimationImpl* impl_ = nullptr;

public:
  using OnFrameFunc = std::function<void(const libvrm::bvh::Frame& frame)>;

  Animation(asio::io_context& io);
  ~Animation();
  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh);
  void OnFrame(const OnFrameFunc& onFrame);
  void Stop();
};
