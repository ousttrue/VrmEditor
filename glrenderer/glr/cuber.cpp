#include "cuber.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <glr/rendering_env.h>

namespace glr {
Cuber::Cuber() {}

void
Cuber::Render(const RenderingEnv& env)
{
  if (!m_cuber) {
    m_cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
    m_liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  }
  m_cuber->Render(&env.ProjectionMatrix._11,
                  &env.ViewMatrix._11,
                  Instances.data(),
                  Instances.size());
  m_liner->Render(
    &env.ProjectionMatrix._11, &env.ViewMatrix._11, m_gizmo.m_lines);
}
}
