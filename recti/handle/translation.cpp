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
GetMoveType(const ModelContext& current, bool allowAxisFlip, State* state)
{
  if (!Intersects(current.mOperation, TRANSLATE) || state->mbUsing) {
    return MT_NONE;
  }

  MOVETYPE type = MT_NONE;

  // compute
  for (int i = 0; i < 3 && type == MT_NONE; i++) {

    Tripod tripod(i);
    if (state->Using(current.mActualID)) {
      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      tripod.belowAxisLimit = state->mBelowAxisLimit[i];
      tripod.belowPlaneLimit = state->mBelowPlaneLimit[i];
      tripod.dirAxis *= state->mAxisFactor[i];
      tripod.dirPlaneX *= state->mAxisFactor[(i + 1) % 3];
      tripod.dirPlaneY *= state->mAxisFactor[(i + 2) % 3];
    } else {
      tripod.ComputeTripodAxisAndVisibility(current, allowAxisFlip);
      // and store values
      state->mAxisFactor[i] = tripod.mulAxis;
      state->mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
      state->mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
      state->mBelowAxisLimit[i] = tripod.belowAxisLimit;
      state->mBelowPlaneLimit[i] = tripod.belowPlaneLimit;
    }

    tripod.dirAxis.TransformVector(current.mModel);
    tripod.dirPlaneX.TransformVector(current.mModel);
    tripod.dirPlaneY.TransformVector(current.mModel);

    auto posOnPlan = current.mCameraMouse.Ray.IntersectPlane(
      BuildPlan(current.mModel.position(), tripod.dirAxis));

    // screen
    const Vec2 axisStartOnScreen =
      current.mCameraMouse.WorldToPos(current.mModel.position() +
                                      tripod.dirAxis * current.mScreenFactor *
                                        0.1f) -
      current.mCameraMouse.Camera.LeftTop();

    // screen
    const Vec2 axisEndOnScreen =
      current.mCameraMouse.WorldToPos(current.mModel.position() +
                                      tripod.dirAxis * current.mScreenFactor) -
      current.mCameraMouse.Camera.LeftTop();

    auto screenCoord = current.mCameraMouse.ScreenMousePos();
    Vec4 closestPointOnAxis =
      PointOnSegment(screenCoord,
                     { axisStartOnScreen.X, axisStartOnScreen.Y },
                     { axisEndOnScreen.X, axisEndOnScreen.Y });
    if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
        Intersects(current.mOperation,
                   static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
    {
      type = (MOVETYPE)(MT_MOVE_X + i);
    }

    const float dx = tripod.dirPlaneX.Dot3(
      (posOnPlan - current.mModel.position()) * (1.f / current.mScreenFactor));
    const float dy = tripod.dirPlaneY.Dot3(
      (posOnPlan - current.mModel.position()) * (1.f / current.mScreenFactor));
    if (tripod.belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
        dy >= quadUV[1] && dy <= quadUV[3] &&
        Contains(current.mOperation, TRANSLATE_PLANS[i])) {
      type = (MOVETYPE)(MT_MOVE_YZ + i);
    }
  }
  return type;
}

void
Translation::Begin(const ModelContext& current, MOVETYPE type)
{
  // find new possible way to move
  Vec4 movePlanNormal[] = {
    current.mModel.right(),           current.mModel.up(), current.mModel.dir(),
    current.mModel.right(),           current.mModel.up(), current.mModel.dir(),
    -current.mCameraMouse.CameraDir()
  };

  Vec4 cameraToModelNormalized =
    Normalized(current.mModel.position() - current.mCameraMouse.CameraEye());
  for (unsigned int i = 0; i < 3; i++) {
    Vec4 orthoVector = Cross(movePlanNormal[i], cameraToModelNormalized);
    movePlanNormal[i].Cross(orthoVector);
    movePlanNormal[i].Normalize();
  }
  // pickup plan
  mTranslationPlan =
    BuildPlan(current.mModel.position(), movePlanNormal[type - MT_MOVE_X]);
  mTranslationPlanOrigin =
    current.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
  mMatrixOrigin = current.mModel.position();

  mRelativeOrigin = (mTranslationPlanOrigin - current.mModel.position()) *
                    (1.f / current.mScreenFactor);
}

bool
Translation::Drag(const ModelContext& current,
                  const State& state,
                  const float* snap,
                  float* matrix,
                  float* deltaMatrix)
{
  const Vec4 newPos = current.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);

  // compute delta
  const Vec4 newOrigin = newPos - mRelativeOrigin * current.mScreenFactor;
  Vec4 delta = newOrigin - current.mModel.position();

  // 1 axis constraint
  if (state.mCurrentOperation >= MT_MOVE_X &&
      state.mCurrentOperation <= MT_MOVE_Z) {
    const int axisIndex = state.mCurrentOperation - MT_MOVE_X;
    const Vec4& axisValue = current.mModel.component(axisIndex);
    const float lengthOnAxis = Dot(axisValue, delta);
    delta = axisValue * lengthOnAxis;
  }

  // snap
  if (snap) {
    Vec4 cumulativeDelta = current.mModel.position() + delta - mMatrixOrigin;
    const bool applyRotationLocaly =
      current.mMode == LOCAL || state.mCurrentOperation == MT_MOVE_SCREEN;
    if (applyRotationLocaly) {
      Mat4 modelSourceNormalized = current.mModelSource;
      modelSourceNormalized.OrthoNormalize();
      Mat4 modelSourceNormalizedInverse;
      modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
      cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
      ComputeSnap(cumulativeDelta, snap);
      cumulativeDelta.TransformVector(modelSourceNormalized);
    } else {
      ComputeSnap(cumulativeDelta, snap);
    }
    delta = mMatrixOrigin + cumulativeDelta - current.mModel.position();
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

  const Mat4 res = current.mModelSource * deltaMatrixTranslation;
  *(Mat4*)matrix = res;

  return modified;
}

Result
Translation::HandleTranslation(const ModelContext& current,
                               bool allowAxisFlip,
                               State& state,
                               const float* snap,
                               float* matrix,
                               float* deltaMatrix)
{
  if (!Intersects(current.mOperation, TRANSLATE)) {
    return {};
  }

  if (state.Using(current.mActualID) &&
      IsTranslateType(state.mCurrentOperation)) {
    // drag
    auto modified = Drag(current, state, snap, matrix, deltaMatrix);
    if (!current.mCameraMouse.Mouse.LeftDown) {
      state.mbUsing = false;
    }

    return { state.mCurrentOperation, modified };
  }

  auto type = GetMoveType(current, allowAxisFlip, &state);
  if (type != MT_NONE) {
    // hover
    if (current.mCameraMouse.Mouse.LeftDown) {
      // begin
      state.mbUsing = true;
      state.mEditingID = current.mActualID;
      state.mCurrentOperation = type;
      Begin(current, type);
    }
  }
  return { type };
}

void
Translation::DrawTranslationGizmo(const ModelContext& current,
                                  bool allowAxisFlip,
                                  MOVETYPE type,
                                  const State& state,
                                  const Style& style,
                                  const std::shared_ptr<DrawList>& drawList)
{
  if (!Intersects(current.mOperation, TRANSLATE)) {
    return;
  }

  // colors
  uint32_t colors[7];
  style.ComputeColors(colors, type, TRANSLATE);

  const Vec2 origin =
    current.mCameraMouse.WorldToPos(current.mModel.position());

  // draw
  for (int i = 0; i < 3; ++i) {
    Tripod tripod(i);

    // when using, use stored factors so the gizmo doesn't flip when we
    // translate
    tripod.belowAxisLimit = state.mBelowAxisLimit[i];
    tripod.belowPlaneLimit = state.mBelowPlaneLimit[i];
    tripod.dirAxis *= state.mAxisFactor[i];
    tripod.dirPlaneX *= state.mAxisFactor[(i + 1) % 3];
    tripod.dirPlaneY *= state.mAxisFactor[(i + 2) % 3];

    if (!state.mbUsing || (state.mbUsing && type == MT_MOVE_X + i)) {
      // draw axis
      if (tripod.belowAxisLimit &&
          Intersects(current.mOperation,
                     static_cast<OPERATION>(TRANSLATE_X << i))) {
        Vec2 baseSSpace =
          worldToPos(tripod.dirAxis * 0.1f * current.mScreenFactor,
                     current.mMVP,
                     current.mCameraMouse.Camera.Viewport);
        Vec2 worldDirSSpace = worldToPos(tripod.dirAxis * current.mScreenFactor,
                                         current.mMVP,
                                         current.mCameraMouse.Camera.Viewport);

        drawList->AddLine(baseSSpace,
                          worldDirSSpace,
                          colors[i + 1],
                          style.TranslationLineThickness);

        // Arrow head begin
        Vec2 dir(origin - worldDirSSpace);

        float d = sqrtf(dir.SqrLength());
        dir /= d; // Normalize
        dir *= style.TranslationLineArrowSize;

        Vec2 ortogonalDir(dir.Y, -dir.X); // Perpendicular vector
        Vec2 a(worldDirSSpace + dir);
        drawList->AddTriangleFilled(worldDirSSpace - dir,
                                    a + ortogonalDir,
                                    a - ortogonalDir,
                                    colors[i + 1]);
        // Arrow head end

        if (state.mAxisFactor[i] < 0.f) {
          drawList->DrawHatchedAxis(current, tripod.dirAxis, style);
        }
      }
    }
    // draw plane
    if (!state.mbUsing || (state.mbUsing && type == MT_MOVE_YZ + i)) {
      if (tripod.belowPlaneLimit &&
          Contains(current.mOperation, TRANSLATE_PLANS[i])) {
        Vec2 screenQuadPts[4];
        for (int j = 0; j < 4; ++j) {
          Vec4 cornerWorldPos = (tripod.dirPlaneX * quadUV[j * 2] +
                                 tripod.dirPlaneY * quadUV[j * 2 + 1]) *
                                current.mScreenFactor;
          screenQuadPts[j] = worldToPos(
            cornerWorldPos, current.mMVP, current.mCameraMouse.Camera.Viewport);
        }
        drawList->AddPolyline((const VEC2*)screenQuadPts,
                              4,
                              style.GetColorU32(DIRECTION_X + i),
                              true,
                              1.0f);
        drawList->AddConvexPolyFilled(
          (const VEC2*)screenQuadPts, 4, colors[i + 4]);
      }
    }
  }

  drawList->AddCircleFilled(
    current.mScreenSquareCenter, style.CenterCircleSize, colors[0], 32);

  if (state.Using(current.mActualID) && IsTranslateType(type)) {
    uint32_t translationLineColor = style.GetColorU32(TRANSLATION_LINE);

    Vec2 sourcePosOnScreen = current.mCameraMouse.WorldToPos(mMatrixOrigin);
    Vec2 destinationPosOnScreen =
      current.mCameraMouse.WorldToPos(current.mModel.position());
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
    Vec4 deltaInfo = current.mModel.position() - mMatrixOrigin;
    int componentInfoIndex = (type - MT_MOVE_X) * 3;
    snprintf(tmps,
             sizeof(tmps),
             translationInfoMask[type - MT_MOVE_X],
             deltaInfo[translationInfoIndex[componentInfoIndex]],
             deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
             deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
    drawList->AddText(
      Vec2(destinationPosOnScreen.X + 15, destinationPosOnScreen.Y + 15),
      style.GetColorU32(TEXT_SHADOW),
      tmps);
    drawList->AddText(
      Vec2(destinationPosOnScreen.X + 14, destinationPosOnScreen.Y + 14),
      style.GetColorU32(TEXT),
      tmps);
  }
}

} // namespace
