#include "rotation_gizmo.h"
#include <iostream>
#include <numbers>

namespace recti {

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

static const int HALF_CIRCLE_SEGMENT_COUNT = 64;

static float
ScreenRadius(const ModelContext& current)
{
  // return current.CameraMouse.Camera.Height() * current.ScreenFactor *
  //        (ROTATION_DISPLAY_FACTOR * 1.2 / 2);
  return 120;
}

static MOVETYPE
GetType(const recti::ModelContext& current)
{
  auto& mousePos = current.CameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.x - current.ScreenSquareCenter.x,
                              mousePos.y - current.ScreenSquareCenter.y,
                              0.f,
                              0.f };

  const recti::Vec4 planNormals[] = { current.Model.right(),
                                      current.Model.up(),
                                      current.Model.dir() };

  recti::Vec4 modelViewPos;
  modelViewPos.TransformPoint(current.Model.position(),
                              current.CameraMouse.Camera.ViewMatrix);

  for (int i = 0; i < 3; i++) {
    if (!Intersects(current.Operation,
                    static_cast<recti::OPERATION>(recti::ROTATE_X << i))) {
      continue;
    }
    // pickup plan
    recti::Vec4 pickupPlan =
      BuildPlan(current.Model.position(), planNormals[i]);

    const recti::Vec4 intersectWorldPos =
      current.CameraMouse.Ray.IntersectPlane(pickupPlan);
    recti::Vec4 intersectViewPos;
    intersectViewPos.TransformPoint(intersectWorldPos,
                                    current.CameraMouse.Camera.ViewMatrix);

    if (fabs(modelViewPos.z) - fabs(intersectViewPos.z) < -FLT_EPSILON) {
      continue;
    }

    const recti::Vec4 localPos = intersectWorldPos - current.Model.position();
    recti::Vec4 idealPosOnCircle = Normalized(localPos);
    idealPosOnCircle.TransformVector(current.ModelInverse);
    const recti::Vec2 idealPosOnCircleScreen = recti::worldToPos(
      idealPosOnCircle * ROTATION_DISPLAY_FACTOR * current.ScreenFactor,
      current.MVP,
      current.CameraMouse.Camera.Viewport);

    const recti::Vec2 distanceOnScreen = idealPosOnCircleScreen - mousePos;

    const float distance =
      recti::Vec4{ distanceOnScreen.x, distanceOnScreen.y }.Length();
    if (distance < 8.f) // pixel size
    {
      return (recti::MOVETYPE)(recti::MT_ROTATE_X + i);
    }
  }

  float mRadiusSquareCenter = ScreenRadius(current);
  float dist = deltaScreen.Length();
  if (Intersects(current.Operation, recti::ROTATE_SCREEN) &&
      dist >= (mRadiusSquareCenter - 4.0f) &&
      dist < (mRadiusSquareCenter + 4.0f)) {
    return recti::MT_ROTATE_SCREEN;
  }

  return MT_NONE;
}

static void
ComputeColors(uint32_t colors[7], MOVETYPE type, const Style& style)
{
  uint32_t selectionColor = style.GetColorU32(SELECTION);

  colors[0] = (type == MT_ROTATE_SCREEN) ? selectionColor : COL32_WHITE();
  for (int i = 0; i < 3; i++) {
    colors[i + 1] = (type == (int)(MT_ROTATE_X + i))
                      ? selectionColor
                      : style.GetColorU32(DIRECTION_X + i);
  }
}

MOVETYPE
RotationGizmo::Hover(const ModelContext& current)
{
  if (!Intersects(current.Operation, ROTATE)) {
    return MT_NONE;
  }
  return GetType(current);
}

void
RotationGizmo::Draw(const ModelContext& current,
                    MOVETYPE active,
                    MOVETYPE hover,
                    const Style& style,
                    DrawList& drawList)
{
  // colors
  uint32_t colors[7];
  ComputeColors(colors, hover, style);

  recti::Vec4 cameraToModelNormalized;
  if (current.CameraMouse.Camera.IsOrthographic) {
    cameraToModelNormalized = -current.CameraMouse.mViewInverse.dir();
  } else {
    cameraToModelNormalized =
      Normalized(current.Model.position() - current.CameraMouse.CameraEye());
  }

  cameraToModelNormalized.TransformVector(current.ModelInverse);

  for (int axis = 0; axis < 3; axis++) {
    if (!Intersects(current.Operation,
                    static_cast<recti::OPERATION>(recti::ROTATE_Z >> axis))) {
      continue;
    }
    const bool activeAxis = active == recti::MT_ROTATE_Z - axis;
    const bool usingAxis = (active == MT_NONE || activeAxis);
    const int circleMul = !activeAxis ? 1 : 2;

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
                        current.ScreenFactor * ROTATION_DISPLAY_FACTOR;
      circlePos[i] = recti::worldToPos(
        pos, current.MVP, current.CameraMouse.Camera.Viewport);
    }

    if (usingAxis) {
      drawList.AddPolyline((const recti::VEC2*)circlePos,
                           circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                           colors[3 - axis],
                           false,
                           style.RotationLineThickness);
    }
  }

  bool hasRSC = Intersects(current.Operation, recti::ROTATE_SCREEN);
  if (hasRSC && (active == MT_NONE || active == recti::MT_ROTATE_SCREEN)) {
    auto r = ScreenRadius(current);
    drawList.AddCircle(current.CameraMouse.WorldToPos(current.Model.position()),
                       r,
                       colors[0],
                       64,
                       style.RotationOuterLineThickness);
  }
}

} // namespace
