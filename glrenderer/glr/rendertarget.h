#pragma once
#include <functional>
#include <memory>

namespace grapho {

namespace camera {
struct Camera;
}

namespace gl3 {
struct Fbo;
class Texture;
}

} // namespace

namespace glr {
using RenderFunc = std::function<void(const grapho::camera::Camera& camera)>;

struct RenderTarget
{
  std::shared_ptr<grapho::camera::Camera> Camera;
  std::shared_ptr<grapho::gl3::Fbo> Fbo;
  std::shared_ptr<grapho::gl3::Texture> FboTexture;
  RenderFunc render;

  RenderTarget(const std::shared_ptr<grapho::camera::Camera>& camera);
  uint32_t Begin(float x,
                 float y,
                 float width,
                 float height,
                 const float color[4]);
  void End(bool isActive,
           bool isHovered,
           bool isRightDown,
           bool isMiddleDown,
           int mouseDeltaX,
           int mouseDeltaY,
           int mouseWheel);
};

} // namespace
