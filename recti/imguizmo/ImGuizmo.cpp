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

#include "matrix_t.h"
#include "vec_t.h"
#include <cfloat>
#include <numbers>

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

static const vec_t directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                         { 0.f, 1.f, 0.f, 0 },
                                         { 0.f, 0.f, 1.f, 0 } };

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

static const float quadMin = 0.5f;
static const float quadMax = 0.8f;
static const float quadUV[8] = { quadMin, quadMin, quadMin, quadMax,
                                 quadMax, quadMax, quadMax, quadMin };
static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
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

  RGBA Colors[COLOR::COUNT] = {
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

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImGuizmo.h"
#include "imgui.h"
#include "imgui_internal.h"

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

struct Context
{
private:
  Context()
    : mbUsing(false)
    , mbEnable(true)
    , mbUsingBounds(false)
  {
  }

public:
  static Context& Instance()
  {
    static Context s_instance;
    return s_instance;
  }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  float height() const { return mHeight; }

  float GetSegmentLengthClipSpace(const vec_t& start,
                                  const vec_t& end,
                                  const bool localCoordinates = false) const
  {
    vec_t startOfSegment = start;
    const matrix_t& mvp = localCoordinates ? mMVPLocal : mMVP;
    startOfSegment.TransformPoint(mvp);
    if (fabsf(startOfSegment.w) >
        FLT_EPSILON) // check for axis aligned with camera direction
    {
      startOfSegment *= 1.f / startOfSegment.w;
    }

    vec_t endOfSegment = end;
    endOfSegment.TransformPoint(mvp);
    if (fabsf(endOfSegment.w) >
        FLT_EPSILON) // check for axis aligned with camera direction
    {
      endOfSegment *= 1.f / endOfSegment.w;
    }

    vec_t clipSpaceAxis = endOfSegment - startOfSegment;
    clipSpaceAxis.y /= mDisplayRatio;
    float segmentLengthInClipSpace = sqrtf(clipSpaceAxis.x * clipSpaceAxis.x +
                                           clipSpaceAxis.y * clipSpaceAxis.y);
    return segmentLengthInClipSpace;
  }

  float GetParallelogram(const vec_t& ptO,
                         const vec_t& ptA,
                         const vec_t& ptB) const
  {
    vec_t pts[] = { ptO, ptA, ptB };
    for (unsigned int i = 0; i < 3; i++) {
      pts[i].TransformPoint(mMVP);
      if (fabsf(pts[i].w) >
          FLT_EPSILON) // check for axis aligned with camera direction
      {
        pts[i] *= 1.f / pts[i].w;
      }
    }
    vec_t segA = pts[1] - pts[0];
    vec_t segB = pts[2] - pts[0];
    segA.y /= mDisplayRatio;
    segB.y /= mDisplayRatio;
    vec_t segAOrtho = makeVect(-segA.y, segA.x);
    segAOrtho.Normalize();
    float dt = segAOrtho.Dot3(segB);
    float surface = sqrtf(segA.x * segA.x + segA.y * segA.y) * fabsf(dt);
    return surface;
  }

  void ComputeTripodAxisAndVisibility(const int axisIndex,
                                      vec_t& dirAxis,
                                      vec_t& dirPlaneX,
                                      vec_t& dirPlaneY,
                                      bool& belowAxisLimit,
                                      bool& belowPlaneLimit,
                                      const bool localCoordinates = false) const
  {
    dirAxis = directionUnary[axisIndex];
    dirPlaneX = directionUnary[(axisIndex + 1) % 3];
    dirPlaneY = directionUnary[(axisIndex + 2) % 3];

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID)) {
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
        makeVect(0.f, 0.f, 0.f), dirAxis, localCoordinates);
      float lenDirMinus = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), -dirAxis, localCoordinates);

      float lenDirPlaneX = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), dirPlaneX, localCoordinates);
      float lenDirMinusPlaneX = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), -dirPlaneX, localCoordinates);

      float lenDirPlaneY = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), dirPlaneY, localCoordinates);
      float lenDirMinusPlaneY = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), -dirPlaneY, localCoordinates);

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
      float axisLengthInClipSpace = GetSegmentLengthClipSpace(
        makeVect(0.f, 0.f, 0.f), dirAxis * mScreenFactor, localCoordinates);

      float paraSurf = GetParallelogram(makeVect(0.f, 0.f, 0.f),
                                        dirPlaneX * mScreenFactor,
                                        dirPlaneY * mScreenFactor);
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
                       vec_t* gizmoHitProportion,
                       const vec_t& screenCoord) const
  {
    if (!Intersects(op, TRANSLATE) || mbUsing || !mbMouseOver) {
      return MT_NONE;
    }
    ImGuiIO& io = ImGui::GetIO();
    MOVETYPE type = MT_NONE;

    // screen
    if (io.MousePos.x >= mScreenSquareMin.x &&
        io.MousePos.x <= mScreenSquareMax.x &&
        io.MousePos.y >= mScreenSquareMin.y &&
        io.MousePos.y <= mScreenSquareMax.y && Contains(op, TRANSLATE)) {
      type = MT_MOVE_SCREEN;
    }

    // compute
    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      vec_t dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(
        i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit);
      dirAxis.TransformVector(mModel);
      dirPlaneX.TransformVector(mModel);
      dirPlaneY.TransformVector(mModel);

      const float len = IntersectRayPlane(
        mRayOrigin, mRayVector, BuildPlan(mModel.v.position, dirAxis));
      vec_t posOnPlan = mRayOrigin + mRayVector * len;

      const ImVec2 axisStartOnScreen =
        worldToPos(mModel.v.position + dirAxis * mScreenFactor * 0.1f,
                   mViewProjection) -
        leftTop();
      const ImVec2 axisEndOnScreen =
        worldToPos(mModel.v.position + dirAxis * mScreenFactor,
                   mViewProjection) -
        leftTop();

      vec_t closestPointOnAxis = PointOnSegment(
        screenCoord, makeVect(axisStartOnScreen), makeVect(axisEndOnScreen));
      if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
          Intersects(op,
                     static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
      {
        type = (MOVETYPE)(MT_MOVE_X + i);
      }

      const float dx =
        dirPlaneX.Dot3((posOnPlan - mModel.v.position) * (1.f / mScreenFactor));
      const float dy =
        dirPlaneY.Dot3((posOnPlan - mModel.v.position) * (1.f / mScreenFactor));
      if (belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
          dy >= quadUV[1] && dy <= quadUV[3] &&
          Contains(op, TRANSLATE_PLANS[i])) {
        type = (MOVETYPE)(MT_MOVE_YZ + i);
      }

      if (gizmoHitProportion) {
        *gizmoHitProportion = makeVect(dx, dy, 0.f);
      }
    }
    return type;
  }

  MOVETYPE
  GetRotateType(OPERATION op) const
  {
    if (mbUsing) {
      return MT_NONE;
    }
    ImGuiIO& io = ImGui::GetIO();
    MOVETYPE type = MT_NONE;

    vec_t deltaScreen = { io.MousePos.x - mScreenSquareCenter.x,
                          io.MousePos.y - mScreenSquareCenter.y,
                          0.f,
                          0.f };
    float dist = deltaScreen.Length();
    if (Intersects(op, ROTATE_SCREEN) && dist >= (mRadiusSquareCenter - 4.0f) &&
        dist < (mRadiusSquareCenter + 4.0f)) {
      type = MT_ROTATE_SCREEN;
    }

    const vec_t planNormals[] = { mModel.v.right, mModel.v.up, mModel.v.dir };

    vec_t modelViewPos;
    modelViewPos.TransformPoint(mModel.v.position, mViewMat);

    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      if (!Intersects(op, static_cast<OPERATION>(ROTATE_X << i))) {
        continue;
      }
      // pickup plan
      vec_t pickupPlan = BuildPlan(mModel.v.position, planNormals[i]);

      const float len = IntersectRayPlane(mRayOrigin, mRayVector, pickupPlan);
      const vec_t intersectWorldPos = mRayOrigin + mRayVector * len;
      vec_t intersectViewPos;
      intersectViewPos.TransformPoint(intersectWorldPos, mViewMat);

      if (ImAbs(modelViewPos.z) - ImAbs(intersectViewPos.z) < -FLT_EPSILON) {
        continue;
      }

      const vec_t localPos = intersectWorldPos - mModel.v.position;
      vec_t idealPosOnCircle = Normalized(localPos);
      idealPosOnCircle.TransformVector(mModelInverse);
      const ImVec2 idealPosOnCircleScreen = worldToPos(
        idealPosOnCircle * ROTATION_DISPLAY_FACTOR * mScreenFactor, mMVP);

      // mDrawList->AddCircle(idealPosOnCircleScreen, 5.f,
      // IM_COL32_WHITE);
      const ImVec2 distanceOnScreen = idealPosOnCircleScreen - io.MousePos;

      const float distance = makeVect(distanceOnScreen).Length();
      if (distance < 8.f) // pixel size
      {
        type = (MOVETYPE)(MT_ROTATE_X + i);
      }
    }

    return type;
  }

  MOVETYPE GetScaleType(OPERATION op) const
  {
    if (mbUsing) {
      return MT_NONE;
    }
    ImGuiIO& io = ImGui::GetIO();
    MOVETYPE type = MT_NONE;

    // screen
    if (io.MousePos.x >= mScreenSquareMin.x &&
        io.MousePos.x <= mScreenSquareMax.x &&
        io.MousePos.y >= mScreenSquareMin.y &&
        io.MousePos.y <= mScreenSquareMax.y && Contains(op, SCALE)) {
      type = MT_SCALE_XYZ;
    }

    // compute
    for (int i = 0; i < 3 && type == MT_NONE; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
        continue;
      }
      vec_t dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(i,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit,
                                     true);
      dirAxis.TransformVector(mModelLocal);
      dirPlaneX.TransformVector(mModelLocal);
      dirPlaneY.TransformVector(mModelLocal);

      const float len = IntersectRayPlane(
        mRayOrigin, mRayVector, BuildPlan(mModelLocal.v.position, dirAxis));
      vec_t posOnPlan = mRayOrigin + mRayVector * len;

      const float startOffset =
        Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.0f : 0.1f;
      const float endOffset =
        Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.4f : 1.0f;
      const ImVec2 posOnPlanScreen = worldToPos(posOnPlan, mViewProjection);
      const ImVec2 axisStartOnScreen = worldToPos(
        mModelLocal.v.position + dirAxis * mScreenFactor * startOffset,
        mViewProjection);
      const ImVec2 axisEndOnScreen =
        worldToPos(mModelLocal.v.position + dirAxis * mScreenFactor * endOffset,
                   mViewProjection);

      vec_t closestPointOnAxis = PointOnSegment(makeVect(posOnPlanScreen),
                                                makeVect(axisStartOnScreen),
                                                makeVect(axisEndOnScreen));

      if ((closestPointOnAxis - makeVect(posOnPlanScreen)).Length() <
          12.f) // pixel size
      {
        type = (MOVETYPE)(MT_SCALE_X + i);
      }
    }

    // universal

    vec_t deltaScreen = { io.MousePos.x - mScreenSquareCenter.x,
                          io.MousePos.y - mScreenSquareCenter.y,
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

      vec_t dirPlaneX, dirPlaneY, dirAxis;
      bool belowAxisLimit, belowPlaneLimit;
      ComputeTripodAxisAndVisibility(i,
                                     dirAxis,
                                     dirPlaneX,
                                     dirPlaneY,
                                     belowAxisLimit,
                                     belowPlaneLimit,
                                     true);

      // draw axis
      if (belowAxisLimit) {
        bool hasTranslateOnAxis =
          Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        // ImVec2 baseSSpace = worldToPos(dirAxis * 0.1f *
        // mScreenFactor, mMVPLocal); ImVec2
        // worldDirSSpaceNoScale = worldToPos(dirAxis
        // * markerScale * mScreenFactor, mMVP);
        ImVec2 worldDirSSpace =
          worldToPos((dirAxis * markerScale) * mScreenFactor, mMVPLocal);

        float distance = sqrtf(ImLengthSqr(worldDirSSpace - io.MousePos));
        if (distance < 12.f) {
          type = (MOVETYPE)(MT_SCALE_X + i);
        }
      }
    }
    return type;
  }

  // return true if mouse cursor is over any gizmo control (axis, plan or screen
  // component)
  bool IsOver() const
  {
    return (Intersects(mOperation, TRANSLATE) &&
            GetMoveType(mOperation, NULL, screenCoord()) != MT_NONE) ||
           (Intersects(mOperation, ROTATE) &&
            GetRotateType(mOperation) != MT_NONE) ||
           (Intersects(mOperation, SCALE) &&
            GetScaleType(mOperation) != MT_NONE) ||
           IsUsing();
  }

  // return true if the cursor is over the operation's gizmo
  bool IsOver(OPERATION op) const
  {
    if (IsUsing()) {
      return true;
    }
    if (Intersects(op, SCALE) && GetScaleType(op) != MT_NONE) {
      return true;
    }
    if (Intersects(op, ROTATE) && GetRotateType(op) != MT_NONE) {
      return true;
    }
    if (Intersects(op, TRANSLATE) &&
        GetMoveType(op, NULL, screenCoord()) != MT_NONE) {
      return true;
    }
    return false;
  }

  // return true if mouse IsOver or if the gizmo is in moving state
  bool IsUsing() const
  {
    return (mbUsing && (mActualID == -1 || mActualID == mEditingID)) ||
           mbUsingBounds;
  }

  bool CanActivate() const
  {
    if (ImGui::IsMouseClicked(0)) {
      if (mAllowActiveHoverItem) {
        return true;
      } else {
        if (!ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) {
          return true;
        }
      }
    }
    return false;
  }

  void SetDrawlist(ImDrawList* drawlist)
  {
    mDrawList = drawlist ? drawlist : ImGui::GetWindowDrawList();
  }

  void setRect(float x, float y, float width, float height)
  {
    SetDrawlist(nullptr);

    mX = x;
    mY = y;
    mWidth = width;
    mHeight = height;
    mXMax = mX + mWidth;
    mYMax = mY + mXMax;
    mDisplayRatio = width / height;
  }

  ImVec2 leftTop() const { return { mX, mY }; }
  ImVec2 size() const { return { mWidth, mHeight }; }
  bool IsInContextRect(const ImVec2& p) const
  {
    return IsWithin(p.x, mX, mXMax) && IsWithin(p.y, mY, mYMax);
  }

  vec_t screenCoord() const
  {
    ImGuiIO& io = ImGui::GetIO();
    return makeVect(io.MousePos - leftTop());
  }

  ImVec2 worldToPos(const vec_t& worldPos, const matrix_t& mat) const
  {
    return ::worldToPos(worldPos, mat, { mX, mY, mWidth, mHeight });
  }

  Style mStyle;

  MODE mMode;
  matrix_t mViewMat;
  matrix_t mProjectionMat;
  matrix_t mModel;
  matrix_t mModelLocal; // orthonormalized model
  matrix_t mModelInverse;
  matrix_t mModelSource;
  matrix_t mModelSourceInverse;
  matrix_t mMVP;
  matrix_t
    mMVPLocal; // MVP with full model matrix whereas mMVP's model matrix might
               // only be translation in case of World space edition
  matrix_t mViewProjection;

  vec_t mModelScaleOrigin;
  vec_t mCameraEye;
  vec_t mCameraRight;
  vec_t mCameraDir;
  vec_t mCameraUp;
  vec_t mRayOrigin;
  vec_t mRayVector;

  float mRadiusSquareCenter;
  ImVec2 mScreenSquareCenter;
  ImVec2 mScreenSquareMin;
  ImVec2 mScreenSquareMax;

  float mScreenFactor;
  vec_t mRelativeOrigin;

  bool mbUsing;
  bool mbEnable;
  bool mbMouseOver;
  bool mReversed; // reversed projection matrix

  // translation
  vec_t mTranslationPlan;
  vec_t mTranslationPlanOrigin;
  vec_t mMatrixOrigin;
  vec_t mTranslationLastDelta;

  // rotation
  vec_t mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;
  // vec_t mWorldToLocalAxis;

  // scale
  vec_t mScale;
  vec_t mScaleValueOrigin;
  vec_t mScaleLast;
  float mSaveMousePosx;

  // save axis factor when using gizmo
  mutable bool mBelowAxisLimit[3];
  mutable bool mBelowPlaneLimit[3];
  mutable float mAxisFactor[3];

  // bounds stretching
  vec_t mBoundsPivot;
  vec_t mBoundsAnchor;
  vec_t mBoundsPlan;
  vec_t mBoundsLocalPivot;
  int mBoundsBestAxis;
  int mBoundsAxis[2];
  bool mbUsingBounds;
  matrix_t mBoundsMatrix;

  //
  MOVETYPE mCurrentOperation;

private:
  float mX = 0.f;
  float mY = 0.f;
  float mWidth = 0.f;
  float mHeight = 0.f;

public:
  float mXMax = 0.f;
  float mYMax = 0.f;
  float mDisplayRatio = 1.f;

  bool mIsOrthographic = false;

  int64_t mActualID = -1;
  int64_t mEditingID = -1;
  OPERATION mOperation = OPERATION(-1);

  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

private:
  bool mAllowActiveHoverItem = true;

public:
  bool IsHoveringWindow()
  {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = ImGui::FindWindowByName(mDrawList->_OwnerName);
    if (g.HoveredWindow == window) // Mouse hovering drawlist window
      return true;
    if (g.HoveredWindow != NULL) // Any other window is hovered
      return false;
    if (ImGui::IsMouseHoveringRect(
          window->InnerRect.Min,
          window->InnerRect.Max,
          false)) // Hovering drawlist window rect, while no other window is
                  // hovered (for _NoInputs windows)
      return true;
    return false;
  }

  void ComputeContext(const float* view,
                      const float* projection,
                      float* matrix,
                      MODE mode)
  {
    this->mMode = mode;
    this->mViewMat = *(matrix_t*)view;
    this->mProjectionMat = *(matrix_t*)projection;
    this->mbMouseOver = IsHoveringWindow();

    this->mModelLocal = *(matrix_t*)matrix;
    this->mModelLocal.OrthoNormalize();

    if (mode == LOCAL) {
      this->mModel = this->mModelLocal;
    } else {
      this->mModel.Translation(((matrix_t*)matrix)->v.position);
    }
    this->mModelSource = *(matrix_t*)matrix;
    this->mModelScaleOrigin.Set(this->mModelSource.v.right.Length(),
                                this->mModelSource.v.up.Length(),
                                this->mModelSource.v.dir.Length());

    this->mModelInverse.Inverse(this->mModel);
    this->mModelSourceInverse.Inverse(this->mModelSource);
    this->mViewProjection = this->mViewMat * this->mProjectionMat;
    this->mMVP = this->mModel * this->mViewProjection;
    this->mMVPLocal = this->mModelLocal * this->mViewProjection;

    matrix_t viewInverse;
    viewInverse.Inverse(this->mViewMat);
    this->mCameraDir = viewInverse.v.dir;
    this->mCameraEye = viewInverse.v.position;
    this->mCameraRight = viewInverse.v.right;
    this->mCameraUp = viewInverse.v.up;

    // projection reverse
    vec_t nearPos, farPos;
    nearPos.Transform(makeVect(0, 0, 1.f, 1.f), this->mProjectionMat);
    farPos.Transform(makeVect(0, 0, 2.f, 1.f), this->mProjectionMat);

    this->mReversed = (nearPos.z / nearPos.w) > (farPos.z / farPos.w);

    // compute scale from the size of camera right vector projected on screen at
    // the matrix position
    vec_t pointRight = viewInverse.v.right;
    pointRight.TransformPoint(this->mViewProjection);

    this->mScreenFactor = this->mGizmoSizeClipSpace /
                          (pointRight.x / pointRight.w -
                           this->mMVP.v.position.x / this->mMVP.v.position.w);

    vec_t rightViewInverse = viewInverse.v.right;
    rightViewInverse.TransformVector(this->mModelInverse);
    float rightLength =
      GetSegmentLengthClipSpace(makeVect(0.f, 0.f), rightViewInverse);
    this->mScreenFactor = this->mGizmoSizeClipSpace / rightLength;

    ImVec2 centerSSpace = worldToPos(makeVect(0.f, 0.f), this->mMVP);
    this->mScreenSquareCenter = centerSSpace;
    this->mScreenSquareMin =
      ImVec2(centerSSpace.x - 10.f, centerSSpace.y - 10.f);
    this->mScreenSquareMax =
      ImVec2(centerSSpace.x + 10.f, centerSSpace.y + 10.f);

    ComputeCameraRay(this->mRayOrigin, this->mRayVector);
  }
  void ComputeColors(ImU32* colors, int type, OPERATION operation)
  {
    if (mbEnable) {
      ImU32 selectionColor = GetColorU32(SELECTION);

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
      ImU32 inactiveColor = GetColorU32(INACTIVE);
      for (int i = 0; i < 7; i++) {
        colors[i] = inactiveColor;
      }
    }
  }

  ImU32 GetColorU32(int idx) const
  {
    IM_ASSERT(idx < COLOR::COUNT);
    return ImGui::ColorConvertFloat4ToU32(*((ImVec4*)&mStyle.Colors[idx]));
  }

  void DrawHatchedAxis(const vec_t& axis)
  {
    if (mStyle.HatchedAxisLineThickness <= 0.0f) {
      return;
    }

    for (int j = 1; j < 10; j++) {
      ImVec2 baseSSpace2 =
        worldToPos(axis * 0.05f * (float)(j * 2) * mScreenFactor, mMVP);
      ImVec2 worldDirSSpace2 =
        worldToPos(axis * 0.05f * (float)(j * 2 + 1) * mScreenFactor, mMVP);
      mDrawList->AddLine(baseSSpace2,
                         worldDirSSpace2,
                         GetColorU32(HATCHED_AXIS_LINES),
                         mStyle.HatchedAxisLineThickness);
    }
  }

  ImDrawList* mDrawList;
  void DrawScaleGizmo(OPERATION op, MOVETYPE type)
  {
    ImDrawList* drawList = mDrawList;

    if (!Intersects(op, SCALE)) {
      return;
    }

    // colors
    ImU32 colors[7];
    ComputeColors(colors, type, SCALE);

    // draw
    vec_t scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID)) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
        continue;
      }
      const bool usingAxis = (mbUsing && type == MT_SCALE_X + i);
      if (!mbUsing || usingAxis) {
        vec_t dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(i,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit,
                                       true);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          ImVec2 baseSSpace = worldToPos(dirAxis * 0.1f * mScreenFactor, mMVP);
          ImVec2 worldDirSSpaceNoScale =
            worldToPos(dirAxis * markerScale * mScreenFactor, mMVP);
          ImVec2 worldDirSSpace = worldToPos(
            (dirAxis * markerScale * scaleDisplay[i]) * mScreenFactor, mMVP);

          if (mbUsing && (mActualID == -1 || mActualID == mEditingID)) {
            ImU32 scaleLineColor = GetColorU32(SCALE_LINE);
            drawList->AddLine(baseSSpace,
                              worldDirSSpaceNoScale,
                              scaleLineColor,
                              mStyle.ScaleLineThickness);
            drawList->AddCircleFilled(worldDirSSpaceNoScale,
                                      mStyle.ScaleLineCircleSize,
                                      scaleLineColor);
          }

          if (!hasTranslateOnAxis || mbUsing) {
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

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsScaleType(type)) {
      // ImVec2 sourcePosOnScreen = worldToPos(mMatrixOrigin,
      // mViewProjection);
      ImVec2 destinationPosOnScreen =
        worldToPos(mModel.v.position, mViewProjection);
      /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
      destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
      *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
      drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
      drawList->AddLine(ImVec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y
      + dif.y), ImVec2(destinationPosOnScreen.x - dif.x,
      destinationPosOnScreen.y - dif.y), translationLineColor, 2.f);
      */
      char tmps[512];
      // vec_t deltaInfo = mModel.v.position -
      // mMatrixOrigin;
      int componentInfoIndex = (type - MT_SCALE_X) * 3;
      ImFormatString(tmps,
                     sizeof(tmps),
                     scaleInfoMask[type - MT_SCALE_X],
                     scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
        GetColorU32(TEXT_SHADOW),
        tmps);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
        GetColorU32(TEXT),
        tmps);
    }
  }

  void DrawRotationGizmo(OPERATION op, MOVETYPE type)
  {
    if (!Intersects(op, ROTATE)) {
      return;
    }
    ImDrawList* drawList = mDrawList;

    // colors
    ImU32 colors[7];
    ComputeColors(colors, type, ROTATE);

    vec_t cameraToModelNormalized;
    if (mIsOrthographic) {
      matrix_t viewInverse;
      viewInverse.Inverse(*(matrix_t*)&mViewMat);
      cameraToModelNormalized = -viewInverse.v.dir;
    } else {
      cameraToModelNormalized = Normalized(mModel.v.position - mCameraEye);
    }

    cameraToModelNormalized.TransformVector(mModelInverse);

    mRadiusSquareCenter = screenRotateSize * height();

    bool hasRSC = Intersects(op, ROTATE_SCREEN);
    for (int axis = 0; axis < 3; axis++) {
      if (!Intersects(op, static_cast<OPERATION>(ROTATE_Z >> axis))) {
        continue;
      }
      const bool usingAxis = (mbUsing && type == MT_ROTATE_Z - axis);
      const int circleMul = (hasRSC && !usingAxis) ? 1 : 2;

      ImVec2* circlePos = (ImVec2*)alloca(
        sizeof(ImVec2) * (circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1));

      float angleStart = atan2f(cameraToModelNormalized[(4 - axis) % 3],
                                cameraToModelNormalized[(3 - axis) % 3]) +
                         std::numbers::pi * 0.5f;

      //
      for (int i = 0; i < circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1; i++) {
        float ng = angleStart + (float)circleMul * std::numbers::pi *
                                  ((float)i / (float)HALF_CIRCLE_SEGMENT_COUNT);
        vec_t axisPos = makeVect(cosf(ng), sinf(ng), 0.f);
        vec_t pos = makeVect(axisPos[axis],
                             axisPos[(axis + 1) % 3],
                             axisPos[(axis + 2) % 3]) *
                    mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] = worldToPos(pos, mMVP);
      }
      if (!mbUsing || usingAxis) {
        drawList->AddPolyline(circlePos,
                              circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                              colors[3 - axis],
                              false,
                              mStyle.RotationLineThickness);
      }

      float radiusAxis = sqrtf((ImLengthSqr(
        worldToPos(mModel.v.position, mViewProjection) - circlePos[0])));
      if (radiusAxis > mRadiusSquareCenter) {
        mRadiusSquareCenter = radiusAxis;
      }
    }
    if (hasRSC && (!mbUsing || type == MT_ROTATE_SCREEN)) {
      drawList->AddCircle(worldToPos(mModel.v.position, mViewProjection),
                          mRadiusSquareCenter,
                          colors[0],
                          64,
                          mStyle.RotationOuterLineThickness);
    }

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsRotateType(type)) {
      ImVec2 circlePos[HALF_CIRCLE_SEGMENT_COUNT + 1];

      circlePos[0] = worldToPos(mModel.v.position, mViewProjection);
      for (unsigned int i = 1; i < HALF_CIRCLE_SEGMENT_COUNT; i++) {
        float ng = mRotationAngle *
                   ((float)(i - 1) / (float)(HALF_CIRCLE_SEGMENT_COUNT - 1));
        matrix_t rotateVectorMatrix;
        rotateVectorMatrix.RotationAxis(mTranslationPlan, ng);
        vec_t pos;
        pos.TransformPoint(mRotationVectorSource, rotateVectorMatrix);
        pos *= mScreenFactor * ROTATION_DISPLAY_FACTOR;
        circlePos[i] = worldToPos(pos + mModel.v.position, mViewProjection);
      }
      drawList->AddConvexPolyFilled(
        circlePos, HALF_CIRCLE_SEGMENT_COUNT, GetColorU32(ROTATION_USING_FILL));
      drawList->AddPolyline(circlePos,
                            HALF_CIRCLE_SEGMENT_COUNT,
                            GetColorU32(ROTATION_USING_BORDER),
                            true,
                            mStyle.RotationLineThickness);

      ImVec2 destinationPosOnScreen = circlePos[1];
      char tmps[512];
      ImFormatString(tmps,
                     sizeof(tmps),
                     rotationInfoMask[type - MT_ROTATE_X],
                     (mRotationAngle / std::numbers::pi) * 180.f,
                     mRotationAngle);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
        GetColorU32(TEXT_SHADOW),
        tmps);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
        GetColorU32(TEXT),
        tmps);
    }
  }

  void DrawScaleUniveralGizmo(OPERATION op, MOVETYPE type)
  {
    ImDrawList* drawList = mDrawList;

    if (!Intersects(op, SCALEU)) {
      return;
    }

    // colors
    ImU32 colors[7];
    ComputeColors(colors, type, SCALEU);

    // draw
    vec_t scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID)) {
      scaleDisplay = mScale;
    }

    for (int i = 0; i < 3; i++) {
      if (!Intersects(op, static_cast<OPERATION>(SCALE_XU << i))) {
        continue;
      }
      const bool usingAxis = (mbUsing && type == MT_SCALE_X + i);
      if (!mbUsing || usingAxis) {
        vec_t dirPlaneX, dirPlaneY, dirAxis;
        bool belowAxisLimit, belowPlaneLimit;
        ComputeTripodAxisAndVisibility(i,
                                       dirAxis,
                                       dirPlaneX,
                                       dirPlaneY,
                                       belowAxisLimit,
                                       belowPlaneLimit,
                                       true);

        // draw axis
        if (belowAxisLimit) {
          bool hasTranslateOnAxis =
            Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
          float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
          // ImVec2 baseSSpace = worldToPos(dirAxis * 0.1f *
          // mScreenFactor, mMVPLocal); ImVec2
          // worldDirSSpaceNoScale = worldToPos(dirAxis * markerScale *
          // mScreenFactor, mMVP);
          ImVec2 worldDirSSpace = worldToPos(
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

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsScaleType(type)) {
      // ImVec2 sourcePosOnScreen = worldToPos(mMatrixOrigin,
      // mViewProjection);
      ImVec2 destinationPosOnScreen =
        worldToPos(mModel.v.position, mViewProjection);
      /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
      destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
      *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
      drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
      drawList->AddLine(ImVec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y
      + dif.y), ImVec2(destinationPosOnScreen.x - dif.x,
      destinationPosOnScreen.y - dif.y), translationLineColor, 2.f);
      */
      char tmps[512];
      // vec_t deltaInfo = mModel.v.position -
      // mMatrixOrigin;
      int componentInfoIndex = (type - MT_SCALE_X) * 3;
      ImFormatString(tmps,
                     sizeof(tmps),
                     scaleInfoMask[type - MT_SCALE_X],
                     scaleDisplay[translationInfoIndex[componentInfoIndex]]);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
        GetColorU32(TEXT_SHADOW),
        tmps);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
        GetColorU32(TEXT),
        tmps);
    }
  }

  void DrawTranslationGizmo(OPERATION op, MOVETYPE type)
  {
    ImDrawList* drawList = mDrawList;
    if (!drawList) {
      return;
    }

    if (!Intersects(op, TRANSLATE)) {
      return;
    }

    // colors
    ImU32 colors[7];
    ComputeColors(colors, type, TRANSLATE);

    const ImVec2 origin = worldToPos(mModel.v.position, mViewProjection);

    // draw
    bool belowAxisLimit = false;
    bool belowPlaneLimit = false;
    for (int i = 0; i < 3; ++i) {
      vec_t dirPlaneX, dirPlaneY, dirAxis;
      ComputeTripodAxisAndVisibility(
        i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit);

      if (!mbUsing || (mbUsing && type == MT_MOVE_X + i)) {
        // draw axis
        if (belowAxisLimit &&
            Intersects(op, static_cast<OPERATION>(TRANSLATE_X << i))) {
          ImVec2 baseSSpace = worldToPos(dirAxis * 0.1f * mScreenFactor, mMVP);
          ImVec2 worldDirSSpace = worldToPos(dirAxis * mScreenFactor, mMVP);

          drawList->AddLine(baseSSpace,
                            worldDirSSpace,
                            colors[i + 1],
                            mStyle.TranslationLineThickness);

          // Arrow head begin
          ImVec2 dir(origin - worldDirSSpace);

          float d = sqrtf(ImLengthSqr(dir));
          dir /= d; // Normalize
          dir *= mStyle.TranslationLineArrowSize;

          ImVec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
          ImVec2 a(worldDirSSpace + dir);
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
      if (!mbUsing || (mbUsing && type == MT_MOVE_YZ + i)) {
        if (belowPlaneLimit && Contains(op, TRANSLATE_PLANS[i])) {
          ImVec2 screenQuadPts[4];
          for (int j = 0; j < 4; ++j) {
            vec_t cornerWorldPos =
              (dirPlaneX * quadUV[j * 2] + dirPlaneY * quadUV[j * 2 + 1]) *
              mScreenFactor;
            screenQuadPts[j] = worldToPos(cornerWorldPos, mMVP);
          }
          drawList->AddPolyline(
            screenQuadPts, 4, GetColorU32(DIRECTION_X + i), true, 1.0f);
          drawList->AddConvexPolyFilled(screenQuadPts, 4, colors[i + 4]);
        }
      }
    }

    drawList->AddCircleFilled(
      mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsTranslateType(type)) {
      ImU32 translationLineColor = GetColorU32(TRANSLATION_LINE);

      ImVec2 sourcePosOnScreen = worldToPos(mMatrixOrigin, mViewProjection);
      ImVec2 destinationPosOnScreen =
        worldToPos(mModel.v.position, mViewProjection);
      vec_t dif = { destinationPosOnScreen.x - sourcePosOnScreen.x,
                    destinationPosOnScreen.y - sourcePosOnScreen.y,
                    0.f,
                    0.f };
      dif.Normalize();
      dif *= 5.f;
      drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
      drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
      drawList->AddLine(
        ImVec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y + dif.y),
        ImVec2(destinationPosOnScreen.x - dif.x,
               destinationPosOnScreen.y - dif.y),
        translationLineColor,
        2.f);

      char tmps[512];
      vec_t deltaInfo = mModel.v.position - mMatrixOrigin;
      int componentInfoIndex = (type - MT_MOVE_X) * 3;
      ImFormatString(tmps,
                     sizeof(tmps),
                     translationInfoMask[type - MT_MOVE_X],
                     deltaInfo[translationInfoIndex[componentInfoIndex]],
                     deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
                     deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
        GetColorU32(TEXT_SHADOW),
        tmps);
      drawList->AddText(
        ImVec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
        GetColorU32(TEXT),
        tmps);
    }
  }

  void HandleAndDrawLocalBounds(const float* bounds,
                                matrix_t* matrix,
                                const float* snapValues,
                                OPERATION operation)
  {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = mDrawList;

    // compute best projection axis
    vec_t axesWorldDirections[3];
    vec_t bestAxisWorldDirection = { 0.0f, 0.0f, 0.0f, 0.0f };
    int axes[3];
    unsigned int numAxes = 1;
    axes[0] = mBoundsBestAxis;
    int bestAxis = axes[0];
    if (!mbUsingBounds) {
      numAxes = 0;
      float bestDot = 0.f;
      for (int i = 0; i < 3; i++) {
        vec_t dirPlaneNormalWorld;
        dirPlaneNormalWorld.TransformVector(directionUnary[i], mModelSource);
        dirPlaneNormalWorld.Normalize();

        float dt = fabsf(Dot(Normalized(mCameraEye - mModelSource.v.position),
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
      vec_t tempDirection = axesWorldDirections[0];
      axesWorldDirections[0] = axesWorldDirections[bestIndex];
      axesWorldDirections[bestIndex] = tempDirection;
    }

    for (unsigned int axisIndex = 0; axisIndex < numAxes; ++axisIndex) {
      bestAxis = axes[axisIndex];
      bestAxisWorldDirection = axesWorldDirections[axisIndex];

      // corners
      vec_t aabb[4];

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

      matrix_t boundsMVP = mModelSource * mViewProjection;
      for (int i = 0; i < 4; i++) {
        ImVec2 worldBound1 = worldToPos(aabb[i], boundsMVP);
        ImVec2 worldBound2 = worldToPos(aabb[(i + 1) % 4], boundsMVP);
        if (!IsInContextRect(worldBound1) || !IsInContextRect(worldBound2)) {
          continue;
        }
        float boundDistance = sqrtf(ImLengthSqr(worldBound1 - worldBound2));
        int stepCount = (int)(boundDistance / 10.f);
        stepCount = min(stepCount, 1000);
        for (int j = 0; j < stepCount; j++) {
          float stepLength = 1.f / (float)stepCount;
          float t1 = (float)j * stepLength;
          float t2 = (float)j * stepLength + stepLength * 0.5f;
          ImVec2 worldBoundSS1 =
            ImLerp(worldBound1, worldBound2, ImVec2(t1, t1));
          ImVec2 worldBoundSS2 =
            ImLerp(worldBound1, worldBound2, ImVec2(t2, t2));
          // drawList->AddLine(worldBoundSS1, worldBoundSS2, IM_COL32(0, 0, 0,
          // 0)
          // + anchorAlpha, 3.f);
          drawList->AddLine(worldBoundSS1,
                            worldBoundSS2,
                            IM_COL32(0xAA, 0xAA, 0xAA, 0) + anchorAlpha,
                            2.f);
        }
        vec_t midPoint = (aabb[i] + aabb[(i + 1) % 4]) * 0.5f;
        ImVec2 midBound = worldToPos(midPoint, boundsMVP);
        static const float AnchorBigRadius = 8.f;
        static const float AnchorSmallRadius = 6.f;
        bool overBigAnchor = ImLengthSqr(worldBound1 - io.MousePos) <=
                             (AnchorBigRadius * AnchorBigRadius);
        bool overSmallAnchor = ImLengthSqr(midBound - io.MousePos) <=
                               (AnchorBigRadius * AnchorBigRadius);

        int type = MT_NONE;
        vec_t gizmoHitProportion;

        if (Intersects(operation, TRANSLATE)) {
          type = GetMoveType(operation, &gizmoHitProportion, screenCoord());
        }
        if (Intersects(operation, ROTATE) && type == MT_NONE) {
          type = GetRotateType(operation);
        }
        if (Intersects(operation, SCALE) && type == MT_NONE) {
          type = GetScaleType(operation);
        }

        if (type != MT_NONE) {
          overBigAnchor = false;
          overSmallAnchor = false;
        }

        ImU32 selectionColor = GetColorU32(SELECTION);

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
        if (!mbUsingBounds && mbEnable && overBigAnchor && CanActivate()) {
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
          mEditingID = mActualID;
          mBoundsMatrix = mModelSource;
        }
        // small anchor on middle of segment
        if (!mbUsingBounds && mbEnable && overSmallAnchor && CanActivate()) {
          vec_t midPointOpposite =
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
          mEditingID = mActualID;
          mBoundsMatrix = mModelSource;
        }
      }

      if (mbUsingBounds && (mActualID == -1 || mActualID == mEditingID)) {
        matrix_t scale;
        scale.SetToIdentity();

        // compute projected mouse position on plan
        const float len =
          IntersectRayPlane(mRayOrigin, mRayVector, mBoundsPlan);
        vec_t newPos = mRayOrigin + mRayVector * len;

        // compute a reference and delta vectors base on mouse move
        vec_t deltaVector = (newPos - mBoundsPivot).Abs();
        vec_t referenceVector = (mBoundsAnchor - mBoundsPivot).Abs();

        // for 1 or 2 axes, compute a ratio that's used for scale and snap it
        // based on resulting length
        for (int i = 0; i < 2; i++) {
          int axisIndex1 = mBoundsAxis[i];
          if (axisIndex1 == -1) {
            continue;
          }

          float ratioAxis = 1.f;
          vec_t axisDir = mBoundsMatrix.component[axisIndex1].Abs();

          float dtAxis = axisDir.Dot(referenceVector);
          float boundSize = bounds[axisIndex1 + 3] - bounds[axisIndex1];
          if (dtAxis > FLT_EPSILON) {
            ratioAxis = axisDir.Dot(deltaVector) / dtAxis;
          }

          if (snapValues) {
            float length = boundSize * ratioAxis;
            ComputeSnap(&length, snapValues[axisIndex1]);
            if (boundSize > FLT_EPSILON) {
              ratioAxis = length / boundSize;
            }
          }
          scale.component[axisIndex1] *= ratioAxis;
        }

        // transform matrix
        matrix_t preScale, postScale;
        preScale.Translation(-mBoundsLocalPivot);
        postScale.Translation(mBoundsLocalPivot);
        matrix_t res = preScale * scale * postScale * mBoundsMatrix;
        *matrix = res;

        // info text
        char tmps[512];
        ImVec2 destinationPosOnScreen =
          worldToPos(mModel.v.position, mViewProjection);
        ImFormatString(
          tmps,
          sizeof(tmps),
          "X: %.2f Y: %.2f Z: %.2f",
          (bounds[3] - bounds[0]) * mBoundsMatrix.component[0].Length() *
            scale.component[0].Length(),
          (bounds[4] - bounds[1]) * mBoundsMatrix.component[1].Length() *
            scale.component[1].Length(),
          (bounds[5] - bounds[2]) * mBoundsMatrix.component[2].Length() *
            scale.component[2].Length());
        drawList->AddText(
          ImVec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
          GetColorU32(TEXT_SHADOW),
          tmps);
        drawList->AddText(
          ImVec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
          GetColorU32(TEXT),
          tmps);
      }

      if (!io.MouseDown[0]) {
        mbUsingBounds = false;
        mEditingID = -1;
      }
      if (mbUsingBounds) {
        break;
      }
    }
  }

  void ComputeCameraRay(vec_t& rayOrigin,
                        vec_t& rayDir,
                        ImVec2 position,
                        ImVec2 size)
  {
    ImGuiIO& io = ImGui::GetIO();

    matrix_t mViewProjInverse;
    mViewProjInverse.Inverse(mViewMat * mProjectionMat);

    const float mox = ((io.MousePos.x - position.x) / size.x) * 2.f - 1.f;
    const float moy =
      (1.f - ((io.MousePos.y - position.y) / size.y)) * 2.f - 1.f;

    const float zNear = mReversed ? (1.f - FLT_EPSILON) : 0.f;
    const float zFar = mReversed ? 0.f : (1.f - FLT_EPSILON);

    rayOrigin.Transform(makeVect(mox, moy, zNear, 1.f), mViewProjInverse);
    rayOrigin *= 1.f / rayOrigin.w;
    vec_t rayEnd;
    rayEnd.Transform(makeVect(mox, moy, zFar, 1.f), mViewProjInverse);
    rayEnd *= 1.f / rayEnd.w;
    rayDir = Normalized(rayEnd - rayOrigin);
  }

  void ComputeCameraRay(vec_t& rayOrigin, vec_t& rayDir)
  {
    ComputeCameraRay(rayOrigin, rayDir, leftTop(), size());
  }

  bool HandleTranslation(float* matrix,
                         float* deltaMatrix,
                         OPERATION op,
                         MOVETYPE& type,
                         const float* snap)
  {
    if (!Intersects(op, TRANSLATE) || type != MT_NONE) {
      return false;
    }
    const ImGuiIO& io = ImGui::GetIO();
    const bool applyRotationLocaly = mMode == LOCAL || type == MT_MOVE_SCREEN;
    bool modified = false;

    // move
    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsTranslateType(mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
      ImGui::SetNextFrameWantCaptureMouse(true);
#else
      ImGui::CaptureMouseFromApp();
#endif
      const float signedLength =
        IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
      const float len = fabsf(signedLength); // near plan
      const vec_t newPos = mRayOrigin + mRayVector * len;

      // compute delta
      const vec_t newOrigin = newPos - mRelativeOrigin * mScreenFactor;
      vec_t delta = newOrigin - mModel.v.position;

      // 1 axis constraint
      if (mCurrentOperation >= MT_MOVE_X && mCurrentOperation <= MT_MOVE_Z) {
        const int axisIndex = mCurrentOperation - MT_MOVE_X;
        const vec_t& axisValue = *(vec_t*)&mModel.m[axisIndex];
        const float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;
      }

      // snap
      if (snap) {
        vec_t cumulativeDelta = mModel.v.position + delta - mMatrixOrigin;
        if (applyRotationLocaly) {
          matrix_t modelSourceNormalized = mModelSource;
          modelSourceNormalized.OrthoNormalize();
          matrix_t modelSourceNormalizedInverse;
          modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
          cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
          ComputeSnap(cumulativeDelta, snap);
          cumulativeDelta.TransformVector(modelSourceNormalized);
        } else {
          ComputeSnap(cumulativeDelta, snap);
        }
        delta = mMatrixOrigin + cumulativeDelta - mModel.v.position;
      }

      if (delta != mTranslationLastDelta) {
        modified = true;
      }
      mTranslationLastDelta = delta;

      // compute matrix & delta
      matrix_t deltaMatrixTranslation;
      deltaMatrixTranslation.Translation(delta);
      if (deltaMatrix) {
        memcpy(deltaMatrix, deltaMatrixTranslation.m16, sizeof(float) * 16);
      }

      const matrix_t res = mModelSource * deltaMatrixTranslation;
      *(matrix_t*)matrix = res;

      if (!io.MouseDown[0]) {
        mbUsing = false;
      }

      type = mCurrentOperation;
    } else {
      // find new possible way to move
      vec_t gizmoHitProportion;
      type = GetMoveType(op, &gizmoHitProportion, screenCoord());
      if (type != MT_NONE) {
#if IMGUI_VERSION_NUM >= 18723
        ImGui::SetNextFrameWantCaptureMouse(true);
#else
        ImGui::CaptureMouseFromApp();
#endif
      }
      if (CanActivate() && type != MT_NONE) {
        mbUsing = true;
        mEditingID = mActualID;
        mCurrentOperation = type;
        vec_t movePlanNormal[] = { mModel.v.right, mModel.v.up, mModel.v.dir,
                                   mModel.v.right, mModel.v.up, mModel.v.dir,
                                   -mCameraDir };

        vec_t cameraToModelNormalized =
          Normalized(mModel.v.position - mCameraEye);
        for (unsigned int i = 0; i < 3; i++) {
          vec_t orthoVector = Cross(movePlanNormal[i], cameraToModelNormalized);
          movePlanNormal[i].Cross(orthoVector);
          movePlanNormal[i].Normalize();
        }
        // pickup plan
        mTranslationPlan =
          BuildPlan(mModel.v.position, movePlanNormal[type - MT_MOVE_X]);
        const float len =
          IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
        mTranslationPlanOrigin = mRayOrigin + mRayVector * len;
        mMatrixOrigin = mModel.v.position;

        mRelativeOrigin =
          (mTranslationPlanOrigin - mModel.v.position) * (1.f / mScreenFactor);
      }
    }
    return modified;
  }

  bool HandleScale(float* matrix,
                   float* deltaMatrix,
                   OPERATION op,
                   MOVETYPE& type,
                   const float* snap)
  {
    if ((!Intersects(op, SCALE) && !Intersects(op, SCALEU)) ||
        type != MT_NONE || !mbMouseOver) {
      return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    bool modified = false;

    if (!mbUsing) {
      // find new possible way to scale
      type = GetScaleType(op);
      if (type != MT_NONE) {
#if IMGUI_VERSION_NUM >= 18723
        ImGui::SetNextFrameWantCaptureMouse(true);
#else
        ImGui::CaptureMouseFromApp();
#endif
      }
      if (CanActivate() && type != MT_NONE) {
        mbUsing = true;
        mEditingID = mActualID;
        mCurrentOperation = type;
        const vec_t movePlanNormal[] = { mModel.v.up,    mModel.v.dir,
                                         mModel.v.right, mModel.v.dir,
                                         mModel.v.up,    mModel.v.right,
                                         -mCameraDir };
        // pickup plan

        mTranslationPlan =
          BuildPlan(mModel.v.position, movePlanNormal[type - MT_SCALE_X]);
        const float len =
          IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
        mTranslationPlanOrigin = mRayOrigin + mRayVector * len;
        mMatrixOrigin = mModel.v.position;
        mScale.Set(1.f, 1.f, 1.f);
        mRelativeOrigin =
          (mTranslationPlanOrigin - mModel.v.position) * (1.f / mScreenFactor);
        mScaleValueOrigin = makeVect(mModelSource.v.right.Length(),
                                     mModelSource.v.up.Length(),
                                     mModelSource.v.dir.Length());
        mSaveMousePosx = io.MousePos.x;
      }
    }
    // scale
    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsScaleType(mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
      ImGui::SetNextFrameWantCaptureMouse(true);
#else
      ImGui::CaptureMouseFromApp();
#endif
      const float len =
        IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
      vec_t newPos = mRayOrigin + mRayVector * len;
      vec_t newOrigin = newPos - mRelativeOrigin * mScreenFactor;
      vec_t delta = newOrigin - mModelLocal.v.position;

      // 1 axis constraint
      if (mCurrentOperation >= MT_SCALE_X && mCurrentOperation <= MT_SCALE_Z) {
        int axisIndex = mCurrentOperation - MT_SCALE_X;
        const vec_t& axisValue = *(vec_t*)&mModelLocal.m[axisIndex];
        float lengthOnAxis = Dot(axisValue, delta);
        delta = axisValue * lengthOnAxis;

        vec_t baseVector = mTranslationPlanOrigin - mModelLocal.v.position;
        float ratio =
          Dot(axisValue, baseVector + delta) / Dot(axisValue, baseVector);

        mScale[axisIndex] = max(ratio, 0.001f);
      } else {
        float scaleDelta = (io.MousePos.x - mSaveMousePosx) * 0.01f;
        mScale.Set(max(1.f + scaleDelta, 0.001f));
      }

      // snap
      if (snap) {
        float scaleSnap[] = { snap[0], snap[0], snap[0] };
        ComputeSnap(mScale, scaleSnap);
      }

      // no 0 allowed
      for (int i = 0; i < 3; i++)
        mScale[i] = max(mScale[i], 0.001f);

      if (mScaleLast != mScale) {
        modified = true;
      }
      mScaleLast = mScale;

      // compute matrix & delta
      matrix_t deltaMatrixScale;
      deltaMatrixScale.Scale(mScale * mScaleValueOrigin);

      matrix_t res = deltaMatrixScale * mModelLocal;
      *(matrix_t*)matrix = res;

      if (deltaMatrix) {
        vec_t deltaScale = mScale * mScaleValueOrigin;

        vec_t originalScaleDivider;
        originalScaleDivider.x = 1 / mModelScaleOrigin.x;
        originalScaleDivider.y = 1 / mModelScaleOrigin.y;
        originalScaleDivider.z = 1 / mModelScaleOrigin.z;

        deltaScale = deltaScale * originalScaleDivider;

        deltaMatrixScale.Scale(deltaScale);
        memcpy(deltaMatrix, deltaMatrixScale.m16, sizeof(float) * 16);
      }

      if (!io.MouseDown[0]) {
        mbUsing = false;
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
                      const float* snap)
  {
    if (!Intersects(op, ROTATE) || type != MT_NONE || !mbMouseOver) {
      return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    bool applyRotationLocaly = mMode == LOCAL;
    bool modified = false;

    if (!mbUsing) {
      type = GetRotateType(op);

      if (type != MT_NONE) {
#if IMGUI_VERSION_NUM >= 18723
        ImGui::SetNextFrameWantCaptureMouse(true);
#else
        ImGui::CaptureMouseFromApp();
#endif
      }

      if (type == MT_ROTATE_SCREEN) {
        applyRotationLocaly = true;
      }

      if (CanActivate() && type != MT_NONE) {
        mbUsing = true;
        mEditingID = mActualID;
        mCurrentOperation = type;
        const vec_t rotatePlanNormal[] = {
          mModel.v.right, mModel.v.up, mModel.v.dir, -mCameraDir
        };
        // pickup plan
        if (applyRotationLocaly) {
          mTranslationPlan =
            BuildPlan(mModel.v.position, rotatePlanNormal[type - MT_ROTATE_X]);
        } else {
          mTranslationPlan = BuildPlan(mModelSource.v.position,
                                       directionUnary[type - MT_ROTATE_X]);
        }

        const float len =
          IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
        vec_t localPos = mRayOrigin + mRayVector * len - mModel.v.position;
        mRotationVectorSource = Normalized(localPos);
        mRotationAngleOrigin = ComputeAngleOnPlan();
      }
    }

    // rotation
    if (mbUsing && (mActualID == -1 || mActualID == mEditingID) &&
        IsRotateType(mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
      ImGui::SetNextFrameWantCaptureMouse(true);
#else
      ImGui::CaptureMouseFromApp();
#endif
      mRotationAngle = ComputeAngleOnPlan();
      if (snap) {
        float snapInRadian = snap[0] * DEG2RAD;
        ComputeSnap(&mRotationAngle, snapInRadian);
      }
      vec_t rotationAxisLocalSpace;

      rotationAxisLocalSpace.TransformVector(
        makeVect(
          mTranslationPlan.x, mTranslationPlan.y, mTranslationPlan.z, 0.f),
        mModelInverse);
      rotationAxisLocalSpace.Normalize();

      matrix_t deltaRotation;
      deltaRotation.RotationAxis(rotationAxisLocalSpace,
                                 mRotationAngle - mRotationAngleOrigin);
      if (mRotationAngle != mRotationAngleOrigin) {
        modified = true;
      }
      mRotationAngleOrigin = mRotationAngle;

      matrix_t scaleOrigin;
      scaleOrigin.Scale(mModelScaleOrigin);

      if (applyRotationLocaly) {
        *(matrix_t*)matrix = scaleOrigin * deltaRotation * mModelLocal;
      } else {
        matrix_t res = mModelSource;
        res.v.position.Set(0.f);

        *(matrix_t*)matrix = res * deltaRotation;
        ((matrix_t*)matrix)->v.position = mModelSource.v.position;
      }

      if (deltaMatrix) {
        *(matrix_t*)deltaMatrix = mModelInverse * deltaRotation * mModel;
      }

      if (!io.MouseDown[0]) {
        mbUsing = false;
        mEditingID = -1;
      }
      type = mCurrentOperation;
    }
    return modified;
  }

  float ComputeAngleOnPlan()
  {
    const float len =
      IntersectRayPlane(mRayOrigin, mRayVector, mTranslationPlan);
    vec_t localPos =
      Normalized(mRayOrigin + mRayVector * len - mModel.v.position);

    vec_t perpendicularVector;
    perpendicularVector.Cross(mRotationVectorSource, mTranslationPlan);
    perpendicularVector.Normalize();
    float acosAngle = Clamp(Dot(localPos, mRotationVectorSource), -1.f, 1.f);
    float angle = acosf(acosAngle);
    angle *= (Dot(localPos, perpendicularVector) < 0.f) ? 1.f : -1.f;
    return angle;
  }

  bool Manipulate(void* id,
                  const float* view,
                  const float* projection,
                  OPERATION operation,
                  MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap,
                  const float* localBounds,
                  const float* boundsSnap)
  {
    mActualID = (int64_t)id;
    // Scope scope;

    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    ComputeContext(
      view, projection, matrix, (operation & SCALE) ? LOCAL : mode);

    // set delta to identity
    if (deltaMatrix) {
      ((matrix_t*)deltaMatrix)->SetToIdentity();
    }

    // behind camera
    vec_t camSpacePosition;
    camSpacePosition.TransformPoint(makeVect(0.f, 0.f, 0.f), mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    // --
    MOVETYPE type = MT_NONE;
    bool manipulated = false;
    if (mbEnable) {
      if (!mbUsingBounds) {
        manipulated =
          HandleTranslation(matrix, deltaMatrix, operation, type, snap) ||
          HandleScale(matrix, deltaMatrix, operation, type, snap) ||
          HandleRotation(matrix, deltaMatrix, operation, type, snap);
      }
    }

    if (localBounds && !mbUsing) {
      HandleAndDrawLocalBounds(
        localBounds, (matrix_t*)matrix, boundsSnap, operation);
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

  void DrawCubes(const float* view,
                 const float* projection,
                 const float* matrices,
                 int matrixCount)
  {
    matrix_t viewInverse;
    viewInverse.Inverse(*(matrix_t*)view);

    struct CubeFace
    {
      float z;
      ImVec2 faceCoordsScreen[4];
      ImU32 color;
    };
    CubeFace* faces = (CubeFace*)_malloca(sizeof(CubeFace) * matrixCount * 6);

    if (!faces) {
      return;
    }

    vec_t frustum[6];
    matrix_t viewProjection = *(matrix_t*)view * *(matrix_t*)projection;
    ComputeFrustumPlanes(frustum, viewProjection.m16);

    int cubeFaceCount = 0;
    for (int cube = 0; cube < matrixCount; cube++) {
      const float* matrix = &matrices[cube * 16];

      matrix_t res =
        *(matrix_t*)matrix * *(matrix_t*)view * *(matrix_t*)projection;

      for (int iFace = 0; iFace < 6; iFace++) {
        const int normalIndex = (iFace % 3);
        const int perpXIndex = (normalIndex + 1) % 3;
        const int perpYIndex = (normalIndex + 2) % 3;
        const float invert = (iFace > 2) ? -1.f : 1.f;

        const vec_t faceCoords[4] = {
          directionUnary[normalIndex] + directionUnary[perpXIndex] +
            directionUnary[perpYIndex],
          directionUnary[normalIndex] + directionUnary[perpXIndex] -
            directionUnary[perpYIndex],
          directionUnary[normalIndex] - directionUnary[perpXIndex] -
            directionUnary[perpYIndex],
          directionUnary[normalIndex] - directionUnary[perpXIndex] +
            directionUnary[perpYIndex],
        };

        // clipping
        /*
        bool skipFace = false;
        for (unsigned int iCoord = 0; iCoord < 4; iCoord++)
        {
           vec_t camSpacePosition;
           camSpacePosition.TransformPoint(faceCoords[iCoord] * 0.5f * invert,
        res); if (camSpacePosition.z < 0.001f)
           {
              skipFace = true;
              break;
           }
        }
        if (skipFace)
        {
           continue;
        }
        */
        vec_t centerPosition, centerPositionVP;
        centerPosition.TransformPoint(
          directionUnary[normalIndex] * 0.5f * invert, *(matrix_t*)matrix);
        centerPositionVP.TransformPoint(
          directionUnary[normalIndex] * 0.5f * invert, res);

        bool inFrustum = true;
        for (int iFrustum = 0; iFrustum < 6; iFrustum++) {
          float dist = DistanceToPlane(centerPosition, frustum[iFrustum]);
          if (dist < 0.f) {
            inFrustum = false;
            break;
          }
        }

        if (!inFrustum) {
          continue;
        }
        CubeFace& cubeFace = faces[cubeFaceCount];

        // 3D->2D
        // ImVec2 faceCoordsScreen[4];
        for (unsigned int iCoord = 0; iCoord < 4; iCoord++) {
          cubeFace.faceCoordsScreen[iCoord] =
            worldToPos(faceCoords[iCoord] * 0.5f * invert, res);
        }

        ImU32 directionColor = GetColorU32(DIRECTION_X + normalIndex);
        cubeFace.color = directionColor | IM_COL32(0x80, 0x80, 0x80, 0);

        cubeFace.z = centerPositionVP.z / centerPositionVP.w;
        cubeFaceCount++;
      }
    }
    qsort(faces,
          cubeFaceCount,
          sizeof(CubeFace),
          [](void const* _a, void const* _b) {
            CubeFace* a = (CubeFace*)_a;
            CubeFace* b = (CubeFace*)_b;
            if (a->z < b->z) {
              return 1;
            }
            return -1;
          });
    // draw face with lighter color
    for (int iFace = 0; iFace < cubeFaceCount; iFace++) {
      const CubeFace& cubeFace = faces[iFace];
      mDrawList->AddConvexPolyFilled(
        cubeFace.faceCoordsScreen, 4, cubeFace.color);
    }

    _freea(faces);
  }

  void DrawGrid(const float* view,
                const float* projection,
                const float* matrix,
                const float gridSize)
  {
    matrix_t viewProjection = *(matrix_t*)view * *(matrix_t*)projection;
    vec_t frustum[6];
    ComputeFrustumPlanes(frustum, viewProjection.m16);
    matrix_t res = *(matrix_t*)matrix * viewProjection;

    for (float f = -gridSize; f <= gridSize; f += 1.f) {
      for (int dir = 0; dir < 2; dir++) {
        vec_t ptA = makeVect(dir ? -gridSize : f, 0.f, dir ? f : -gridSize);
        vec_t ptB = makeVect(dir ? gridSize : f, 0.f, dir ? f : gridSize);
        bool visible = true;
        for (int i = 0; i < 6; i++) {
          float dA = DistanceToPlane(ptA, frustum[i]);
          float dB = DistanceToPlane(ptB, frustum[i]);
          if (dA < 0.f && dB < 0.f) {
            visible = false;
            break;
          }
          if (dA > 0.f && dB > 0.f) {
            continue;
          }
          if (dA < 0.f) {
            float len = fabsf(dA - dB);
            float t = fabsf(dA) / len;
            ptA.Lerp(ptB, t);
          }
          if (dB < 0.f) {
            float len = fabsf(dB - dA);
            float t = fabsf(dB) / len;
            ptB.Lerp(ptA, t);
          }
        }
        if (visible) {
          ImU32 col = IM_COL32(0x80, 0x80, 0x80, 0xFF);
          col = (fmodf(fabsf(f), 10.f) < FLT_EPSILON)
                  ? IM_COL32(0x90, 0x90, 0x90, 0xFF)
                  : col;
          col =
            (fabsf(f) < FLT_EPSILON) ? IM_COL32(0x40, 0x40, 0x40, 0xFF) : col;

          float thickness = 1.f;
          thickness = (fmodf(fabsf(f), 10.f) < FLT_EPSILON) ? 1.5f : thickness;
          thickness = (fabsf(f) < FLT_EPSILON) ? 2.3f : thickness;

          mDrawList->AddLine(
            worldToPos(ptA, res), worldToPos(ptB, res), col, thickness);
        }
      }
    }
  }
};

void
SetRect(float x, float y, float width, float height)
{
  Context::Instance().setRect(x, y, width, height);
}

bool
Manipulate(void* id,
           const float* view,
           const float* projection,
           OPERATION operation,
           MODE mode,
           float* matrix,
           float* deltaMatrix,
           const float* snap,
           const float* localBounds,
           const float* boundsSnap)
{
  return Context::Instance().Manipulate(id,
                                        view,
                                        projection,
                                        operation,
                                        mode,
                                        matrix,
                                        deltaMatrix,
                                        snap,
                                        localBounds,
                                        boundsSnap);
}

} // namespace
