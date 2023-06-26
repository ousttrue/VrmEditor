#include "cuber.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <glr/rendering_env.h>

namespace glr {

Cuber::Cuber() {}

void
Cuber::Render(const grapho::camera::Camera& camera)
{
  if (!m_cuber) {
    m_cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
    m_liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  }

  m_cuber->Render(&camera.ProjectionMatrix._11,
                  &camera.ViewMatrix._11,
                  Instances.data(),
                  Instances.size());
  m_liner->Render(
    &camera.ProjectionMatrix._11, &camera.ViewMatrix._11, m_gizmo.m_lines);
}

} // namespace
