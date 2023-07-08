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

  void SetRect(float x, float y, float w, float h)
  {
    m_im_gizmo->SetRect(x, y, w, h);
  }

  bool Manipulate(void* id,
                  const float* view,
                  const float* projection,
                  const Operation& operation,
                  float* matrix,
                  const Vec2& mousePos,
                  bool mouseLeftDown,
                  float* deltaMatrix,
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
  {
    return m_im_gizmo->Manipulate(id,
                                  view,
                                  projection,
                                  ToOperation(operation),
                                  ToMode(operation),
                                  matrix,
                                  mousePos,
                                  mouseLeftDown,
                                  deltaMatrix,
                                  snap,
                                  localBounds,
                                  boundsSnap);
  }

  const DrawList& GetDrawList() { return m_im_gizmo->GetDrawList(); }
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
Screen::SetRect(float x, float y, float w, float h)
{
  m_impl->SetRect(x, y, w, h);
}

bool
Screen::Manipulate(void* id,
                   const float* view,
                   const float* projection,
                   const Operation& operation,
                   float* matrix,
                   const Vec2& mousePos,
                   bool mouseLeftDown,
                   float* deltaMatrix,
                   const float* snap,
                   const float* localBounds,
                   const float* boundsSnap)
{
  return m_impl->Manipulate(id,
                            view,
                            projection,
                            operation,
                            matrix,
                            mousePos,
                            mouseLeftDown,
                            deltaMatrix,
                            snap,
                            localBounds,
                            boundsSnap);
}

const DrawList&
Screen::GetDrawList()
{
  return m_impl->GetDrawList();
}

} // namespace
