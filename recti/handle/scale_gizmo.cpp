#include "scale_gizmo.h"
#include "../tripod.h"

namespace recti {

static MOVETYPE
GetType(const recti::ModelContext& mCurrent, bool mAllowAxisFlip)
{
  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.Operation,
                    static_cast<recti::OPERATION>(recti::SCALE_X << i))) {
      continue;
    }
    recti::Tripod tripod(i);
    tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
    tripod.dirAxis.TransformVector(mCurrent.ModelLocal);
    tripod.dirPlaneX.TransformVector(mCurrent.ModelLocal);
    tripod.dirPlaneY.TransformVector(mCurrent.ModelLocal);

    recti::Vec4 posOnPlan = mCurrent.CameraMouse.Ray.IntersectPlane(
      BuildPlan(mCurrent.ModelLocal.position(), tripod.dirAxis));

    const float startOffset =
      Contains(mCurrent.Operation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.0f
        : 0.1f;
    const float endOffset =
      Contains(mCurrent.Operation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.4f
        : 1.0f;
    const recti::Vec2 posOnPlanScreen =
      mCurrent.CameraMouse.WorldToPos(posOnPlan);
    const recti::Vec2 axisStartOnScreen = mCurrent.CameraMouse.WorldToPos(
      mCurrent.ModelLocal.position() +
      tripod.dirAxis * mCurrent.ScreenFactor * startOffset);
    const recti::Vec2 axisEndOnScreen = mCurrent.CameraMouse.WorldToPos(
      mCurrent.ModelLocal.position() +
      tripod.dirAxis * mCurrent.ScreenFactor * endOffset);

    recti::Vec4 closestPointOnAxis =
      recti::PointOnSegment({ posOnPlanScreen.x, posOnPlanScreen.y },
                            { axisStartOnScreen.x, axisStartOnScreen.y },
                            { axisEndOnScreen.x, axisEndOnScreen.y });

    if ((closestPointOnAxis -
         recti::Vec4{ posOnPlanScreen.x, posOnPlanScreen.y })
          .Length() < 12.f) // pixel size
    {
      return (recti::MOVETYPE)(recti::MT_SCALE_X + i);
    }
  }

  return MT_NONE;
}

static void
ComputeColors(uint32_t colors[7], MOVETYPE type, const Style& style)
{
  uint32_t selectionColor = style.GetColorU32(SELECTION);

  colors[0] = (type == MT_SCALE_XYZ) ? selectionColor : COL32_WHITE();
  for (int i = 0; i < 3; i++) {
    colors[i + 1] = (type == (int)(MT_SCALE_X + i))
                      ? selectionColor
                      : style.GetColorU32(DIRECTION_X + i);
  }
}

MOVETYPE
ScaleGizmo::Hover(const ModelContext& current)
{
  if (!Intersects(current.Operation, SCALE)) {
    return MT_NONE;
  }
  return GetType(current, m_allowAxisFlip);
}

void
ScaleGizmo::Draw(const ModelContext& mCurrent,
                 MOVETYPE active,
                 MOVETYPE hover,
                 const Style& mStyle,
                 DrawList& drawList)
{
  // colors
  uint32_t colors[7];
  ComputeColors(colors, hover, mStyle);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.Operation, static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }

    const bool usingAxis = (active == MT_NONE || active == MT_SCALE_X + i);
    if (usingAxis) {
      Tripod tripod(i);
      tripod.ComputeTripodAxisAndVisibility(mCurrent, m_allowAxisFlip);

      // draw axis
      if (tripod.belowAxisLimit) {
        bool hasTranslateOnAxis = Contains(
          mCurrent.Operation, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        Vec2 baseSSpace =
          worldToPos(tripod.dirAxis * 0.1f * mCurrent.ScreenFactor,
                     mCurrent.MVP,
                     mCurrent.CameraMouse.Camera.Viewport);
        Vec2 worldDirSSpaceNoScale =
          worldToPos(tripod.dirAxis * markerScale * mCurrent.ScreenFactor,
                     mCurrent.MVP,
                     mCurrent.CameraMouse.Camera.Viewport);
        Vec2 worldDirSSpace =
          worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                       mCurrent.ScreenFactor,
                     mCurrent.MVP,
                     mCurrent.CameraMouse.Camera.Viewport);

        if (!hasTranslateOnAxis || false) {
          drawList.AddLine(baseSSpace,
                           worldDirSSpace,
                           colors[i + 1],
                           mStyle.ScaleLineThickness);
        }
        drawList.AddCircleFilled(
          worldDirSSpace, mStyle.ScaleLineCircleSize, colors[i + 1]);

        // if (mState.mAxisFactor[i] < 0.f) {
        //   drawList->DrawHatchedAxis(
        //     mCurrent, tripod.dirAxis * scaleDisplay[i], mStyle);
        // }
      }
    }
  }

  // draw screen cirle
  drawList.AddCircleFilled(
    mCurrent.ScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);
}

} // namespace
