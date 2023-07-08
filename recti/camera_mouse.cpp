#include "camera_mouse.h"

namespace recti {

void
CameraMouse::Initialize(const struct Camera& camera, const struct Mouse& mouse)
{
  Camera = camera;
  Mouse = mouse;
  mViewInverse.Inverse(Camera.ViewMatrix);
  mViewProjection = Camera.ViewMatrix * Camera.ProjectionMatrix;
  Ray.Initialize(Camera.ViewMatrix,
                 Camera.ProjectionMatrix,
                 Camera.Viewport,
                 Mouse.Position);
}

} // namespace
