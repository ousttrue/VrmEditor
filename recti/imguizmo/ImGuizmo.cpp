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
#include <numbers>

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

  ImDrawList* mDrawList;
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
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];

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
  int mCurrentOperation;

  float mX = 0.f;
  float mY = 0.f;
  float mWidth = 0.f;
  float mHeight = 0.f;
  float mXMax = 0.f;
  float mYMax = 0.f;
  float mDisplayRatio = 1.f;

  bool mIsOrthographic = false;

  int64_t mActualID = -1;
  int64_t mEditingID = -1;
  OPERATION mOperation = OPERATION(-1);

  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

  bool mAllowActiveHoverItem = false;

  void ComputeContext(const float* view,
                      const float* projection,
                      float* matrix,
                      MODE mode);
};

inline Context&
GetContext()
{
  return Context::Instance();
}

// call BeginFrame right after ImGui_XXXX_NewFrame();
void
BeginFrame()
{
  const ImU32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs |
                      ImGuiWindowFlags_NoSavedSettings |
                      ImGuiWindowFlags_NoFocusOnAppearing |
                      ImGuiWindowFlags_NoBringToFrontOnFocus;

#ifdef IMGUI_HAS_VIEWPORT
  ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
#else
  ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowSize(io.DisplaySize);
  ImGui::SetNextWindowPos(ImVec2(0, 0));
#endif

  ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
  ImGui::PushStyleColor(ImGuiCol_Border, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  ImGui::Begin("gizmo", NULL, flags);
  GetContext().mDrawList = ImGui::GetWindowDrawList();
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(2);
}

// this is necessary because when imguizmo is compiled into a dll, and imgui
// into another globals are not shared between them. More details at
// https://stackoverflow.com/questions/19373061/what-happens-to-global-and-static-variables-in-a-shared-library-when-it-is-dynam
// expose method to set imgui context
void
SetImGuiContext(ImGuiContext* ctx)
{
  ImGui::SetCurrentContext(ctx);
}

// return true if mouse IsOver or if the gizmo is in moving state
bool
IsUsing()
{
  return (GetContext().mbUsing &&
          (GetContext().mActualID == -1 ||
           GetContext().mActualID == GetContext().mEditingID)) ||
         GetContext().mbUsingBounds;
}

static float
GetSegmentLengthClipSpace(const vec_t& start,
                          const vec_t& end,
                          const bool localCoordinates = false)
{
  vec_t startOfSegment = start;
  const matrix_t& mvp =
    localCoordinates ? GetContext().mMVPLocal : GetContext().mMVP;
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
  clipSpaceAxis.y /= GetContext().mDisplayRatio;
  float segmentLengthInClipSpace = sqrtf(clipSpaceAxis.x * clipSpaceAxis.x +
                                         clipSpaceAxis.y * clipSpaceAxis.y);
  return segmentLengthInClipSpace;
}

static float
GetParallelogram(const vec_t& ptO, const vec_t& ptA, const vec_t& ptB)
{
  vec_t pts[] = { ptO, ptA, ptB };
  for (unsigned int i = 0; i < 3; i++) {
    pts[i].TransformPoint(GetContext().mMVP);
    if (fabsf(pts[i].w) >
        FLT_EPSILON) // check for axis aligned with camera direction
    {
      pts[i] *= 1.f / pts[i].w;
    }
  }
  vec_t segA = pts[1] - pts[0];
  vec_t segB = pts[2] - pts[0];
  segA.y /= GetContext().mDisplayRatio;
  segB.y /= GetContext().mDisplayRatio;
  vec_t segAOrtho = makeVect(-segA.y, segA.x);
  segAOrtho.Normalize();
  float dt = segAOrtho.Dot3(segB);
  float surface = sqrtf(segA.x * segA.x + segA.y * segA.y) * fabsf(dt);
  return surface;
}

static const vec_t directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                         { 0.f, 1.f, 0.f, 0 },
                                         { 0.f, 0.f, 1.f, 0 } };

static void
ComputeTripodAxisAndVisibility(const int axisIndex,
                               vec_t& dirAxis,
                               vec_t& dirPlaneX,
                               vec_t& dirPlaneY,
                               bool& belowAxisLimit,
                               bool& belowPlaneLimit,
                               const bool localCoordinates = false)
{
  dirAxis = directionUnary[axisIndex];
  dirPlaneX = directionUnary[(axisIndex + 1) % 3];
  dirPlaneY = directionUnary[(axisIndex + 2) % 3];

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID)) {
    // when using, use stored factors so the gizmo doesn't flip when we
    // translate
    belowAxisLimit = GetContext().mBelowAxisLimit[axisIndex];
    belowPlaneLimit = GetContext().mBelowPlaneLimit[axisIndex];

    dirAxis *= GetContext().mAxisFactor[axisIndex];
    dirPlaneX *= GetContext().mAxisFactor[(axisIndex + 1) % 3];
    dirPlaneY *= GetContext().mAxisFactor[(axisIndex + 2) % 3];
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
    bool& allowFlip = GetContext().mAllowAxisFlip;
    float mulAxis = (allowFlip && lenDir < lenDirMinus &&
                     fabsf(lenDir - lenDirMinus) > FLT_EPSILON)
                      ? -1.f
                      : 1.f;
    float mulAxisX = (allowFlip && lenDirPlaneX < lenDirMinusPlaneX &&
                      fabsf(lenDirPlaneX - lenDirMinusPlaneX) > FLT_EPSILON)
                       ? -1.f
                       : 1.f;
    float mulAxisY = (allowFlip && lenDirPlaneY < lenDirMinusPlaneY &&
                      fabsf(lenDirPlaneY - lenDirMinusPlaneY) > FLT_EPSILON)
                       ? -1.f
                       : 1.f;
    dirAxis *= mulAxis;
    dirPlaneX *= mulAxisX;
    dirPlaneY *= mulAxisY;

    // for axis
    float axisLengthInClipSpace =
      GetSegmentLengthClipSpace(makeVect(0.f, 0.f, 0.f),
                                dirAxis * GetContext().mScreenFactor,
                                localCoordinates);

    float paraSurf = GetParallelogram(makeVect(0.f, 0.f, 0.f),
                                      dirPlaneX * GetContext().mScreenFactor,
                                      dirPlaneY * GetContext().mScreenFactor);
    belowPlaneLimit = (paraSurf > 0.0025f);
    belowAxisLimit = (axisLengthInClipSpace > 0.02f);

    // and store values
    GetContext().mAxisFactor[axisIndex] = mulAxis;
    GetContext().mAxisFactor[(axisIndex + 1) % 3] = mulAxisX;
    GetContext().mAxisFactor[(axisIndex + 2) % 3] = mulAxisY;
    GetContext().mBelowAxisLimit[axisIndex] = belowAxisLimit;
    GetContext().mBelowPlaneLimit[axisIndex] = belowPlaneLimit;
  }
}

static ImVec2
worldToPos(const vec_t& worldPos,
           const matrix_t& mat,
           ImVec2 position = ImVec2(GetContext().mX, GetContext().mY),
           ImVec2 size = ImVec2(GetContext().mWidth, GetContext().mHeight))
{
  vec_t trans;
  trans.TransformPoint(worldPos, mat);
  trans *= 0.5f / trans.w;
  trans += makeVect(0.5f, 0.5f);
  trans.y = 1.f - trans.y;
  trans.x *= size.x;
  trans.y *= size.y;
  trans.x += position.x;
  trans.y += position.y;
  return ImVec2(trans.x, trans.y);
}

static MOVETYPE
GetScaleType(OPERATION op)
{
  if (GetContext().mbUsing) {
    return MT_NONE;
  }
  ImGuiIO& io = ImGui::GetIO();
  MOVETYPE type = MT_NONE;

  // screen
  if (io.MousePos.x >= GetContext().mScreenSquareMin.x &&
      io.MousePos.x <= GetContext().mScreenSquareMax.x &&
      io.MousePos.y >= GetContext().mScreenSquareMin.y &&
      io.MousePos.y <= GetContext().mScreenSquareMax.y && Contains(op, SCALE)) {
    type = MT_SCALE_XYZ;
  }

  // compute
  for (int i = 0; i < 3 && type == MT_NONE; i++) {
    if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }
    vec_t dirPlaneX, dirPlaneY, dirAxis;
    bool belowAxisLimit, belowPlaneLimit;
    ComputeTripodAxisAndVisibility(
      i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit, true);
    dirAxis.TransformVector(GetContext().mModelLocal);
    dirPlaneX.TransformVector(GetContext().mModelLocal);
    dirPlaneY.TransformVector(GetContext().mModelLocal);

    const float len = IntersectRayPlane(
      GetContext().mRayOrigin,
      GetContext().mRayVector,
      BuildPlan(GetContext().mModelLocal.v.position, dirAxis));
    vec_t posOnPlan = GetContext().mRayOrigin + GetContext().mRayVector * len;

    const float startOffset =
      Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.0f : 0.1f;
    const float endOffset =
      Contains(op, static_cast<OPERATION>(TRANSLATE_X << i)) ? 1.4f : 1.0f;
    const ImVec2 posOnPlanScreen =
      worldToPos(posOnPlan, GetContext().mViewProjection);
    const ImVec2 axisStartOnScreen =
      worldToPos(GetContext().mModelLocal.v.position +
                   dirAxis * GetContext().mScreenFactor * startOffset,
                 GetContext().mViewProjection);
    const ImVec2 axisEndOnScreen =
      worldToPos(GetContext().mModelLocal.v.position +
                   dirAxis * GetContext().mScreenFactor * endOffset,
                 GetContext().mViewProjection);

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

  vec_t deltaScreen = { io.MousePos.x - GetContext().mScreenSquareCenter.x,
                        io.MousePos.y - GetContext().mScreenSquareCenter.y,
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
    ComputeTripodAxisAndVisibility(
      i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit, true);

    // draw axis
    if (belowAxisLimit) {
      bool hasTranslateOnAxis =
        Contains(op, static_cast<OPERATION>(TRANSLATE_X << i));
      float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
      // ImVec2 baseSSpace = worldToPos(dirAxis * 0.1f *
      // GetContext().mScreenFactor, GetContext().mMVPLocal); ImVec2
      // worldDirSSpaceNoScale = worldToPos(dirAxis
      // * markerScale * GetContext().mScreenFactor, GetContext().mMVP);
      ImVec2 worldDirSSpace =
        worldToPos((dirAxis * markerScale) * GetContext().mScreenFactor,
                   GetContext().mMVPLocal);

      float distance = sqrtf(ImLengthSqr(worldDirSSpace - io.MousePos));
      if (distance < 12.f) {
        type = (MOVETYPE)(MT_SCALE_X + i);
      }
    }
  }
  return type;
}

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

static MOVETYPE
GetRotateType(OPERATION op)
{
  if (GetContext().mbUsing) {
    return MT_NONE;
  }
  ImGuiIO& io = ImGui::GetIO();
  MOVETYPE type = MT_NONE;

  vec_t deltaScreen = { io.MousePos.x - GetContext().mScreenSquareCenter.x,
                        io.MousePos.y - GetContext().mScreenSquareCenter.y,
                        0.f,
                        0.f };
  float dist = deltaScreen.Length();
  if (Intersects(op, ROTATE_SCREEN) &&
      dist >= (GetContext().mRadiusSquareCenter - 4.0f) &&
      dist < (GetContext().mRadiusSquareCenter + 4.0f)) {
    type = MT_ROTATE_SCREEN;
  }

  const vec_t planNormals[] = { GetContext().mModel.v.right,
                                GetContext().mModel.v.up,
                                GetContext().mModel.v.dir };

  vec_t modelViewPos;
  modelViewPos.TransformPoint(GetContext().mModel.v.position,
                              GetContext().mViewMat);

  for (int i = 0; i < 3 && type == MT_NONE; i++) {
    if (!Intersects(op, static_cast<OPERATION>(ROTATE_X << i))) {
      continue;
    }
    // pickup plan
    vec_t pickupPlan =
      BuildPlan(GetContext().mModel.v.position, planNormals[i]);

    const float len = IntersectRayPlane(
      GetContext().mRayOrigin, GetContext().mRayVector, pickupPlan);
    const vec_t intersectWorldPos =
      GetContext().mRayOrigin + GetContext().mRayVector * len;
    vec_t intersectViewPos;
    intersectViewPos.TransformPoint(intersectWorldPos, GetContext().mViewMat);

    if (ImAbs(modelViewPos.z) - ImAbs(intersectViewPos.z) < -FLT_EPSILON) {
      continue;
    }

    const vec_t localPos = intersectWorldPos - GetContext().mModel.v.position;
    vec_t idealPosOnCircle = Normalized(localPos);
    idealPosOnCircle.TransformVector(GetContext().mModelInverse);
    const ImVec2 idealPosOnCircleScreen = worldToPos(
      idealPosOnCircle * ROTATION_DISPLAY_FACTOR * GetContext().mScreenFactor,
      GetContext().mMVP);

    // GetContext().mDrawList->AddCircle(idealPosOnCircleScreen, 5.f,
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

static int
GetMoveType(OPERATION op, vec_t* gizmoHitProportion)
{
  if (!Intersects(op, TRANSLATE) || GetContext().mbUsing ||
      !GetContext().mbMouseOver) {
    return MT_NONE;
  }
  ImGuiIO& io = ImGui::GetIO();
  int type = MT_NONE;

  // screen
  if (io.MousePos.x >= GetContext().mScreenSquareMin.x &&
      io.MousePos.x <= GetContext().mScreenSquareMax.x &&
      io.MousePos.y >= GetContext().mScreenSquareMin.y &&
      io.MousePos.y <= GetContext().mScreenSquareMax.y &&
      Contains(op, TRANSLATE)) {
    type = MT_MOVE_SCREEN;
  }

  const vec_t screenCoord =
    makeVect(io.MousePos - ImVec2(GetContext().mX, GetContext().mY));

  // compute
  for (int i = 0; i < 3 && type == MT_NONE; i++) {
    vec_t dirPlaneX, dirPlaneY, dirAxis;
    bool belowAxisLimit, belowPlaneLimit;
    ComputeTripodAxisAndVisibility(
      i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit);
    dirAxis.TransformVector(GetContext().mModel);
    dirPlaneX.TransformVector(GetContext().mModel);
    dirPlaneY.TransformVector(GetContext().mModel);

    const float len =
      IntersectRayPlane(GetContext().mRayOrigin,
                        GetContext().mRayVector,
                        BuildPlan(GetContext().mModel.v.position, dirAxis));
    vec_t posOnPlan = GetContext().mRayOrigin + GetContext().mRayVector * len;

    const ImVec2 axisStartOnScreen =
      worldToPos(GetContext().mModel.v.position +
                   dirAxis * GetContext().mScreenFactor * 0.1f,
                 GetContext().mViewProjection) -
      ImVec2(GetContext().mX, GetContext().mY);
    const ImVec2 axisEndOnScreen =
      worldToPos(GetContext().mModel.v.position +
                   dirAxis * GetContext().mScreenFactor,
                 GetContext().mViewProjection) -
      ImVec2(GetContext().mX, GetContext().mY);

    vec_t closestPointOnAxis = PointOnSegment(
      screenCoord, makeVect(axisStartOnScreen), makeVect(axisEndOnScreen));
    if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
        Intersects(op, static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
    {
      type = MT_MOVE_X + i;
    }

    const float dx =
      dirPlaneX.Dot3((posOnPlan - GetContext().mModel.v.position) *
                     (1.f / GetContext().mScreenFactor));
    const float dy =
      dirPlaneY.Dot3((posOnPlan - GetContext().mModel.v.position) *
                     (1.f / GetContext().mScreenFactor));
    if (belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
        dy >= quadUV[1] && dy <= quadUV[3] &&
        Contains(op, TRANSLATE_PLANS[i])) {
      type = MT_MOVE_YZ + i;
    }

    if (gizmoHitProportion) {
      *gizmoHitProportion = makeVect(dx, dy, 0.f);
    }
  }
  return type;
}

// return true if the cursor is over the operation's gizmo
bool
IsOver(OPERATION op)
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
  if (Intersects(op, TRANSLATE) && GetMoveType(op, NULL) != MT_NONE) {
    return true;
  }
  return false;
}

// return true if mouse cursor is over any gizmo control (axis, plan or screen
// component)
bool
IsOver();

// enable/disable the gizmo. Stay in the state until next call to Enable.
// gizmo is rendered with gray half transparent color when disabled
void
Enable(bool enable);

// default is false
void
SetOrthographic(bool isOrthographic);

// Render a cube with face color corresponding to face normal. Usefull for
// debug/tests
void
DrawCubes(const float* view,
          const float* projection,
          const float* matrices,
          int matrixCount);
void
DrawGrid(const float* view,
         const float* projection,
         const float* matrix,
         const float gridSize);

//
// Please note that this cubeview is patented by Autodesk :
// https://patents.google.com/patent/US7782319B2/en It seems to be a defensive
// patent in the US. I don't think it will bring troubles using it as other
// software are using the same mechanics. But just in case, you are now warned!
//
void
ViewManipulate(float* view,
               float length,
               ImVec2 position,
               ImVec2 size,
               ImU32 backgroundColor);

// use this version if you did not call Manipulate before and you are just using
// ViewManipulate
void
ViewManipulate(float* view,
               const float* projection,
               OPERATION operation,
               MODE mode,
               float* matrix,
               float length,
               ImVec2 position,
               ImVec2 size,
               ImU32 backgroundColor);

void
SetGizmoSizeClipSpace(float value);

// Allow axis to flip
// When true (default), the guizmo axis flip for better visibility
// When false, they always stay along the positive world/local axis
void
AllowAxisFlip(bool value);

Style&
GetStyle();

// helper functions for manualy editing translation/rotation/scale with an input
// float translation, rotation and scale float points to 3 floats each Angles
// are in degrees (more suitable for human editing) example: float
// matrixTranslation[3], matrixRotation[3], matrixScale[3];
// ImGuizmo::DecomposeMatrixToComponents(gizmoMatrix.m16, matrixTranslation,
// matrixRotation, matrixScale); ImGui::InputFloat3("Tr", matrixTranslation, 3);
// ImGui::InputFloat3("Rt", matrixRotation, 3);
// ImGui::InputFloat3("Sc", matrixScale, 3);
// ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation,
// matrixScale, gizmoMatrix.m16);
//
// These functions have some numerical stability issues for now. Use with
// caution.
void
DecomposeMatrixToComponents(const float* matrix,
                            float* translation,
                            float* rotation,
                            float* scale);
void
RecomposeMatrixFromComponents(const float* translation,
                              const float* rotation,
                              const float* scale,
                              float* matrix);

const float screenRotateSize = 0.06f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

static bool
IsTranslateType(int type)
{
  return type >= MT_MOVE_X && type <= MT_MOVE_SCREEN;
}

static bool
IsRotateType(int type)
{
  return type >= MT_ROTATE_X && type <= MT_ROTATE_SCREEN;
}

static bool
IsScaleType(int type)
{
  return type >= MT_SCALE_X && type <= MT_SCALE_XYZ;
}

// Matches MT_MOVE_AB order
static const char* translationInfoMask[] = { "X : %5.3f",
                                             "Y : %5.3f",
                                             "Z : %5.3f",
                                             "Y : %5.3f Z : %5.3f",
                                             "X : %5.3f Z : %5.3f",
                                             "X : %5.3f Y : %5.3f",
                                             "X : %5.3f Y : %5.3f Z : %5.3f" };
static const char* scaleInfoMask[] = { "X : %5.2f",
                                       "Y : %5.2f",
                                       "Z : %5.2f",
                                       "XYZ : %5.2f" };
static const char* rotationInfoMask[] = { "X : %5.2f deg %5.2f rad",
                                          "Y : %5.2f deg %5.2f rad",
                                          "Z : %5.2f deg %5.2f rad",
                                          "Screen : %5.2f deg %5.2f rad" };
static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };
static const int HALF_CIRCLE_SEGMENT_COUNT = 64;
static const float snapTension = 0.5f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

Style&
GetStyle()
{
  return GetContext().mStyle;
}

static ImU32
GetColorU32(int idx)
{
  IM_ASSERT(idx < COLOR::COUNT);
  return ImGui::ColorConvertFloat4ToU32(
    *((ImVec4*)&GetContext().mStyle.Colors[idx]));
}

static void
ComputeCameraRay(vec_t& rayOrigin,
                 vec_t& rayDir,
                 ImVec2 position = ImVec2(GetContext().mX, GetContext().mY),
                 ImVec2 size = ImVec2(GetContext().mWidth,
                                      GetContext().mHeight))
{
  ImGuiIO& io = ImGui::GetIO();

  matrix_t mViewProjInverse;
  mViewProjInverse.Inverse(GetContext().mViewMat * GetContext().mProjectionMat);

  const float mox = ((io.MousePos.x - position.x) / size.x) * 2.f - 1.f;
  const float moy = (1.f - ((io.MousePos.y - position.y) / size.y)) * 2.f - 1.f;

  const float zNear = GetContext().mReversed ? (1.f - FLT_EPSILON) : 0.f;
  const float zFar = GetContext().mReversed ? 0.f : (1.f - FLT_EPSILON);

  rayOrigin.Transform(makeVect(mox, moy, zNear, 1.f), mViewProjInverse);
  rayOrigin *= 1.f / rayOrigin.w;
  vec_t rayEnd;
  rayEnd.Transform(makeVect(mox, moy, zFar, 1.f), mViewProjInverse);
  rayEnd *= 1.f / rayEnd.w;
  rayDir = Normalized(rayEnd - rayOrigin);
}

static float
DistanceToPlane(const vec_t& point, const vec_t& plan)
{
  return plan.Dot3(point) + plan.w;
}

static bool
IsInContextRect(ImVec2 p)
{
  return IsWithin(p.x, GetContext().mX, GetContext().mXMax) &&
         IsWithin(p.y, GetContext().mY, GetContext().mYMax);
}

static bool
IsHoveringWindow()
{
  ImGuiContext& g = *ImGui::GetCurrentContext();
  ImGuiWindow* window =
    ImGui::FindWindowByName(GetContext().mDrawList->_OwnerName);
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

void
SetDrawlist(ImDrawList* drawlist)
{
  GetContext().mDrawList = drawlist ? drawlist : ImGui::GetWindowDrawList();
}

void
SetRect(float x, float y, float width, float height)
{
  SetDrawlist(nullptr);
  GetContext().mX = x;
  GetContext().mY = y;
  GetContext().mWidth = width;
  GetContext().mHeight = height;
  GetContext().mXMax = GetContext().mX + GetContext().mWidth;
  GetContext().mYMax = GetContext().mY + GetContext().mXMax;
  GetContext().mDisplayRatio = width / height;
}

void
SetOrthographic(bool isOrthographic)
{
  GetContext().mIsOrthographic = isOrthographic;
}

bool
IsOver()
{
  return (Intersects(GetContext().mOperation, TRANSLATE) &&
          GetMoveType(GetContext().mOperation, NULL) != MT_NONE) ||
         (Intersects(GetContext().mOperation, ROTATE) &&
          GetRotateType(GetContext().mOperation) != MT_NONE) ||
         (Intersects(GetContext().mOperation, SCALE) &&
          GetScaleType(GetContext().mOperation) != MT_NONE) ||
         IsUsing();
}

void
Enable(bool enable)
{
  GetContext().mbEnable = enable;
  if (!enable) {
    GetContext().mbUsing = false;
    GetContext().mbUsingBounds = false;
  }
}

void
Context::ComputeContext(const float* view,
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
  this->mScreenSquareMin = ImVec2(centerSSpace.x - 10.f, centerSSpace.y - 10.f);
  this->mScreenSquareMax = ImVec2(centerSSpace.x + 10.f, centerSSpace.y + 10.f);

  ComputeCameraRay(this->mRayOrigin, this->mRayVector);
}

static void
ComputeColors(ImU32* colors, int type, OPERATION operation)
{
  if (GetContext().mbEnable) {
    ImU32 selectionColor = GetColorU32(SELECTION);

    switch (operation) {
      case TRANSLATE:
        colors[0] = (type == MT_MOVE_SCREEN) ? selectionColor : IM_COL32_WHITE;
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
      // note: this internal function is only called with three possible values
      // for operation
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

static void
ComputeSnap(float* value, float snap)
{
  if (snap <= FLT_EPSILON) {
    return;
  }

  float modulo = fmodf(*value, snap);
  float moduloRatio = fabsf(modulo) / snap;
  if (moduloRatio < snapTension) {
    *value -= modulo;
  } else if (moduloRatio > (1.f - snapTension)) {
    *value = *value - modulo + snap * ((*value < 0.f) ? -1.f : 1.f);
  }
}
static void
ComputeSnap(vec_t& value, const float* snap)
{
  for (int i = 0; i < 3; i++) {
    ComputeSnap(&value[i], snap[i]);
  }
}

static float
ComputeAngleOnPlan()
{
  const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                      GetContext().mRayVector,
                                      GetContext().mTranslationPlan);
  vec_t localPos =
    Normalized(GetContext().mRayOrigin + GetContext().mRayVector * len -
               GetContext().mModel.v.position);

  vec_t perpendicularVector;
  perpendicularVector.Cross(GetContext().mRotationVectorSource,
                            GetContext().mTranslationPlan);
  perpendicularVector.Normalize();
  float acosAngle =
    Clamp(Dot(localPos, GetContext().mRotationVectorSource), -1.f, 1.f);
  float angle = acosf(acosAngle);
  angle *= (Dot(localPos, perpendicularVector) < 0.f) ? 1.f : -1.f;
  return angle;
}

static void
DrawRotationGizmo(OPERATION op, int type)
{
  if (!Intersects(op, ROTATE)) {
    return;
  }
  ImDrawList* drawList = GetContext().mDrawList;

  // colors
  ImU32 colors[7];
  ComputeColors(colors, type, ROTATE);

  vec_t cameraToModelNormalized;
  if (GetContext().mIsOrthographic) {
    matrix_t viewInverse;
    viewInverse.Inverse(*(matrix_t*)&GetContext().mViewMat);
    cameraToModelNormalized = -viewInverse.v.dir;
  } else {
    cameraToModelNormalized =
      Normalized(GetContext().mModel.v.position - GetContext().mCameraEye);
  }

  cameraToModelNormalized.TransformVector(GetContext().mModelInverse);

  GetContext().mRadiusSquareCenter = screenRotateSize * GetContext().mHeight;

  bool hasRSC = Intersects(op, ROTATE_SCREEN);
  for (int axis = 0; axis < 3; axis++) {
    if (!Intersects(op, static_cast<OPERATION>(ROTATE_Z >> axis))) {
      continue;
    }
    const bool usingAxis = (GetContext().mbUsing && type == MT_ROTATE_Z - axis);
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
                  GetContext().mScreenFactor * ROTATION_DISPLAY_FACTOR;
      circlePos[i] = worldToPos(pos, GetContext().mMVP);
    }
    if (!GetContext().mbUsing || usingAxis) {
      drawList->AddPolyline(circlePos,
                            circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                            colors[3 - axis],
                            false,
                            GetContext().mStyle.RotationLineThickness);
    }

    float radiusAxis = sqrtf((ImLengthSqr(
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection) -
      circlePos[0])));
    if (radiusAxis > GetContext().mRadiusSquareCenter) {
      GetContext().mRadiusSquareCenter = radiusAxis;
    }
  }
  if (hasRSC && (!GetContext().mbUsing || type == MT_ROTATE_SCREEN)) {
    drawList->AddCircle(
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection),
      GetContext().mRadiusSquareCenter,
      colors[0],
      64,
      GetContext().mStyle.RotationOuterLineThickness);
  }

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsRotateType(type)) {
    ImVec2 circlePos[HALF_CIRCLE_SEGMENT_COUNT + 1];

    circlePos[0] =
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection);
    for (unsigned int i = 1; i < HALF_CIRCLE_SEGMENT_COUNT; i++) {
      float ng = GetContext().mRotationAngle *
                 ((float)(i - 1) / (float)(HALF_CIRCLE_SEGMENT_COUNT - 1));
      matrix_t rotateVectorMatrix;
      rotateVectorMatrix.RotationAxis(GetContext().mTranslationPlan, ng);
      vec_t pos;
      pos.TransformPoint(GetContext().mRotationVectorSource,
                         rotateVectorMatrix);
      pos *= GetContext().mScreenFactor * ROTATION_DISPLAY_FACTOR;
      circlePos[i] = worldToPos(pos + GetContext().mModel.v.position,
                                GetContext().mViewProjection);
    }
    drawList->AddConvexPolyFilled(
      circlePos, HALF_CIRCLE_SEGMENT_COUNT, GetColorU32(ROTATION_USING_FILL));
    drawList->AddPolyline(circlePos,
                          HALF_CIRCLE_SEGMENT_COUNT,
                          GetColorU32(ROTATION_USING_BORDER),
                          true,
                          GetContext().mStyle.RotationLineThickness);

    ImVec2 destinationPosOnScreen = circlePos[1];
    char tmps[512];
    ImFormatString(tmps,
                   sizeof(tmps),
                   rotationInfoMask[type - MT_ROTATE_X],
                   (GetContext().mRotationAngle / std::numbers::pi) * 180.f,
                   GetContext().mRotationAngle);
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

static void
DrawHatchedAxis(const vec_t& axis)
{
  if (GetContext().mStyle.HatchedAxisLineThickness <= 0.0f) {
    return;
  }

  for (int j = 1; j < 10; j++) {
    ImVec2 baseSSpace2 =
      worldToPos(axis * 0.05f * (float)(j * 2) * GetContext().mScreenFactor,
                 GetContext().mMVP);
    ImVec2 worldDirSSpace2 =
      worldToPos(axis * 0.05f * (float)(j * 2 + 1) * GetContext().mScreenFactor,
                 GetContext().mMVP);
    GetContext().mDrawList->AddLine(
      baseSSpace2,
      worldDirSSpace2,
      GetColorU32(HATCHED_AXIS_LINES),
      GetContext().mStyle.HatchedAxisLineThickness);
  }
}

static void
DrawScaleGizmo(OPERATION op, int type)
{
  ImDrawList* drawList = GetContext().mDrawList;

  if (!Intersects(op, SCALE)) {
    return;
  }

  // colors
  ImU32 colors[7];
  ComputeColors(colors, type, SCALE);

  // draw
  vec_t scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID)) {
    scaleDisplay = GetContext().mScale;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(op, static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }
    const bool usingAxis = (GetContext().mbUsing && type == MT_SCALE_X + i);
    if (!GetContext().mbUsing || usingAxis) {
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
        ImVec2 baseSSpace = worldToPos(
          dirAxis * 0.1f * GetContext().mScreenFactor, GetContext().mMVP);
        ImVec2 worldDirSSpaceNoScale =
          worldToPos(dirAxis * markerScale * GetContext().mScreenFactor,
                     GetContext().mMVP);
        ImVec2 worldDirSSpace =
          worldToPos((dirAxis * markerScale * scaleDisplay[i]) *
                       GetContext().mScreenFactor,
                     GetContext().mMVP);

        if (GetContext().mbUsing &&
            (GetContext().mActualID == -1 ||
             GetContext().mActualID == GetContext().mEditingID)) {
          ImU32 scaleLineColor = GetColorU32(SCALE_LINE);
          drawList->AddLine(baseSSpace,
                            worldDirSSpaceNoScale,
                            scaleLineColor,
                            GetContext().mStyle.ScaleLineThickness);
          drawList->AddCircleFilled(worldDirSSpaceNoScale,
                                    GetContext().mStyle.ScaleLineCircleSize,
                                    scaleLineColor);
        }

        if (!hasTranslateOnAxis || GetContext().mbUsing) {
          drawList->AddLine(baseSSpace,
                            worldDirSSpace,
                            colors[i + 1],
                            GetContext().mStyle.ScaleLineThickness);
        }
        drawList->AddCircleFilled(worldDirSSpace,
                                  GetContext().mStyle.ScaleLineCircleSize,
                                  colors[i + 1]);

        if (GetContext().mAxisFactor[i] < 0.f) {
          DrawHatchedAxis(dirAxis * scaleDisplay[i]);
        }
      }
    }
  }

  // draw screen cirle
  drawList->AddCircleFilled(GetContext().mScreenSquareCenter,
                            GetContext().mStyle.CenterCircleSize,
                            colors[0],
                            32);

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsScaleType(type)) {
    // ImVec2 sourcePosOnScreen = worldToPos(GetContext().mMatrixOrigin,
    // GetContext().mViewProjection);
    ImVec2 destinationPosOnScreen =
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection);
    /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
    destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
    *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
    drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
    drawList->AddLine(ImVec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y +
    dif.y), ImVec2(destinationPosOnScreen.x - dif.x, destinationPosOnScreen.y -
    dif.y), translationLineColor, 2.f);
    */
    char tmps[512];
    // vec_t deltaInfo = GetContext().mModel.v.position -
    // GetContext().mMatrixOrigin;
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

static void
DrawScaleUniveralGizmo(OPERATION op, int type)
{
  ImDrawList* drawList = GetContext().mDrawList;

  if (!Intersects(op, SCALEU)) {
    return;
  }

  // colors
  ImU32 colors[7];
  ComputeColors(colors, type, SCALEU);

  // draw
  vec_t scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID)) {
    scaleDisplay = GetContext().mScale;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(op, static_cast<OPERATION>(SCALE_XU << i))) {
      continue;
    }
    const bool usingAxis = (GetContext().mbUsing && type == MT_SCALE_X + i);
    if (!GetContext().mbUsing || usingAxis) {
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
        // GetContext().mScreenFactor, GetContext().mMVPLocal); ImVec2
        // worldDirSSpaceNoScale = worldToPos(dirAxis * markerScale *
        // GetContext().mScreenFactor, GetContext().mMVP);
        ImVec2 worldDirSSpace =
          worldToPos((dirAxis * markerScale * scaleDisplay[i]) *
                       GetContext().mScreenFactor,
                     GetContext().mMVPLocal);

#if 0
               if (GetContext().mbUsing && (GetContext().mActualID == -1 || GetContext().mActualID == GetContext().mEditingID))
               {
                  drawList->AddLine(baseSSpace, worldDirSSpaceNoScale, IM_COL32(0x40, 0x40, 0x40, 0xFF), 3.f);
                  drawList->AddCircleFilled(worldDirSSpaceNoScale, 6.f, IM_COL32(0x40, 0x40, 0x40, 0xFF));
               }
               /*
               if (!hasTranslateOnAxis || GetContext().mbUsing)
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
  drawList->AddCircle(GetContext().mScreenSquareCenter,
                      20.f,
                      colors[0],
                      32,
                      GetContext().mStyle.CenterCircleSize);

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsScaleType(type)) {
    // ImVec2 sourcePosOnScreen = worldToPos(GetContext().mMatrixOrigin,
    // GetContext().mViewProjection);
    ImVec2 destinationPosOnScreen =
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection);
    /*vec_t dif(destinationPosOnScreen.x - sourcePosOnScreen.x,
    destinationPosOnScreen.y - sourcePosOnScreen.y); dif.Normalize(); dif
    *= 5.f; drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
    drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
    drawList->AddLine(ImVec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y +
    dif.y), ImVec2(destinationPosOnScreen.x - dif.x, destinationPosOnScreen.y -
    dif.y), translationLineColor, 2.f);
    */
    char tmps[512];
    // vec_t deltaInfo = GetContext().mModel.v.position -
    // GetContext().mMatrixOrigin;
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

static void
DrawTranslationGizmo(OPERATION op, int type)
{
  ImDrawList* drawList = GetContext().mDrawList;
  if (!drawList) {
    return;
  }

  if (!Intersects(op, TRANSLATE)) {
    return;
  }

  // colors
  ImU32 colors[7];
  ComputeColors(colors, type, TRANSLATE);

  const ImVec2 origin =
    worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection);

  // draw
  bool belowAxisLimit = false;
  bool belowPlaneLimit = false;
  for (int i = 0; i < 3; ++i) {
    vec_t dirPlaneX, dirPlaneY, dirAxis;
    ComputeTripodAxisAndVisibility(
      i, dirAxis, dirPlaneX, dirPlaneY, belowAxisLimit, belowPlaneLimit);

    if (!GetContext().mbUsing ||
        (GetContext().mbUsing && type == MT_MOVE_X + i)) {
      // draw axis
      if (belowAxisLimit &&
          Intersects(op, static_cast<OPERATION>(TRANSLATE_X << i))) {
        ImVec2 baseSSpace = worldToPos(
          dirAxis * 0.1f * GetContext().mScreenFactor, GetContext().mMVP);
        ImVec2 worldDirSSpace =
          worldToPos(dirAxis * GetContext().mScreenFactor, GetContext().mMVP);

        drawList->AddLine(baseSSpace,
                          worldDirSSpace,
                          colors[i + 1],
                          GetContext().mStyle.TranslationLineThickness);

        // Arrow head begin
        ImVec2 dir(origin - worldDirSSpace);

        float d = sqrtf(ImLengthSqr(dir));
        dir /= d; // Normalize
        dir *= GetContext().mStyle.TranslationLineArrowSize;

        ImVec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
        ImVec2 a(worldDirSSpace + dir);
        drawList->AddTriangleFilled(worldDirSSpace - dir,
                                    a + ortogonalDir,
                                    a - ortogonalDir,
                                    colors[i + 1]);
        // Arrow head end

        if (GetContext().mAxisFactor[i] < 0.f) {
          DrawHatchedAxis(dirAxis);
        }
      }
    }
    // draw plane
    if (!GetContext().mbUsing ||
        (GetContext().mbUsing && type == MT_MOVE_YZ + i)) {
      if (belowPlaneLimit && Contains(op, TRANSLATE_PLANS[i])) {
        ImVec2 screenQuadPts[4];
        for (int j = 0; j < 4; ++j) {
          vec_t cornerWorldPos =
            (dirPlaneX * quadUV[j * 2] + dirPlaneY * quadUV[j * 2 + 1]) *
            GetContext().mScreenFactor;
          screenQuadPts[j] = worldToPos(cornerWorldPos, GetContext().mMVP);
        }
        drawList->AddPolyline(
          screenQuadPts, 4, GetColorU32(DIRECTION_X + i), true, 1.0f);
        drawList->AddConvexPolyFilled(screenQuadPts, 4, colors[i + 4]);
      }
    }
  }

  drawList->AddCircleFilled(GetContext().mScreenSquareCenter,
                            GetContext().mStyle.CenterCircleSize,
                            colors[0],
                            32);

  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsTranslateType(type)) {
    ImU32 translationLineColor = GetColorU32(TRANSLATION_LINE);

    ImVec2 sourcePosOnScreen =
      worldToPos(GetContext().mMatrixOrigin, GetContext().mViewProjection);
    ImVec2 destinationPosOnScreen =
      worldToPos(GetContext().mModel.v.position, GetContext().mViewProjection);
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
    vec_t deltaInfo =
      GetContext().mModel.v.position - GetContext().mMatrixOrigin;
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

static bool
CanActivate()
{
  if (ImGui::IsMouseClicked(0)) {
    if (GetContext().mAllowActiveHoverItem) {
      return true;
    } else {
      if (!ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) {
        return true;
      }
    }
  }
  return false;
}

static void
HandleAndDrawLocalBounds(const float* bounds,
                         matrix_t* matrix,
                         const float* snapValues,
                         OPERATION operation)
{
  ImGuiIO& io = ImGui::GetIO();
  ImDrawList* drawList = GetContext().mDrawList;

  // compute best projection axis
  vec_t axesWorldDirections[3];
  vec_t bestAxisWorldDirection = { 0.0f, 0.0f, 0.0f, 0.0f };
  int axes[3];
  unsigned int numAxes = 1;
  axes[0] = GetContext().mBoundsBestAxis;
  int bestAxis = axes[0];
  if (!GetContext().mbUsingBounds) {
    numAxes = 0;
    float bestDot = 0.f;
    for (int i = 0; i < 3; i++) {
      vec_t dirPlaneNormalWorld;
      dirPlaneNormalWorld.TransformVector(directionUnary[i],
                                          GetContext().mModelSource);
      dirPlaneNormalWorld.Normalize();

      float dt = fabsf(Dot(Normalized(GetContext().mCameraEye -
                                      GetContext().mModelSource.v.position),
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
      GetContext().mbEnable ? IM_COL32_BLACK : IM_COL32(0, 0, 0, 0x80);

    matrix_t boundsMVP =
      GetContext().mModelSource * GetContext().mViewProjection;
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
        ImVec2 worldBoundSS1 = ImLerp(worldBound1, worldBound2, ImVec2(t1, t1));
        ImVec2 worldBoundSS2 = ImLerp(worldBound1, worldBound2, ImVec2(t2, t2));
        // drawList->AddLine(worldBoundSS1, worldBoundSS2, IM_COL32(0, 0, 0, 0)
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
        type = GetMoveType(operation, &gizmoHitProportion);
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
      if (!GetContext().mbUsingBounds && GetContext().mbEnable &&
          overBigAnchor && CanActivate()) {
        GetContext().mBoundsPivot.TransformPoint(aabb[(i + 2) % 4],
                                                 GetContext().mModelSource);
        GetContext().mBoundsAnchor.TransformPoint(aabb[i],
                                                  GetContext().mModelSource);
        GetContext().mBoundsPlan =
          BuildPlan(GetContext().mBoundsAnchor, bestAxisWorldDirection);
        GetContext().mBoundsBestAxis = bestAxis;
        GetContext().mBoundsAxis[0] = secondAxis;
        GetContext().mBoundsAxis[1] = thirdAxis;

        GetContext().mBoundsLocalPivot.Set(0.f);
        GetContext().mBoundsLocalPivot[secondAxis] =
          aabb[oppositeIndex][secondAxis];
        GetContext().mBoundsLocalPivot[thirdAxis] =
          aabb[oppositeIndex][thirdAxis];

        GetContext().mbUsingBounds = true;
        GetContext().mEditingID = GetContext().mActualID;
        GetContext().mBoundsMatrix = GetContext().mModelSource;
      }
      // small anchor on middle of segment
      if (!GetContext().mbUsingBounds && GetContext().mbEnable &&
          overSmallAnchor && CanActivate()) {
        vec_t midPointOpposite = (aabb[(i + 2) % 4] + aabb[(i + 3) % 4]) * 0.5f;
        GetContext().mBoundsPivot.TransformPoint(midPointOpposite,
                                                 GetContext().mModelSource);
        GetContext().mBoundsAnchor.TransformPoint(midPoint,
                                                  GetContext().mModelSource);
        GetContext().mBoundsPlan =
          BuildPlan(GetContext().mBoundsAnchor, bestAxisWorldDirection);
        GetContext().mBoundsBestAxis = bestAxis;
        int indices[] = { secondAxis, thirdAxis };
        GetContext().mBoundsAxis[0] = indices[i % 2];
        GetContext().mBoundsAxis[1] = -1;

        GetContext().mBoundsLocalPivot.Set(0.f);
        GetContext().mBoundsLocalPivot[GetContext().mBoundsAxis[0]] =
          aabb[oppositeIndex]
              [indices[i % 2]]; // bounds[GetContext().mBoundsAxis[0]] * (((i +
                                // 1) & 2) ? 1.f : -1.f);

        GetContext().mbUsingBounds = true;
        GetContext().mEditingID = GetContext().mActualID;
        GetContext().mBoundsMatrix = GetContext().mModelSource;
      }
    }

    if (GetContext().mbUsingBounds &&
        (GetContext().mActualID == -1 ||
         GetContext().mActualID == GetContext().mEditingID)) {
      matrix_t scale;
      scale.SetToIdentity();

      // compute projected mouse position on plan
      const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                          GetContext().mRayVector,
                                          GetContext().mBoundsPlan);
      vec_t newPos = GetContext().mRayOrigin + GetContext().mRayVector * len;

      // compute a reference and delta vectors base on mouse move
      vec_t deltaVector = (newPos - GetContext().mBoundsPivot).Abs();
      vec_t referenceVector =
        (GetContext().mBoundsAnchor - GetContext().mBoundsPivot).Abs();

      // for 1 or 2 axes, compute a ratio that's used for scale and snap it
      // based on resulting length
      for (int i = 0; i < 2; i++) {
        int axisIndex1 = GetContext().mBoundsAxis[i];
        if (axisIndex1 == -1) {
          continue;
        }

        float ratioAxis = 1.f;
        vec_t axisDir = GetContext().mBoundsMatrix.component[axisIndex1].Abs();

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
      preScale.Translation(-GetContext().mBoundsLocalPivot);
      postScale.Translation(GetContext().mBoundsLocalPivot);
      matrix_t res = preScale * scale * postScale * GetContext().mBoundsMatrix;
      *matrix = res;

      // info text
      char tmps[512];
      ImVec2 destinationPosOnScreen = worldToPos(GetContext().mModel.v.position,
                                                 GetContext().mViewProjection);
      ImFormatString(tmps,
                     sizeof(tmps),
                     "X: %.2f Y: %.2f Z: %.2f",
                     (bounds[3] - bounds[0]) *
                       GetContext().mBoundsMatrix.component[0].Length() *
                       scale.component[0].Length(),
                     (bounds[4] - bounds[1]) *
                       GetContext().mBoundsMatrix.component[1].Length() *
                       scale.component[1].Length(),
                     (bounds[5] - bounds[2]) *
                       GetContext().mBoundsMatrix.component[2].Length() *
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
      GetContext().mbUsingBounds = false;
      GetContext().mEditingID = -1;
    }
    if (GetContext().mbUsingBounds) {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

static bool
HandleTranslation(float* matrix,
                  float* deltaMatrix,
                  OPERATION op,
                  int& type,
                  const float* snap)
{
  if (!Intersects(op, TRANSLATE) || type != MT_NONE) {
    return false;
  }
  const ImGuiIO& io = ImGui::GetIO();
  const bool applyRotationLocaly =
    GetContext().mMode == LOCAL || type == MT_MOVE_SCREEN;
  bool modified = false;

  // move
  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsTranslateType(GetContext().mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
    ImGui::SetNextFrameWantCaptureMouse(true);
#else
    ImGui::CaptureMouseFromApp();
#endif
    const float signedLength = IntersectRayPlane(GetContext().mRayOrigin,
                                                 GetContext().mRayVector,
                                                 GetContext().mTranslationPlan);
    const float len = fabsf(signedLength); // near plan
    const vec_t newPos =
      GetContext().mRayOrigin + GetContext().mRayVector * len;

    // compute delta
    const vec_t newOrigin =
      newPos - GetContext().mRelativeOrigin * GetContext().mScreenFactor;
    vec_t delta = newOrigin - GetContext().mModel.v.position;

    // 1 axis constraint
    if (GetContext().mCurrentOperation >= MT_MOVE_X &&
        GetContext().mCurrentOperation <= MT_MOVE_Z) {
      const int axisIndex = GetContext().mCurrentOperation - MT_MOVE_X;
      const vec_t& axisValue = *(vec_t*)&GetContext().mModel.m[axisIndex];
      const float lengthOnAxis = Dot(axisValue, delta);
      delta = axisValue * lengthOnAxis;
    }

    // snap
    if (snap) {
      vec_t cumulativeDelta =
        GetContext().mModel.v.position + delta - GetContext().mMatrixOrigin;
      if (applyRotationLocaly) {
        matrix_t modelSourceNormalized = GetContext().mModelSource;
        modelSourceNormalized.OrthoNormalize();
        matrix_t modelSourceNormalizedInverse;
        modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
        cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
        ComputeSnap(cumulativeDelta, snap);
        cumulativeDelta.TransformVector(modelSourceNormalized);
      } else {
        ComputeSnap(cumulativeDelta, snap);
      }
      delta = GetContext().mMatrixOrigin + cumulativeDelta -
              GetContext().mModel.v.position;
    }

    if (delta != GetContext().mTranslationLastDelta) {
      modified = true;
    }
    GetContext().mTranslationLastDelta = delta;

    // compute matrix & delta
    matrix_t deltaMatrixTranslation;
    deltaMatrixTranslation.Translation(delta);
    if (deltaMatrix) {
      memcpy(deltaMatrix, deltaMatrixTranslation.m16, sizeof(float) * 16);
    }

    const matrix_t res = GetContext().mModelSource * deltaMatrixTranslation;
    *(matrix_t*)matrix = res;

    if (!io.MouseDown[0]) {
      GetContext().mbUsing = false;
    }

    type = GetContext().mCurrentOperation;
  } else {
    // find new possible way to move
    vec_t gizmoHitProportion;
    type = GetMoveType(op, &gizmoHitProportion);
    if (type != MT_NONE) {
#if IMGUI_VERSION_NUM >= 18723
      ImGui::SetNextFrameWantCaptureMouse(true);
#else
      ImGui::CaptureMouseFromApp();
#endif
    }
    if (CanActivate() && type != MT_NONE) {
      GetContext().mbUsing = true;
      GetContext().mEditingID = GetContext().mActualID;
      GetContext().mCurrentOperation = type;
      vec_t movePlanNormal[] = {
        GetContext().mModel.v.right, GetContext().mModel.v.up,
        GetContext().mModel.v.dir,   GetContext().mModel.v.right,
        GetContext().mModel.v.up,    GetContext().mModel.v.dir,
        -GetContext().mCameraDir
      };

      vec_t cameraToModelNormalized =
        Normalized(GetContext().mModel.v.position - GetContext().mCameraEye);
      for (unsigned int i = 0; i < 3; i++) {
        vec_t orthoVector = Cross(movePlanNormal[i], cameraToModelNormalized);
        movePlanNormal[i].Cross(orthoVector);
        movePlanNormal[i].Normalize();
      }
      // pickup plan
      GetContext().mTranslationPlan = BuildPlan(
        GetContext().mModel.v.position, movePlanNormal[type - MT_MOVE_X]);
      const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                          GetContext().mRayVector,
                                          GetContext().mTranslationPlan);
      GetContext().mTranslationPlanOrigin =
        GetContext().mRayOrigin + GetContext().mRayVector * len;
      GetContext().mMatrixOrigin = GetContext().mModel.v.position;

      GetContext().mRelativeOrigin =
        (GetContext().mTranslationPlanOrigin - GetContext().mModel.v.position) *
        (1.f / GetContext().mScreenFactor);
    }
  }
  return modified;
}

static bool
HandleScale(float* matrix,
            float* deltaMatrix,
            OPERATION op,
            int& type,
            const float* snap)
{
  if ((!Intersects(op, SCALE) && !Intersects(op, SCALEU)) || type != MT_NONE ||
      !GetContext().mbMouseOver) {
    return false;
  }
  ImGuiIO& io = ImGui::GetIO();
  bool modified = false;

  if (!GetContext().mbUsing) {
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
      GetContext().mbUsing = true;
      GetContext().mEditingID = GetContext().mActualID;
      GetContext().mCurrentOperation = type;
      const vec_t movePlanNormal[] = {
        GetContext().mModel.v.up,    GetContext().mModel.v.dir,
        GetContext().mModel.v.right, GetContext().mModel.v.dir,
        GetContext().mModel.v.up,    GetContext().mModel.v.right,
        -GetContext().mCameraDir
      };
      // pickup plan

      GetContext().mTranslationPlan = BuildPlan(
        GetContext().mModel.v.position, movePlanNormal[type - MT_SCALE_X]);
      const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                          GetContext().mRayVector,
                                          GetContext().mTranslationPlan);
      GetContext().mTranslationPlanOrigin =
        GetContext().mRayOrigin + GetContext().mRayVector * len;
      GetContext().mMatrixOrigin = GetContext().mModel.v.position;
      GetContext().mScale.Set(1.f, 1.f, 1.f);
      GetContext().mRelativeOrigin =
        (GetContext().mTranslationPlanOrigin - GetContext().mModel.v.position) *
        (1.f / GetContext().mScreenFactor);
      GetContext().mScaleValueOrigin =
        makeVect(GetContext().mModelSource.v.right.Length(),
                 GetContext().mModelSource.v.up.Length(),
                 GetContext().mModelSource.v.dir.Length());
      GetContext().mSaveMousePosx = io.MousePos.x;
    }
  }
  // scale
  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsScaleType(GetContext().mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
    ImGui::SetNextFrameWantCaptureMouse(true);
#else
    ImGui::CaptureMouseFromApp();
#endif
    const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                        GetContext().mRayVector,
                                        GetContext().mTranslationPlan);
    vec_t newPos = GetContext().mRayOrigin + GetContext().mRayVector * len;
    vec_t newOrigin =
      newPos - GetContext().mRelativeOrigin * GetContext().mScreenFactor;
    vec_t delta = newOrigin - GetContext().mModelLocal.v.position;

    // 1 axis constraint
    if (GetContext().mCurrentOperation >= MT_SCALE_X &&
        GetContext().mCurrentOperation <= MT_SCALE_Z) {
      int axisIndex = GetContext().mCurrentOperation - MT_SCALE_X;
      const vec_t& axisValue = *(vec_t*)&GetContext().mModelLocal.m[axisIndex];
      float lengthOnAxis = Dot(axisValue, delta);
      delta = axisValue * lengthOnAxis;

      vec_t baseVector = GetContext().mTranslationPlanOrigin -
                         GetContext().mModelLocal.v.position;
      float ratio =
        Dot(axisValue, baseVector + delta) / Dot(axisValue, baseVector);

      GetContext().mScale[axisIndex] = max(ratio, 0.001f);
    } else {
      float scaleDelta = (io.MousePos.x - GetContext().mSaveMousePosx) * 0.01f;
      GetContext().mScale.Set(max(1.f + scaleDelta, 0.001f));
    }

    // snap
    if (snap) {
      float scaleSnap[] = { snap[0], snap[0], snap[0] };
      ComputeSnap(GetContext().mScale, scaleSnap);
    }

    // no 0 allowed
    for (int i = 0; i < 3; i++)
      GetContext().mScale[i] = max(GetContext().mScale[i], 0.001f);

    if (GetContext().mScaleLast != GetContext().mScale) {
      modified = true;
    }
    GetContext().mScaleLast = GetContext().mScale;

    // compute matrix & delta
    matrix_t deltaMatrixScale;
    deltaMatrixScale.Scale(GetContext().mScale *
                           GetContext().mScaleValueOrigin);

    matrix_t res = deltaMatrixScale * GetContext().mModelLocal;
    *(matrix_t*)matrix = res;

    if (deltaMatrix) {
      vec_t deltaScale = GetContext().mScale * GetContext().mScaleValueOrigin;

      vec_t originalScaleDivider;
      originalScaleDivider.x = 1 / GetContext().mModelScaleOrigin.x;
      originalScaleDivider.y = 1 / GetContext().mModelScaleOrigin.y;
      originalScaleDivider.z = 1 / GetContext().mModelScaleOrigin.z;

      deltaScale = deltaScale * originalScaleDivider;

      deltaMatrixScale.Scale(deltaScale);
      memcpy(deltaMatrix, deltaMatrixScale.m16, sizeof(float) * 16);
    }

    if (!io.MouseDown[0]) {
      GetContext().mbUsing = false;
      GetContext().mScale.Set(1.f, 1.f, 1.f);
    }

    type = GetContext().mCurrentOperation;
  }
  return modified;
}

static bool
HandleRotation(float* matrix,
               float* deltaMatrix,
               OPERATION op,
               int& type,
               const float* snap)
{
  if (!Intersects(op, ROTATE) || type != MT_NONE || !GetContext().mbMouseOver) {
    return false;
  }
  ImGuiIO& io = ImGui::GetIO();
  bool applyRotationLocaly = GetContext().mMode == LOCAL;
  bool modified = false;

  if (!GetContext().mbUsing) {
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
      GetContext().mbUsing = true;
      GetContext().mEditingID = GetContext().mActualID;
      GetContext().mCurrentOperation = type;
      const vec_t rotatePlanNormal[] = { GetContext().mModel.v.right,
                                         GetContext().mModel.v.up,
                                         GetContext().mModel.v.dir,
                                         -GetContext().mCameraDir };
      // pickup plan
      if (applyRotationLocaly) {
        GetContext().mTranslationPlan = BuildPlan(
          GetContext().mModel.v.position, rotatePlanNormal[type - MT_ROTATE_X]);
      } else {
        GetContext().mTranslationPlan =
          BuildPlan(GetContext().mModelSource.v.position,
                    directionUnary[type - MT_ROTATE_X]);
      }

      const float len = IntersectRayPlane(GetContext().mRayOrigin,
                                          GetContext().mRayVector,
                                          GetContext().mTranslationPlan);
      vec_t localPos = GetContext().mRayOrigin + GetContext().mRayVector * len -
                       GetContext().mModel.v.position;
      GetContext().mRotationVectorSource = Normalized(localPos);
      GetContext().mRotationAngleOrigin = ComputeAngleOnPlan();
    }
  }

  // rotation
  if (GetContext().mbUsing &&
      (GetContext().mActualID == -1 ||
       GetContext().mActualID == GetContext().mEditingID) &&
      IsRotateType(GetContext().mCurrentOperation)) {
#if IMGUI_VERSION_NUM >= 18723
    ImGui::SetNextFrameWantCaptureMouse(true);
#else
    ImGui::CaptureMouseFromApp();
#endif
    GetContext().mRotationAngle = ComputeAngleOnPlan();
    if (snap) {
      float snapInRadian = snap[0] * DEG2RAD;
      ComputeSnap(&GetContext().mRotationAngle, snapInRadian);
    }
    vec_t rotationAxisLocalSpace;

    rotationAxisLocalSpace.TransformVector(
      makeVect(GetContext().mTranslationPlan.x,
               GetContext().mTranslationPlan.y,
               GetContext().mTranslationPlan.z,
               0.f),
      GetContext().mModelInverse);
    rotationAxisLocalSpace.Normalize();

    matrix_t deltaRotation;
    deltaRotation.RotationAxis(rotationAxisLocalSpace,
                               GetContext().mRotationAngle -
                                 GetContext().mRotationAngleOrigin);
    if (GetContext().mRotationAngle != GetContext().mRotationAngleOrigin) {
      modified = true;
    }
    GetContext().mRotationAngleOrigin = GetContext().mRotationAngle;

    matrix_t scaleOrigin;
    scaleOrigin.Scale(GetContext().mModelScaleOrigin);

    if (applyRotationLocaly) {
      *(matrix_t*)matrix =
        scaleOrigin * deltaRotation * GetContext().mModelLocal;
    } else {
      matrix_t res = GetContext().mModelSource;
      res.v.position.Set(0.f);

      *(matrix_t*)matrix = res * deltaRotation;
      ((matrix_t*)matrix)->v.position = GetContext().mModelSource.v.position;
    }

    if (deltaMatrix) {
      *(matrix_t*)deltaMatrix =
        GetContext().mModelInverse * deltaRotation * GetContext().mModel;
    }

    if (!io.MouseDown[0]) {
      GetContext().mbUsing = false;
      GetContext().mEditingID = -1;
    }
    type = GetContext().mCurrentOperation;
  }
  return modified;
}

void
DecomposeMatrixToComponents(const float* matrix,
                            float* translation,
                            float* rotation,
                            float* scale)
{
  matrix_t mat = *(matrix_t*)matrix;

  scale[0] = mat.v.right.Length();
  scale[1] = mat.v.up.Length();
  scale[2] = mat.v.dir.Length();

  mat.OrthoNormalize();

  rotation[0] = RAD2DEG * atan2f(mat.m[1][2], mat.m[2][2]);
  rotation[1] =
    RAD2DEG *
    atan2f(-mat.m[0][2],
           sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2]));
  rotation[2] = RAD2DEG * atan2f(mat.m[0][1], mat.m[0][0]);

  translation[0] = mat.v.position.x;
  translation[1] = mat.v.position.y;
  translation[2] = mat.v.position.z;
}

void
RecomposeMatrixFromComponents(const float* translation,
                              const float* rotation,
                              const float* scale,
                              float* matrix)
{
  matrix_t& mat = *(matrix_t*)matrix;

  matrix_t rot[3];
  for (int i = 0; i < 3; i++) {
    rot[i].RotationAxis(directionUnary[i], rotation[i] * DEG2RAD);
  }

  mat = rot[0] * rot[1] * rot[2];

  float validScale[3];
  for (int i = 0; i < 3; i++) {
    if (fabsf(scale[i]) < FLT_EPSILON) {
      validScale[i] = 0.001f;
    } else {
      validScale[i] = scale[i];
    }
  }
  mat.v.right *= validScale[0];
  mat.v.up *= validScale[1];
  mat.v.dir *= validScale[2];
  mat.v.position.Set(translation[0], translation[1], translation[2], 1.f);
}

void
SetID(int id)
{
  GetContext().mActualID = id;
}

void
AllowAxisFlip(bool value)
{
  GetContext().mAllowAxisFlip = value;
}

struct Scope
{
  Scope() { ImGuizmo::GetContext().mAllowActiveHoverItem = true; }
  ~Scope() { ImGuizmo::GetContext().mAllowActiveHoverItem = false; }
};

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
  GetContext().mActualID = (int64_t)id;
  Scope scope;

  // Scale is always local or matrix will be skewed when applying world scale or
  // oriented matrix
  GetContext().ComputeContext(
    view, projection, matrix, (operation & SCALE) ? LOCAL : mode);

  // set delta to identity
  if (deltaMatrix) {
    ((matrix_t*)deltaMatrix)->SetToIdentity();
  }

  // behind camera
  vec_t camSpacePosition;
  camSpacePosition.TransformPoint(makeVect(0.f, 0.f, 0.f), GetContext().mMVP);
  if (!GetContext().mIsOrthographic && camSpacePosition.z < 0.001f) {
    return false;
  }

  // --
  int type = MT_NONE;
  bool manipulated = false;
  if (GetContext().mbEnable) {
    if (!GetContext().mbUsingBounds) {
      manipulated =
        HandleTranslation(matrix, deltaMatrix, operation, type, snap) ||
        HandleScale(matrix, deltaMatrix, operation, type, snap) ||
        HandleRotation(matrix, deltaMatrix, operation, type, snap);
    }
  }

  if (localBounds && !GetContext().mbUsing) {
    HandleAndDrawLocalBounds(
      localBounds, (matrix_t*)matrix, boundsSnap, operation);
  }

  GetContext().mOperation = operation;
  if (!GetContext().mbUsingBounds) {
    DrawRotationGizmo(operation, type);
    DrawTranslationGizmo(operation, type);
    DrawScaleGizmo(operation, type);
    DrawScaleUniveralGizmo(operation, type);
  }
  return manipulated;
}

void
SetGizmoSizeClipSpace(float value)
{
  GetContext().mGizmoSizeClipSpace = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ComputeFrustumPlanes(vec_t* frustum, const float* clip)
{
  frustum[0].x = clip[3] - clip[0];
  frustum[0].y = clip[7] - clip[4];
  frustum[0].z = clip[11] - clip[8];
  frustum[0].w = clip[15] - clip[12];

  frustum[1].x = clip[3] + clip[0];
  frustum[1].y = clip[7] + clip[4];
  frustum[1].z = clip[11] + clip[8];
  frustum[1].w = clip[15] + clip[12];

  frustum[2].x = clip[3] + clip[1];
  frustum[2].y = clip[7] + clip[5];
  frustum[2].z = clip[11] + clip[9];
  frustum[2].w = clip[15] + clip[13];

  frustum[3].x = clip[3] - clip[1];
  frustum[3].y = clip[7] - clip[5];
  frustum[3].z = clip[11] - clip[9];
  frustum[3].w = clip[15] - clip[13];

  frustum[4].x = clip[3] - clip[2];
  frustum[4].y = clip[7] - clip[6];
  frustum[4].z = clip[11] - clip[10];
  frustum[4].w = clip[15] - clip[14];

  frustum[5].x = clip[3] + clip[2];
  frustum[5].y = clip[7] + clip[6];
  frustum[5].z = clip[11] + clip[10];
  frustum[5].w = clip[15] + clip[14];

  for (int i = 0; i < 6; i++) {
    frustum[i].Normalize();
  }
}

void
DrawCubes(const float* view,
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
      centerPosition.TransformPoint(directionUnary[normalIndex] * 0.5f * invert,
                                    *(matrix_t*)matrix);
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
  qsort(
    faces, cubeFaceCount, sizeof(CubeFace), [](void const* _a, void const* _b) {
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
    GetContext().mDrawList->AddConvexPolyFilled(
      cubeFace.faceCoordsScreen, 4, cubeFace.color);
  }

  _freea(faces);
}

void
DrawGrid(const float* view,
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
        col = (fabsf(f) < FLT_EPSILON) ? IM_COL32(0x40, 0x40, 0x40, 0xFF) : col;

        float thickness = 1.f;
        thickness = (fmodf(fabsf(f), 10.f) < FLT_EPSILON) ? 1.5f : thickness;
        thickness = (fabsf(f) < FLT_EPSILON) ? 2.3f : thickness;

        GetContext().mDrawList->AddLine(
          worldToPos(ptA, res), worldToPos(ptB, res), col, thickness);
      }
    }
  }
}

void
ViewManipulate(float* view,
               const float* projection,
               OPERATION operation,
               MODE mode,
               float* matrix,
               float length,
               ImVec2 position,
               ImVec2 size,
               ImU32 backgroundColor)
{
  // Scale is always local or matrix will be skewed when applying world scale or
  // oriented matrix
  GetContext().ComputeContext(
    view, projection, matrix, (operation & SCALE) ? LOCAL : mode);
  ViewManipulate(view, length, position, size, backgroundColor);
}

void
ViewManipulate(float* view,
               float length,
               ImVec2 position,
               ImVec2 size,
               ImU32 backgroundColor)
{
  static bool isDraging = false;
  static bool isClicking = false;
  static bool isInside = false;
  static vec_t interpolationUp;
  static vec_t interpolationDir;
  static int interpolationFrames = 0;
  const vec_t referenceUp = makeVect(0.f, 1.f, 0.f);

  matrix_t svgView, svgProjection;
  svgView = GetContext().mViewMat;
  svgProjection = GetContext().mProjectionMat;

  ImGuiIO& io = ImGui::GetIO();
  GetContext().mDrawList->AddRectFilled(
    position, position + size, backgroundColor);
  matrix_t viewInverse;
  viewInverse.Inverse(*(matrix_t*)view);

  const vec_t camTarget = viewInverse.v.position - viewInverse.v.dir * length;

  // view/projection matrices
  const float distance = 3.f;
  matrix_t cubeProjection, cubeView;
  float fov = acosf(distance / (sqrtf(distance * distance + 3.f))) * RAD2DEG;
  Perspective(
    fov / sqrtf(2.f), size.x / size.y, 0.01f, 1000.f, cubeProjection.m16);

  vec_t dir =
    makeVect(viewInverse.m[2][0], viewInverse.m[2][1], viewInverse.m[2][2]);
  vec_t up =
    makeVect(viewInverse.m[1][0], viewInverse.m[1][1], viewInverse.m[1][2]);
  vec_t eye = dir * distance;
  vec_t zero = makeVect(0.f, 0.f);
  LookAt(&eye.x, &zero.x, &up.x, cubeView.m16);

  // set context
  GetContext().mViewMat = cubeView;
  GetContext().mProjectionMat = cubeProjection;
  ComputeCameraRay(
    GetContext().mRayOrigin, GetContext().mRayVector, position, size);

  const matrix_t res = cubeView * cubeProjection;

  // panels
  static const ImVec2 panelPosition[9] = {
    ImVec2(0.75f, 0.75f), ImVec2(0.25f, 0.75f), ImVec2(0.f, 0.75f),
    ImVec2(0.75f, 0.25f), ImVec2(0.25f, 0.25f), ImVec2(0.f, 0.25f),
    ImVec2(0.75f, 0.f),   ImVec2(0.25f, 0.f),   ImVec2(0.f, 0.f)
  };

  static const ImVec2 panelSize[9] = {
    ImVec2(0.25f, 0.25f), ImVec2(0.5f, 0.25f), ImVec2(0.25f, 0.25f),
    ImVec2(0.25f, 0.5f),  ImVec2(0.5f, 0.5f),  ImVec2(0.25f, 0.5f),
    ImVec2(0.25f, 0.25f), ImVec2(0.5f, 0.25f), ImVec2(0.25f, 0.25f)
  };

  // tag faces
  bool boxes[27]{};
  static int overBox = -1;
  for (int iPass = 0; iPass < 2; iPass++) {
    for (int iFace = 0; iFace < 6; iFace++) {
      const int normalIndex = (iFace % 3);
      const int perpXIndex = (normalIndex + 1) % 3;
      const int perpYIndex = (normalIndex + 2) % 3;
      const float invert = (iFace > 2) ? -1.f : 1.f;
      const vec_t indexVectorX = directionUnary[perpXIndex] * invert;
      const vec_t indexVectorY = directionUnary[perpYIndex] * invert;
      const vec_t boxOrigin =
        directionUnary[normalIndex] * -invert - indexVectorX - indexVectorY;

      // plan local space
      const vec_t n = directionUnary[normalIndex] * invert;
      vec_t viewSpaceNormal = n;
      vec_t viewSpacePoint = n * 0.5f;
      viewSpaceNormal.TransformVector(cubeView);
      viewSpaceNormal.Normalize();
      viewSpacePoint.TransformPoint(cubeView);
      const vec_t viewSpaceFacePlan =
        BuildPlan(viewSpacePoint, viewSpaceNormal);

      // back face culling
      if (viewSpaceFacePlan.w > 0.f) {
        continue;
      }

      const vec_t facePlan = BuildPlan(n * 0.5f, n);

      const float len = IntersectRayPlane(
        GetContext().mRayOrigin, GetContext().mRayVector, facePlan);
      vec_t posOnPlan =
        GetContext().mRayOrigin + GetContext().mRayVector * len - (n * 0.5f);

      float localx = Dot(directionUnary[perpXIndex], posOnPlan) * invert + 0.5f;
      float localy = Dot(directionUnary[perpYIndex], posOnPlan) * invert + 0.5f;

      // panels
      const vec_t dx = directionUnary[perpXIndex];
      const vec_t dy = directionUnary[perpYIndex];
      const vec_t origin = directionUnary[normalIndex] - dx - dy;
      for (int iPanel = 0; iPanel < 9; iPanel++) {
        vec_t boxCoord = boxOrigin + indexVectorX * float(iPanel % 3) +
                         indexVectorY * float(iPanel / 3) +
                         makeVect(1.f, 1.f, 1.f);
        const ImVec2 p = panelPosition[iPanel] * 2.f;
        const ImVec2 s = panelSize[iPanel] * 2.f;
        ImVec2 faceCoordsScreen[4];
        vec_t panelPos[4] = { dx * p.x + dy * p.y,
                              dx * p.x + dy * (p.y + s.y),
                              dx * (p.x + s.x) + dy * (p.y + s.y),
                              dx * (p.x + s.x) + dy * p.y };

        for (unsigned int iCoord = 0; iCoord < 4; iCoord++) {
          faceCoordsScreen[iCoord] = worldToPos(
            (panelPos[iCoord] + origin) * 0.5f * invert, res, position, size);
        }

        const ImVec2 panelCorners[2] = {
          panelPosition[iPanel], panelPosition[iPanel] + panelSize[iPanel]
        };
        bool insidePanel =
          localx > panelCorners[0].x && localx < panelCorners[1].x &&
          localy > panelCorners[0].y && localy < panelCorners[1].y;
        int boxCoordInt = int(boxCoord.x * 9.f + boxCoord.y * 3.f + boxCoord.z);
        IM_ASSERT(boxCoordInt < 27);
        boxes[boxCoordInt] |=
          insidePanel && (!isDraging) && GetContext().mbMouseOver;

        // draw face with lighter color
        if (iPass) {
          ImU32 directionColor = GetColorU32(DIRECTION_X + normalIndex);
          GetContext().mDrawList->AddConvexPolyFilled(
            faceCoordsScreen,
            4,
            (directionColor | IM_COL32(0x80, 0x80, 0x80, 0x80)) |
              (isInside ? IM_COL32(0x08, 0x08, 0x08, 0) : 0));
          if (boxes[boxCoordInt]) {
            GetContext().mDrawList->AddConvexPolyFilled(
              faceCoordsScreen, 4, IM_COL32(0xF0, 0xA0, 0x60, 0x80));

            if (io.MouseDown[0] && !isClicking && !isDraging &&
                GImGui->ActiveId == 0) {
              overBox = boxCoordInt;
              isClicking = true;
              isDraging = true;
            }
          }
        }
      }
    }
  }
  if (interpolationFrames) {
    interpolationFrames--;
    vec_t newDir = viewInverse.v.dir;
    newDir.Lerp(interpolationDir, 0.2f);
    newDir.Normalize();

    vec_t newUp = viewInverse.v.up;
    newUp.Lerp(interpolationUp, 0.3f);
    newUp.Normalize();
    newUp = interpolationUp;
    vec_t newEye = camTarget + newDir * length;
    LookAt(&newEye.x, &camTarget.x, &newUp.x, view);
  }
  isInside = GetContext().mbMouseOver &&
             ImRect(position, position + size).Contains(io.MousePos);

  if (io.MouseDown[0] && (fabsf(io.MouseDelta[0]) || fabsf(io.MouseDelta[1])) &&
      isClicking) {
    isClicking = false;
  }

  if (!io.MouseDown[0]) {
    if (isClicking) {
      // apply new view direction
      int cx = overBox / 9;
      int cy = (overBox - cx * 9) / 3;
      int cz = overBox % 3;
      interpolationDir =
        makeVect(1.f - (float)cx, 1.f - (float)cy, 1.f - (float)cz);
      interpolationDir.Normalize();

      if (fabsf(Dot(interpolationDir, referenceUp)) > 1.0f - 0.01f) {
        vec_t right = viewInverse.v.right;
        if (fabsf(right.x) > fabsf(right.z)) {
          right.z = 0.f;
        } else {
          right.x = 0.f;
        }
        right.Normalize();
        interpolationUp = Cross(interpolationDir, right);
        interpolationUp.Normalize();
      } else {
        interpolationUp = referenceUp;
      }
      interpolationFrames = 40;
    }
    isClicking = false;
    isDraging = false;
  }

  if (isDraging) {
    matrix_t rx, ry, roll;

    rx.RotationAxis(referenceUp, -io.MouseDelta.x * 0.01f);
    ry.RotationAxis(viewInverse.v.right, -io.MouseDelta.y * 0.01f);

    roll = rx * ry;

    vec_t newDir = viewInverse.v.dir;
    newDir.TransformVector(roll);
    newDir.Normalize();

    // clamp
    vec_t planDir = Cross(viewInverse.v.right, referenceUp);
    planDir.y = 0.f;
    planDir.Normalize();
    float dt = Dot(planDir, newDir);
    if (dt < 0.0f) {
      newDir += planDir * dt;
      newDir.Normalize();
    }

    vec_t newEye = camTarget + newDir * length;
    LookAt(&newEye.x, &camTarget.x, &referenceUp.x, view);
  }

  // restore view/projection because it was used to compute ray
  GetContext().ComputeContext(svgView.m16,
                              svgProjection.m16,
                              GetContext().mModelSource.m16,
                              GetContext().mMode);
}

} // namespace
