#pragma once
#include <functional>
#include <memory>

namespace grapho {

namespace camera {
struct OrbitView;
}

namespace gl3 {
struct Fbo;
class Texture;
}

} // namespace

namespace glr {
using RenderFunc = std::function<void(const grapho::camera::OrbitView& view)>;

struct RenderTarget
{
  std::shared_ptr<grapho::camera::OrbitView> View;
  std::shared_ptr<grapho::gl3::Fbo> Fbo;
  std::shared_ptr<grapho::gl3::Texture> FboTexture;
  RenderFunc render;

  RenderTarget(const std::shared_ptr<grapho::camera::OrbitView>& view);
  int m_width;
  int m_height;
  uint32_t Begin(int width, int height, const float color[4]);
  void End(bool isActive,
           bool isHovered,
           bool isRightDown,
           bool isMiddleDown,
           int mouseDeltaX,
           int mouseDeltaY,
           int mouseWheel);
};

} // namespace
