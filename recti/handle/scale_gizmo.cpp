#include "scale_gizmo.h"
#include "../tripod.h"

namespace recti {

static MOVETYPE
GetType(const recti::ModelContext& current, bool allowAxisFlip)
{
  for (int i = 0; i < 3; i++) {
    if (!Intersects(current.Operation,
                    static_cast<recti::OPERATION>(recti::SCALE_X << i))) {
      continue;
    }
    recti::Tripod tripod(current.MVP,
                         current.CameraMouse.Camera.AspectRatio(),
                         current.ScreenFactor,
                         allowAxisFlip,
                         i);
    tripod.Axis.TransformVector(current.ModelLocal);
    tripod.PlaneX.TransformVector(current.ModelLocal);
    tripod.PlaneY.TransformVector(current.ModelLocal);

    recti::Vec4 posOnPlan = current.CameraMouse.Ray.IntersectPlane(
      BuildPlan(current.ModelLocal.position(), tripod.Axis));

    const float startOffset =
      Contains(current.Operation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.0f
        : 0.1f;
    const float endOffset =
      Contains(current.Operation,
               static_cast<recti::OPERATION>(recti::TRANSLATE_X << i))
        ? 1.4f
        : 1.0f;
    const recti::Vec2 posOnPlanScreen =
      current.CameraMouse.WorldToPos(posOnPlan);
    const recti::Vec2 axisStartOnScreen = current.CameraMouse.WorldToPos(
      current.ModelLocal.position() +
      tripod.Axis * current.ScreenFactor * startOffset);
    const recti::Vec2 axisEndOnScreen = current.CameraMouse.WorldToPos(
      current.ModelLocal.position() +
      tripod.Axis * current.ScreenFactor * endOffset);

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
  // if (!Intersects(current.Operation, SCALE)) {
  // }
  return GetType(current, m_allowAxisFlip);
}

void
ScaleGizmo::Draw(const ModelContext& current,
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
    if (!Intersects(current.Operation, static_cast<OPERATION>(SCALE_X << i))) {
      continue;
    }

    const bool usingAxis = (active == MT_NONE || active == MT_SCALE_X + i);
    if (usingAxis) {
      Tripod tripod(current.MVP,
                    current.CameraMouse.Camera.AspectRatio(),
                    current.ScreenFactor,
                    m_allowAxisFlip,
                    i);

      // draw axis
      if (tripod.VisibleAxis) {
        bool hasTranslateOnAxis =
          Contains(current.Operation, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        Vec2 baseSSpace = worldToPos(tripod.Axis * 0.1f * current.ScreenFactor,
                                     current.MVP,
                                     current.CameraMouse.Camera.Viewport);
        Vec2 worldDirSSpaceNoScale =
          worldToPos(tripod.Axis * markerScale * current.ScreenFactor,
                     current.MVP,
                     current.CameraMouse.Camera.Viewport);
        Vec2 worldDirSSpace = worldToPos(
          (tripod.Axis * markerScale * scaleDisplay[i]) * current.ScreenFactor,
          current.MVP,
          current.CameraMouse.Camera.Viewport);

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
        //     current, tripod.dirAxis * scaleDisplay[i], mStyle);
        // }
      }
    }
  }

  // draw screen cirle
  drawList.AddCircleFilled(
    current.ScreenSquareCenter, mStyle.CenterCircleSize, colors[0], 32);
}

//
// Uniform
//
static MOVETYPE
GetUniformType(const recti::ModelContext& current, bool allowAxisFlip)
{
  auto& mousePos = current.CameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.x - current.ScreenSquareCenter.x,
                              mousePos.y - current.ScreenSquareCenter.y,
                              0.f,
                              0.f };
  float dist = deltaScreen.Length();
  if (Contains(current.Operation, recti::SCALEU) && dist >= 17.0f &&
      dist < 23.0f) {
    return recti::MT_SCALE_XYZ;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(current.Operation,
                    static_cast<recti::OPERATION>(recti::SCALE_XU << i))) {
      continue;
    }

    recti::Tripod tripod(current.MVP,
                         current.CameraMouse.Camera.AspectRatio(),
                         current.ScreenFactor,
                         allowAxisFlip,
                         i);
    tripod.Axis.TransformVector(current.ModelLocal);
    tripod.PlaneX.TransformVector(current.ModelLocal);
    tripod.PlaneY.TransformVector(current.ModelLocal);

    // draw axis
    if (tripod.VisibleAxis) {
      bool hasTranslateOnAxis =
        Contains(current.Operation,
                 static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
      float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
      recti::Vec2 worldDirSSpace =
        recti::worldToPos((tripod.Axis * markerScale) * current.ScreenFactor,
                          current.MVPLocal,
                          current.CameraMouse.Camera.Viewport);

      float distance = sqrtf((worldDirSSpace - mousePos).SqrLength());
      if (distance < 12.f) {
        return (recti::MOVETYPE)(recti::MT_SCALE_X + i);
      }
    }
  }

  return MT_NONE;
}

} // namespace
