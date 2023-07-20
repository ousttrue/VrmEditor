#include "rotation.h"
#include <numbers>

namespace recti {

static const float ZPI = 3.14159265358979323846f;
static const float DEG2RAD = (ZPI / 180.f);

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

const float screenRotateSize = 0.06f;

static const int HALF_CIRCLE_SEGMENT_COUNT = 64;

MOVETYPE
Rotation::GetType(const recti::ModelContext& mCurrent,
                  float mRadiusSquareCenter,
                  const recti::State& mState)
{
  if (mState.mbUsing) {
    return recti::MT_NONE;
  }
  recti::MOVETYPE type = recti::MT_NONE;

  auto& mousePos = mCurrent.mCameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.x - mCurrent.mScreenSquareCenter.x,
                              mousePos.y - mCurrent.mScreenSquareCenter.y,
                              0.f,
                              0.f };
  float dist = deltaScreen.Length();
  if (Intersects(mCurrent.mOperation, recti::ROTATE_SCREEN) &&
      dist >= (mRadiusSquareCenter - 4.0f) &&
      dist < (mRadiusSquareCenter + 4.0f)) {
    type = recti::MT_ROTATE_SCREEN;
  }

  const recti::Vec4 planNormals[] = { mCurrent.mModel.right(),
                                      mCurrent.mModel.up(),
                                      mCurrent.mModel.dir() };

  recti::Vec4 modelViewPos;
  modelViewPos.TransformPoint(mCurrent.mModel.position(),
                              mCurrent.mCameraMouse.Camera.ViewMatrix);

  for (int i = 0; i < 3 && type == recti::MT_NONE; i++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::ROTATE_X << i))) {
      continue;
    }
    // pickup plan
    recti::Vec4 pickupPlan =
      BuildPlan(mCurrent.mModel.position(), planNormals[i]);

    const recti::Vec4 intersectWorldPos =
      mCurrent.mCameraMouse.Ray.IntersectPlane(pickupPlan);
    recti::Vec4 intersectViewPos;
    intersectViewPos.TransformPoint(intersectWorldPos,
                                    mCurrent.mCameraMouse.Camera.ViewMatrix);

    if (fabs(modelViewPos.z) - fabs(intersectViewPos.z) < -FLT_EPSILON) {
      continue;
    }

    const recti::Vec4 localPos = intersectWorldPos - mCurrent.mModel.position();
    recti::Vec4 idealPosOnCircle = Normalized(localPos);
    idealPosOnCircle.TransformVector(mCurrent.mModelInverse);
    const recti::Vec2 idealPosOnCircleScreen = recti::worldToPos(
      idealPosOnCircle * ROTATION_DISPLAY_FACTOR * mCurrent.mScreenFactor,
      mCurrent.mMVP,
      mCurrent.mCameraMouse.Camera.Viewport);

    const recti::Vec2 distanceOnScreen = idealPosOnCircleScreen - mousePos;

    const float distance =
      recti::Vec4{ distanceOnScreen.x, distanceOnScreen.y }.Length();
    if (distance < 8.f) // pixel size
    {
      type = (recti::MOVETYPE)(recti::MT_ROTATE_X + i);
    }
  }

  return type;
}

void
Rotation::DrawGizmo(const recti::ModelContext& mCurrent,
                    float mRadiusSquareCenter,
                    bool mIsOrthographic,
                    recti::MOVETYPE type,
                    const State& mState,
                    const Style& mStyle,
                    const std::shared_ptr<DrawList>& drawList)
{
  // colors
  uint32_t colors[7];
  ComputeColors(colors, type, mStyle);

  recti::Vec4 cameraToModelNormalized;
  if (mIsOrthographic) {
    cameraToModelNormalized = -mCurrent.mCameraMouse.mViewInverse.dir();
  } else {
    cameraToModelNormalized = Normalized(mCurrent.mModel.position() -
                                         mCurrent.mCameraMouse.CameraEye());
  }

  cameraToModelNormalized.TransformVector(mCurrent.mModelInverse);

  mRadiusSquareCenter =
    screenRotateSize * mCurrent.mCameraMouse.Camera.Height();

  bool hasRSC = Intersects(mCurrent.mOperation, recti::ROTATE_SCREEN);
  for (int axis = 0; axis < 3; axis++) {
    if (!Intersects(mCurrent.mOperation,
                    static_cast<recti::OPERATION>(recti::ROTATE_Z >> axis))) {
      continue;
    }
    const bool usingAxis =
      (mState.mbUsing && type == recti::MT_ROTATE_Z - axis);
    const int circleMul = (hasRSC && !usingAxis) ? 1 : 2;

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
                        mCurrent.mScreenFactor * ROTATION_DISPLAY_FACTOR;
      circlePos[i] = recti::worldToPos(
        pos, mCurrent.mMVP, mCurrent.mCameraMouse.Camera.Viewport);
    }
    if (!mState.mbUsing || usingAxis) {
      drawList->AddPolyline((const recti::VEC2*)circlePos,
                            circleMul * HALF_CIRCLE_SEGMENT_COUNT + 1,
                            colors[3 - axis],
                            false,
                            mStyle.RotationLineThickness);
    }

    float radiusAxis =
      sqrtf((mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position()) -
             circlePos[0])
              .SqrLength());
    if (radiusAxis > mRadiusSquareCenter) {
      mRadiusSquareCenter = radiusAxis;
    }
  }
  if (hasRSC && (!mState.mbUsing || type == recti::MT_ROTATE_SCREEN)) {
    drawList->AddCircle(
      mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position()),
      mRadiusSquareCenter,
      colors[0],
      64,
      mStyle.RotationOuterLineThickness);
  }
}

void
Rotation::ComputeColors(uint32_t colors[7], MOVETYPE type, const Style& style)
{
  uint32_t selectionColor = style.GetColorU32(SELECTION);

  colors[0] = (type == MT_ROTATE_SCREEN) ? selectionColor : COL32_WHITE();
  for (int i = 0; i < 3; i++) {
    colors[i + 1] = (type == (int)(MT_ROTATE_X + i))
                      ? selectionColor
                      : style.GetColorU32(DIRECTION_X + i);
  }
}

} // namespace
