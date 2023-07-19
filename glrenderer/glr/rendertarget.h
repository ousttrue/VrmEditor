#pragma once
#include <functional>
#include <memory>

namespace grapho {

namespace camera {
struct Viewport;
struct MouseState;
}

namespace gl3 {
struct RenderTarget;
}

} // namespace

namespace glr {
using RenderFunc = std::function<void(const grapho::camera::Viewport&,
                                      const grapho::camera::MouseState&)>;

struct RenderTarget
{
  std::shared_ptr<grapho::gl3::RenderTarget> RT;
  RenderFunc Render;

  RenderTarget(const RenderFunc& render);
  uint32_t Begin(float width, float height, const float color[4]);
  void End(const grapho::camera::Viewport& viewport,
           const grapho::camera::MouseState& mouse);
};

} // namespace
