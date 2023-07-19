#pragma once
#include "camera_mouse.h"
#include "drawcommand.h"
#include "handle/rotation.h"
#include "handle/scale.h"
#include "handle/translation.h"
#include "operation.h"
#include "state.h"
#include <memory>

namespace recti {

class Screen
{
  // over frame
  State mState = {};

  // current frame
  CameraMouse mCameraMouse;

  // draw
  std::shared_ptr<DrawList> mDrawList;
  Style mStyle;

  float mRadiusSquareCenter = 0.0f;

  Translation mT;
  Rotation mR;
  Scale mS;

  bool mIsOrthographic = false;
  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

public:
  Screen() { mDrawList = std::make_shared<DrawList>(); }

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

    // --
    auto resultT = mT.HandleTranslation(
      mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
    Result resultR{};
    Result resultS{};
    if (!resultT.Modified) {
      resultS = mS.HandleScale(
        mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
      if (!resultS.Modified) {
        resultR = mR.HandleRotation(
          mCurrent, mRadiusSquareCenter, mState, snap, matrix, deltaMatrix);
      }
    }

    auto type = static_cast<MOVETYPE>(resultT.DrawType | resultR.DrawType |
                                      resultS.DrawType);

    mR.DrawRotationGizmo(mCurrent,
                         mRadiusSquareCenter,
                         mIsOrthographic,
                         type,
                         mState,
                         mStyle,
                         mDrawList);
    mT.DrawTranslationGizmo(
      mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);
    mS.DrawScaleGizmo(mCurrent, type, mState, mStyle, mDrawList);
    mS.DrawScaleUniveralGizmo(
      mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);

    return resultT.Modified || resultR.Modified || resultS.Modified;
  }

  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr)
  {
    return Manipulate((int64_t)id,
                      ToOperation(operation),
                      ToMode(operation),
                      matrix,
                      deltaMatrix,
                      snap);
  }

  const DrawList& End() const { return *mDrawList; }

  void DrawCubes(const float* cubes, uint32_t count)
  {
    mDrawList->DrawCubes(mCameraMouse, cubes, count, mStyle);
  }
};

} // namespace
