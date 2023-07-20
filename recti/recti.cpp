#include "recti.h"
#include "handle/state.h"

#include "handle/rotation.h"
#include "handle/rotationDragHandle.h"
#include "handle/scale.h"
#include "handle/scaleDragHandle.h"
#include "handle/translation.h"
#include "handle/translationDragHandle.h"

#include <iostream>

namespace recti {

struct ScreenImpl
{
  // over frame
  State mState = {};

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
    ModelContext mCurrent(
      actualID, operation, mode, matrix, mCameraMouse, mGizmoSizeClipSpace);

    // set delta to identity
    if (deltaMatrix) {
      ((Mat4*)deltaMatrix)->SetToIdentity();
    }

    // behind camera
    Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, mCurrent.mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    if (auto handle = mState.DragHandle) {
      auto modified = handle->Drag(mCurrent, mState, snap, matrix, deltaMatrix);
      handle->Draw(mCurrent, mStyle, mDrawList);
      if (!mCurrent.mCameraMouse.Mouse.LeftDown) {
        // drag end
        mState.mbUsing = false;
        mState.DragHandle = {};
        mState.mEditingID = -1;
      }
      // return modified;
      return true;
    }

    // hover
    auto hoverT = MT_NONE;
    if (Intersects(mCurrent.mOperation, TRANSLATE)) {
      hoverT = Translation::GetType(mCurrent, mAllowAxisFlip, &mState);
      if (hoverT != MT_NONE) {
        // hover
        if (mCurrent.mCameraMouse.Mouse.LeftDown) {
          // begin drag
          mState.mbUsing = true;
          mState.mEditingID = mCurrent.mActualID;
          mState.mCurrentOperation = hoverT;
          mState.DragHandle =
            std::make_shared<TranslationDragHandle>(mCurrent, hoverT);
        }
      }

      Translation::DrawGizmo(
        mCurrent, mAllowAxisFlip, hoverT, mState, mStyle, mDrawList);
    }

    auto hoverS = MT_NONE;
    if (Intersects(mCurrent.mOperation, SCALE)) {
      hoverS = Scale::GetType(mCurrent, mAllowAxisFlip, &mState);
      if (hoverS != MT_NONE) {
        // hover
        if (mCurrent.mCameraMouse.Mouse.LeftDown) {
          // begin drag
          mState.mbUsing = true;
          mState.mEditingID = mCurrent.mActualID;
          mState.mCurrentOperation = hoverS;

          if (mState.mCurrentOperation >= MT_SCALE_X &&
              mState.mCurrentOperation <= MT_SCALE_Z) {
            mState.DragHandle =
              std::make_shared<ScaleDragHandle>(mCurrent, hoverS);
          } else {
            // uniform
            mState.DragHandle =
              std::make_shared<ScaleUDragHandle>(mCurrent, hoverS);
          }
        }
      }

      Scale::DrawGizmo(mCurrent, hoverS, mState, mStyle, mDrawList);
      Scale::DrawUniveralGizmo(
        mCurrent, mAllowAxisFlip, hoverS, mState, mStyle, mDrawList);
    }

    auto hoverR = MT_NONE;
    if (Intersects(mCurrent.mOperation, ROTATE)) {
      hoverR = Rotation::GetType(mCurrent, mRadiusSquareCenter, mState);
      if (hoverR != MT_NONE) {
        // hover
        if (mCurrent.mCameraMouse.Mouse.LeftDown) {
          // begin drag
          mState.mbUsing = true;
          mState.mEditingID = mCurrent.mActualID;
          mState.mCurrentOperation = hoverR;
          mState.DragHandle =
            std::make_shared<RotationDragHandle>(mCurrent, hoverR);
        }
      }

      Rotation::DrawGizmo(mCurrent,
                          mRadiusSquareCenter,
                          mIsOrthographic,
                          hoverR,
                          mState,
                          mStyle,
                          mDrawList);
    }

    return mState.DragHandle != nullptr;
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
