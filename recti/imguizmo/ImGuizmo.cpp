// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "../drawcommand.h"
#include "../handle/rotation.h"
#include "../handle/translation.h"
#include "../mat4.h"
#include "../model_context.h"
#include "../ray.h"
#include "../state.h"
#include "../style.h"
#include "../tripod.h"
#include "../vec2.h"
#include "../vec4.h"
#include <assert.h>
#include <cfloat>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// Matches MT_MOVE_AB order
static const char* scaleInfoMask[] = { "X : %5.2f",
                                       "Y : %5.2f",
                                       "Z : %5.2f",
                                       "XYZ : %5.2f" };

static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };

static const recti::Vec4 directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                               { 0.f, 1.f, 0.f, 0 },
                                               { 0.f, 0.f, 1.f, 0 } };

struct RGBA
{
  float r;
  float g;
  float b;
  float a;
};

#include "ImGuizmo.h"

namespace ImGuizmo {

static recti::MOVETYPE
GetScaleType(const recti::ModelContext& mCurrent,
             bool mAllowAxisFlip,
             recti::State* state)
{
  if (state->mbUsing) {
    return recti::MT_NONE;
  }
  recti::MOVETYPE type = recti::MT_NONE;

  // compute
  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::SCALE_X << i))) {
      continue;
    }
    recti::Tripod tripod(i);
    if (state->Using(mCurrent.mActualID)) {
      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      tripod.belowAxisLimit = state->mBelowAxisLimit[i];
      tripod.belowPlaneLimit = state->mBelowPlaneLimit[i];
      tripod.dirAxis *= state->mAxisFactor[i];
      tripod.dirPlaneX *= state->mAxisFactor[(i + 1) % 3];
      tripod.dirPlaneY *= state->mAxisFactor[(i + 2) % 3];
    } else {
      tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
      // and store values
      state->mAxisFactor[i] = tripod.mulAxis;
      state->mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
      state->mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
      state->mBelowAxisLimit[i] = tripod.belowAxisLimit;
      state->mBelowPlaneLimit[i] = tripod.belowPlaneLimit;
    }

    tripod.dirAxis.TransformVector(mCurrent.mModelLocal);
    tripod.dirPlaneX.TransformVector(mCurrent.mModelLocal);
    tripod.dirPlaneY.TransformVector(mCurrent.mModelLocal);

    recti::Vec4 posOnPlan = mCurrent.mCameraMouse.Ray.IntersectPlane(
      BuildPlan(mCurrent.mModelLocal.position(), tripod.dirAxis));

    const float startOffset =
      Contains(mCurrent.mOperation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.0f
        : 0.1f;
    const float endOffset =
      Contains(mCurrent.mOperation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.4f
        : 1.0f;
    const recti::Vec2 posOnPlanScreen =
      mCurrent.mCameraMouse.WorldToPos(posOnPlan);
    const recti::Vec2 axisStartOnScreen = mCurrent.mCameraMouse.WorldToPos(
      mCurrent.mModelLocal.position() +
      tripod.dirAxis * mCurrent.mScreenFactor * startOffset);
    const recti::Vec2 axisEndOnScreen = mCurrent.mCameraMouse.WorldToPos(
      mCurrent.mModelLocal.position() +
      tripod.dirAxis * mCurrent.mScreenFactor * endOffset);

    recti::Vec4 closestPointOnAxis =
      recti::PointOnSegment({ posOnPlanScreen.X, posOnPlanScreen.Y },
                            { axisStartOnScreen.X, axisStartOnScreen.Y },
                            { axisEndOnScreen.X, axisEndOnScreen.Y });

    if ((closestPointOnAxis -
         recti::Vec4{ posOnPlanScreen.X, posOnPlanScreen.Y })
          .Length() < 12.f) // pixel size
    {
      type = (recti::MOVETYPE)(recti::MT_SCALE_X + i);
    }
  }

  // universal
  auto& mousePos = mCurrent.mCameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.X - mCurrent.mScreenSquareCenter.X,
                              mousePos.Y - mCurrent.mScreenSquareCenter.Y,
                              0.f,
                              0.f };
  float dist = deltaScreen.Length();
  if (Contains(mCurrent.mOperation, recti::SCALEU) && dist >= 17.0f &&
      dist < 23.0f) {
    type = recti::MT_SCALE_XYZ;
  }

  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::SCALE_XU << i))) {
      continue;
    }

    recti::Tripod tripod(i);
    if (state->Using(mCurrent.mActualID)) {
      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      tripod.belowAxisLimit = state->mBelowAxisLimit[i];
      tripod.belowPlaneLimit = state->mBelowPlaneLimit[i];
      tripod.dirAxis *= state->mAxisFactor[i];
      tripod.dirPlaneX *= state->mAxisFactor[(i + 1) % 3];
      tripod.dirPlaneY *= state->mAxisFactor[(i + 2) % 3];
    } else {
      tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
      // and store values
      state->mAxisFactor[i] = tripod.mulAxis;
      state->mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
      state->mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
      state->mBelowAxisLimit[i] = tripod.belowAxisLimit;
      state->mBelowPlaneLimit[i] = tripod.belowPlaneLimit;
    }

    // draw axis
    if (tripod.belowAxisLimit) {
      bool hasTranslateOnAxis =
        Contains(mCurrent.mOperation,
                 static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
      float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
      recti::Vec2 worldDirSSpace = recti::worldToPos(
        (tripod.dirAxis * markerScale) * mCurrent.mScreenFactor,
        mCurrent.mMVPLocal,
        mCurrent.mCameraMouse.Camera.Viewport);

      float distance = sqrtf((worldDirSSpace - mousePos).SqrLength());
      if (distance < 12.f) {
        type = (recti::MOVETYPE)(recti::MT_SCALE_X + i);
      }
    }
  }
  return type;
}

class ContextImpl
{
  recti::CameraMouse mCameraMouse;

  // over frame
  recti::State mState = {};

  std::shared_ptr<recti::DrawList> mDrawList;
  recti::Style mStyle;

  float mRadiusSquareCenter;

  recti::Vec4 mRelativeOrigin;

  recti::Translation mT;
  recti::Rotation mR;

  // scale
  recti::Vec4 mScale;
  recti::Vec4 mScaleValueOrigin;
  recti::Vec4 mScaleLast;
  float mSaveMousePosx;

  //
  bool mIsOrthographic = false;

  // recti::OPERATION mOperation = recti::OPERATION(-1);

  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

public:
  ContextImpl() { mDrawList = std::make_shared<recti::DrawList>(); }

private:
  void DrawScaleGizmo(const recti::ModelContext& mCurrent, recti::MOVETYPE type)
  {
    auto drawList = mDrawList;

    if (!Intersects(mCurrent.mOperation, recti::SCALE)) {
      return;
    }

    // colors
    uint32_t colors[7];
    mStyle.ComputeColors(colors, type, recti::SCALE);

    // draw
    recti::Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mState.Using(mCurrent.mActualID)) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(mCurrent.mOperation,
                      static_cast<recti::OPERATION>(recti::SCALE_X << i))) {
        continue;
      }
      const bool usingAxis = (mState.mbUsing && type == recti::MT_SCALE_X + i);
      if (!mState.mbUsing || usingAxis) {
        recti::Tripod tripod(i);
        if (mState.Using(mCurrent.mActualID)) {
          // when using, use stored factors so the gizmo doesn't flip when we
          // translate
          tripod.belowAxisLimit = mState.mBelowAxisLimit[i];
          tripod.belowPlaneLimit = mState.mBelowPlaneLimit[i];
          tripod.dirAxis *= mState.mAxisFactor[i];
          tripod.dirPlaneX *= mState.mAxisFactor[(i + 1) % 3];
          tripod.dirPlaneY *= mState.mAxisFactor[(i + 2) % 3];
        } else {
          tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
          // and store values
          mState.mAxisFactor[i] = tripod.mulAxis;
          mState.mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
          mState.mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
          mState.mBelowAxisLimit[i] = tripod.belowAxisLimit;
          mState.mBelowPlaneLimit[i] = tripod.belowPlaneLimit;
        }

        // draw axis
        if (tripod.belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(mCurrent.mOperation,
                     static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          recti::Vec2 baseSSpace =
            recti::worldToPos(tripod.dirAxis * 0.1f * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);
          recti::Vec2 worldDirSSpaceNoScale = recti::worldToPos(
            tripod.dirAxis * markerScale * mCurrent.mScreenFactor,
            mCurrent.mMVP,
            mCurrent.mCameraMouse.Camera.Viewport);
          recti::Vec2 worldDirSSpace =
            recti::worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                                mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);

          if (mState.Using(mCurrent.mActualID)) {
            uint32_t scaleLineColor = mStyle.GetColorU32(recti::SCALE_LINE);
            drawList->AddLine(baseSSpace,
                              worldDirSSpaceNoScale,
                              scaleLineColor,
                              mStyle.ScaleLineThickness);
            drawList->AddCircleFilled(worldDirSSpaceNoScale,
                                      mStyle.ScaleLineCircleSize,
                                      scaleLineColor);
          }

          if (!hasTranslateOnAxis || mState.mbUsing) {
            drawList->AddLine(baseSSpace,
                              worldDirSSpace,
                              colors[i + 1],
                              mStyle.ScaleLineThickness);
          }
          drawList->AddCircleFilled(
            worldDirSSpace, mStyle.ScaleLineCircleSize, colors[i + 1]);

          if (mState.mAxisFactor[i] < 0.f) {
            mDrawList->DrawHatchedAxis(
              mCurrent, tripod.dirAxis * scaleDisplay[i], mStyle);
          }
        }
      }
    }

    // draw screen cirle
    drawList->AddCircleFilled(
      mCurrent.mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

    if (mState.Using(mCurrent.mActualID) && IsScaleType(type)) {
      recti::Vec2 destinationPosOnScreen =
        recti::worldToPos(mCurrent.mModel.position(),
                          mCameraMouse.mViewProjection,
                          mCurrent.mCameraMouse.Camera.Viewport);
      char tmps[512];
      // vec_t deltaInfo = mModel.position() -
      // mMatrixOrigin;
      int componentInfoIndex = (type - recti::MT_SCALE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               scaleInfoMask[type - recti::MT_SCALE_X],
               scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        mStyle.GetColorU32(recti::TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        mStyle.GetColorU32(recti::TEXT),
                        tmps);
    }
  }

  void DrawScaleUniveralGizmo(const recti::ModelContext& mCurrent,
                              recti::MOVETYPE type)
  {
    auto drawList = mDrawList;

    if (!Intersects(mCurrent.mOperation, recti::SCALEU)) {
      return;
    }

    // colors
    uint32_t colors[7];
    mStyle.ComputeColors(colors, type, recti::SCALEU);

    // draw
    recti::Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mState.Using(mCurrent.mActualID)) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(mCurrent.mOperation,
                      static_cast<recti::OPERATION>(recti::SCALE_XU << i))) {
        continue;
      }
      const bool usingAxis = (mState.mbUsing && type == recti::MT_SCALE_X + i);
      if (!mState.mbUsing || usingAxis) {
        recti::Tripod tripod(i);
        if (mState.Using(mCurrent.mActualID)) {
          // when using, use stored factors so the gizmo doesn't flip when we
          // translate
          tripod.belowAxisLimit = mState.mBelowAxisLimit[i];
          tripod.belowPlaneLimit = mState.mBelowPlaneLimit[i];
          tripod.dirAxis *= mState.mAxisFactor[i];
          tripod.dirPlaneX *= mState.mAxisFactor[(i + 1) % 3];
          tripod.dirPlaneY *= mState.mAxisFactor[(i + 2) % 3];
        } else {
          tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
          // and store values
          mState.mAxisFactor[i] = tripod.mulAxis;
          mState.mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
          mState.mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
          mState.mBelowAxisLimit[i] = tripod.belowAxisLimit;
          mState.mBelowPlaneLimit[i] = tripod.belowPlaneLimit;
        }

        // draw axis
        if (tripod.belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(mCurrent.mOperation,
                     static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          recti::Vec2 worldDirSSpace =
            recti::worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                                mCurrent.mScreenFactor,
                              mCurrent.mMVPLocal,
                              mCurrent.mCameraMouse.Camera.Viewport);

          drawList->AddCircleFilled(worldDirSSpace, 12.f, colors[i + 1]);
        }
      }
    }

    // draw screen cirle
    drawList->AddCircle(mCurrent.mScreenSquareCenter,
                        20.f,
                        colors[0],
                        32,
                        mStyle.CenterCircleSize);

    if (mState.Using(mCurrent.mOperation) && IsScaleType(type)) {
      recti::Vec2 destinationPosOnScreen =
        mCameraMouse.WorldToPos(mCurrent.mModel.position());

      char tmps[512];
      // vec_t deltaInfo = mModel.position() -
      // mMatrixOrigin;
      int componentInfoIndex = (type - recti::MT_SCALE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               scaleInfoMask[type - recti::MT_SCALE_X],
               scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        mStyle.GetColorU32(recti::TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        mStyle.GetColorU32(recti::TEXT),
                        tmps);
    }
  }

  bool HandleScale(const recti::ModelContext& mCurrent,
                   float* matrix,
                   float* deltaMatrix,
                   recti::MOVETYPE& type,
                   const float* snap)
  {
    if ((!Intersects(mCurrent.mOperation, recti::SCALE) &&
         !Intersects(mCurrent.mOperation, recti::SCALEU)) ||
        type != recti::MT_NONE) {
      return false;
    }
    bool modified = false;

    auto& mouse = mCurrent.mCameraMouse.Mouse;
    if (!mState.mbUsing) {
      // find new possible way to scale
      type = GetScaleType(mCurrent, mAllowAxisFlip, &mState);
      if (type != recti::MT_NONE) {
      }
      if (mouse.LeftDown && type != recti::MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mCurrent.mActualID;
        mState.mCurrentOperation = type;
        const recti::Vec4 movePlanNormal[] = {
          mCurrent.mModel.up(),     mCurrent.mModel.dir(),
          mCurrent.mModel.right(),  mCurrent.mModel.dir(),
          mCurrent.mModel.up(),     mCurrent.mModel.right(),
          -mCameraMouse.CameraDir()
        };
        // pickup plan

        mT.mTranslationPlan = BuildPlan(
          mCurrent.mModel.position(), movePlanNormal[type - recti::MT_SCALE_X]);
        mT.mTranslationPlanOrigin =
          mCameraMouse.Ray.IntersectPlane(mT.mTranslationPlan);
        mT.mMatrixOrigin = mCurrent.mModel.position();
        mScale.Set(1.f, 1.f, 1.f);
        mRelativeOrigin =
          (mT.mTranslationPlanOrigin - mCurrent.mModel.position()) *
          (1.f / mCurrent.mScreenFactor);
        mScaleValueOrigin = { mCurrent.mModelSource.right().Length(),
                              mCurrent.mModelSource.up().Length(),
                              mCurrent.mModelSource.dir().Length() };
        mSaveMousePosx = mouse.Position.X;
      }
    }
    // scale
    if (mState.Using(mCurrent.mActualID) &&
        IsScaleType(mState.mCurrentOperation)) {
      recti::Vec4 newPos = mCameraMouse.Ray.IntersectPlane(mT.mTranslationPlan);
      recti::Vec4 newOrigin =
        newPos - mT.mRelativeOrigin * mCurrent.mScreenFactor;
      recti::Vec4 delta = newOrigin - mCurrent.mModelLocal.position();

      // 1 axis constraint
      if (mState.mCurrentOperation >= recti::MT_SCALE_X &&
          mState.mCurrentOperation <= recti::MT_SCALE_Z) {
        int axisIndex = mState.mCurrentOperation - recti::MT_SCALE_X;
        const recti::Vec4& axisValue =
          mCurrent.mModelLocal.component(axisIndex);
        float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;

        recti::Vec4 baseVector =
          mT.mTranslationPlanOrigin - mCurrent.mModelLocal.position();
        float ratio =
          Dot(axisValue, baseVector + delta) / Dot(axisValue, baseVector);

        mScale[axisIndex] = recti::max(ratio, 0.001f);
      } else {
        float scaleDelta = (mouse.Position.X - mSaveMousePosx) * 0.01f;
        mScale.Set(recti::max(1.f + scaleDelta, 0.001f));
      }

      // snap
      if (snap) {
        float scaleSnap[] = { snap[0], snap[0], snap[0] };
        ComputeSnap(mScale, scaleSnap);
      }

      // no 0 allowed
      for (int i = 0; i < 3; i++)
        mScale[i] = recti::max(mScale[i], 0.001f);

      if (mScaleLast != mScale) {
        modified = true;
      }
      mScaleLast = mScale;

      // compute matrix & delta
      recti::Mat4 deltaMatrixScale;
      deltaMatrixScale.Scale(mScale * mScaleValueOrigin);

      recti::Mat4 res = deltaMatrixScale * mCurrent.mModelLocal;
      *(recti::Mat4*)matrix = res;

      if (deltaMatrix) {
        recti::Vec4 deltaScale = mScale * mScaleValueOrigin;

        recti::Vec4 originalScaleDivider;
        originalScaleDivider.x = 1 / mCurrent.mModelScaleOrigin.x;
        originalScaleDivider.y = 1 / mCurrent.mModelScaleOrigin.y;
        originalScaleDivider.z = 1 / mCurrent.mModelScaleOrigin.z;

        deltaScale = deltaScale * originalScaleDivider;

        deltaMatrixScale.Scale(deltaScale);
        memcpy(deltaMatrix, &deltaMatrixScale.m00, sizeof(float) * 16);
      }

      if (!mouse.LeftDown) {
        mState.mbUsing = false;
        mScale.Set(1.f, 1.f, 1.f);
      }

      type = mState.mCurrentOperation;
    }
    return modified;
  }

public:
  //
  // entery point
  //
  void Begin(const recti::Camera& camera, const recti::Mouse& mouse)
  {
    mDrawList->m_commands.clear();
    mCameraMouse.Initialize(camera, mouse);
  }

  bool Manipulate(int64_t actualID,
                  recti::OPERATION operation,
                  recti::MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap)
  {
    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    recti::ModelContext mCurrent(
      actualID, operation, mode, matrix, mCameraMouse, mGizmoSizeClipSpace);

    // set delta to identity
    if (deltaMatrix) {
      ((recti::Mat4*)deltaMatrix)->SetToIdentity();
    }

    // behind camera
    recti::Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, mCurrent.mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    // --
    auto result = mT.HandleTranslation(
      mCurrent, mAllowAxisFlip, snap, mState, matrix, deltaMatrix);
    if (!result.Modified) {
      result.Modified =
        HandleScale(mCurrent, matrix, deltaMatrix, result.DrawType, snap);
      if (!result.Modified) {
        result = mR.HandleRotation(
          mCurrent, mRadiusSquareCenter, mState, snap, matrix, deltaMatrix);
      }
    }

    mR.DrawRotationGizmo(mCurrent,
                         mRadiusSquareCenter,
                         mIsOrthographic,
                         result.DrawType,
                         mState,
                         mStyle,
                         mDrawList);
    mT.DrawTranslationGizmo(
      mCurrent, mAllowAxisFlip, result.DrawType, mStyle, mState, mDrawList);
    DrawScaleGizmo(mCurrent, result.DrawType);
    DrawScaleUniveralGizmo(mCurrent, result.DrawType);

    return result.Modified;
  }

  const recti::DrawList& GetDrawList() const { return *mDrawList; }
};

//
// Context
//
Context::Context()
  : m_impl(new ContextImpl)
{
}

Context::~Context()
{
  delete m_impl;
}

void
Context::Begin(const recti::Camera& camera, const recti::Mouse& mouse)
{
  m_impl->Begin(camera, mouse);
}

bool
Context::Manipulate(void* id,
                    recti::OPERATION operation,
                    recti::MODE mode,
                    float* matrix,
                    float* deltaMatrix,
                    const float* snap)
{
  return m_impl->Manipulate(
    (int64_t)id, operation, mode, matrix, deltaMatrix, snap);
}

const recti::DrawList&
Context::End()
{
  return m_impl->GetDrawList();
}

} // namespace
