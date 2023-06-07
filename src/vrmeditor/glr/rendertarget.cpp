#include <GL/glew.h>

#include "rendertarget.h"
#include <functional>
#include <glr/rendering_env.h>
#include <grapho/gl3/fbo.h>
#include <grapho/orbitview.h>
#include <memory>

namespace glr {
RenderTarget::RenderTarget(const std::shared_ptr<grapho::OrbitView>& view)
  : View(view)
  , Fbo(new grapho::gl3::Fbo)
{
}

uint32_t
RenderTarget::Begin(int width, int height, const float color[4])
{
  if (width == 0 || height == 0) {
    return 0;
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

  m_width = width;
  m_height = height;
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
  View->SetSize(m_width, m_height);
  if (isActive) {
    if (isRightDown) {
      View->YawPitch(mouseDeltaX, mouseDeltaY);
    }
    if (isMiddleDown) {
      View->Shift(mouseDeltaX, mouseDeltaY);
    }
  }
  if (isHovered) {
    View->Dolly(mouseWheel);
  }
  if (render) {
    render(*View);
  }

  Fbo->Unbind();
}

} // namespace

