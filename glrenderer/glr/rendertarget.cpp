#include <GL/glew.h>

#include "rendertarget.h"
#include <grapho/gl3/fbo.h>

namespace glr {

RenderTarget::RenderTarget(const RenderFunc& render)
  : Render(render)
{
}

uint32_t
RenderTarget::Begin(float width, float height, const float color[4])
{
  return RT->Begin(width, height, color);
}

void
RenderTarget::End(const grapho::camera::Viewport& viewport,
                  const grapho::camera::MouseState& mouse)
{
  Render(viewport, mouse);
  RT->End();
}

} // namespace
