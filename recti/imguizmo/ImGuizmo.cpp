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
#include "../mat4.h"
#include "../model_context.h"
#include "../ray.h"
#include "../style.h"
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
static const char* translationInfoMask[] = { "X : %5.3f",
                                             "Y : %5.3f",
                                             "Z : %5.3f",
                                             "Y : %5.3f Z : %5.3f",
                                             "X : %5.3f Z : %5.3f",
                                             "X : %5.3f Y : %5.3f",
                                             "X : %5.3f Y : %5.3f Z : %5.3f" };
static const char* rotationInfoMask[] = { "X : %5.2f deg %5.2f rad",
                                          "Y : %5.2f deg %5.2f rad",
                                          "Z : %5.2f deg %5.2f rad",
                                          "Screen : %5.2f deg %5.2f rad" };
static const int HALF_CIRCLE_SEGMENT_COUNT = 64;

const float screenRotateSize = 0.06f;

static const char* scaleInfoMask[] = { "X : %5.2f",
                                       "Y : %5.2f",
                                       "Z : %5.2f",
                                       "XYZ : %5.2f" };

static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };

static const recti::Vec4 directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                               { 0.f, 1.f, 0.f, 0 },
                                               { 0.f, 0.f, 1.f, 0 } };

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

static const float quadMin = 0.5f;
static const float quadMax = 0.8f;
static const float quadUV[8] = { quadMin, quadMin, quadMin, quadMax,
                                 quadMax, quadMax, quadMax, quadMin };
static const float ZPI = 3.14159265358979323846f;
static const float DEG2RAD = (ZPI / 180.f);

struct RGBA
{
  float r;
  float g;
  float b;
  float a;
};

#include "ImGuizmo.h"

static const recti::OPERATION TRANSLATE_PLANS[3] = {
  recti::TRANSLATE_Y | recti::TRANSLATE_Z,
  recti::TRANSLATE_X | recti::TRANSLATE_Z,
  recti::TRANSLATE_X | recti::TRANSLATE_Y
};

namespace ImGuizmo {

struct State
{
  int64_t mEditingID = -1;
  bool mbUsing = false;
  // save axis factor when using gizmo
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];
  bool mbUsingBounds = false;

  bool Using(uint64_t actualID) const
  {
    if (mbUsing) {
      if (actualID == -1) {
        return true;
      }
      if (actualID == mEditingID) {
        return true;
      }
    }
    return false;
  }

  // return true if mouse IsOver or if the gizmo is in moving state
  bool UsingOrBounds(int64_t actualID) const
  {
    return Using(actualID) || mbUsingBounds;
  }
};

static void
ComputeTripodAxisAndVisibility(const recti::ModelContext& mCurrent,
                               bool mAllowAxisFlip,
                               const int axisIndex,
                               State* state,
                               recti::Vec4& dirAxis,
                               recti::Vec4& dirPlaneX,
                               recti::Vec4& dirPlaneY,
                               bool& belowAxisLimit,
                               bool& belowPlaneLimit)
{
  if (state->Using(mCurrent.mActualID)) {
    // when using, use stored factors so the gizmo doesn't flip when we
    // translate
    belowAxisLimit = state->mBelowAxisLimit[axisIndex];
    belowPlaneLimit = state->mBelowPlaneLimit[axisIndex];
    dirAxis *= state->mAxisFactor[axisIndex];
    dirPlaneX *= state->mAxisFactor[(axisIndex + 1) % 3];
    dirPlaneY *= state->mAxisFactor[(axisIndex + 2) % 3];
  } else {
    // new method
    dirAxis = directionUnary[axisIndex];
    dirPlaneX = directionUnary[(axisIndex + 1) % 3];
    dirPlaneY = directionUnary[(axisIndex + 2) % 3];

    float lenDir =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                dirAxis,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());
    float lenDirMinus =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                -dirAxis,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());

    float lenDirPlaneX =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                dirPlaneX,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());
    float lenDirMinusPlaneX =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                -dirPlaneX,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());

    float lenDirPlaneY =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                dirPlaneY,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());
    float lenDirMinusPlaneY =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                -dirPlaneY,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());

    // For readability
    float mulAxis = (mAllowAxisFlip && lenDir < lenDirMinus &&
                     fabsf(lenDir - lenDirMinus) > FLT_EPSILON)
                      ? -1.f
                      : 1.f;
    float mulAxisX = (mAllowAxisFlip && lenDirPlaneX < lenDirMinusPlaneX &&
                      fabsf(lenDirPlaneX - lenDirMinusPlaneX) > FLT_EPSILON)
                       ? -1.f
                       : 1.f;
    float mulAxisY = (mAllowAxisFlip && lenDirPlaneY < lenDirMinusPlaneY &&
                      fabsf(lenDirPlaneY - lenDirMinusPlaneY) > FLT_EPSILON)
                       ? -1.f
                       : 1.f;
    dirAxis *= mulAxis;
    dirPlaneX *= mulAxisX;
    dirPlaneY *= mulAxisY;

    // for axis
    float axisLengthInClipSpace =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                                dirAxis * mCurrent.mScreenFactor,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.DisplayRatio());

    float paraSurf =
      GetParallelogram({ 0.f, 0.f, 0.f },
                       dirPlaneX * mCurrent.mScreenFactor,
                       dirPlaneY * mCurrent.mScreenFactor,
                       mCurrent.mMVP,
                       mCurrent.mCameraMouse.Camera.DisplayRatio());
    belowPlaneLimit = (paraSurf > 0.0025f);
    belowAxisLimit = (axisLengthInClipSpace > 0.02f);

    // and store values
    state->mAxisFactor[axisIndex] = mulAxis;
    state->mAxisFactor[(axisIndex + 1) % 3] = mulAxisX;
    state->mAxisFactor[(axisIndex + 2) % 3] = mulAxisY;
    state->mBelowAxisLimit[axisIndex] = belowAxisLimit;
    state->mBelowPlaneLimit[axisIndex] = belowPlaneLimit;
  }
}

static recti::MOVETYPE
GetMoveType(const recti::ModelContext& mCurrent,
            bool mAllowAxisFlip,
            State* state)
{
  if (!Intersects(mCurrent.mOperation, recti::TRANSLATE) || state->mbUsing) {
    return recti::MT_NONE;
  }

  recti::MOVETYPE type = recti::MT_NONE;

  // compute
  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
    bool belowAxisLimit, belowPlaneLimit;
    ComputeTripodAxisAndVisibility(mCurrent,
                                   mAllowAxisFlip,
                                   i,
                                   state,
                                   dirAxis,
                                   dirPlaneX,
                                   dirPlaneY,
                                   belowAxisLimit,
                                   belowPlaneLimit);
    dirAxis.TransformVector(mCurrent.mModel);
    dirPlaneX.TransformVector(mCurrent.mModel);
    dirPlaneY.TransformVector(mCurrent.mModel);

    auto posOnPlan = mCurrent.mCameraMouse.Ray.IntersectPlane(
      BuildPlan(mCurrent.mModel.position(), dirAxis));

    // screen
    const recti::Vec2 axisStartOnScreen =
      mCurrent.mCameraMouse.WorldToPos(
        mCurrent.mModel.position() + dirAxis * mCurrent.mScreenFactor * 0.1f) -
      mCurrent.mCameraMouse.Camera.LeftTop();

    // screen
    const recti::Vec2 axisEndOnScreen =
      mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position() +
                                       dirAxis * mCurrent.mScreenFactor) -
      mCurrent.mCameraMouse.Camera.LeftTop();

    auto screenCoord = mCurrent.mCameraMouse.ScreenMousePos();
    recti::Vec4 closestPointOnAxis =
      PointOnSegment(screenCoord,
                     { axisStartOnScreen.X, axisStartOnScreen.Y },
                     { axisEndOnScreen.X, axisEndOnScreen.Y });
    if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
        Intersects(
          mCurrent.mOperation,
          static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))) // pixel size
    {
      type = (recti::MOVETYPE)(recti::MT_MOVE_X + i);
    }

    const float dx = dirPlaneX.Dot3((posOnPlan - mCurrent.mModel.position()) *
                                    (1.f / mCurrent.mScreenFactor));
    const float dy = dirPlaneY.Dot3((posOnPlan - mCurrent.mModel.position()) *
                                    (1.f / mCurrent.mScreenFactor));
    if (belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
        dy >= quadUV[1] && dy <= quadUV[3] &&
        Contains(mCurrent.mOperation, TRANSLATE_PLANS[i])) {
      type = (recti::MOVETYPE)(recti::MT_MOVE_YZ + i);
    }
  }
  return type;
}

static recti::MOVETYPE
GetRotateType(const recti::ModelContext& mCurrent,
              float mRadiusSquareCenter,
              const State& mState)
{
  if (mState.mbUsing) {
    return recti::MT_NONE;
  }
  recti::MOVETYPE type = recti::MT_NONE;

  auto& mousePos = mCurrent.mCameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.X - mCurrent.mScreenSquareCenter.X,
                              mousePos.Y - mCurrent.mScreenSquareCenter.Y,
                              0.f,
                              0.f };
  float dist = deltaScreen.Length();
  if (Intersects(mCurrent.mOperation, recti::ROTATE_SCREEN) &&
      dist >= (mRadiusSquareCenter - 4.0f) &&
      dist < (mRadiusSquareCenter + 4.0f)) {
    type = recti::MT_ROTATE_SCREEN;
  }

  const recti::Vec4 planNormals[] = { mCurrent.mModel.right(),
                                      mCurrent.mModel.up(),
                                      mCurrent.mModel.dir() };

  recti::Vec4 modelViewPos;
  modelViewPos.TransformPoint(mCurrent.mModel.position(),
                              mCurrent.mCameraMouse.Camera.ViewMatrix);

  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::ROTATE_X << i))) {
      continue;
    }
    // pickup plan
    recti::Vec4 pickupPlan =
      BuildPlan(mCurrent.mModel.position(), planNormals[i]);

    const recti::Vec4 intersectWorldPos =
      mCurrent.mCameraMouse.Ray.IntersectPlane(pickupPlan);
    recti::Vec4 intersectViewPos;
    intersectViewPos.TransformPoint(intersectWorldPos,
                                    mCurrent.mCameraMouse.Camera.ViewMatrix);

    if (fabs(modelViewPos.z) - fabs(intersectViewPos.z) < -FLT_EPSILON) {
      continue;
    }

    const recti::Vec4 localPos = intersectWorldPos - mCurrent.mModel.position();
    recti::Vec4 idealPosOnCircle = Normalized(localPos);
    idealPosOnCircle.TransformVector(mCurrent.mModelInverse);
    const recti::Vec2 idealPosOnCircleScreen = recti::worldToPos(
      idealPosOnCircle * ROTATION_DISPLAY_FACTOR * mCurrent.mScreenFactor,
      mCurrent.mMVP,
      mCurrent.mCameraMouse.Camera.Viewport);

    const recti::Vec2 distanceOnScreen = idealPosOnCircleScreen - mousePos;

    const float distance =
      recti::Vec4{ distanceOnScreen.X, distanceOnScreen.Y }.Length();
    if (distance < 8.f) // pixel size
    {
      type = (recti::MOVETYPE)(recti::MT_ROTATE_X + i);
    }
  }

  return type;
}

static recti::MOVETYPE
GetScaleType(const recti::ModelContext& mCurrent,
             bool mAllowAxisFlip,
             State* state)
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
    recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
    bool belowAxisLimit, belowPlaneLimit;
    ComputeTripodAxisAndVisibility(mCurrent,
                                   mAllowAxisFlip,
                                   i,
                                   state,
                                   dirAxis,
                                   dirPlaneX,
                                   dirPlaneY,
                                   belowAxisLimit,
                                   belowPlaneLimit);
    dirAxis.TransformVector(mCurrent.mModelLocal);
    dirPlaneX.TransformVector(mCurrent.mModelLocal);
    dirPlaneY.TransformVector(mCurrent.mModelLocal);

    recti::Vec4 posOnPlan = mCurrent.mCameraMouse.Ray.IntersectPlane(
      BuildPlan(mCurrent.mModelLocal.position(), dirAxis));

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
      dirAxis * mCurrent.mScreenFactor * startOffset);
    const recti::Vec2 axisEndOnScreen = mCurrent.mCameraMouse.WorldToPos(
      mCurrent.mModelLocal.position() +
      dirAxis * mCurrent.mScreenFactor * endOffset);

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

    recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
    bool belowAxisLimit, belowPlaneLimit;
    ComputeTripodAxisAndVisibility(mCurrent,
                                   mAllowAxisFlip,
                                   i,
                                   state,
                                   dirAxis,
                                   dirPlaneX,
                                   dirPlaneY,
                                   belowAxisLimit,
                                   belowPlaneLimit);

    // draw axis
    if (belowAxisLimit) {
      bool hasTranslateOnAxis =
        Contains(mCurrent.mOperation,
                 static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
      float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
      recti::Vec2 worldDirSSpace =
        recti::worldToPos((dirAxis * markerScale) * mCurrent.mScreenFactor,
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

static float
ComputeAngleOnPlan(const recti::ModelContext& mCurrent,
                   const recti::Vec4& mRotationVectorSource,
                   const recti::Vec4& mTranslationPlan)
{
  recti::Vec4 localPos =
    Normalized(mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan) -
               mCurrent.mModel.position());

  recti::Vec4 perpendicularVector;
  perpendicularVector.Cross(mRotationVectorSource, mTranslationPlan);
  perpendicularVector.Normalize();
  float acosAngle =
    recti::Clamp(Dot(localPos, mRotationVectorSource), -1.f, 1.f);
  float angle = acosf(acosAngle);
  angle *= (Dot(localPos, perpendicularVector) < 0.f) ? 1.f : -1.f;
  return angle;
}

class ContextImpl
{
  recti::CameraMouse mCameraMouse;

  // over frame
  State mState = {};

  std::shared_ptr<recti::DrawList> mDrawList;
  recti::Style mStyle;

  float mRadiusSquareCenter;

  recti::Vec4 mRelativeOrigin;

  // translation
  recti::Vec4 mTranslationPlan;
  recti::Vec4 mTranslationPlanOrigin;
  recti::Vec4 mMatrixOrigin;
  recti::Vec4 mTranslationLastDelta;

  // rotation
  recti::Vec4 mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;

  // scale
  recti::Vec4 mScale;
  recti::Vec4 mScaleValueOrigin;
  recti::Vec4 mScaleLast;
  float mSaveMousePosx;

  // bounds stretching
  recti::Vec4 mBoundsPivot;
  recti::Vec4 mBoundsAnchor;
  recti::Vec4 mBoundsPlan;
  recti::Vec4 mBoundsLocalPivot;
  int mBoundsBestAxis;
  int mBoundsAxis[2];
  recti::Mat4 mBoundsMatrix;

  //
  recti::MOVETYPE mCurrentOperation;

  bool mIsOrthographic = false;

  recti::OPERATION mOperation = recti::OPERATION(-1);

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
        recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(mCurrent,
                                       mAllowAxisFlip,
                                       i,
                                       &mState,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(mCurrent.mOperation,
                     static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          recti::Vec2 baseSSpace =
            recti::worldToPos(dirAxis * 0.1f * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);
          recti::Vec2 worldDirSSpaceNoScale =
            recti::worldToPos(dirAxis * markerScale * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);
          recti::Vec2 worldDirSSpace = recti::worldToPos(
            (dirAxis * markerScale * scaleDisplay[i]) * mCurrent.mScreenFactor,
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
              mCurrent, dirAxis * scaleDisplay[i], mStyle);
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

  void DrawRotationGizmo(const recti::ModelContext& mCurrent,
                         recti::MOVETYPE type)
  {
    if (!Intersects(mCurrent.mOperation, recti::ROTATE)) {
      return;
    }
    auto drawList = mDrawList;

    // colors
    uint32_t colors[7];
    mStyle.ComputeColors(colors, type, recti::ROTATE);

    recti::Vec4 cameraToModelNormalized;
    if (mIsOrthographic) {
      cameraToModelNormalized = -mCameraMouse.mViewInverse.dir();
    } else {
      cameraToModelNormalized =
        Normalized(mCurrent.mModel.position() - mCameraMouse.CameraEye());
    }

    cameraToModelNormalized.TransformVector(mCurrent.mModelInverse);

    mRadiusSquareCenter = screenRotateSize * mCameraMouse.Camera.Height();

    bool hasRSC = Intersects(mCurrent.mOperation, recti::ROTATE_SCREEN);
    for (int axis = 0; axis < 3; axis++) {
      if (!Intersects(mCurrent.mOperation,
                      static_cast<recti::OPERATION>(recti::ROTATE_Z >> axis))) {
        continue;
      }
      const bool usingAxis =
        (mState.mbUsing && type == recti::MT_ROTATE_Z - axis);
      const int circleMul = (hasRSC && !usingAxis) ? 1 : 2;

      recti::Vec2* circlePos = (recti::Vec2*)alloca(
        sizeof(recti::Vec2) * (circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1));

      float angleStart = atan2f(cameraToModelNormalized[(4 - axis) % 3],
                                cameraToModelNormalized[(3 - axis) % 3]) +
                         std::numbers::pi * 0.5f;

      //
      for (int i = 0; i < circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1; i++) {
        float ng = angleStart + (float)circleMul * std::numbers::pi *
                                  ((float)i / (float)HALF_CIRCLE_SEGMENT_COUNT);
        recti::Vec4 axisPos = { cosf(ng), sinf(ng), 0.f };
        recti::Vec4 pos = recti::Vec4{ axisPos[axis],
                                       axisPos[(axis + 1) % 3],
                                       axisPos[(axis + 2) % 3] } *
                          mCurrent.mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] = recti::worldToPos(
          pos, mCurrent.mMVP, mCurrent.mCameraMouse.Camera.Viewport);
      }
      if (!mState.mbUsing || usingAxis) {
        drawList->AddPolyline((const recti::VEC2*)circlePos,
                              circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                              colors[3 - axis],
                              false,
                              mStyle.RotationLineThickness);
      }

      float radiusAxis = sqrtf(
        (mCameraMouse.WorldToPos(mCurrent.mModel.position()) - circlePos[0])
          .SqrLength());
      if (radiusAxis > mRadiusSquareCenter) {
        mRadiusSquareCenter = radiusAxis;
      }
    }
    if (hasRSC && (!mState.mbUsing || type == recti::MT_ROTATE_SCREEN)) {
      drawList->AddCircle(mCameraMouse.WorldToPos(mCurrent.mModel.position()),
                          mRadiusSquareCenter,
                          colors[0],
                          64,
                          mStyle.RotationOuterLineThickness);
    }

    if (mState.Using(mCurrent.mActualID) && IsRotateType(type)) {
      recti::Vec2 circlePos[HALF_CIRCLE_SEGMENT_COUNT + 1];

      circlePos[0] = mCameraMouse.WorldToPos(mCurrent.mModel.position());
      for (unsigned int i = 1; i < HALF_CIRCLE_SEGMENT_COUNT; i++) {
        float ng = mRotationAngle *
                   ((float)(i - 1) / (float)(HALF_CIRCLE_SEGMENT_COUNT - 1));
        recti::Mat4 rotateVectorMatrix;
        rotateVectorMatrix.RotationAxis(mTranslationPlan, ng);
        recti::Vec4 pos;
        pos.TransformPoint(mRotationVectorSource, rotateVectorMatrix);
        pos *= mCurrent.mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] =
          mCameraMouse.WorldToPos(pos + mCurrent.mModel.position());
      }
      drawList->AddConvexPolyFilled(
        (const recti::VEC2*)circlePos,
        HALF_CIRCLE_SEGMENT_COUNT,
        mStyle.GetColorU32(recti::ROTATION_USING_FILL));
      drawList->AddPolyline((const recti::VEC2*)circlePos,
                            HALF_CIRCLE_SEGMENT_COUNT,
                            mStyle.GetColorU32(recti::ROTATION_USING_BORDER),
                            true,
                            mStyle.RotationLineThickness);

      recti::Vec2 destinationPosOnScreen = circlePos[1];
      char tmps[512];
      snprintf(tmps,
               sizeof(tmps),
               rotationInfoMask[type - recti::MT_ROTATE_X],
               (mRotationAngle / std::numbers::pi) * 180.f,
               mRotationAngle);
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
        recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(mCurrent,
                                       mAllowAxisFlip,
                                       i,
                                       &mState,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(mCurrent.mOperation,
                     static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          recti::Vec2 worldDirSSpace = recti::worldToPos(
            (dirAxis * markerScale * scaleDisplay[i]) * mCurrent.mScreenFactor,
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

  void DrawTranslationGizmo(const recti::ModelContext& mCurrent,
                            recti::MOVETYPE type)
  {
    auto drawList = mDrawList;
    if (!drawList) {
      return;
    }

    if (!Intersects(mCurrent.mOperation, recti::TRANSLATE)) {
      return;
    }

    // colors
    uint32_t colors[7];
    mStyle.ComputeColors(colors, type, recti::TRANSLATE);

    const recti::Vec2 origin =
      mCameraMouse.WorldToPos(mCurrent.mModel.position());

    // draw
    bool belowAxisLimit = false;
    bool belowPlaneLimit = false;
    for (int i = 0; i < 3; ++i) {
      recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
      ComputeTripodAxisAndVisibility(mCurrent,
                                     mAllowAxisFlip,
                                     i,
                                     &mState,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit);

      if (!mState.mbUsing || (mState.mbUsing && type == recti::MT_MOVE_X + i)) {
        // draw axis
        if (belowAxisLimit && Intersects(mCurrent.mOperation,
                                         static_cast<recti::OPERATION>(
                                           recti::TRANSLATE_X << i))) {
          recti::Vec2 baseSSpace =
            recti::worldToPos(dirAxis * 0.1f * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);
          recti::Vec2 worldDirSSpace =
            recti::worldToPos(dirAxis * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.Viewport);

          drawList->AddLine(baseSSpace,
                            worldDirSSpace,
                            colors[i + 1],
                            mStyle.TranslationLineThickness);

          // Arrow head begin
          recti::Vec2 dir(origin - worldDirSSpace);

          float d = sqrtf(dir.SqrLength());
          dir /= d; // Normalize
          dir *= mStyle.TranslationLineArrowSize;

          recti::Vec2 ortogonalDir(dir.Y, -dir.X); // Perpendicular vector
          recti::Vec2 a(worldDirSSpace + dir);
          drawList->AddTriangleFilled(worldDirSSpace - dir,
                                      a + ortogonalDir,
                                      a - ortogonalDir,
                                      colors[i + 1]);
          // Arrow head end

          if (mState.mAxisFactor[i] < 0.f) {
            mDrawList->DrawHatchedAxis(mCurrent, dirAxis, mStyle);
          }
        }
      }
      // draw plane
      if (!mState.mbUsing ||
          (mState.mbUsing && type == recti::MT_MOVE_YZ + i)) {
        if (belowPlaneLimit &&
            Contains(mCurrent.mOperation, TRANSLATE_PLANS[i])) {
          recti::Vec2 screenQuadPts[4];
          for (int j = 0; j < 4; ++j) {
            recti::Vec4 cornerWorldPos =
              (dirPlaneX * quadUV[j * 2] + dirPlaneY * quadUV[j * 2 + 1]) *
              mCurrent.mScreenFactor;
            screenQuadPts[j] =
              recti::worldToPos(cornerWorldPos,
                                mCurrent.mMVP,
                                mCurrent.mCameraMouse.Camera.Viewport);
          }
          drawList->AddPolyline((const recti::VEC2*)screenQuadPts,
                                4,
                                mStyle.GetColorU32(recti::DIRECTION_X + i),
                                true,
                                1.0f);
          drawList->AddConvexPolyFilled(
            (const recti::VEC2*)screenQuadPts, 4, colors[i + 4]);
        }
      }
    }

    drawList->AddCircleFilled(
      mCurrent.mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

    if (mState.Using(mCurrent.mActualID) && IsTranslateType(type)) {
      uint32_t translationLineColor =
        mStyle.GetColorU32(recti::TRANSLATION_LINE);

      recti::Vec2 sourcePosOnScreen = mCameraMouse.WorldToPos(mMatrixOrigin);
      recti::Vec2 destinationPosOnScreen =
        mCameraMouse.WorldToPos(mCurrent.mModel.position());
      recti::Vec4 dif = { destinationPosOnScreen.X - sourcePosOnScreen.X,
                          destinationPosOnScreen.Y - sourcePosOnScreen.Y,
                          0.f,
                          0.f };
      dif.Normalize();
      dif *= 5.f;
      drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
      drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
      drawList->AddLine(
        recti::Vec2(sourcePosOnScreen.X + dif.x, sourcePosOnScreen.Y + dif.y),
        recti::Vec2(destinationPosOnScreen.X - dif.x,
                    destinationPosOnScreen.Y - dif.y),
        translationLineColor,
        2.f);

      char tmps[512];
      recti::Vec4 deltaInfo = mCurrent.mModel.position() - mMatrixOrigin;
      int componentInfoIndex = (type - recti::MT_MOVE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               translationInfoMask[type - recti::MT_MOVE_X],
               deltaInfo[translationInfoIndex[componentInfoIndex]],
               deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
               deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
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

  void HandleAndDrawLocalBounds(const recti::ModelContext& mCurrent,
                                const float* bounds,
                                const float* snapValues,
                                recti::Mat4* matrix)
  {
    // ImGuiIO& io = ImGui::GetIO();
    auto drawList = mDrawList;

    // compute best projection axis
    recti::Vec4 axesWorldDirections[3];
    recti::Vec4 bestAxisWorldDirection = { 0.0f, 0.0f, 0.0f, 0.0f };
    int axes[3];
    unsigned int numAxes = 1;
    axes[0] = mBoundsBestAxis;
    int bestAxis = axes[0];
    if (!mState.mbUsingBounds) {
      numAxes = 0;
      float bestDot = 0.f;
      for (int i = 0; i < 3; i++) {
        recti::Vec4 dirPlaneNormalWorld;
        dirPlaneNormalWorld.TransformVector(directionUnary[i],
                                            mCurrent.mModelSource);
        dirPlaneNormalWorld.Normalize();

        float dt = fabsf(Dot(Normalized(mCameraMouse.CameraEye() -
                                        mCurrent.mModelSource.position()),
                             dirPlaneNormalWorld));
        if (dt >= bestDot) {
          bestDot = dt;
          bestAxis = i;
          bestAxisWorldDirection = dirPlaneNormalWorld;
        }

        if (dt >= 0.1f) {
          axes[numAxes] = i;
          axesWorldDirections[numAxes] = dirPlaneNormalWorld;
          ++numAxes;
        }
      }
    }

    if (numAxes == 0) {
      axes[0] = bestAxis;
      axesWorldDirections[0] = bestAxisWorldDirection;
      numAxes = 1;
    }

    else if (bestAxis != axes[0]) {
      unsigned int bestIndex = 0;
      for (unsigned int i = 0; i < numAxes; i++) {
        if (axes[i] == bestAxis) {
          bestIndex = i;
          break;
        }
      }
      int tempAxis = axes[0];
      axes[0] = axes[bestIndex];
      axes[bestIndex] = tempAxis;
      recti::Vec4 tempDirection = axesWorldDirections[0];
      axesWorldDirections[0] = axesWorldDirections[bestIndex];
      axesWorldDirections[bestIndex] = tempDirection;
    }

    auto& mouse = mCurrent.mCameraMouse.Mouse;
    for (unsigned int axisIndex = 0; axisIndex < numAxes; ++axisIndex) {
      bestAxis = axes[axisIndex];
      bestAxisWorldDirection = axesWorldDirections[axisIndex];

      // corners
      recti::Vec4 aabb[4];

      int secondAxis = (bestAxis + 1) % 3;
      int thirdAxis = (bestAxis + 2) % 3;

      for (int i = 0; i < 4; i++) {
        aabb[i][3] = aabb[i][bestAxis] = 0.f;
        aabb[i][secondAxis] = bounds[secondAxis + 3 * (i >> 1)];
        aabb[i][thirdAxis] = bounds[thirdAxis + 3 * ((i >> 1) ^ (i & 1))];
      }

      // draw bounds
      unsigned int anchorAlpha = recti::COL32_BLACK();

      recti::Mat4 boundsMVP =
        mCurrent.mModelSource * mCameraMouse.mViewProjection;
      for (int i = 0; i < 4; i++) {
        recti::Vec2 worldBound1 = recti::worldToPos(
          aabb[i], boundsMVP, mCurrent.mCameraMouse.Camera.Viewport);
        recti::Vec2 worldBound2 = recti::worldToPos(
          aabb[(i + 1) % 4], boundsMVP, mCurrent.mCameraMouse.Camera.Viewport);
        if (!mCameraMouse.Camera.IsInContextRect(worldBound1) ||
            !mCameraMouse.Camera.IsInContextRect(worldBound2)) {
          continue;
        }
        float boundDistance = sqrtf((worldBound1 - worldBound2).SqrLength());
        int stepCount = (int)(boundDistance / 10.f);
        stepCount = recti::min(stepCount, 1000);
        for (int j = 0; j < stepCount; j++) {
          float stepLength = 1.f / (float)stepCount;
          float t1 = (float)j * stepLength;
          float t2 = (float)j * stepLength + stepLength * 0.5f;
          recti::Vec2 worldBoundSS1 =
            Lerp(worldBound1, worldBound2, recti::Vec2(t1, t1));
          recti::Vec2 worldBoundSS2 =
            Lerp(worldBound1, worldBound2, recti::Vec2(t2, t2));
          // drawList->AddLine(worldBoundSS1, worldBoundSS2, IM_COL32(0, 0, 0,
          // 0)
          // + anchorAlpha, 3.f);
          drawList->AddLine(worldBoundSS1,
                            worldBoundSS2,
                            recti::COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha,
                            2.f);
        }
        recti::Vec4 midPoint = (aabb[i] + aabb[(i + 1) % 4]) * 0.5f;
        recti::Vec2 midBound = recti::worldToPos(
          midPoint, boundsMVP, mCurrent.mCameraMouse.Camera.Viewport);
        static const float AnchorBigRadius = 8.f;
        static const float AnchorSmallRadius = 6.f;
        bool overBigAnchor = (worldBound1 - mouse.Position).SqrLength() <=
                             (AnchorBigRadius * AnchorBigRadius);
        bool overSmallAnchor = (midBound - mouse.Position).SqrLength() <=
                               (AnchorBigRadius * AnchorBigRadius);

        int type = recti::MT_NONE;

        if (Intersects(mCurrent.mOperation, recti::TRANSLATE)) {
          type = GetMoveType(mCurrent, mAllowAxisFlip, &mState);
        }
        if (Intersects(mCurrent.mOperation, recti::ROTATE) &&
            type == recti::MT_NONE) {
          type = GetRotateType(mCurrent, mRadiusSquareCenter, mState);
        }
        if (Intersects(mCurrent.mOperation, recti::SCALE) &&
            type == recti::MT_NONE) {
          type = GetScaleType(mCurrent, mAllowAxisFlip, &mState);
        }

        if (type != recti::MT_NONE) {
          overBigAnchor = false;
          overSmallAnchor = false;
        }

        uint32_t selectionColor = mStyle.GetColorU32(recti::SELECTION);

        unsigned int bigAnchorColor =
          overBigAnchor ? selectionColor
                        : (recti::COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha);
        unsigned int smallAnchorColor =
          overSmallAnchor ? selectionColor
                          : (recti::COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha);

        drawList->AddCircleFilled(
          worldBound1, AnchorBigRadius, recti::COL32_BLACK());
        drawList->AddCircleFilled(
          worldBound1, AnchorBigRadius - 1.2f, bigAnchorColor);

        drawList->AddCircleFilled(
          midBound, AnchorSmallRadius, recti::COL32_BLACK());
        drawList->AddCircleFilled(
          midBound, AnchorSmallRadius - 1.2f, smallAnchorColor);
        int oppositeIndex = (i + 2) % 4;
        // big anchor on corners
        if (!mState.mbUsingBounds && overBigAnchor && mouse.LeftDown) {
          mBoundsPivot.TransformPoint(aabb[(i + 2) % 4], mCurrent.mModelSource);
          mBoundsAnchor.TransformPoint(aabb[i], mCurrent.mModelSource);
          mBoundsPlan = BuildPlan(mBoundsAnchor, bestAxisWorldDirection);
          mBoundsBestAxis = bestAxis;
          mBoundsAxis[0] = secondAxis;
          mBoundsAxis[1] = thirdAxis;

          mBoundsLocalPivot.Set(0.f);
          mBoundsLocalPivot[secondAxis] = aabb[oppositeIndex][secondAxis];
          mBoundsLocalPivot[thirdAxis] = aabb[oppositeIndex][thirdAxis];

          mState.mbUsingBounds = true;
          mState.mEditingID = mCurrent.mActualID;
          mBoundsMatrix = mCurrent.mModelSource;
        }
        // small anchor on middle of segment
        if (!mState.mbUsingBounds && overSmallAnchor && mouse.LeftDown) {
          recti::Vec4 midPointOpposite =
            (aabb[(i + 2) % 4] + aabb[(i + 3) % 4]) * 0.5f;
          mBoundsPivot.TransformPoint(midPointOpposite, mCurrent.mModelSource);
          mBoundsAnchor.TransformPoint(midPoint, mCurrent.mModelSource);
          mBoundsPlan = BuildPlan(mBoundsAnchor, bestAxisWorldDirection);
          mBoundsBestAxis = bestAxis;
          int indices[] = { secondAxis, thirdAxis };
          mBoundsAxis[0] = indices[i % 2];
          mBoundsAxis[1] = -1;

          mBoundsLocalPivot.Set(0.f);
          mBoundsLocalPivot[mBoundsAxis[0]] =
            aabb[oppositeIndex]
                [indices[i % 2]]; // bounds[mBoundsAxis[0]] * (((i
                                  // + 1) & 2) ? 1.f : -1.f);

          mState.mbUsingBounds = true;
          mState.mEditingID = mCurrent.mActualID;
          mBoundsMatrix = mCurrent.mModelSource;
        }
      }

      if (mState.Using(mCurrent.mActualID)) {
        recti::Mat4 scale;
        scale.SetToIdentity();

        // compute projected mouse position on plan
        recti::Vec4 newPos = mCameraMouse.Ray.IntersectPlane(mBoundsPlan);

        // compute a reference and delta vectors base on mouse move
        recti::Vec4 deltaVector = (newPos - mBoundsPivot).Abs();
        recti::Vec4 referenceVector = (mBoundsAnchor - mBoundsPivot).Abs();

        // for 1 or 2 axes, compute a ratio that's used for scale and snap it
        // based on resulting length
        for (int i = 0; i < 2; i++) {
          int axisIndex1 = mBoundsAxis[i];
          if (axisIndex1 == -1) {
            continue;
          }

          float ratioAxis = 1.f;
          recti::Vec4 axisDir = mBoundsMatrix.component(axisIndex1).Abs();

          float dtAxis = axisDir.Dot(referenceVector);
          float boundSize = bounds[axisIndex1 + 3] - bounds[axisIndex1];
          if (dtAxis > FLT_EPSILON) {
            ratioAxis = axisDir.Dot(deltaVector) / dtAxis;
          }

          if (snapValues) {
            float length = boundSize * ratioAxis;
            recti::ComputeSnap(&length, snapValues[axisIndex1]);
            if (boundSize > FLT_EPSILON) {
              ratioAxis = length / boundSize;
            }
          }
          scale.component(axisIndex1) *= ratioAxis;
        }

        // transform matrix
        recti::Mat4 preScale, postScale;
        preScale.Translation(-mBoundsLocalPivot);
        postScale.Translation(mBoundsLocalPivot);
        recti::Mat4 res = preScale * scale * postScale * mBoundsMatrix;
        *matrix = res;

        // info text
        char tmps[512];
        recti::Vec2 destinationPosOnScreen =
          mCameraMouse.WorldToPos(mCurrent.mModel.position());
        snprintf(tmps,
                 sizeof(tmps),
                 "X: %.2f Y: %.2f Z: %.2f",
                 (bounds[3] - bounds[0]) * mBoundsMatrix.component(0).Length() *
                   scale.component(0).Length(),
                 (bounds[4] - bounds[1]) * mBoundsMatrix.component(1).Length() *
                   scale.component(1).Length(),
                 (bounds[5] - bounds[2]) * mBoundsMatrix.component(2).Length() *
                   scale.component(2).Length());
        drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                      destinationPosOnScreen.Y + 15),
                          mStyle.GetColorU32(recti::TEXT_SHADOW),
                          tmps);
        drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                      destinationPosOnScreen.Y + 14),
                          mStyle.GetColorU32(recti::TEXT),
                          tmps);
      }

      if (!mouse.LeftDown) {
        mState.mbUsingBounds = false;
        mState.mEditingID = -1;
      }
      if (mState.mbUsingBounds) {
        break;
      }
    }
  }

  bool HandleTranslation(const recti::ModelContext& mCurrent,
                         float* matrix,
                         float* deltaMatrix,
                         recti::MOVETYPE& type,
                         const float* snap)
  {
    if (!Intersects(mCurrent.mOperation, recti::TRANSLATE) ||
        type != recti::MT_NONE) {
      return false;
    }
    const bool applyRotationLocaly =
      mCurrent.mMode == recti::LOCAL || type == recti::MT_MOVE_SCREEN;
    bool modified = false;

    // move
    auto& mouse = mCurrent.mCameraMouse.Mouse;
    if (mState.Using(mCurrent.mActualID) &&
        IsTranslateType(mCurrentOperation)) {
      const recti::Vec4 newPos =
        mCameraMouse.Ray.IntersectPlane(mTranslationPlan);

      // compute delta
      const recti::Vec4 newOrigin =
        newPos - mRelativeOrigin * mCurrent.mScreenFactor;
      recti::Vec4 delta = newOrigin - mCurrent.mModel.position();

      // 1 axis constraint
      if (mCurrentOperation >= recti::MT_MOVE_X &&
          mCurrentOperation <= recti::MT_MOVE_Z) {
        const int axisIndex = mCurrentOperation - recti::MT_MOVE_X;
        const recti::Vec4& axisValue = mCurrent.mModel.component(axisIndex);
        const float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;
      }

      // snap
      if (snap) {
        recti::Vec4 cumulativeDelta =
          mCurrent.mModel.position() + delta - mMatrixOrigin;
        if (applyRotationLocaly) {
          recti::Mat4 modelSourceNormalized = mCurrent.mModelSource;
          modelSourceNormalized.OrthoNormalize();
          recti::Mat4 modelSourceNormalizedInverse;
          modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
          cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
          ComputeSnap(cumulativeDelta, snap);
          cumulativeDelta.TransformVector(modelSourceNormalized);
        } else {
          ComputeSnap(cumulativeDelta, snap);
        }
        delta = mMatrixOrigin + cumulativeDelta - mCurrent.mModel.position();
      }

      if (delta != mTranslationLastDelta) {
        modified = true;
      }
      mTranslationLastDelta = delta;

      // compute matrix & delta
      recti::Mat4 deltaMatrixTranslation;
      deltaMatrixTranslation.Translation(delta);
      if (deltaMatrix) {
        memcpy(deltaMatrix, &deltaMatrixTranslation.m00, sizeof(float) * 16);
      }

      const recti::Mat4 res = mCurrent.mModelSource * deltaMatrixTranslation;
      *(recti::Mat4*)matrix = res;

      if (!mouse.LeftDown) {
        mState.mbUsing = false;
      }

      type = mCurrentOperation;
    } else {
      // find new possible way to move
      type = GetMoveType(mCurrent, mAllowAxisFlip, &mState);
      if (type != recti::MT_NONE) {
      }
      if (mouse.LeftDown && type != recti::MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mCurrent.mActualID;
        mCurrentOperation = type;
        recti::Vec4 movePlanNormal[] = {
          mCurrent.mModel.right(),  mCurrent.mModel.up(), mCurrent.mModel.dir(),
          mCurrent.mModel.right(),  mCurrent.mModel.up(), mCurrent.mModel.dir(),
          -mCameraMouse.CameraDir()
        };

        recti::Vec4 cameraToModelNormalized =
          Normalized(mCurrent.mModel.position() - mCameraMouse.CameraEye());
        for (unsigned int i = 0; i < 3; i++) {
          recti::Vec4 orthoVector =
            Cross(movePlanNormal[i], cameraToModelNormalized);
          movePlanNormal[i].Cross(orthoVector);
          movePlanNormal[i].Normalize();
        }
        // pickup plan
        mTranslationPlan = BuildPlan(mCurrent.mModel.position(),
                                     movePlanNormal[type - recti::MT_MOVE_X]);
        mTranslationPlanOrigin =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
        mMatrixOrigin = mCurrent.mModel.position();

        mRelativeOrigin =
          (mTranslationPlanOrigin - mCurrent.mModel.position()) *
          (1.f / mCurrent.mScreenFactor);
      }
    }
    return modified;
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
        mCurrentOperation = type;
        const recti::Vec4 movePlanNormal[] = {
          mCurrent.mModel.up(),     mCurrent.mModel.dir(),
          mCurrent.mModel.right(),  mCurrent.mModel.dir(),
          mCurrent.mModel.up(),     mCurrent.mModel.right(),
          -mCameraMouse.CameraDir()
        };
        // pickup plan

        mTranslationPlan = BuildPlan(mCurrent.mModel.position(),
                                     movePlanNormal[type - recti::MT_SCALE_X]);
        mTranslationPlanOrigin =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
        mMatrixOrigin = mCurrent.mModel.position();
        mScale.Set(1.f, 1.f, 1.f);
        mRelativeOrigin =
          (mTranslationPlanOrigin - mCurrent.mModel.position()) *
          (1.f / mCurrent.mScreenFactor);
        mScaleValueOrigin = { mCurrent.mModelSource.right().Length(),
                              mCurrent.mModelSource.up().Length(),
                              mCurrent.mModelSource.dir().Length() };
        mSaveMousePosx = mouse.Position.X;
      }
    }
    // scale
    if (mState.Using(mCurrent.mActualID) && IsScaleType(mCurrentOperation)) {
      recti::Vec4 newPos = mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
      recti::Vec4 newOrigin = newPos - mRelativeOrigin * mCurrent.mScreenFactor;
      recti::Vec4 delta = newOrigin - mCurrent.mModelLocal.position();

      // 1 axis constraint
      if (mCurrentOperation >= recti::MT_SCALE_X &&
          mCurrentOperation <= recti::MT_SCALE_Z) {
        int axisIndex = mCurrentOperation - recti::MT_SCALE_X;
        const recti::Vec4& axisValue =
          mCurrent.mModelLocal.component(axisIndex);
        float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;

        recti::Vec4 baseVector =
          mTranslationPlanOrigin - mCurrent.mModelLocal.position();
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

      type = mCurrentOperation;
    }
    return modified;
  }

  bool HandleRotation(const recti::ModelContext& mCurrent,
                      float* matrix,
                      float* deltaMatrix,
                      recti::MOVETYPE& type,
                      const float* snap)
  {
    if (!Intersects(mCurrent.mOperation, recti::ROTATE) ||
        type != recti::MT_NONE) {
      return false;
    }
    bool applyRotationLocaly = mCurrent.mMode == recti::LOCAL;
    bool modified = false;

    auto& mouse = mCurrent.mCameraMouse.Mouse;
    if (!mState.mbUsing) {
      type = GetRotateType(mCurrent, mRadiusSquareCenter, mState);

      if (type != recti::MT_NONE) {
      }

      if (type == recti::MT_ROTATE_SCREEN) {
        applyRotationLocaly = true;
      }

      if (mouse.LeftDown && type != recti::MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mCurrent.mActualID;
        mCurrentOperation = type;
        const recti::Vec4 rotatePlanNormal[] = { mCurrent.mModel.right(),
                                                 mCurrent.mModel.up(),
                                                 mCurrent.mModel.dir(),
                                                 -mCameraMouse.CameraDir() };
        // pickup plan
        if (applyRotationLocaly) {
          mTranslationPlan =
            BuildPlan(mCurrent.mModel.position(),
                      rotatePlanNormal[type - recti::MT_ROTATE_X]);
        } else {
          mTranslationPlan =
            BuildPlan(mCurrent.mModelSource.position(),
                      directionUnary[type - recti::MT_ROTATE_X]);
        }

        recti::Vec4 localPos =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan) -
          mCurrent.mModel.position();
        mRotationVectorSource = Normalized(localPos);
        mRotationAngleOrigin =
          ComputeAngleOnPlan(mCurrent, mRotationVectorSource, mTranslationPlan);
      }
    }

    // rotation
    if (mState.Using(mCurrent.mActualID) && IsRotateType(mCurrentOperation)) {
      mRotationAngle =
        ComputeAngleOnPlan(mCurrent, mRotationVectorSource, mTranslationPlan);
      if (snap) {
        float snapInRadian = snap[0] * DEG2RAD;
        recti::ComputeSnap(&mRotationAngle, snapInRadian);
      }
      recti::Vec4 rotationAxisLocalSpace;

      rotationAxisLocalSpace.TransformVector(
        { mTranslationPlan.x, mTranslationPlan.y, mTranslationPlan.z, 0.f },
        mCurrent.mModelInverse);
      rotationAxisLocalSpace.Normalize();

      recti::Mat4 deltaRotation;
      deltaRotation.RotationAxis(rotationAxisLocalSpace,
                                 mRotationAngle - mRotationAngleOrigin);
      if (mRotationAngle != mRotationAngleOrigin) {
        modified = true;
      }
      mRotationAngleOrigin = mRotationAngle;

      recti::Mat4 scaleOrigin;
      scaleOrigin.Scale(mCurrent.mModelScaleOrigin);

      if (applyRotationLocaly) {
        *(recti::Mat4*)matrix =
          scaleOrigin * deltaRotation * mCurrent.mModelLocal;
      } else {
        recti::Mat4 res = mCurrent.mModelSource;
        res.position().Set(0.f);

        *(recti::Mat4*)matrix = res * deltaRotation;
        ((recti::Mat4*)matrix)->position() = mCurrent.mModelSource.position();
      }

      if (deltaMatrix) {
        *(recti::Mat4*)deltaMatrix =
          mCurrent.mModelInverse * deltaRotation * mCurrent.mModel;
      }

      if (!mouse.LeftDown) {
        mState.mbUsing = false;
        mState.mEditingID = -1;
      }
      type = mCurrentOperation;
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
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
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
    recti::MOVETYPE type = recti::MT_NONE;
    bool manipulated = false;
    {
      if (!mState.mbUsingBounds) {
        manipulated =
          HandleTranslation(mCurrent, matrix, deltaMatrix, type, snap) ||
          HandleScale(mCurrent, matrix, deltaMatrix, type, snap) ||
          HandleRotation(mCurrent, matrix, deltaMatrix, type, snap);
      }
    }

    if (localBounds && !mState.mbUsing) {
      HandleAndDrawLocalBounds(
        mCurrent, localBounds, boundsSnap, (recti::Mat4*)matrix);
    }

    mOperation = operation;
    if (!mState.mbUsingBounds) {
      DrawRotationGizmo(mCurrent, type);
      DrawTranslationGizmo(mCurrent, type);
      DrawScaleGizmo(mCurrent, type);
      DrawScaleUniveralGizmo(mCurrent, type);
    }
    return manipulated;
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
                    const float* snap,
                    const float* localBounds,
                    const float* boundsSnap)
{
  return m_impl->Manipulate((int64_t)id,
                            operation,
                            mode,
                            matrix,
                            deltaMatrix,
                            snap,
                            localBounds,
                            boundsSnap);
}

const recti::DrawList&
Context::End()
{
  return m_impl->GetDrawList();
}

} // namespace
