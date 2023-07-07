#include <GL/glew.h>

#include "rendertarget.h"
#include <functional>
#include <glr/rendering_env.h>
#include <grapho/camera/camera.h>
#include <grapho/gl3/fbo.h>
#include <memory>

namespace glr {
RenderTarget::RenderTarget(
  const std::shared_ptr<grapho::camera::Camera>& camera)
  : Camera(camera)
{
  if (!Camera) {
    Camera = std::make_shared<grapho::camera::Camera>();
  }
}

uint32_t
RenderTarget::Begin(int x, int y, int width, int height, const float color[4])
{
  if (width == 0 || height == 0) {
    return 0;
  }
  if (!Fbo) {
    Fbo = std::make_shared<grapho::gl3::Fbo>();
  }

  if (FboTexture) {
    if (FboTexture->Width() != width || FboTexture->Height() != height) {
      FboTexture = nullptr;
    }
  }
  if (!FboTexture) {
    FboTexture = grapho::gl3::Texture::Create({
      width,
      height,
      grapho::PixelFormat::u8_RGB,
      grapho::ColorSpace::Linear,
    });
    Fbo->AttachTexture2D(FboTexture->Handle());
    Fbo->AttachDepth(width, height);
  }

  Fbo->Bind();
  grapho::gl3::ClearViewport({
    .Width = width,
    .Height = height,
    .Color = { color[0], color[1], color[2], color[3] },
    .Depth = 1.0f,
  });

  Camera->Projection.SetRect(x, y, width, height);

  return FboTexture->Handle();
}

void
RenderTarget::End(bool isActive,
                  bool isHovered,
                  bool isRightDown,
                  bool isMiddleDown,
                  int mouseDeltaX,
                  int mouseDeltaY,
                  int mouseWheel)
{
  // update camera
  if (isActive) {
    if (isRightDown) {
      Camera->YawPitch(mouseDeltaX, mouseDeltaY);
    }
    if (isMiddleDown) {
      Camera->Shift(mouseDeltaX, mouseDeltaY);
    }
  }
  if (isHovered) {
    Camera->Dolly(mouseWheel);
  }
  Camera->Update();
  if (render) {
    render(*Camera);
  }

  Fbo->Unbind();
}

} // namespace
