#include "recti.h"
#include "handle/handle.h"
#include "handle/rotation.h"
#include "handle/rotationDragHandle.h"
#include "handle/scale.h"
#include "handle/scaleDragHandle.h"
#include "handle/translationDragHandle.h"
#include "handle/translationGizmo.h"
#include <assert.h>
#include <list>

namespace recti {

struct ScreenImpl
{
  // over frame
  std::shared_ptr<IDragHandle> DragHandle;

  std::list<std::shared_ptr<IGizmo>> Gizmos = {
    std::make_shared<TranslationGizmo>(),
  };

  // current frame
  CameraMouse mCameraMouse;

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
    }

    return {};
  }

  // // hover
  // auto hoverT = MT_NONE;
  // if (Intersects(mCurrent.mOperation, TRANSLATE)) {
  //   hoverT = Translation::GetType(mCurrent, mAllowAxisFlip);
  //   if (hoverT != MT_NONE) {
  //     // hover
  //     if (mCurrent.mCameraMouse.Mouse.LeftDown) {
  //       // begin drag
  //       DragHandle =
  //         std::make_shared<TranslationDragHandle>(mCurrent, hoverT);
  //     }
  //   }
  //
  //   Translation::DrawGizmo(
  //     mCurrent, mAllowAxisFlip, hoverT, mStyle, mDrawList);
  // }
  //
  // auto hoverS = MT_NONE;
  // if (Intersects(mCurrent.mOperation, SCALE)) {
  //   hoverS = Scale::GetType(mCurrent, mAllowAxisFlip);
  //   if (hoverS != MT_NONE) {
  //     // hover
  //     if (mCurrent.mCameraMouse.Mouse.LeftDown) {
  //       // begin drag
  //       if (hoverS >= MT_SCALE_X && hoverS <= MT_SCALE_Z) {
  //         DragHandle = std::make_shared<ScaleDragHandle>(mCurrent, hoverS);
  //       } else {
  //         // uniform
  //         DragHandle = std::make_shared<ScaleUDragHandle>(mCurrent,
  //         hoverS);
  //       }
  //     }
  //   }
  //
  //   Scale::DrawGizmo(mCurrent, mAllowAxisFlip, hoverS, mStyle, mDrawList);
  //   Scale::DrawUniveralGizmo(
  //     mCurrent, mAllowAxisFlip, hoverS, mStyle, mDrawList);
  // }
  //
  // auto hoverR = MT_NONE;
  // if (Intersects(mCurrent.mOperation, ROTATE)) {
  //   hoverR = Rotation::GetType(mCurrent, mRadiusSquareCenter);
  //   if (hoverR != MT_NONE) {
  //     // hover
  //     if (mCurrent.mCameraMouse.Mouse.LeftDown) {
  //       // begin drag
  //       DragHandle = std::make_shared<RotationDragHandle>(mCurrent,
  //       hoverR);
  //     }
  //   }
  //
  //   Rotation::DrawGizmo(mCurrent,
  //                       mRadiusSquareCenter,
  //                       mIsOrthographic,
  //                       hoverR,
  //                       mStyle,
  //                       mDrawList);
  // }

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
