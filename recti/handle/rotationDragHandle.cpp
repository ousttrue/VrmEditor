#include "rotationDragHandle.h"
#include <numbers>

namespace recti {

static const float ZPI = 3.14159265358979323846f;
static const float DEG2RAD = (ZPI / 180.f);

static const int HALF_CIRCLE_SEGMENT_COUNT = 64;

// scale a bit so translate axis do not touch when in universal
const float ROTATION_DISPLAY_FACTOR = 1.2f;

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

RotationDragHandle::RotationDragHandle(const ModelContext& mCurrent,
                                       MOVETYPE type)
  : m_type(type)
{
  // auto& mouse = mCurrent.mCameraMouse.Mouse;
  // auto type = GetType(mCurrent, mRadiusSquareCenter, mState);

  bool applyRotationLocaly = mCurrent.mMode == recti::LOCAL;
  if (m_type == recti::MT_ROTATE_SCREEN) {
    applyRotationLocaly = true;
  }

  // mState.mbUsing = true;
  // mState.mEditingID = mCurrent.mActualID;
  // mState.mCurrentOperation = type;
  const recti::Vec4 rotatePlanNormal[] = { mCurrent.mModel.right(),
                                           mCurrent.mModel.up(),
                                           mCurrent.mModel.dir(),
                                           mCurrent.mCameraMouse.CameraDir() };
  // pickup plan
  if (applyRotationLocaly) {
    mTranslationPlan = BuildPlan(mCurrent.mModel.position(),
                                 rotatePlanNormal[m_type - recti::MT_ROTATE_X]);
  } else {
    mTranslationPlan =
      BuildPlan(mCurrent.mModelSource.position(),
                Vec4::DirectionUnary[m_type - recti::MT_ROTATE_X]);
  }

  recti::Vec4 localPos =
    mCurrent.mCameraMouse.Ray.IntersectPlane(mTranslationPlan) -
    mCurrent.mModel.position();
  mRotationVectorSource = Normalized(localPos);
  mRotationAngleOrigin =
    ComputeAngleOnPlan(mCurrent, mRotationVectorSource, mTranslationPlan);
}

bool
RotationDragHandle::Drag(const recti::ModelContext& mCurrent,
                         const float* snap,
                         float* matrix,
                         float* deltaMatrix)
{

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
    *(recti::Mat4*)matrix = scaleOrigin * deltaRotation * mCurrent.mModelLocal;
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

  return modified;
}

void
RotationDragHandle::Draw(const ModelContext& mCurrent,
                         const Style& mStyle,
                         std::shared_ptr<DrawList>& drawList)
{
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
    circlePos[i] =
      mCurrent.mCameraMouse.WorldToPos(pos + mCurrent.mModel.position());
  }
  drawList->AddConvexPolyFilled((const recti::VEC2*)circlePos,
                                HALF_CIRCLE_SEGMENT_COUNT,
                                mStyle.GetColorU32(recti::ROTATION_USING_FILL));
  drawList->AddPolyline((const recti::VEC2*)circlePos,
                        HALF_CIRCLE_SEGMENT_COUNT,
                        mStyle.GetColorU32(recti::ROTATION_USING_BORDER),
                        true,
                        mStyle.RotationLineThickness);

  recti::Vec2 destinationPosOnScreen = circlePos[1];

  static const char* rotationInfoMask[] = { "X : %5.2f deg %5.2f rad",
                                            "Y : %5.2f deg %5.2f rad",
                                            "Z : %5.2f deg %5.2f rad",
                                            "Screen : %5.2f deg %5.2f rad" };
  char tmps[512];
  snprintf(tmps,
           sizeof(tmps),
           rotationInfoMask[m_type - recti::MT_ROTATE_X],
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

} // namespace
