#include "scaleDragHandle.h"

namespace recti {

static const char* scaleInfoMask[] = { "X : %5.2f",
                                       "Y : %5.2f",
                                       "Z : %5.2f",
                                       "XYZ : %5.2f" };

static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                            0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };

//
// ScaleDragHandle
//
ScaleDragHandle::ScaleDragHandle(const ModelContext& mCurrent, MOVETYPE type)
  : m_type(type)
{
  const Vec4 movePlanNormal[] = { mCurrent.Model.up(),
                                  mCurrent.Model.dir(),
                                  mCurrent.Model.right(),
                                  mCurrent.Model.dir(),
                                  mCurrent.Model.up(),
                                  mCurrent.Model.right(),
                                  -mCurrent.CameraMouse.CameraDir() };

  // pickup plan

  mTranslationPlan =
    BuildPlan(mCurrent.Model.position(), movePlanNormal[type - MT_SCALE_X]);
  mTranslationPlanOrigin =
    mCurrent.CameraMouse.Ray.IntersectPlane(mTranslationPlan);
  mMatrixOrigin = mCurrent.Model.position();
  mScale.Set(1.f, 1.f, 1.f);
  mRelativeOrigin = (mTranslationPlanOrigin - mCurrent.Model.position()) *
                    (1.f / mCurrent.ScreenFactor);
  mScaleValueOrigin = { mCurrent.ModelSource.right().Length(),
                        mCurrent.ModelSource.up().Length(),
                        mCurrent.ModelSource.dir().Length() };

  auto& mouse = mCurrent.CameraMouse.Mouse;
  mSaveMousePosx = mouse.Position.x;
}

bool
ScaleDragHandle::Drag(const ModelContext& mCurrent,
                      const float* snap,
                      float* matrix,
                      float* deltaMatrix)
{
  // drag
  Vec4 newPos = mCurrent.CameraMouse.Ray.IntersectPlane(mTranslationPlan);
  Vec4 newOrigin = newPos - mRelativeOrigin * mCurrent.ScreenFactor;
  Vec4 delta = newOrigin - mCurrent.ModelLocal.position();

  // 1 axis constraint
  {
    int axisIndex = m_type - MT_SCALE_X;
    const Vec4& axisValue = mCurrent.ModelLocal.component(axisIndex);
    float lengthOnAxis = Dot(axisValue, delta);
    delta = axisValue * lengthOnAxis;

    Vec4 baseVector = mTranslationPlanOrigin - mCurrent.ModelLocal.position();
    float ratio =
      Dot(axisValue, baseVector + delta) / Dot(axisValue, baseVector);

    mScale[axisIndex] = max(ratio, 0.001f);
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

  Mat4 res = deltaMatrixScale * mCurrent.ModelLocal;
  *(Mat4*)matrix = res;

  if (deltaMatrix) {
    Vec4 deltaScale = mScale * mScaleValueOrigin;

    Vec4 originalScaleDivider;
    originalScaleDivider.x = 1 / mCurrent.ModelScaleOrigin.x;
    originalScaleDivider.y = 1 / mCurrent.ModelScaleOrigin.y;
    originalScaleDivider.z = 1 / mCurrent.ModelScaleOrigin.z;

    deltaScale = deltaScale * originalScaleDivider;

    deltaMatrixScale.Scale(deltaScale);
    memcpy(deltaMatrix, &deltaMatrixScale.m00, sizeof(float) * 16);
  }

  return modified;
}

void
ScaleDragHandle::Draw(const ModelContext& mCurrent,
                      const Style& mStyle,
                      DrawList& drawList)
{
  Vec2 destinationPosOnScreen =
    worldToPos(mCurrent.Model.position(),
               mCurrent.CameraMouse.mViewProjection,
               mCurrent.CameraMouse.Camera.Viewport);
  char tmps[512];
  int componentInfoIndex = (m_type - MT_SCALE_X) * 3;
  snprintf(tmps,
           sizeof(tmps),
           scaleInfoMask[m_type - MT_SCALE_X],
           mScale[translationInfoIndex[componentInfoIndex]]);
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
    mStyle.GetColorU32(TEXT_SHADOW),
    tmps);
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
    mStyle.GetColorU32(TEXT),
    tmps);
}

//
// ScaleUDragHandle
//
ScaleUDragHandle::ScaleUDragHandle(const ModelContext& mCurrent, MOVETYPE type)
  : ScaleDragHandle(mCurrent, type)
{
}

bool
ScaleUDragHandle::Drag(const ModelContext& mCurrent,
                       const float* snap,
                       float* matrix,
                       float* deltaMatrix)
{
  {
    float scaleDelta =
      (mCurrent.CameraMouse.Mouse.Position.x - mSaveMousePosx) * 0.01f;
    mScale.Set(max(1.f + scaleDelta, 0.001f));
  }

  return false;
}

void
ScaleUDragHandle::Draw(const ModelContext& mCurrent,
                       const Style& mStyle,
                       DrawList& drawList)
{

  Vec2 destinationPosOnScreen =
    mCurrent.CameraMouse.WorldToPos(mCurrent.Model.position());

  char tmps[512];
  int componentInfoIndex = (m_type - MT_SCALE_X) * 3;
  snprintf(tmps,
           sizeof(tmps),
           scaleInfoMask[m_type - MT_SCALE_X],
           mScale[translationInfoIndex[componentInfoIndex]]);
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
    mStyle.GetColorU32(TEXT_SHADOW),
    tmps);
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
    mStyle.GetColorU32(TEXT),
    tmps);

  // if (mState.Using(mCurrent.mActualID)) {
  //   uint32_t scaleLineColor = mStyle.GetColorU32(SCALE_LINE);
  //   drawList->AddLine(baseSSpace,
  //                     worldDirSSpaceNoScale,
  //                     scaleLineColor,
  //                     mStyle.ScaleLineThickness);
  //   drawList->AddCircleFilled(
  //     worldDirSSpaceNoScale, mStyle.ScaleLineCircleSize, scaleLineColor);
  // }
}

} // namespace
