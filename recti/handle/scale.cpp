#include "scale.h"
#include "../tripod.h"

namespace recti {

MOVETYPE
Scale::GetType(const recti::ModelContext& mCurrent,
               bool mAllowAxisFlip,
               recti::State* state)
{
  recti::MOVETYPE type = recti::MT_NONE;

  // compute
  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::SCALE_X << i))) {
      continue;
    }
    recti::Tripod tripod(i);
    tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
    // and store values
    state->mAxisFactor[i] = tripod.mulAxis;
    state->mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
    state->mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
    state->mBelowAxisLimit[i] = tripod.belowAxisLimit;
    state->mBelowPlaneLimit[i] = tripod.belowPlaneLimit;

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
    tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
    // and store values
    state->mAxisFactor[i] = tripod.mulAxis;
    state->mAxisFactor[(i + 1) % 3] = tripod.mulAxisX;
    state->mAxisFactor[(i + 2) % 3] = tripod.mulAxisY;
    state->mBelowAxisLimit[i] = tripod.belowAxisLimit;
    state->mBelowPlaneLimit[i] = tripod.belowPlaneLimit;

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

void
Scale::DrawGizmo(const ModelContext& mCurrent,
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
  ComputeColors(colors, type, mStyle);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }

    const bool usingAxis = (false && type == MT_SCALE_X + i);
    if (!false || usingAxis) {
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

        if (!hasTranslateOnAxis || false) {
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
}

void
Scale::DrawUniveralGizmo(const ModelContext& mCurrent,
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
  ComputeColors(colors, type, mStyle);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<OPERATION>(SCALE_XU << i))) {
      continue;
    }
    const bool usingAxis = (false && type == MT_SCALE_X + i);
    if (!false || usingAxis) {
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
}

void
Scale::ComputeColors(uint32_t colors[7], MOVETYPE type, const Style& style)
{
  uint32_t selectionColor = style.GetColorU32(SELECTION);

  colors[0] = (type == MT_SCALE_XYZ) ? selectionColor : COL32_WHITE();
  for (int i = 0; i < 3; i++) {
    colors[i + 1] = (type == (int)(MT_SCALE_X + i))
                      ? selectionColor
                      : style.GetColorU32(DIRECTION_X + i);
  }
}

} // namespace
