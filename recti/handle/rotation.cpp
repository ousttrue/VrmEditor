#include "rotation.h"
#include <numbers>

namespace recti {

static const float ZPI = 3.14159265358979323846f;
static const float DEG2RAD = (ZPI / 180.f);

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

const float screenRotateSize = 0.06f;

static const char* rotationInfoMask[] = { "X : %5.2f deg %5.2f rad",
                                          "Y : %5.2f deg %5.2f rad",
                                          "Z : %5.2f deg %5.2f rad",
                                          "Screen : %5.2f deg %5.2f rad" };
static const int HALF_CIRCLE_SEGMENT_COUNT = 64;

static recti::MOVETYPE
GetRotateType(const recti::ModelContext& mCurrent,
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

static float
ComputeAngleOnPlan(const recti::ModelContext& mCurrent,
                   const recti::Vec4& mRotationVectorSource,
                   const recti::Vec4& mTranslationPlan)
{
  recti::Vec4 localPos =
    Normalized(mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan) -
               mCurrent.mModel.position());

  recti::Vec4 perpendicularVector;
  perpendicularVector.Cross(mRotationVectorSource, mTranslationPlan);
  perpendicularVector.Normalize();
  float acosAngle =
    recti::Clamp(Dot(localPos, mRotationVectorSource), -1.f, 1.f);
  float angle = acosf(acosAngle);
  angle *= (Dot(localPos, perpendicularVector) < 0.f) ? 1.f : -1.f;
  return angle;
}

Result
Rotation::HandleRotation(const recti::ModelContext& mCurrent,
                         float mRadiusSquareCenter,
                         State& mState,
                         const float* snap,
                         float* matrix,
                         float* deltaMatrix)
{
  if (!Intersects(mCurrent.mOperation, recti::ROTATE)) {
    return {};
  }

  // rotation
  if (mState.Using(mCurrent.mActualID) &&
      IsRotateType(mState.mCurrentOperation)) {

    // DRAG
    mRotationAngle =
      ComputeAngleOnPlan(mCurrent, mRotationVectorSource, mTranslationPlan);
    if (snap) {
      float snapInRadian = snap[0] * DEG2RAD;
      recti::ComputeSnap(&mRotationAngle, snapInRadian);
    }
    recti::Vec4 rotationAxisLocalSpace;

    rotationAxisLocalSpace.TransformVector(
      { mTranslationPlan.x, mTranslationPlan.y, mTranslationPlan.z, 0.f },
      mCurrent.mModelInverse);
    rotationAxisLocalSpace.Normalize();

    recti::Mat4 deltaRotation;
    deltaRotation.RotationAxis(rotationAxisLocalSpace,
                               mRotationAngle - mRotationAngleOrigin);
    auto modified = false;
    if (mRotationAngle != mRotationAngleOrigin) {
      modified = true;
    }
    mRotationAngleOrigin = mRotationAngle;

    recti::Mat4 scaleOrigin;
    scaleOrigin.Scale(mCurrent.mModelScaleOrigin);

    bool applyRotationLocaly = mCurrent.mMode == recti::LOCAL;
    if (applyRotationLocaly) {
      *(recti::Mat4*)matrix =
        scaleOrigin * deltaRotation * mCurrent.mModelLocal;
    } else {
      recti::Mat4 res = mCurrent.mModelSource;
      res.position().Set(0.f);

      *(recti::Mat4*)matrix = res * deltaRotation;
      ((recti::Mat4*)matrix)->position() = mCurrent.mModelSource.position();
    }

    if (deltaMatrix) {
      *(recti::Mat4*)deltaMatrix =
        mCurrent.mModelInverse * deltaRotation * mCurrent.mModel;
    }

    if (!mCurrent.mCameraMouse.Mouse.LeftDown) {
      mState.mbUsing = false;
      mState.mEditingID = -1;
    }
    return { mState.mCurrentOperation, modified };
  }

  auto& mouse = mCurrent.mCameraMouse.Mouse;
  auto type = GetRotateType(mCurrent, mRadiusSquareCenter, mState);

  bool applyRotationLocaly = mCurrent.mMode == recti::LOCAL;
  if (type == recti::MT_ROTATE_SCREEN) {
    applyRotationLocaly = true;
  }

  if (mouse.LeftDown && type != recti::MT_NONE) {
    mState.mbUsing = true;
    mState.mEditingID = mCurrent.mActualID;
    mState.mCurrentOperation = type;
    const recti::Vec4 rotatePlanNormal[] = {
      mCurrent.mModel.right(),
      mCurrent.mModel.up(),
      mCurrent.mModel.dir(),
      mCurrent.mCameraMouse.CameraDir()
    };
    // pickup plan
    if (applyRotationLocaly) {
      mTranslationPlan = BuildPlan(mCurrent.mModel.position(),
                                   rotatePlanNormal[type - recti::MT_ROTATE_X]);
    } else {
      mTranslationPlan =
        BuildPlan(mCurrent.mModelSource.position(),
                  Vec4::DirectionUnary[type - recti::MT_ROTATE_X]);
    }

    recti::Vec4 localPos =
      mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan) -
      mCurrent.mModel.position();
    mRotationVectorSource = Normalized(localPos);
    mRotationAngleOrigin =
      ComputeAngleOnPlan(mCurrent, mRotationVectorSource, mTranslationPlan);
  }

  return { type };
}

void
Rotation::DrawRotationGizmo(const recti::ModelContext& mCurrent,
                            float mRadiusSquareCenter,
                            bool mIsOrthographic,
                            recti::MOVETYPE type,
                            const State& mState,
                            const Style& mStyle,
                            const std::shared_ptr<DrawList>& drawList)
{
  if (!Intersects(mCurrent.mOperation, recti::ROTATE)) {
    return;
  }
  // auto drawList = mDrawList;

  // colors
  uint32_t colors[7];
  mStyle.ComputeColors(colors, type, recti::ROTATE);

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
      sqrtf((mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position()) - circlePos[0])
              .SqrLength());
    if (radiusAxis > mRadiusSquareCenter) {
      mRadiusSquareCenter = radiusAxis;
    }
  }
  if (hasRSC && (!mState.mbUsing || type == recti::MT_ROTATE_SCREEN)) {
    drawList->AddCircle(mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position()),
                        mRadiusSquareCenter,
                        colors[0],
                        64,
                        mStyle.RotationOuterLineThickness);
  }

  if (mState.Using(mCurrent.mActualID) && IsRotateType(type)) {
    recti::Vec2 circlePos[HALF_CIRCLE_SEGMENT_COUNT + 1];

    circlePos[0] = mCurrent.mCameraMouse.WorldToPos(mCurrent.mModel.position());
    for (unsigned int i = 1; i < HALF_CIRCLE_SEGMENT_COUNT; i++) {
      float ng = mRotationAngle *
                 ((float)(i - 1) / (float)(HALF_CIRCLE_SEGMENT_COUNT - 1));
      recti::Mat4 rotateVectorMatrix;
      rotateVectorMatrix.RotationAxis(mTranslationPlan, ng);
      recti::Vec4 pos;
      pos.TransformPoint(mRotationVectorSource, rotateVectorMatrix);
      pos *= mCurrent.mScreenFactor * ROTATION_DISPLAY_FACTOR;
      circlePos[i] = mCurrent.mCameraMouse.WorldToPos(pos + mCurrent.mModel.position());
    }
    drawList->AddConvexPolyFilled(
      (const recti::VEC2*)circlePos,
      HALF_CIRCLE_SEGMENT_COUNT,
      mStyle.GetColorU32(recti::ROTATION_USING_FILL));
    drawList->AddPolyline((const recti::VEC2*)circlePos,
                          HALF_CIRCLE_SEGMENT_COUNT,
                          mStyle.GetColorU32(recti::ROTATION_USING_BORDER),
                          true,
                          mStyle.RotationLineThickness);

    recti::Vec2 destinationPosOnScreen = circlePos[1];
    char tmps[512];
    snprintf(tmps,
             sizeof(tmps),
             rotationInfoMask[type - recti::MT_ROTATE_X],
             (mRotationAngle / std::numbers::pi) * 180.f,
             mRotationAngle);
    drawList->AddText(
      recti::Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
      mStyle.GetColorU32(recti::TEXT_SHADOW),
      tmps);
    drawList->AddText(
      recti::Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
      mStyle.GetColorU32(recti::TEXT),
      tmps);
  }
}

} // namespace
