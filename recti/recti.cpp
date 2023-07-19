#include "recti.h"
#include "imguizmo/ImGuizmo.h"
#include <memory>
#include <stdint.h>

namespace recti {

struct ScreenImpl
{
  std::shared_ptr<ImGuizmo::Context> m_im_gizmo;

  ScreenImpl() { m_im_gizmo = std::make_shared<ImGuizmo::Context>(); }

  void Begin(const Camera& camera, const Mouse& mouse)
  {
    m_im_gizmo->Begin(camera, mouse);
  }

  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap)
  {
    return m_im_gizmo->Manipulate(
      id, ToOperation(operation), ToMode(operation), matrix, deltaMatrix, snap);
  }

  const DrawList& End() { return m_im_gizmo->End(); }

  void DrawCubes(const float* cubes, uint32_t count)
  {
    m_im_gizmo->DrawCubes(cubes, count);
  }
};

//
// Screen
//
Screen::Screen()
  : m_impl(new ScreenImpl)
{
}

Screen::~Screen()
{
  delete m_impl;
}

void
Screen::Begin(const Camera& camera, const Mouse& mouse)
{
  m_impl->Begin(camera, mouse);
}

bool
Screen::Manipulate(void* id,
                   const Operation& operation,
                   float* matrix,
                   float* deltaMatrix,
                   const float* snap)
{
  return m_impl->Manipulate(id, operation, matrix, deltaMatrix, snap);
}

const DrawList&
Screen::End()
{
  return m_impl->End();
}

void
Screen::DrawCubes(const float* cubes, uint32_t count)
{
  m_impl->DrawCubes(cubes, count);
}

} // namespace
