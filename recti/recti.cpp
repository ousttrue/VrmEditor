#include "recti.h"
#include "handle/rotation.h"
#include "handle/scale.h"
#include "handle/translation.h"
#include "state.h"

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

  Translation mT;
  Rotation mR;
  Scale mS;

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

    // --
    // auto resultT = mT.HandleTranslation(
    //   mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
    // Result resultR{};
    // Result resultS{};
    // if (!resultT.Modified) {
    //   resultS = mS.HandleScale(
    //     mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
    //   if (!resultS.Modified) {
    //     resultR = mR.HandleRotation(
    //       mCurrent, mRadiusSquareCenter, mState, snap, matrix, deltaMatrix);
    //   }
    // }
    //
    // auto type = static_cast<MOVETYPE>(resultT.DrawType | resultR.DrawType |
    //                                   resultS.DrawType);

    if (Intersects(mCurrent.mOperation, ROTATE)) {
      auto type =
        Rotation::GetRotateType(mCurrent, mRadiusSquareCenter, mState);
      mR.DrawRotationGizmo(mCurrent,
                           mRadiusSquareCenter,
                           mIsOrthographic,
                           type,
                           mState,
                           mStyle,
                           mDrawList);
    }
    if (Intersects(mCurrent.mOperation, TRANSLATE)) {
      auto type = Translation::GetMoveType(mCurrent, mAllowAxisFlip, &mState);
      mT.DrawTranslationGizmo(
        mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);
    }
    if (Intersects(mCurrent.mOperation, SCALE)) {
      auto type = Scale::GetScaleType(mCurrent, mAllowAxisFlip, &mState);
      mS.DrawScaleGizmo(mCurrent, type, mState, mStyle, mDrawList);
      mS.DrawScaleUniveralGizmo(
        mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);
    }

    return false;
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
