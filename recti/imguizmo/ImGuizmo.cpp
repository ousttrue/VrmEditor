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

#include "../mat4.h"
#include "../ray.h"
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

static inline float
ImSaturate(float f)
{
  return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f;
}
#define IM_F32_TO_INT8_SAT(_VAL)                                               \
  ((int)(ImSaturate(_VAL) * 255.0f + 0.5f)) // Saturated, always output 0..255
uint32_t
ColorConvertFloat4ToU32(const recti::Vec4& in)
{
  uint32_t out;
  out = ((uint32_t)IM_F32_TO_INT8_SAT(in.x)) << 0;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.y)) << 8;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.z)) << 16;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.w)) << 24;
  return out;
}

#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#define IM_COL32(R, G, B, A)                                                   \
  (((uint32_t)(A) << IM_COL32_A_SHIFT) | ((uint32_t)(B) << IM_COL32_B_SHIFT) | \
   ((uint32_t)(G) << IM_COL32_G_SHIFT) | ((uint32_t)(R) << IM_COL32_R_SHIFT))

const auto IM_COL32_WHITE = ColorConvertFloat4ToU32({ 1, 1, 1, 1 });
const auto IM_COL32_BLACK = ColorConvertFloat4ToU32({ 0, 0, 0, 1 });

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

enum COLOR
{
  DIRECTION_X,      // directionColor[0]
  DIRECTION_Y,      // directionColor[1]
  DIRECTION_Z,      // directionColor[2]
  PLANE_X,          // planeColor[0]
  PLANE_Y,          // planeColor[1]
  PLANE_Z,          // planeColor[2]
  SELECTION,        // selectionColor
  INACTIVE,         // inactiveColor
  TRANSLATION_LINE, // translationLineColor
  SCALE_LINE,
  ROTATION_USING_BORDER,
  ROTATION_USING_FILL,
  HATCHED_AXIS_LINES,
  TEXT,
  TEXT_SHADOW,
  COUNT
};

struct Style
{
  // Thickness of lines for translation gizmo
  float TranslationLineThickness = 3.0f;

  // Size of arrow at the end of lines for translation gizmo
  float TranslationLineArrowSize = 6.0f;

  // Thickness of lines for rotation gizmo
  float RotationLineThickness = 2.0f;

  // Thickness of line surrounding the rotation gizmo
  float RotationOuterLineThickness = 3.0f;

  // Thickness of lines for scale gizmo
  float ScaleLineThickness = 3.0f;

  // Size of circle at the end of lines for scale gizmo
  float ScaleLineCircleSize = 6.0f;

  // Thickness of hatched axis lines
  float HatchedAxisLineThickness = 6.0f;

  // Size of circle at the center of the translate/scale gizmo
  float CenterCircleSize = 6.0f;

  recti::Vec4 Colors[COLOR::COUNT] = {
    { 0.666f, 0.000f, 0.000f, 1.000f }, { 0.000f, 0.666f, 0.000f, 1.000f },
    { 0.000f, 0.000f, 0.666f, 1.000f }, { 0.666f, 0.000f, 0.000f, 0.380f },
    { 0.000f, 0.666f, 0.000f, 0.380f }, { 0.000f, 0.000f, 0.666f, 0.380f },
    { 1.000f, 0.500f, 0.062f, 0.541f }, { 0.600f, 0.600f, 0.600f, 0.600f },
    { 0.666f, 0.666f, 0.666f, 0.666f }, { 0.250f, 0.250f, 0.250f, 1.000f },
    { 1.000f, 0.500f, 0.062f, 1.000f }, { 1.000f, 0.500f, 0.062f, 0.500f },
    { 0.000f, 0.000f, 0.000f, 0.500f }, { 1.000f, 1.000f, 1.000f, 1.000f },
    { 0.000f, 0.000f, 0.000f, 1.000f },
  };
};

#include "ImGuizmo.h"

namespace ImGuizmo {

static const OPERATION TRANSLATE_PLANS[3] = { TRANSLATE_Y | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Y };

enum MOVETYPE
{
  MT_NONE,
  MT_MOVE_X,
  MT_MOVE_Y,
  MT_MOVE_Z,
  MT_MOVE_YZ,
  MT_MOVE_ZX,
  MT_MOVE_XY,
  MT_MOVE_SCREEN,
  MT_ROTATE_X,
  MT_ROTATE_Y,
  MT_ROTATE_Z,
  MT_ROTATE_SCREEN,
  MT_SCALE_X,
  MT_SCALE_Y,
  MT_SCALE_Z,
  MT_SCALE_XYZ
};
inline bool
IsTranslateType(MOVETYPE type)
{
  return type >= MT_MOVE_X && type <= MT_MOVE_SCREEN;
}
inline bool
IsRotateType(MOVETYPE type)
{
  return type >= MT_ROTATE_X && type <= MT_ROTATE_SCREEN;
}
inline bool
IsScaleType(MOVETYPE type)
{
  return type >= MT_SCALE_X && type <= MT_SCALE_XYZ;
}

struct State
{
  int64_t mActualID = -1;
  int64_t mEditingID = -1;
  bool mbUsing = false;

  bool Using() const
  {
    if (mbUsing) {
      if (mActualID == -1) {
        return true;
      }
      if (mActualID == mEditingID) {
        return true;
      }
    }
    return false;
  }
};

class ContextImpl
{
  recti::CameraMouse mCameraMouse;

  std::shared_ptr<recti::DrawList> mDrawList;
  Style mStyle;

  State mState = {};

  MODE mMode;

  recti::Mat4 mModel;
  recti::Mat4 mModelLocal; // orthonormalized model
  recti::Mat4 mModelInverse;
  recti::Mat4 mModelSource;
  recti::Mat4 mModelSourceInverse;
  recti::Mat4 mMVP;
  recti::Mat4
    mMVPLocal; // MVP with full model matrix whereas mMVP's model matrix
               // might only be translation in case of World space edition

  recti::Vec4 mModelScaleOrigin;

  float mRadiusSquareCenter;
  recti::Vec2 mScreenSquareCenter;

  float mScreenFactor;
  recti::Vec4 mRelativeOrigin;

  bool mbEnable;

  // translation
  recti::Vec4 mTranslationPlan;
  recti::Vec4 mTranslationPlanOrigin;
  recti::Vec4 mMatrixOrigin;
  recti::Vec4 mTranslationLastDelta;

  // rotation
  recti::Vec4 mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;
  // vec_t mWorldToLocalAxis;

  // scale
  recti::Vec4 mScale;
  recti::Vec4 mScaleValueOrigin;
  recti::Vec4 mScaleLast;
  float mSaveMousePosx;

  // save axis factor when using gizmo
  mutable bool mBelowAxisLimit[3];
  mutable bool mBelowPlaneLimit[3];
  mutable float mAxisFactor[3];

  // bounds stretching
  recti::Vec4 mBoundsPivot;
  recti::Vec4 mBoundsAnchor;
  recti::Vec4 mBoundsPlan;
  recti::Vec4 mBoundsLocalPivot;
  int mBoundsBestAxis;
  int mBoundsAxis[2];
  bool mbUsingBounds;
  recti::Mat4 mBoundsMatrix;

  //
  MOVETYPE mCurrentOperation;

  bool mIsOrthographic = false;

  OPERATION mOperation = OPERATION(-1);

  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

  bool mAllowActiveHoverItem = true;

public:
  ContextImpl()
    : mbEnable(true)
    , mbUsingBounds(false)
  {
    mDrawList = std::make_shared<recti::DrawList>();
  }

private:
  void ComputeTripodAxisAndVisibility(const int axisIndex,
                                      const recti::Mat4& mvp,
                                      const State& state,
                                      recti::Vec4& dirAxis,
                                      recti::Vec4& dirPlaneX,
                                      recti::Vec4& dirPlaneY,
                                      bool& belowAxisLimit,
                                      bool& belowPlaneLimit) const
  {
    dirAxis = directionUnary[axisIndex];
    dirPlaneX = directionUnary[(axisIndex + 1) % 3];
    dirPlaneY = directionUnary[(axisIndex + 2) % 3];

    if (state.Using()) {
      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      belowAxisLimit = mBelowAxisLimit[axisIndex];
      belowPlaneLimit = mBelowPlaneLimit[axisIndex];

      dirAxis *= mAxisFactor[axisIndex];
      dirPlaneX *= mAxisFactor[(axisIndex + 1) % 3];
      dirPlaneY *= mAxisFactor[(axisIndex + 2) % 3];
    } else {
      // new method
      float lenDir = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, dirAxis, mvp, mCameraMouse.Camera.DisplayRatio());
      float lenDirMinus = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, -dirAxis, mvp, mCameraMouse.Camera.DisplayRatio());

      float lenDirPlaneX = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, dirPlaneX, mvp, mCameraMouse.Camera.DisplayRatio());
      float lenDirMinusPlaneX = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, -dirPlaneX, mvp, mCameraMouse.Camera.DisplayRatio());

      float lenDirPlaneY = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, dirPlaneY, mvp, mCameraMouse.Camera.DisplayRatio());
      float lenDirMinusPlaneY = GetSegmentLengthClipSpace(
        { 0.f, 0.f, 0.f }, -dirPlaneY, mvp, mCameraMouse.Camera.DisplayRatio());

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
                                  dirAxis * mScreenFactor,
                                  mvp,
                                  mCameraMouse.Camera.DisplayRatio());

      float paraSurf = GetParallelogram({ 0.f, 0.f, 0.f },
                                        dirPlaneX * mScreenFactor,
                                        dirPlaneY * mScreenFactor,
                                        mMVP,
                                        mCameraMouse.Camera.DisplayRatio());
      belowPlaneLimit = (paraSurf > 0.0025f);
      belowAxisLimit = (axisLengthInClipSpace > 0.02f);

      // and store values
      mAxisFactor[axisIndex] = mulAxis;
      mAxisFactor[(axisIndex + 1) % 3] = mulAxisX;
      mAxisFactor[(axisIndex + 2) % 3] = mulAxisY;
      mBelowAxisLimit[axisIndex] = belowAxisLimit;
      mBelowPlaneLimit[axisIndex] = belowPlaneLimit;
    }
  }

  MOVETYPE GetMoveType(OPERATION op,
                       const recti::Vec4& screenCoord,
                       const recti::Vec2& mousePos) const
  {
    if (!Intersects(op, TRANSLATE) || mState.mbUsing) {
      return MT_NONE;
    }

    MOVETYPE type = MT_NONE;

    // compute
    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(i,
                                     mMVP,
                                     mState,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit);
      dirAxis.TransformVector(mModel);
      dirPlaneX.TransformVector(mModel);
      dirPlaneY.TransformVector(mModel);

      auto posOnPlan =
        mCameraMouse.Ray.IntersectPlane(BuildPlan(mModel.position(), dirAxis));

      const recti::Vec2 axisStartOnScreen =
        worldToPos(mModel.position() + dirAxis * mScreenFactor * 0.1f,
                   mCameraMouse.mViewProjection) -
        mCameraMouse.Camera.LeftTop();
      const recti::Vec2 axisEndOnScreen =
        worldToPos(mModel.position() + dirAxis * mScreenFactor,
                   mCameraMouse.mViewProjection) -
        mCameraMouse.Camera.LeftTop();

      recti::Vec4 closestPointOnAxis =
        PointOnSegment(screenCoord,
                       { axisStartOnScreen.X, axisStartOnScreen.Y },
                       { axisEndOnScreen.X, axisEndOnScreen.Y });
      if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
          Intersects(op,
                     static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
      {
        type = (MOVETYPE)(MT_MOVE_X + i);
      }

      const float dx =
        dirPlaneX.Dot3((posOnPlan - mModel.position()) * (1.f / mScreenFactor));
      const float dy =
        dirPlaneY.Dot3((posOnPlan - mModel.position()) * (1.f / mScreenFactor));
      if (belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
          dy >= quadUV[1] && dy <= quadUV[3] &&
          Contains(op, TRANSLATE_PLANS[i])) {
        type = (MOVETYPE)(MT_MOVE_YZ + i);
      }
    }
    return type;
  }

  MOVETYPE
  GetRotateType(OPERATION op, const recti::Vec2& mousePos) const
  {
    if (mState.mbUsing) {
      return MT_NONE;
    }
    MOVETYPE type = MT_NONE;

    recti::Vec4 deltaScreen = { mousePos.X - mScreenSquareCenter.X,
                                mousePos.Y - mScreenSquareCenter.Y,
                                0.f,
                                0.f };
    float dist = deltaScreen.Length();
    if (Intersects(op, ROTATE_SCREEN) && dist >= (mRadiusSquareCenter - 4.0f) &&
        dist < (mRadiusSquareCenter + 4.0f)) {
      type = MT_ROTATE_SCREEN;
    }

    const recti::Vec4 planNormals[] = { mModel.right(),
                                        mModel.up(),
                                        mModel.dir() };

    recti::Vec4 modelViewPos;
    modelViewPos.TransformPoint(mModel.position(),
                                mCameraMouse.Camera.ViewMatrix);

    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      if (!Intersects(op, static_cast<OPERATION>(ROTATE_X << i))) {
        continue;
      }
      // pickup plan
      recti::Vec4 pickupPlan = BuildPlan(mModel.position(), planNormals[i]);

      const recti::Vec4 intersectWorldPos =
        mCameraMouse.Ray.IntersectPlane(pickupPlan);
      recti::Vec4 intersectViewPos;
      intersectViewPos.TransformPoint(intersectWorldPos,
                                      mCameraMouse.Camera.ViewMatrix);

      if (fabs(modelViewPos.z) - fabs(intersectViewPos.z) < -FLT_EPSILON) {
        continue;
      }

      const recti::Vec4 localPos = intersectWorldPos - mModel.position();
      recti::Vec4 idealPosOnCircle = Normalized(localPos);
      idealPosOnCircle.TransformVector(mModelInverse);
      const recti::Vec2 idealPosOnCircleScreen = worldToPos(
        idealPosOnCircle * ROTATION_DISPLAY_FACTOR * mScreenFactor, mMVP);

      const recti::Vec2 distanceOnScreen = idealPosOnCircleScreen - mousePos;

      const float distance =
        recti::Vec4{ distanceOnScreen.X, distanceOnScreen.Y }.Length();
      if (distance < 8.f) // pixel size
      {
        type = (MOVETYPE)(MT_ROTATE_X + i);
      }
    }

    return type;
  }

  MOVETYPE GetScaleType(OPERATION op, const recti::Vec2& mousePos) const
  {
    if (mState.mbUsing) {
      return MT_NONE;
    }
    MOVETYPE type = MT_NONE;

    // compute
    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
        continue;
      }
      recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(i,
                                     mMVPLocal,
                                     mState,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit);
      dirAxis.TransformVector(mModelLocal);
      dirPlaneX.TransformVector(mModelLocal);
      dirPlaneY.TransformVector(mModelLocal);

      recti::Vec4 posOnPlan = mCameraMouse.Ray.IntersectPlane(
        BuildPlan(mModelLocal.position(), dirAxis));

      const float startOffset =
        Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.0f : 0.1f;
      const float endOffset =
        Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.4f : 1.0f;
      const recti::Vec2 posOnPlanScreen =
        worldToPos(posOnPlan, mCameraMouse.mViewProjection);
      const recti::Vec2 axisStartOnScreen = worldToPos(
        mModelLocal.position() + dirAxis * mScreenFactor * startOffset,
        mCameraMouse.mViewProjection);
      const recti::Vec2 axisEndOnScreen =
        worldToPos(mModelLocal.position() + dirAxis * mScreenFactor * endOffset,
                   mCameraMouse.mViewProjection);

      recti::Vec4 closestPointOnAxis =
        recti::PointOnSegment({ posOnPlanScreen.X, posOnPlanScreen.Y },
                              { axisStartOnScreen.X, axisStartOnScreen.Y },
                              { axisEndOnScreen.X, axisEndOnScreen.Y });

      if ((closestPointOnAxis -
           recti::Vec4{ posOnPlanScreen.X, posOnPlanScreen.Y })
            .Length() < 12.f) // pixel size
      {
        type = (MOVETYPE)(MT_SCALE_X + i);
      }
    }

    // universal

    recti::Vec4 deltaScreen = { mousePos.X - mScreenSquareCenter.X,
                                mousePos.Y - mScreenSquareCenter.Y,
                                0.f,
                                0.f };
    float dist = deltaScreen.Length();
    if (Contains(op, SCALEU) && dist >= 17.0f && dist < 23.0f) {
      type = MT_SCALE_XYZ;
    }

    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_XU << i))) {
        continue;
      }

      recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(i,
                                     mMVPLocal,
                                     mState,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit);

      // draw axis
      if (belowAxisLimit) {
        bool hasTranslateOnAxis =
          Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        // recti::Vec2 baseSSpace = worldToPos(dirAxis * 0.1f *
        // mScreenFactor, mMVPLocal); recti::Vec2
        // worldDirSSpaceNoScale = worldToPos(dirAxis
        // * markerScale * mScreenFactor, mMVP);
        recti::Vec2 worldDirSSpace =
          worldToPos((dirAxis * markerScale) * mScreenFactor, mMVPLocal);

        float distance = sqrtf((worldDirSSpace - mousePos).SqrLength());
        if (distance < 12.f) {
          type = (MOVETYPE)(MT_SCALE_X + i);
        }
      }
    }
    return type;
  }

  // return true if mouse cursor is over any gizmo control (axis, plan or screen
  // component)
  bool IsOver(const recti::Vec2& mousePos) const
  {
    return (Intersects(mOperation, TRANSLATE) &&
            GetMoveType(mOperation, mCameraMouse.ScreenMousePos(), mousePos) !=
              MT_NONE) ||
           (Intersects(mOperation, ROTATE) &&
            GetRotateType(mOperation, mousePos) != MT_NONE) ||
           (Intersects(mOperation, SCALE) &&
            GetScaleType(mOperation, mousePos) != MT_NONE) ||
           IsUsing();
  }

  // return true if the cursor is over the operation's gizmo
  bool IsOver(OPERATION op, const recti::Vec2& mousePos) const
  {
    if (IsUsing()) {
      return true;
    }
    if (Intersects(op, SCALE) && GetScaleType(op, mousePos) != MT_NONE) {
      return true;
    }
    if (Intersects(op, ROTATE) && GetRotateType(op, mousePos) != MT_NONE) {
      return true;
    }
    if (Intersects(op, TRANSLATE) &&
        GetMoveType(op, mCameraMouse.ScreenMousePos(), mousePos) != MT_NONE) {
      return true;
    }
    return false;
  }

  // return true if mouse IsOver or if the gizmo is in moving state
  bool IsUsing() const { return mState.Using() || mbUsingBounds; }

  recti::Vec2 worldToPos(const recti::Vec4& worldPos,
                         const recti::Mat4& mat) const
  {
    auto [x, y] =
      recti::worldToPos(worldPos, mat, mCameraMouse.Camera.Viewport);
    return { x, y };
  }

  void ComputeContext(float* matrix, MODE mode)
  {
    this->mMode = mode;

    this->mModelLocal = *(recti::Mat4*)matrix;
    this->mModelLocal.OrthoNormalize();

    if (mode == LOCAL) {
      this->mModel = this->mModelLocal;
    } else {
      this->mModel.Translation(((recti::Mat4*)matrix)->position());
    }
    this->mModelSource = *(recti::Mat4*)matrix;
    this->mModelScaleOrigin.Set(this->mModelSource.right().Length(),
                                this->mModelSource.up().Length(),
                                this->mModelSource.dir().Length());

    this->mModelInverse.Inverse(this->mModel);
    this->mModelSourceInverse.Inverse(this->mModelSource);
    this->mMVP = this->mModel * this->mCameraMouse.mViewProjection;
    this->mMVPLocal = this->mModelLocal * this->mCameraMouse.mViewProjection;

    // compute scale from the size of camera right vector projected on screen at
    // the matrix position
    recti::Vec4 pointRight = mCameraMouse.mViewInverse.right();
    pointRight.TransformPoint(this->mCameraMouse.mViewProjection);

    this->mScreenFactor = this->mGizmoSizeClipSpace /
                          (pointRight.x / pointRight.w -
                           this->mMVP.position().x / this->mMVP.position().w);
    recti::Vec4 rightViewInverse = mCameraMouse.mViewInverse.right();
    rightViewInverse.TransformVector(this->mModelInverse);
    float rightLength = GetSegmentLengthClipSpace(
      { 0.f, 0.f }, rightViewInverse, mMVP, mCameraMouse.Camera.DisplayRatio());
    this->mScreenFactor = this->mGizmoSizeClipSpace / rightLength;

    recti::Vec2 centerSSpace = worldToPos({ 0.f, 0.f }, this->mMVP);
    this->mScreenSquareCenter = centerSSpace;
  }

  void ComputeColors(uint32_t* colors, int type, OPERATION operation)
  {
    if (mbEnable) {
      uint32_t selectionColor = GetColorU32(SELECTION);

      switch (operation) {
        case TRANSLATE:
          colors[0] =
            (type == MT_MOVE_SCREEN) ? selectionColor : IM_COL32_WHITE;
          for (int i = 0; i < 3; i++) {
            colors[i + 1] = (type == (int)(MT_MOVE_X + i))
                              ? selectionColor
                              : GetColorU32(DIRECTION_X + i);
            colors[i + 4] = (type == (int)(MT_MOVE_YZ + i))
                              ? selectionColor
                              : GetColorU32(PLANE_X + i);
            colors[i + 4] =
              (type == MT_MOVE_SCREEN) ? selectionColor : colors[i + 4];
          }
          break;
        case ROTATE:
          colors[0] =
            (type == MT_ROTATE_SCREEN) ? selectionColor : IM_COL32_WHITE;
          for (int i = 0; i < 3; i++) {
            colors[i + 1] = (type == (int)(MT_ROTATE_X + i))
                              ? selectionColor
                              : GetColorU32(DIRECTION_X + i);
          }
          break;
        case SCALEU:
        case SCALE:
          colors[0] = (type == MT_SCALE_XYZ) ? selectionColor : IM_COL32_WHITE;
          for (int i = 0; i < 3; i++) {
            colors[i + 1] = (type == (int)(MT_SCALE_X + i))
                              ? selectionColor
                              : GetColorU32(DIRECTION_X + i);
          }
          break;
        // note: this internal function is only called with three possible
        // values for operation
        default:
          break;
      }
    } else {
      uint32_t inactiveColor = GetColorU32(INACTIVE);
      for (int i = 0; i < 7; i++) {
        colors[i] = inactiveColor;
      }
    }
  }

  uint32_t GetColorU32(int idx) const
  {
    assert(idx < COLOR::COUNT);
    return ColorConvertFloat4ToU32(mStyle.Colors[idx]);
  }

  void DrawHatchedAxis(const recti::Vec4& axis)
  {
    if (mStyle.HatchedAxisLineThickness <= 0.0f) {
      return;
    }

    for (int j = 1; j < 10; j++) {
      recti::Vec2 baseSSpace2 =
        worldToPos(axis * 0.05f * (float)(j * 2) * mScreenFactor, mMVP);
      recti::Vec2 worldDirSSpace2 =
        worldToPos(axis * 0.05f * (float)(j * 2 + 1) * mScreenFactor, mMVP);
      mDrawList->AddLine(baseSSpace2,
                         worldDirSSpace2,
                         GetColorU32(HATCHED_AXIS_LINES),
                         mStyle.HatchedAxisLineThickness);
    }
  }

  void DrawScaleGizmo(OPERATION op, MOVETYPE type)
  {
    auto drawList = mDrawList;

    if (!Intersects(op, SCALE)) {
      return;
    }

    // colors
    uint32_t colors[7];
    ComputeColors(colors, type, SCALE);

    // draw
    recti::Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mState.Using()) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
        continue;
      }
      const bool usingAxis = (mState.mbUsing && type == MT_SCALE_X + i);
      if (!mState.mbUsing || usingAxis) {
        recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(i,
                                       mMVPLocal,
                                       mState,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          recti::Vec2 baseSSpace =
            worldToPos(dirAxis * 0.1f * mScreenFactor, mMVP);
          recti::Vec2 worldDirSSpaceNoScale =
            worldToPos(dirAxis * markerScale * mScreenFactor, mMVP);
          recti::Vec2 worldDirSSpace = worldToPos(
            (dirAxis * markerScale * scaleDisplay[i]) * mScreenFactor, mMVP);

          if (mState.Using()) {
            uint32_t scaleLineColor = GetColorU32(SCALE_LINE);
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

          if (mAxisFactor[i] < 0.f) {
            DrawHatchedAxis(dirAxis * scaleDisplay[i]);
          }
        }
      }
    }

    // draw screen cirle
    drawList->AddCircleFilled(
      mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

    if (mState.Using() && IsScaleType(type)) {
      // recti::Vec2 sourcePosOnScreen = worldToPos(mMatrixOrigin,
      // mViewProjection);
      recti::Vec2 destinationPosOnScreen =
        worldToPos(mModel.position(), mCameraMouse.mViewProjection);
      /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
      destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
      *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f,
      translationLineColor); drawList->AddCircle(destinationPosOnScreen, 6.f,
      translationLineColor); drawList->AddLine(recti::Vec2(sourcePosOnScreen.x +
      dif.x, sourcePosOnScreen.y
      + dif.y), recti::Vec2(destinationPosOnScreen.x - dif.x,
      destinationPosOnScreen.y - dif.y), translationLineColor, 2.f);
      */
      char tmps[512];
      // vec_t deltaInfo = mModel.position() -
      // mMatrixOrigin;
      int componentInfoIndex = (type - MT_SCALE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               scaleInfoMask[type - MT_SCALE_X],
               scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        GetColorU32(TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        GetColorU32(TEXT),
                        tmps);
    }
  }

  void DrawRotationGizmo(OPERATION op, MOVETYPE type)
  {
    if (!Intersects(op, ROTATE)) {
      return;
    }
    auto drawList = mDrawList;

    // colors
    uint32_t colors[7];
    ComputeColors(colors, type, ROTATE);

    recti::Vec4 cameraToModelNormalized;
    if (mIsOrthographic) {
      cameraToModelNormalized = -mCameraMouse.mViewInverse.dir();
    } else {
      cameraToModelNormalized =
        Normalized(mModel.position() - mCameraMouse.CameraEye());
    }

    cameraToModelNormalized.TransformVector(mModelInverse);

    mRadiusSquareCenter = screenRotateSize * mCameraMouse.Camera.Height();

    bool hasRSC = Intersects(op, ROTATE_SCREEN);
    for (int axis = 0; axis < 3; axis++) {
      if (!Intersects(op, static_cast<OPERATION>(ROTATE_Z >> axis))) {
        continue;
      }
      const bool usingAxis = (mState.mbUsing && type == MT_ROTATE_Z - axis);
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
                          mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] = worldToPos(pos, mMVP);
      }
      if (!mState.mbUsing || usingAxis) {
        drawList->AddPolyline((const recti::VEC2*)circlePos,
                              circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                              colors[3 - axis],
                              false,
                              mStyle.RotationLineThickness);
      }

      float radiusAxis =
        sqrtf((worldToPos(mModel.position(), mCameraMouse.mViewProjection) -
               circlePos[0])
                .SqrLength());
      if (radiusAxis > mRadiusSquareCenter) {
        mRadiusSquareCenter = radiusAxis;
      }
    }
    if (hasRSC && (!mState.mbUsing || type == MT_ROTATE_SCREEN)) {
      drawList->AddCircle(
        worldToPos(mModel.position(), mCameraMouse.mViewProjection),
        mRadiusSquareCenter,
        colors[0],
        64,
        mStyle.RotationOuterLineThickness);
    }

    if (mState.Using() && IsRotateType(type)) {
      recti::Vec2 circlePos[HALF_CIRCLE_SEGMENT_COUNT + 1];

      circlePos[0] =
        worldToPos(mModel.position(), mCameraMouse.mViewProjection);
      for (unsigned int i = 1; i < HALF_CIRCLE_SEGMENT_COUNT; i++) {
        float ng = mRotationAngle *
                   ((float)(i - 1) / (float)(HALF_CIRCLE_SEGMENT_COUNT - 1));
        recti::Mat4 rotateVectorMatrix;
        rotateVectorMatrix.RotationAxis(mTranslationPlan, ng);
        recti::Vec4 pos;
        pos.TransformPoint(mRotationVectorSource, rotateVectorMatrix);
        pos *= mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] =
          worldToPos(pos + mModel.position(), mCameraMouse.mViewProjection);
      }
      drawList->AddConvexPolyFilled((const recti::VEC2*)circlePos,
                                    HALF_CIRCLE_SEGMENT_COUNT,
                                    GetColorU32(ROTATION_USING_FILL));
      drawList->AddPolyline((const recti::VEC2*)circlePos,
                            HALF_CIRCLE_SEGMENT_COUNT,
                            GetColorU32(ROTATION_USING_BORDER),
                            true,
                            mStyle.RotationLineThickness);

      recti::Vec2 destinationPosOnScreen = circlePos[1];
      char tmps[512];
      snprintf(tmps,
               sizeof(tmps),
               rotationInfoMask[type - MT_ROTATE_X],
               (mRotationAngle / std::numbers::pi) * 180.f,
               mRotationAngle);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        GetColorU32(TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        GetColorU32(TEXT),
                        tmps);
    }
  }

  void DrawScaleUniveralGizmo(OPERATION op, MOVETYPE type)
  {
    auto drawList = mDrawList;

    if (!Intersects(op, SCALEU)) {
      return;
    }

    // colors
    uint32_t colors[7];
    ComputeColors(colors, type, SCALEU);

    // draw
    recti::Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mState.Using()) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_XU << i))) {
        continue;
      }
      const bool usingAxis = (mState.mbUsing && type == MT_SCALE_X + i);
      if (!mState.mbUsing || usingAxis) {
        recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(i,
                                       mMVPLocal,
                                       mState,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          // recti::Vec2 baseSSpace = worldToPos(dirAxis * 0.1f *
          // mScreenFactor, mMVPLocal); recti::Vec2
          // worldDirSSpaceNoScale = worldToPos(dirAxis * markerScale *
          // mScreenFactor, mMVP);
          recti::Vec2 worldDirSSpace = worldToPos(
            (dirAxis * markerScale * scaleDisplay[i]) * mScreenFactor,
            mMVPLocal);

#if 0
               if (mbUsing && (mActualID == -1 || mActualID == mEditingID))
               {
                  drawList->AddLine(baseSSpace, worldDirSSpaceNoScale, IM_COL32(0x40, 0x40, 0x40, 0xFF), 3.f);
                  drawList->AddCircleFilled(worldDirSSpaceNoScale, 6.f, IM_COL32(0x40, 0x40, 0x40, 0xFF));
               }
               /*
               if (!hasTranslateOnAxis || mbUsing)
               {
                  drawList->AddLine(baseSSpace, worldDirSSpace, colors[i + 1], 3.f);
               }
               */
#endif
          drawList->AddCircleFilled(worldDirSSpace, 12.f, colors[i + 1]);
        }
      }
    }

    // draw screen cirle
    drawList->AddCircle(
      mScreenSquareCenter, 20.f, colors[0], 32, mStyle.CenterCircleSize);

    if (mState.Using() && IsScaleType(type)) {
      // recti::Vec2 sourcePosOnScreen = worldToPos(mMatrixOrigin,
      // mViewProjection);
      recti::Vec2 destinationPosOnScreen =
        worldToPos(mModel.position(), mCameraMouse.mViewProjection);
      /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
      destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
      *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
      drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
      drawList->AddLine(recti::Vec2(sourcePosOnScreen.x + dif.x,
      sourcePosOnScreen.y
      + dif.y), recti::Vec2(destinationPosOnScreen.x - dif.x,
      destinationPosOnScreen.y - dif.y), translationLineColor, 2.f);
      */
      char tmps[512];
      // vec_t deltaInfo = mModel.position() -
      // mMatrixOrigin;
      int componentInfoIndex = (type - MT_SCALE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               scaleInfoMask[type - MT_SCALE_X],
               scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        GetColorU32(TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        GetColorU32(TEXT),
                        tmps);
    }
  }

  void DrawTranslationGizmo(OPERATION op, MOVETYPE type)
  {
    auto drawList = mDrawList;
    if (!drawList) {
      return;
    }

    if (!Intersects(op, TRANSLATE)) {
      return;
    }

    // colors
    uint32_t colors[7];
    ComputeColors(colors, type, TRANSLATE);

    const recti::Vec2 origin =
      worldToPos(mModel.position(), mCameraMouse.mViewProjection);

    // draw
    bool belowAxisLimit = false;
    bool belowPlaneLimit = false;
    for (int i = 0; i < 3; ++i) {
      recti::Vec4 dirPlaneX, dirPlaneY, dirAxis;
      ComputeTripodAxisAndVisibility(i,
                                     mMVP,
                                     mState,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit);

      if (!mState.mbUsing || (mState.mbUsing && type == MT_MOVE_X + i)) {
        // draw axis
        if (belowAxisLimit &&
            Intersects(op, static_cast<OPERATION>(TRANSLATE_X << i))) {
          recti::Vec2 baseSSpace =
            worldToPos(dirAxis * 0.1f * mScreenFactor, mMVP);
          recti::Vec2 worldDirSSpace =
            worldToPos(dirAxis * mScreenFactor, mMVP);

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

          if (mAxisFactor[i] < 0.f) {
            DrawHatchedAxis(dirAxis);
          }
        }
      }
      // draw plane
      if (!mState.mbUsing || (mState.mbUsing && type == MT_MOVE_YZ + i)) {
        if (belowPlaneLimit && Contains(op, TRANSLATE_PLANS[i])) {
          recti::Vec2 screenQuadPts[4];
          for (int j = 0; j < 4; ++j) {
            recti::Vec4 cornerWorldPos =
              (dirPlaneX * quadUV[j * 2] + dirPlaneY * quadUV[j * 2 + 1]) *
              mScreenFactor;
            screenQuadPts[j] = worldToPos(cornerWorldPos, mMVP);
          }
          drawList->AddPolyline((const recti::VEC2*)screenQuadPts,
                                4,
                                GetColorU32(DIRECTION_X + i),
                                true,
                                1.0f);
          drawList->AddConvexPolyFilled(
            (const recti::VEC2*)screenQuadPts, 4, colors[i + 4]);
        }
      }
    }

    drawList->AddCircleFilled(
      mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

    if (mState.Using() && IsTranslateType(type)) {
      uint32_t translationLineColor = GetColorU32(TRANSLATION_LINE);

      recti::Vec2 sourcePosOnScreen =
        worldToPos(mMatrixOrigin, mCameraMouse.mViewProjection);
      recti::Vec2 destinationPosOnScreen =
        worldToPos(mModel.position(), mCameraMouse.mViewProjection);
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
      recti::Vec4 deltaInfo = mModel.position() - mMatrixOrigin;
      int componentInfoIndex = (type - MT_MOVE_X) * 3;
      snprintf(tmps,
               sizeof(tmps),
               translationInfoMask[type - MT_MOVE_X],
               deltaInfo[translationInfoIndex[componentInfoIndex]],
               deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
               deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 15,
                                    destinationPosOnScreen.Y + 15),
                        GetColorU32(TEXT_SHADOW),
                        tmps);
      drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                    destinationPosOnScreen.Y + 14),
                        GetColorU32(TEXT),
                        tmps);
    }
  }

  void HandleAndDrawLocalBounds(const float* bounds,
                                recti::Mat4* matrix,
                                const float* snapValues,
                                OPERATION operation,
                                const recti::Mouse& mouse)
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
    if (!mbUsingBounds) {
      numAxes = 0;
      float bestDot = 0.f;
      for (int i = 0; i < 3; i++) {
        recti::Vec4 dirPlaneNormalWorld;
        dirPlaneNormalWorld.TransformVector(directionUnary[i], mModelSource);
        dirPlaneNormalWorld.Normalize();

        float dt = fabsf(
          Dot(Normalized(mCameraMouse.CameraEye() - mModelSource.position()),
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
      unsigned int anchorAlpha =
        mbEnable ? IM_COL32_BLACK : IM_COL32(0, 0, 0, 0x80);

      recti::Mat4 boundsMVP = mModelSource * mCameraMouse.mViewProjection;
      for (int i = 0; i < 4; i++) {
        recti::Vec2 worldBound1 = worldToPos(aabb[i], boundsMVP);
        recti::Vec2 worldBound2 = worldToPos(aabb[(i + 1) % 4], boundsMVP);
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
                            IM_COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha,
                            2.f);
        }
        recti::Vec4 midPoint = (aabb[i] + aabb[(i + 1) % 4]) * 0.5f;
        recti::Vec2 midBound = worldToPos(midPoint, boundsMVP);
        static const float AnchorBigRadius = 8.f;
        static const float AnchorSmallRadius = 6.f;
        bool overBigAnchor = (worldBound1 - mouse.Position).SqrLength() <=
                             (AnchorBigRadius * AnchorBigRadius);
        bool overSmallAnchor = (midBound - mouse.Position).SqrLength() <=
                               (AnchorBigRadius * AnchorBigRadius);

        int type = MT_NONE;

        if (Intersects(operation, TRANSLATE)) {
          type = GetMoveType(
            operation, mCameraMouse.ScreenMousePos(), mouse.Position);
        }
        if (Intersects(operation, ROTATE) && type == MT_NONE) {
          type = GetRotateType(operation, mouse.Position);
        }
        if (Intersects(operation, SCALE) && type == MT_NONE) {
          type = GetScaleType(operation, mouse.Position);
        }

        if (type != MT_NONE) {
          overBigAnchor = false;
          overSmallAnchor = false;
        }

        uint32_t selectionColor = GetColorU32(SELECTION);

        unsigned int bigAnchorColor =
          overBigAnchor ? selectionColor
                        : (IM_COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha);
        unsigned int smallAnchorColor =
          overSmallAnchor ? selectionColor
                          : (IM_COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha);

        drawList->AddCircleFilled(worldBound1, AnchorBigRadius, IM_COL32_BLACK);
        drawList->AddCircleFilled(
          worldBound1, AnchorBigRadius - 1.2f, bigAnchorColor);

        drawList->AddCircleFilled(midBound, AnchorSmallRadius, IM_COL32_BLACK);
        drawList->AddCircleFilled(
          midBound, AnchorSmallRadius - 1.2f, smallAnchorColor);
        int oppositeIndex = (i + 2) % 4;
        // big anchor on corners
        if (!mbUsingBounds && mbEnable && overBigAnchor && mouse.LeftDown) {
          mBoundsPivot.TransformPoint(aabb[(i + 2) % 4], mModelSource);
          mBoundsAnchor.TransformPoint(aabb[i], mModelSource);
          mBoundsPlan = BuildPlan(mBoundsAnchor, bestAxisWorldDirection);
          mBoundsBestAxis = bestAxis;
          mBoundsAxis[0] = secondAxis;
          mBoundsAxis[1] = thirdAxis;

          mBoundsLocalPivot.Set(0.f);
          mBoundsLocalPivot[secondAxis] = aabb[oppositeIndex][secondAxis];
          mBoundsLocalPivot[thirdAxis] = aabb[oppositeIndex][thirdAxis];

          mbUsingBounds = true;
          mState.mEditingID = mState.mActualID;
          mBoundsMatrix = mModelSource;
        }
        // small anchor on middle of segment
        if (!mbUsingBounds && mbEnable && overSmallAnchor && mouse.LeftDown) {
          recti::Vec4 midPointOpposite =
            (aabb[(i + 2) % 4] + aabb[(i + 3) % 4]) * 0.5f;
          mBoundsPivot.TransformPoint(midPointOpposite, mModelSource);
          mBoundsAnchor.TransformPoint(midPoint, mModelSource);
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

          mbUsingBounds = true;
          mState.mEditingID = mState.mActualID;
          mBoundsMatrix = mModelSource;
        }
      }

      if (mState.Using()) {
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
          worldToPos(mModel.position(), mCameraMouse.mViewProjection);
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
                          GetColorU32(TEXT_SHADOW),
                          tmps);
        drawList->AddText(recti::Vec2(destinationPosOnScreen.X + 14,
                                      destinationPosOnScreen.Y + 14),
                          GetColorU32(TEXT),
                          tmps);
      }

      if (!mouse.LeftDown) {
        mbUsingBounds = false;
        mState.mEditingID = -1;
      }
      if (mbUsingBounds) {
        break;
      }
    }
  }

  bool HandleTranslation(float* matrix,
                         float* deltaMatrix,
                         OPERATION op,
                         MOVETYPE& type,
                         const float* snap,
                         const recti::Mouse& mouse)
  {
    if (!Intersects(op, TRANSLATE) || type != MT_NONE) {
      return false;
    }
    const bool applyRotationLocaly = mMode == LOCAL || type == MT_MOVE_SCREEN;
    bool modified = false;

    // move
    if (mState.Using() && IsTranslateType(mCurrentOperation)) {
      const recti::Vec4 newPos =
        mCameraMouse.Ray.IntersectPlane(mTranslationPlan);

      // compute delta
      const recti::Vec4 newOrigin = newPos - mRelativeOrigin * mScreenFactor;
      recti::Vec4 delta = newOrigin - mModel.position();

      // 1 axis constraint
      if (mCurrentOperation >= MT_MOVE_X && mCurrentOperation <= MT_MOVE_Z) {
        const int axisIndex = mCurrentOperation - MT_MOVE_X;
        const recti::Vec4& axisValue = mModel.component(axisIndex);
        const float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;
      }

      // snap
      if (snap) {
        recti::Vec4 cumulativeDelta = mModel.position() + delta - mMatrixOrigin;
        if (applyRotationLocaly) {
          recti::Mat4 modelSourceNormalized = mModelSource;
          modelSourceNormalized.OrthoNormalize();
          recti::Mat4 modelSourceNormalizedInverse;
          modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
          cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
          ComputeSnap(cumulativeDelta, snap);
          cumulativeDelta.TransformVector(modelSourceNormalized);
        } else {
          ComputeSnap(cumulativeDelta, snap);
        }
        delta = mMatrixOrigin + cumulativeDelta - mModel.position();
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

      const recti::Mat4 res = mModelSource * deltaMatrixTranslation;
      *(recti::Mat4*)matrix = res;

      if (!mouse.LeftDown) {
        mState.mbUsing = false;
      }

      type = mCurrentOperation;
    } else {
      // find new possible way to move
      type = GetMoveType(op, mCameraMouse.ScreenMousePos(), mouse.Position);
      if (type != MT_NONE) {
      }
      if (mouse.LeftDown && type != MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mState.mActualID;
        mCurrentOperation = type;
        recti::Vec4 movePlanNormal[] = {
          mModel.right(),           mModel.up(), mModel.dir(),
          mModel.right(),           mModel.up(), mModel.dir(),
          -mCameraMouse.CameraDir()
        };

        recti::Vec4 cameraToModelNormalized =
          Normalized(mModel.position() - mCameraMouse.CameraEye());
        for (unsigned int i = 0; i < 3; i++) {
          recti::Vec4 orthoVector =
            Cross(movePlanNormal[i], cameraToModelNormalized);
          movePlanNormal[i].Cross(orthoVector);
          movePlanNormal[i].Normalize();
        }
        // pickup plan
        mTranslationPlan =
          BuildPlan(mModel.position(), movePlanNormal[type - MT_MOVE_X]);
        mTranslationPlanOrigin =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
        mMatrixOrigin = mModel.position();

        mRelativeOrigin =
          (mTranslationPlanOrigin - mModel.position()) * (1.f / mScreenFactor);
      }
    }
    return modified;
  }

  bool HandleScale(float* matrix,
                   float* deltaMatrix,
                   OPERATION op,
                   MOVETYPE& type,
                   const float* snap,
                   const recti::Mouse& mouse)
  {
    if ((!Intersects(op, SCALE) && !Intersects(op, SCALEU)) ||
        type != MT_NONE) {
      return false;
    }
    bool modified = false;

    if (!mState.mbUsing) {
      // find new possible way to scale
      type = GetScaleType(op, mouse.Position);
      if (type != MT_NONE) {
      }
      if (mouse.LeftDown && type != MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mState.mActualID;
        mCurrentOperation = type;
        const recti::Vec4 movePlanNormal[] = {
          mModel.up(), mModel.dir(),   mModel.right(),           mModel.dir(),
          mModel.up(), mModel.right(), -mCameraMouse.CameraDir()
        };
        // pickup plan

        mTranslationPlan =
          BuildPlan(mModel.position(), movePlanNormal[type - MT_SCALE_X]);
        mTranslationPlanOrigin =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
        mMatrixOrigin = mModel.position();
        mScale.Set(1.f, 1.f, 1.f);
        mRelativeOrigin =
          (mTranslationPlanOrigin - mModel.position()) * (1.f / mScreenFactor);
        mScaleValueOrigin = { mModelSource.right().Length(),
                              mModelSource.up().Length(),
                              mModelSource.dir().Length() };
        mSaveMousePosx = mouse.Position.X;
      }
    }
    // scale
    if (mState.Using() && IsScaleType(mCurrentOperation)) {
      recti::Vec4 newPos = mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
      recti::Vec4 newOrigin = newPos - mRelativeOrigin * mScreenFactor;
      recti::Vec4 delta = newOrigin - mModelLocal.position();

      // 1 axis constraint
      if (mCurrentOperation >= MT_SCALE_X && mCurrentOperation <= MT_SCALE_Z) {
        int axisIndex = mCurrentOperation - MT_SCALE_X;
        const recti::Vec4& axisValue = mModelLocal.component(axisIndex);
        float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;

        recti::Vec4 baseVector =
          mTranslationPlanOrigin - mModelLocal.position();
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

      recti::Mat4 res = deltaMatrixScale * mModelLocal;
      *(recti::Mat4*)matrix = res;

      if (deltaMatrix) {
        recti::Vec4 deltaScale = mScale * mScaleValueOrigin;

        recti::Vec4 originalScaleDivider;
        originalScaleDivider.x = 1 / mModelScaleOrigin.x;
        originalScaleDivider.y = 1 / mModelScaleOrigin.y;
        originalScaleDivider.z = 1 / mModelScaleOrigin.z;

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

  bool HandleRotation(float* matrix,
                      float* deltaMatrix,
                      OPERATION op,
                      MOVETYPE& type,
                      const float* snap,
                      const recti::Mouse& mouse)
  {
    if (!Intersects(op, ROTATE) || type != MT_NONE) {
      return false;
    }
    bool applyRotationLocaly = mMode == LOCAL;
    bool modified = false;

    if (!mState.mbUsing) {
      type = GetRotateType(op, mouse.Position);

      if (type != MT_NONE) {
      }

      if (type == MT_ROTATE_SCREEN) {
        applyRotationLocaly = true;
      }

      if (mouse.LeftDown && type != MT_NONE) {
        mState.mbUsing = true;
        mState.mEditingID = mState.mActualID;
        mCurrentOperation = type;
        const recti::Vec4 rotatePlanNormal[] = {
          mModel.right(), mModel.up(), mModel.dir(), -mCameraMouse.CameraDir()
        };
        // pickup plan
        if (applyRotationLocaly) {
          mTranslationPlan =
            BuildPlan(mModel.position(), rotatePlanNormal[type - MT_ROTATE_X]);
        } else {
          mTranslationPlan = BuildPlan(mModelSource.position(),
                                       directionUnary[type - MT_ROTATE_X]);
        }

        recti::Vec4 localPos =
          mCameraMouse.Ray.IntersectPlane(mTranslationPlan) - mModel.position();
        mRotationVectorSource = Normalized(localPos);
        mRotationAngleOrigin = ComputeAngleOnPlan();
      }
    }

    // rotation
    if (mState.Using() && IsRotateType(mCurrentOperation)) {
      mRotationAngle = ComputeAngleOnPlan();
      if (snap) {
        float snapInRadian = snap[0] * DEG2RAD;
        recti::ComputeSnap(&mRotationAngle, snapInRadian);
      }
      recti::Vec4 rotationAxisLocalSpace;

      rotationAxisLocalSpace.TransformVector(
        { mTranslationPlan.x, mTranslationPlan.y, mTranslationPlan.z, 0.f },
        mModelInverse);
      rotationAxisLocalSpace.Normalize();

      recti::Mat4 deltaRotation;
      deltaRotation.RotationAxis(rotationAxisLocalSpace,
                                 mRotationAngle - mRotationAngleOrigin);
      if (mRotationAngle != mRotationAngleOrigin) {
        modified = true;
      }
      mRotationAngleOrigin = mRotationAngle;

      recti::Mat4 scaleOrigin;
      scaleOrigin.Scale(mModelScaleOrigin);

      if (applyRotationLocaly) {
        *(recti::Mat4*)matrix = scaleOrigin * deltaRotation * mModelLocal;
      } else {
        recti::Mat4 res = mModelSource;
        res.position().Set(0.f);

        *(recti::Mat4*)matrix = res * deltaRotation;
        ((recti::Mat4*)matrix)->position() = mModelSource.position();
      }

      if (deltaMatrix) {
        *(recti::Mat4*)deltaMatrix = mModelInverse * deltaRotation * mModel;
      }

      if (!mouse.LeftDown) {
        mState.mbUsing = false;
        mState.mEditingID = -1;
      }
      type = mCurrentOperation;
    }
    return modified;
  }

  float ComputeAngleOnPlan()
  {
    recti::Vec4 localPos = Normalized(
      mCameraMouse.Ray.IntersectPlane(mTranslationPlan) - mModel.position());

    recti::Vec4 perpendicularVector;
    perpendicularVector.Cross(mRotationVectorSource, mTranslationPlan);
    perpendicularVector.Normalize();
    float acosAngle =
      recti::Clamp(Dot(localPos, mRotationVectorSource), -1.f, 1.f);
    float angle = acosf(acosAngle);
    angle *= (Dot(localPos, perpendicularVector) < 0.f) ? 1.f : -1.f;
    return angle;
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

  bool Manipulate(void* id,
                  OPERATION operation,
                  MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
  {
    mState.mActualID = (int64_t)id;
    // Scope scope;

    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    ComputeContext(matrix, (operation & SCALE) ? LOCAL : mode);

    // set delta to identity
    if (deltaMatrix) {
      ((recti::Mat4*)deltaMatrix)->SetToIdentity();
    }

    // behind camera
    recti::Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    // --
    MOVETYPE type = MT_NONE;
    bool manipulated = false;
    if (mbEnable) {
      if (!mbUsingBounds) {
        manipulated =
          HandleTranslation(
            matrix, deltaMatrix, operation, type, snap, mCameraMouse.Mouse) ||
          HandleScale(
            matrix, deltaMatrix, operation, type, snap, mCameraMouse.Mouse) ||
          HandleRotation(
            matrix, deltaMatrix, operation, type, snap, mCameraMouse.Mouse);
      }
    }

    if (localBounds && !mState.mbUsing) {
      HandleAndDrawLocalBounds(localBounds,
                               (recti::Mat4*)matrix,
                               boundsSnap,
                               operation,
                               mCameraMouse.Mouse);
    }

    mOperation = operation;
    if (!mbUsingBounds) {
      DrawRotationGizmo(operation, type);
      DrawTranslationGizmo(operation, type);
      DrawScaleGizmo(operation, type);
      DrawScaleUniveralGizmo(operation, type);
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
                    OPERATION operation,
                    MODE mode,
                    float* matrix,
                    float* deltaMatrix,
                    const float* snap,
                    const float* localBounds,
                    const float* boundsSnap)
{
  return m_impl->Manipulate(
    id, operation, mode, matrix, deltaMatrix, snap, localBounds, boundsSnap);
}

const recti::DrawList&
Context::End()
{
  return m_impl->GetDrawList();
}

} // namespace
