#include "recti.h"
#include "imguizmo/ImGuizmo.h"
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
  void SetRect(float x, float y, float w, float h)
  {
    ImGuizmo::SetRect(x, y, w, h);
  }

  bool Manipulate(void* id,
                  const float* view,
                  const float* projection,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
  {
    return ImGuizmo::Manipulate(id, view,
                                projection,
                                ToOperation(operation),
                                ToMode(operation),
                                matrix,
                                deltaMatrix,
                                snap,
                                localBounds,
                                boundsSnap);
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
                            deltaMatrix,
                            snap,
                            localBounds,
                            boundsSnap);
}

} // namespace
