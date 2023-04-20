#include "cuber.h"
#include "rendering_env.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/scene.h>

Cuber::Cuber()
{
  m_cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  m_liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  cuber::PushGrid(m_lines);
}

void
Cuber::Render(const RenderingEnv& env)
{
  m_cuber->Render(
    env.ProjectionMatrix, env.ViewMatrix, Instances.data(), Instances.size());
  m_liner->Render(env.ProjectionMatrix, env.ViewMatrix, m_lines);
}
