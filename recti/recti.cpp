#include "recti.h"
#include "imguizmo/ImGuizmo.h"
#include <memory>
#include <stdint.h>

namespace recti {

static ImGuizmo::OPERATION
ToOperation(const Operation& o)
{
  ImGuizmo::OPERATION operation = {};
  if (o.EnableT) {
    operation |= ImGuizmo::TRANSLATE;
  }
  if (o.EnableR) {
    operation |= ImGuizmo::ROTATE;
  }
  if (o.EnableS) {
    operation |= ImGuizmo::SCALE;
  }
  return operation;
}

static ImGuizmo::MODE
ToMode(const Operation& o)
{
  return ImGuizmo::LOCAL;
}

//
// ScreenImpl
//
struct ScreenImpl
{
  std::shared_ptr<ImGuizmo::Context> m_im_gizmo;

  ScreenImpl() { m_im_gizmo = std::make_shared<ImGuizmo::Context>(); }

  void Begin(const float* view,
             const float* projection,
             float x,
             float y,
             float width,
             float height,
             const recti::Vec2& mousePos,
             bool mouseLeftDown)
  {
    m_im_gizmo->Begin(
      view, projection, x, y, width, height, mousePos, mouseLeftDown);
  }

  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
  {
    return m_im_gizmo->Manipulate(id,
                                  ToOperation(operation),
                                  ToMode(operation),
                                  matrix,
                                  deltaMatrix,
                                  snap,
                                  localBounds,
                                  boundsSnap);
  }

  const DrawList& End() { return m_im_gizmo->End(); }
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
Screen::Begin(const float* view,
              const float* projection,
              float x,
              float y,
              float width,
              float height,
              const recti::Vec2& mousePos,
              bool mouseLeftDown)
{
  m_impl->Begin(view, projection, x, y, width, height, mousePos, mouseLeftDown);
}

bool
Screen::Manipulate(void* id,
                   const Operation& operation,
                   float* matrix,
                   float* deltaMatrix,
                   const float* snap,
                   const float* localBounds,
                   const float* boundsSnap)
{
  return m_impl->Manipulate(
    id, operation, matrix, deltaMatrix, snap, localBounds, boundsSnap);
}

const DrawList&
Screen::End()
{
  return m_impl->End();
}

} // namespace
