#include "scale.h"
#include "../tripod.h"

namespace recti {

static const char* scaleInfoMask[] = { "X : %5.2f",
                                       "Y : %5.2f",
                                       "Z : %5.2f",
                                       "XYZ : %5.2f" };

static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };

recti::MOVETYPE
Scale::GetScaleType(const recti::ModelContext& mCurrent,
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
      recti::PointOnSegment({ posOnPlanScreen.x, posOnPlanScreen.y },
                            { axisStartOnScreen.x, axisStartOnScreen.y },
                            { axisEndOnScreen.x, axisEndOnScreen.y });

    if ((closestPointOnAxis -
         recti::Vec4{ posOnPlanScreen.x, posOnPlanScreen.y })
          .Length() < 12.f) // pixel size
    {
      type = (recti::MOVETYPE)(recti::MT_SCALE_X + i);
    }
  }

  // universal
  auto& mousePos = mCurrent.mCameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.x - mCurrent.mScreenSquareCenter.x,
                              mousePos.y - mCurrent.mScreenSquareCenter.y,
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
Result
Scale::HandleScale(const ModelContext& mCurrent,
                   bool mAllowAxisFlip,
                   State& mState,
                   const float* snap,
                   float* matrix,
                   float* deltaMatrix)
{
  if ((!Intersects(mCurrent.mOperation, SCALE) &&
       !Intersects(mCurrent.mOperation, SCALEU))) {
    return {};
  }

  // scale
  if (mState.Using(mCurrent.mActualID) &&
      IsScaleType(mState.mCurrentOperation)) {
    // drag
    Vec4 newPos = mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
    Vec4 newOrigin = newPos - mRelativeOrigin * mCurrent.mScreenFactor;
    Vec4 delta = newOrigin - mCurrent.mModelLocal.position();

    // 1 axis constraint
    if (mState.mCurrentOperation >= MT_SCALE_X &&
        mState.mCurrentOperation <= MT_SCALE_Z) {
      int axisIndex = mState.mCurrentOperation - MT_SCALE_X;
      const Vec4& axisValue = mCurrent.mModelLocal.component(axisIndex);
      float lengthOnAxis = Dot(axisValue, delta);
      delta = axisValue * lengthOnAxis;

      Vec4 baseVector =
        mTranslationPlanOrigin - mCurrent.mModelLocal.position();
      float ratio =
        Dot(axisValue, baseVector + delta) / Dot(axisValue, baseVector);

      mScale[axisIndex] = max(ratio, 0.001f);
    } else {
      float scaleDelta =
        (mCurrent.mCameraMouse.Mouse.Position.x - mSaveMousePosx) * 0.01f;
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

    bool modified = false;
    if (mScaleLast != mScale) {
      modified = true;
    }
    mScaleLast = mScale;

    // compute matrix & delta
    Mat4 deltaMatrixScale;
    deltaMatrixScale.Scale(mScale * mScaleValueOrigin);

    Mat4 res = deltaMatrixScale * mCurrent.mModelLocal;
    *(Mat4*)matrix = res;

    if (deltaMatrix) {
      Vec4 deltaScale = mScale * mScaleValueOrigin;

      Vec4 originalScaleDivider;
      originalScaleDivider.x = 1 / mCurrent.mModelScaleOrigin.x;
      originalScaleDivider.y = 1 / mCurrent.mModelScaleOrigin.y;
      originalScaleDivider.z = 1 / mCurrent.mModelScaleOrigin.z;

      deltaScale = deltaScale * originalScaleDivider;

      deltaMatrixScale.Scale(deltaScale);
      memcpy(deltaMatrix, &deltaMatrixScale.m00, sizeof(float) * 16);
    }

    if (!mCurrent.mCameraMouse.Mouse.LeftDown) {
      mState.mbUsing = false;
      mScale.Set(1.f, 1.f, 1.f);
    }

    return { mState.mCurrentOperation, modified };
  }

  auto& mouse = mCurrent.mCameraMouse.Mouse;

  // find new possible way to scale
  auto type = GetScaleType(mCurrent, mAllowAxisFlip, &mState);
  if (mouse.LeftDown && type != MT_NONE) {
    mState.mbUsing = true;
    mState.mEditingID = mCurrent.mActualID;
    mState.mCurrentOperation = type;
    const Vec4 movePlanNormal[] = { mCurrent.mModel.up(),
                                    mCurrent.mModel.dir(),
                                    mCurrent.mModel.right(),
                                    mCurrent.mModel.dir(),
                                    mCurrent.mModel.up(),
                                    mCurrent.mModel.right(),
                                    -mCurrent.mCameraMouse.CameraDir() };
    // pickup plan

    mTranslationPlan =
      BuildPlan(mCurrent.mModel.position(), movePlanNormal[type - MT_SCALE_X]);
    mTranslationPlanOrigin =
      mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
    mMatrixOrigin = mCurrent.mModel.position();
    mScale.Set(1.f, 1.f, 1.f);
    mRelativeOrigin = (mTranslationPlanOrigin - mCurrent.mModel.position()) *
                      (1.f / mCurrent.mScreenFactor);
    mScaleValueOrigin = { mCurrent.mModelSource.right().Length(),
                          mCurrent.mModelSource.up().Length(),
                          mCurrent.mModelSource.dir().Length() };
    mSaveMousePosx = mouse.Position.x;
  }

  return { type };
}

void
Scale::DrawScaleGizmo(const ModelContext& mCurrent,
                      MOVETYPE type,
                      const State& mState,
                      const Style& mStyle,
                      const std::shared_ptr<DrawList>& drawList)
{
  if (!Intersects(mCurrent.mOperation, SCALE)) {
    return;
  }

  // colors
  uint32_t colors[7];
  mStyle.ComputeColors(colors, type, SCALE);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  if (mState.Using(mCurrent.mActualID)) {
    scaleDisplay = mScale;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }
    const bool usingAxis = (mState.mbUsing && type == MT_SCALE_X + i);
    if (!mState.mbUsing || usingAxis) {
      Tripod tripod(i);

      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      tripod.belowAxisLimit = mState.mBelowAxisLimit[i];
      tripod.belowPlaneLimit = mState.mBelowPlaneLimit[i];
      tripod.dirAxis *= mState.mAxisFactor[i];
      tripod.dirPlaneX *= mState.mAxisFactor[(i + 1) % 3];
      tripod.dirPlaneY *= mState.mAxisFactor[(i + 2) % 3];

      // draw axis
      if (tripod.belowAxisLimit) {
        bool hasTranslateOnAxis = Contains(
          mCurrent.mOperation, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        Vec2 baseSSpace =
          worldToPos(tripod.dirAxis * 0.1f * mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.Viewport);
        Vec2 worldDirSSpaceNoScale =
          worldToPos(tripod.dirAxis * markerScale * mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.Viewport);
        Vec2 worldDirSSpace =
          worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                       mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.Viewport);

        if (mState.Using(mCurrent.mActualID)) {
          uint32_t scaleLineColor = mStyle.GetColorU32(SCALE_LINE);
          drawList->AddLine(baseSSpace,
                            worldDirSSpaceNoScale,
                            scaleLineColor,
                            mStyle.ScaleLineThickness);
          drawList->AddCircleFilled(
            worldDirSSpaceNoScale, mStyle.ScaleLineCircleSize, scaleLineColor);
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
          drawList->DrawHatchedAxis(
            mCurrent, tripod.dirAxis * scaleDisplay[i], mStyle);
        }
      }
    }
  }

  // draw screen cirle
  drawList->AddCircleFilled(
    mCurrent.mScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);

  if (mState.Using(mCurrent.mActualID) && IsScaleType(type)) {
    Vec2 destinationPosOnScreen =
      worldToPos(mCurrent.mModel.position(),
                 mCurrent.mCameraMouse.mViewProjection,
                 mCurrent.mCameraMouse.Camera.Viewport);
    char tmps[512];
    // vec_t deltaInfo = mModel.position() -
    // mMatrixOrigin;
    int componentInfoIndex = (type - MT_SCALE_X) * 3;
    snprintf(tmps,
             sizeof(tmps),
             scaleInfoMask[type - MT_SCALE_X],
             scaleDisplay[translationInfoIndex[componentInfoIndex]]);
    drawList->AddText(
      Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
      mStyle.GetColorU32(TEXT_SHADOW),
      tmps);
    drawList->AddText(
      Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
      mStyle.GetColorU32(TEXT),
      tmps);
  }
}

void
Scale::DrawScaleUniveralGizmo(const ModelContext& mCurrent,
                              bool mAllowAxisFlip,
                              MOVETYPE type,
                              const State& mState,
                              const Style& mStyle,
                              const std::shared_ptr<DrawList>& drawList)
{
  if (!Intersects(mCurrent.mOperation, SCALEU)) {
    return;
  }

  // colors
  uint32_t colors[7];
  mStyle.ComputeColors(colors, type, SCALEU);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  if (mState.Using(mCurrent.mActualID)) {
    scaleDisplay = mScale;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<OPERATION>(SCALE_XU << i))) {
      continue;
    }
    const bool usingAxis = (mState.mbUsing && type == MT_SCALE_X + i);
    if (!mState.mbUsing || usingAxis) {
      Tripod tripod(i);
      // when using, use stored factors so the gizmo doesn't flip when we
      // translate
      tripod.belowAxisLimit = mState.mBelowAxisLimit[i];
      tripod.belowPlaneLimit = mState.mBelowPlaneLimit[i];
      tripod.dirAxis *= mState.mAxisFactor[i];
      tripod.dirPlaneX *= mState.mAxisFactor[(i + 1) % 3];
      tripod.dirPlaneY *= mState.mAxisFactor[(i + 2) % 3];

      // draw axis
      if (tripod.belowAxisLimit) {
        bool hasTranslateOnAxis = Contains(
          mCurrent.mOperation, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        Vec2 worldDirSSpace =
          worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                       mCurrent.mScreenFactor,
                     mCurrent.mMVPLocal,
                     mCurrent.mCameraMouse.Camera.Viewport);

        drawList->AddCircleFilled(worldDirSSpace, 12.f, colors[i + 1]);
      }
    }
  }

  // draw screen cirle
  drawList->AddCircle(
    mCurrent.mScreenSquareCenter, 20.f, colors[0], 32, mStyle.CenterCircleSize);

  if (mState.Using(mCurrent.mOperation) && IsScaleType(type)) {
    Vec2 destinationPosOnScreen =
      mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position());

    char tmps[512];
    // vec_t deltaInfo = mModel.position() -
    // mMatrixOrigin;
    int componentInfoIndex = (type - MT_SCALE_X) * 3;
    snprintf(tmps,
             sizeof(tmps),
             scaleInfoMask[type - MT_SCALE_X],
             scaleDisplay[translationInfoIndex[componentInfoIndex]]);
    drawList->AddText(
      Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
      mStyle.GetColorU32(TEXT_SHADOW),
      tmps);
    drawList->AddText(
      Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
      mStyle.GetColorU32(TEXT),
      tmps);
  }
}

} // namespace
