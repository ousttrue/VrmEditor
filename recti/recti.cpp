#include "recti.h"
#include "handle/handle.h"
#include "handle/rotation_drag.h"
#include "handle/rotation_gizmo.h"
#include "handle/scale.h"
#include "handle/scaleDragHandle.h"
#include "handle/translationDragHandle.h"
#include "handle/translationGizmo.h"
#include <assert.h>
#include <list>

namespace recti {

struct ScreenImpl
{
  // current frame
  CameraMouse mCameraMouse;

  // hover & draw
  std::list<std::shared_ptr<IGizmo>> Gizmos = {
    std::make_shared<TranslationGizmo>(),
    std::make_shared<RotationGizmo>(),
  };
  // drag & draw
  std::shared_ptr<IDragHandle> DragHandle;

  // draw
  std::shared_ptr<DrawList> mDrawList;
  Style mStyle;

  float mRadiusSquareCenter = 0.0f;

  bool mIsOrthographic = false;
  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

public:
  ScreenImpl() { mDrawList = std::make_shared<DrawList>(); }

  void Begin(const Camera& camera, const Mouse& mouse)
  {
    mDrawList->m_commands.clear();
    mCameraMouse.Initialize(camera, mouse);
  }

  bool Manipulate(int64_t actualID,
                  OPERATION operation,
                  MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap)
  {
    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    ModelContext current(
      actualID, operation, mode, matrix, mCameraMouse, mGizmoSizeClipSpace);

    // behind camera
    Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, current.mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    // set delta to identity
    if (deltaMatrix) {
      ((Mat4*)deltaMatrix)->SetToIdentity();
    }

    // process drag / hover
    MOVETYPE active = MT_NONE;
    MOVETYPE hover = MT_NONE;
    auto isActive = false;
    if (DragHandle) {
      // drag
      DragHandle->Drag(current, snap, matrix, deltaMatrix);
      if (!current.mCameraMouse.Mouse.LeftDown) {
        // drag end
        isActive = true;
        DragHandle = {};
      }
    } else {
      // hover
      hover = Hover(current);
    }

    // draw
    for (auto& gizmo : Gizmos) {
      if (gizmo->Enabled(current.mOperation)) {
        if (DragHandle) {
          active = DragHandle->Type();
        }
        gizmo->Draw(current, active, hover, mStyle, mDrawList);
      }
    }
    if (DragHandle) {
      DragHandle->Draw(current, mStyle, mDrawList);
    }

    return DragHandle != nullptr || isActive;
  }

  MOVETYPE Hover(const ModelContext& current)
  {
    assert(!DragHandle);
    MOVETYPE hover = MT_NONE;

    for (auto& gizmo : Gizmos) {
      if (gizmo->Enabled(current.mOperation)) {
        hover = (MOVETYPE)(hover | gizmo->Hover(current));
      }
    }

    if (current.mCameraMouse.Mouse.LeftDown) {
      DragHandle = CreateDrag(current, hover);
    }
    return hover;
  }

  std::shared_ptr<IDragHandle> CreateDrag(const ModelContext& current,
                                          MOVETYPE hover)
  {
    if (hover >= MT_MOVE_X && hover <= MT_MOVE_SCREEN) {
      return std::make_shared<TranslationDragHandle>(current, hover);
    } else if (hover >= MT_ROTATE_X && hover <= MT_ROTATE_SCREEN) {
      return std::make_shared<RotationDragHandle>(current, hover);
    } else if (hover >= MT_SCALE_X && hover <= MT_SCALE_Z) {
      return std::make_shared<ScaleDragHandle>(current, hover);
    } else if (hover == MT_SCALE_XYZ) {
      return std::make_shared<ScaleUDragHandle>(current, hover);
    }

    return {};
  }

  void DrawCubes(const float* cubes, uint32_t count)
  {
    mDrawList->DrawCubes(mCameraMouse, cubes, count, mStyle);
  }
};

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

const DrawList&
Screen::End() const
{
  return *m_impl->mDrawList;
}

bool
Screen::Manipulate(int64_t actualID,
                   OPERATION operation,
                   MODE mode,
                   float* matrix,
                   float* deltaMatrix,
                   const float* snap)
{
  return m_impl->Manipulate(
    actualID, operation, mode, matrix, deltaMatrix, snap);
}

void
Screen::DrawCubes(const float* cubes, uint32_t count)
{
  m_impl->DrawCubes(cubes, count);
}

} // namespace
