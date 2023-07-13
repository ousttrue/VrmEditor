#include "translation.h"
#include "operation.h"
#include "style.h"
#include "tripod.h"
#include <stdio.h>

namespace recti {

static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };
static const float quadMin = 0.5f;
static const float quadMax = 0.8f;
static const float quadUV[8] = { quadMin, quadMin, quadMin, quadMax,
                                 quadMax, quadMax, quadMax, quadMin };

static const OPERATION TRANSLATE_PLANS[3] = { TRANSLATE_Y | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Y };

static const char* translationInfoMask[] = { "X : %5.3f",
                                             "Y : %5.3f",
                                             "Z : %5.3f",
                                             "Y : %5.3f Z : %5.3f",
                                             "X : %5.3f Z : %5.3f",
                                             "X : %5.3f Y : %5.3f",
                                             "X : %5.3f Y : %5.3f Z : %5.3f" };

static MOVETYPE
GetMoveType(const ModelContext& mCurrent, bool mAllowAxisFlip, State* state)
{
  if (!Intersects(mCurrent.mOperation, TRANSLATE) || state->mbUsing) {
    return MT_NONE;
  }

  MOVETYPE type = MT_NONE;

  // compute
  for (int i = 0; i < 3 && type == MT_NONE; i++) {

    Tripod tripod(i);
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

    tripod.dirAxis.TransformVector(mCurrent.mModel);
    tripod.dirPlaneX.TransformVector(mCurrent.mModel);
    tripod.dirPlaneY.TransformVector(mCurrent.mModel);

    auto posOnPlan = mCurrent.mCameraMouse.Ray.IntersectPlane(
      BuildPlan(mCurrent.mModel.position(), tripod.dirAxis));

    // screen
    const Vec2 axisStartOnScreen =
      mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position() +
                                       tripod.dirAxis * mCurrent.mScreenFactor *
                                         0.1f) -
      mCurrent.mCameraMouse.Camera.LeftTop();

    // screen
    const Vec2 axisEndOnScreen =
      mCurrent.mCameraMouse.WorldToPos(
        mCurrent.mModel.position() + tripod.dirAxis * mCurrent.mScreenFactor) -
      mCurrent.mCameraMouse.Camera.LeftTop();

    auto screenCoord = mCurrent.mCameraMouse.ScreenMousePos();
    Vec4 closestPointOnAxis =
      PointOnSegment(screenCoord,
                     { axisStartOnScreen.X, axisStartOnScreen.Y },
                     { axisEndOnScreen.X, axisEndOnScreen.Y });
    if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
        Intersects(mCurrent.mOperation,
                   static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
    {
      type = (MOVETYPE)(MT_MOVE_X + i);
    }

    const float dx =
      tripod.dirPlaneX.Dot3((posOnPlan - mCurrent.mModel.position()) *
                            (1.f / mCurrent.mScreenFactor));
    const float dy =
      tripod.dirPlaneY.Dot3((posOnPlan - mCurrent.mModel.position()) *
                            (1.f / mCurrent.mScreenFactor));
    if (tripod.belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
        dy >= quadUV[1] && dy <= quadUV[3] &&
        Contains(mCurrent.mOperation, TRANSLATE_PLANS[i])) {
      type = (MOVETYPE)(MT_MOVE_YZ + i);
    }
  }
  return type;
}

void
Translation::Begin(const ModelContext& mCurrent, MOVETYPE type)
{
  // find new possible way to move
  Vec4 movePlanNormal[] = { mCurrent.mModel.right(),
                            mCurrent.mModel.up(),
                            mCurrent.mModel.dir(),
                            mCurrent.mModel.right(),
                            mCurrent.mModel.up(),
                            mCurrent.mModel.dir(),
                            -mCurrent.mCameraMouse.CameraDir() };

  Vec4 cameraToModelNormalized =
    Normalized(mCurrent.mModel.position() - mCurrent.mCameraMouse.CameraEye());
  for (unsigned int i = 0; i < 3; i++) {
    Vec4 orthoVector = Cross(movePlanNormal[i], cameraToModelNormalized);
    movePlanNormal[i].Cross(orthoVector);
    movePlanNormal[i].Normalize();
  }
  // pickup plan
  mTranslationPlan =
    BuildPlan(mCurrent.mModel.position(), movePlanNormal[type - MT_MOVE_X]);
  mTranslationPlanOrigin =
    mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
  mMatrixOrigin = mCurrent.mModel.position();

  mRelativeOrigin = (mTranslationPlanOrigin - mCurrent.mModel.position()) *
                    (1.f / mCurrent.mScreenFactor);
}

bool
Translation::Drag(const ModelContext& mCurrent,
                  const State& mState,
                  const float* snap,
                  float* matrix,
                  float* deltaMatrix)
{
  const Vec4 newPos =
    mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);

  // compute delta
  const Vec4 newOrigin = newPos - mRelativeOrigin * mCurrent.mScreenFactor;
  Vec4 delta = newOrigin - mCurrent.mModel.position();

  // 1 axis constraint
  if (mState.mCurrentOperation >= MT_MOVE_X &&
      mState.mCurrentOperation <= MT_MOVE_Z) {
    const int axisIndex = mState.mCurrentOperation - MT_MOVE_X;
    const Vec4& axisValue = mCurrent.mModel.component(axisIndex);
    const float lengthOnAxis = Dot(axisValue, delta);
    delta = axisValue * lengthOnAxis;
  }

  // snap
  if (snap) {
    Vec4 cumulativeDelta = mCurrent.mModel.position() + delta - mMatrixOrigin;
    const bool applyRotationLocaly =
      mCurrent.mMode == LOCAL || mState.mCurrentOperation == MT_MOVE_SCREEN;
    if (applyRotationLocaly) {
      Mat4 modelSourceNormalized = mCurrent.mModelSource;
      modelSourceNormalized.OrthoNormalize();
      Mat4 modelSourceNormalizedInverse;
      modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
      cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
      ComputeSnap(cumulativeDelta, snap);
      cumulativeDelta.TransformVector(modelSourceNormalized);
    } else {
      ComputeSnap(cumulativeDelta, snap);
    }
    delta = mMatrixOrigin + cumulativeDelta - mCurrent.mModel.position();
  }

  auto modified = false;
  if (delta != mTranslationLastDelta) {
    modified = true;
    mTranslationLastDelta = delta;
  }

  // compute matrix & delta
  Mat4 deltaMatrixTranslation;
  deltaMatrixTranslation.Translation(delta);
  if (deltaMatrix) {
    memcpy(deltaMatrix, &deltaMatrixTranslation.m00, sizeof(float) * 16);
  }

  const Mat4 res = mCurrent.mModelSource * deltaMatrixTranslation;
  *(Mat4*)matrix = res;

  return modified;
}

bool
Translation::HandleTranslation(const ModelContext& mCurrent,
                               bool mAllowAxisFlip,
                               float* matrix,
                               float* deltaMatrix,
                               MOVETYPE& type,
                               const float* snap,
                               State& mState)
{
  if (!Intersects(mCurrent.mOperation, TRANSLATE) || type != MT_NONE) {
    return false;
  }

  if (mState.Using(mCurrent.mActualID) &&
      IsTranslateType(mState.mCurrentOperation)) {
    // drag
    auto modified = Drag(mCurrent, mState, snap, matrix, deltaMatrix);
    if (!mCurrent.mCameraMouse.Mouse.LeftDown) {
      mState.mbUsing = false;
    }
    type = mState.mCurrentOperation;

    return modified;
  }

  type = GetMoveType(mCurrent, mAllowAxisFlip, &mState);
  if (type != MT_NONE) {
    // hover
    if (mCurrent.mCameraMouse.Mouse.LeftDown) {
      // begin
      mState.mbUsing = true;
      mState.mEditingID = mCurrent.mActualID;
      mState.mCurrentOperation = type;
      Begin(mCurrent, type);
    }
  }
  return false;
}

void
Translation::DrawTranslationGizmo(const ModelContext& mCurrent,
                                  bool mAllowAxisFlip,
                                  MOVETYPE type,
                                  Style& mStyle,
                                  State& mState,
                                  const std::shared_ptr<DrawList>& drawList)
{
  if (!Intersects(mCurrent.mOperation, TRANSLATE)) {
    return;
  }

  // colors
  uint32_t colors[7];
  mStyle.ComputeColors(colors, type, TRANSLATE);

  const Vec2 origin =
    mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position());

  // draw
  for (int i = 0; i < 3; ++i) {
    Tripod tripod(i);
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

    if (!mState.mbUsing || (mState.mbUsing && type == MT_MOVE_X + i)) {
      // draw axis
      if (tripod.belowAxisLimit &&
          Intersects(mCurrent.mOperation,
                     static_cast<OPERATION>(TRANSLATE_X << i))) {
        Vec2 baseSSpace =
          worldToPos(tripod.dirAxis * 0.1f * mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.Viewport);
        Vec2 worldDirSSpace =
          worldToPos(tripod.dirAxis * mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.Viewport);

        drawList->AddLine(baseSSpace,
                          worldDirSSpace,
                          colors[i + 1],
                          mStyle.TranslationLineThickness);

        // Arrow head begin
        Vec2 dir(origin - worldDirSSpace);

        float d = sqrtf(dir.SqrLength());
        dir /= d; // Normalize
        dir *= mStyle.TranslationLineArrowSize;

        Vec2 ortogonalDir(dir.Y, -dir.X); // Perpendicular vector
        Vec2 a(worldDirSSpace + dir);
        drawList->AddTriangleFilled(worldDirSSpace - dir,
                                    a + ortogonalDir,
                                    a - ortogonalDir,
                                    colors[i + 1]);
        // Arrow head end

        if (mState.mAxisFactor[i] < 0.f) {
          drawList->DrawHatchedAxis(mCurrent, tripod.dirAxis, mStyle);
        }
      }
    }
    // draw plane
    if (!mState.mbUsing || (mState.mbUsing && type == MT_MOVE_YZ + i)) {
      if (tripod.belowPlaneLimit &&
          Contains(mCurrent.mOperation, TRANSLATE_PLANS[i])) {
        Vec2 screenQuadPts[4];
        for (int j = 0; j < 4; ++j) {
          Vec4 cornerWorldPos = (tripod.dirPlaneX * quadUV[j * 2] +
                                 tripod.dirPlaneY * quadUV[j * 2 + 1]) *
                                mCurrent.mScreenFactor;
          screenQuadPts[j] = worldToPos(cornerWorldPos,
                                        mCurrent.mMVP,
                                        mCurrent.mCameraMouse.Camera.Viewport);
        }
        drawList->AddPolyline((const VEC2*)screenQuadPts,
                              4,
                              mStyle.GetColorU32(DIRECTION_X + i),
                              true,
                              1.0f);
        drawList->AddConvexPolyFilled(
          (const VEC2*)screenQuadPts, 4, colors[i + 4]);
      }
    }
  }

  drawList->AddCircleFilled(
    mCurrent.mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

  if (mState.Using(mCurrent.mActualID) && IsTranslateType(type)) {
    uint32_t translationLineColor = mStyle.GetColorU32(TRANSLATION_LINE);

    Vec2 sourcePosOnScreen = mCurrent.mCameraMouse.WorldToPos(mMatrixOrigin);
    Vec2 destinationPosOnScreen =
      mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position());
    Vec4 dif = { destinationPosOnScreen.X - sourcePosOnScreen.X,
                 destinationPosOnScreen.Y - sourcePosOnScreen.Y,
                 0.f,
                 0.f };
    dif.Normalize();
    dif *= 5.f;
    drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
    drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
    drawList->AddLine(
      Vec2(sourcePosOnScreen.X + dif.x, sourcePosOnScreen.Y + dif.y),
      Vec2(destinationPosOnScreen.X - dif.x, destinationPosOnScreen.Y - dif.y),
      translationLineColor,
      2.f);

    char tmps[512];
    Vec4 deltaInfo = mCurrent.mModel.position() - mMatrixOrigin;
    int componentInfoIndex = (type - MT_MOVE_X) * 3;
    snprintf(tmps,
             sizeof(tmps),
             translationInfoMask[type - MT_MOVE_X],
             deltaInfo[translationInfoIndex[componentInfoIndex]],
             deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
             deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
    drawList->AddText(
      Vec2(destinationPosOnScreen.X + 15, destinationPosOnScreen.Y + 15),
      mStyle.GetColorU32(TEXT_SHADOW),
      tmps);
    drawList->AddText(
      Vec2(destinationPosOnScreen.X + 14, destinationPosOnScreen.Y + 14),
      mStyle.GetColorU32(TEXT),
      tmps);
  }
}

} // namespace
