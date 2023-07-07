#include "recti.h"
#include "imguizmo/ImGuizmo.h"

namespace recti {

static ImGuizmo::OPERATION
ToOperation(const Operation& o)
{
  return ImGuizmo::ROTATE;
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
{};

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
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(x, y, w, h);
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
  ImGuizmo::SetID((int64_t)id);
  // ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
  return ImGuizmo::Manipulate(view,
                              projection,
                              ToOperation(operation),
                              ToMode(operation),
                              matrix,
                              deltaMatrix,
                              snap,
                              localBounds,
                              boundsSnap);
}

} // namespace
