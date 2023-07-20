#include "translation.h"
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

MOVETYPE
Translation::GetType(const ModelContext& current,
                         bool allowAxisFlip,
                         State* state)
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
                     { axisStartOnScreen.x, axisStartOnScreen.y },
                     { axisEndOnScreen.x, axisEndOnScreen.y });
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
Translation::DrawGizmo(const ModelContext& current,
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

        Vec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
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
}

} // namespace
