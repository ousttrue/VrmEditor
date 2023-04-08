#include "cuber.h"
#include "viewporjection.h"
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
Cuber::Render(const ViewProjection& camera)
{
  m_cuber->Render(
    camera.projection, camera.view, Instances.data(), Instances.size());
  m_liner->Render(camera.projection, camera.view, m_lines);
}
