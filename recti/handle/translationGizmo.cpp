#include "translationGizmo.h"
#include "operation.h"
#include "style.h"
#include "tripod.h"
#include <stdio.h>

namespace recti {

static const float quadMin = 0.5f;
static const float quadMax = 0.8f;
static const float quadUV[8] = { quadMin, quadMin, quadMin, quadMax,
                                 quadMax, quadMax, quadMax, quadMin };

static const OPERATION TRANSLATE_PLANS[3] = { TRANSLATE_Y | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Z,
                                              TRANSLATE_X | TRANSLATE_Y };

static MOVETYPE
GetType(const ModelContext& current, bool allowAxisFlip)
{
  // screen
  if (current.MouseInScreenSquare()) {
    return MT_MOVE_SCREEN;
  }

  for (int i = 0; i < 3; i++) {

    Tripod tripod(i);
    tripod.ComputeTripodAxisAndVisibility(current, allowAxisFlip);
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
                     { axisStartOnScreen.x, axisStartOnScreen.y },
                     { axisEndOnScreen.x, axisEndOnScreen.y });
    if ((closestPointOnAxis - screenCoord).Length() < 12.f &&
        Intersects(current.mOperation,
                   static_cast<OPERATION>(TRANSLATE_X << i))) // pixel size
    {
      return (MOVETYPE)(MT_MOVE_X + i);
    }

    const float dx = tripod.dirPlaneX.Dot3(
      (posOnPlan - current.mModel.position()) * (1.f / current.mScreenFactor));
    const float dy = tripod.dirPlaneY.Dot3(
      (posOnPlan - current.mModel.position()) * (1.f / current.mScreenFactor));
    if (tripod.belowPlaneLimit && dx >= quadUV[0] && dx <= quadUV[4] &&
        dy >= quadUV[1] && dy <= quadUV[3] &&
        Contains(current.mOperation, TRANSLATE_PLANS[i])) {
      return (MOVETYPE)(MT_MOVE_YZ + i);
    }
  }

  return MT_NONE;
}

static void
ComputeColors(uint32_t colors[7], MOVETYPE type, const Style& style)
{
  uint32_t selectionColor = style.GetColorU32(SELECTION);

  colors[0] = (type == MT_MOVE_SCREEN) ? selectionColor : COL32_WHITE();
  for (int i = 0; i < 3; i++) {
    colors[i + 1] = (type == (int)(MT_MOVE_X + i))
                      ? selectionColor
                      : style.GetColorU32(DIRECTION_X + i);
    colors[i + 4] = (type == (int)(MT_MOVE_YZ + i))
                      ? selectionColor
                      : style.GetColorU32(PLANE_X + i);
    colors[i + 4] = (type == MT_MOVE_SCREEN) ? selectionColor : colors[i + 4];
  }
}

MOVETYPE
TranslationGizmo::Hover(const ModelContext& current)
{
  return GetType(current, m_allowAxisFlip);
}

void
TranslationGizmo::Draw(const ModelContext& current,
                       MOVETYPE active,
                       MOVETYPE hover,
                       const Style& style,
                       std::shared_ptr<DrawList>& drawList)

{
  // colors
  uint32_t colors[7];
  ComputeColors(colors, hover, style);

  const Vec2 origin =
    current.mCameraMouse.WorldToPos(current.mModel.position());

  // draw
  for (int i = 0; i < 3; ++i) {
    Tripod tripod(i);
    tripod.ComputeTripodAxisAndVisibility(current, m_allowAxisFlip);

    if (active == MT_NONE || (active == MT_MOVE_X + i)) {
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

        Vec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
        Vec2 a(worldDirSSpace + dir);
        drawList->AddTriangleFilled(worldDirSSpace - dir,
                                    a + ortogonalDir,
                                    a - ortogonalDir,
                                    colors[i + 1]);
        // Arrow head end
        // if (state.mAxisFactor[i] < 0.f) {
        //   drawList->DrawHatchedAxis(current, tripod.dirAxis, style);
        // }
      }
    }

    // draw plane
    if (active == MT_NONE || (active == MT_MOVE_YZ + i)) {
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
}

} // namespace