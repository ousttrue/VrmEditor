#include "translationDragHandle.h"

namespace recti {

TranslationDragHandle::TranslationDragHandle(const ModelContext& current,
                                             MOVETYPE type)
  : m_type(type)
{
  // find new possible way to move
  Vec4 movePlanNormal[] = {
    current.mModel.right(),           current.mModel.up(), current.mModel.dir(),
    current.mModel.right(),           current.mModel.up(), current.mModel.dir(),
    -current.mCameraMouse.CameraDir()
  };

  Vec4 cameraToModelNormalized =
    Normalized(current.mModel.position() - current.mCameraMouse.CameraEye());
  for (unsigned int i = 0; i < 3; i++) {
    Vec4 orthoVector = Cross(movePlanNormal[i], cameraToModelNormalized);
    movePlanNormal[i].Cross(orthoVector);
    movePlanNormal[i].Normalize();
  }
  // pickup plan
  mTranslationPlan =
    BuildPlan(current.mModel.position(), movePlanNormal[type - MT_MOVE_X]);
  mTranslationPlanOrigin =
    current.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);
  mMatrixOrigin = current.mModel.position();

  mRelativeOrigin = (mTranslationPlanOrigin - current.mModel.position()) *
                    (1.f / current.mScreenFactor);
}

bool
TranslationDragHandle::Drag(const ModelContext& current,
                            const float* snap,
                            float* matrix,
                            float* deltaMatrix)
{
  const Vec4 newPos = current.mCameraMouse.Ray.IntersectPlane(mTranslationPlan);

  // compute delta
  const Vec4 newOrigin = newPos - mRelativeOrigin * current.mScreenFactor;
  Vec4 delta = newOrigin - current.mModel.position();

  // 1 axis constraint
  if (m_type >= MT_MOVE_X && m_type <= MT_MOVE_Z) {
    const int axisIndex = m_type - MT_MOVE_X;
    const Vec4& axisValue = current.mModel.component(axisIndex);
    const float lengthOnAxis = Dot(axisValue, delta);
    delta = axisValue * lengthOnAxis;
  }

  // snap
  if (snap) {
    Vec4 cumulativeDelta = current.mModel.position() + delta - mMatrixOrigin;
    const bool applyRotationLocaly =
      current.mMode == LOCAL || m_type == MT_MOVE_SCREEN;
    if (applyRotationLocaly) {
      Mat4 modelSourceNormalized = current.mModelSource;
      modelSourceNormalized.OrthoNormalize();
      Mat4 modelSourceNormalizedInverse;
      modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
      cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
      ComputeSnap(cumulativeDelta, snap);
      cumulativeDelta.TransformVector(modelSourceNormalized);
    } else {
      ComputeSnap(cumulativeDelta, snap);
    }
    delta = mMatrixOrigin + cumulativeDelta - current.mModel.position();
  }

  auto modified = false;
  if (delta != mTranslationLastDelta) {
    modified = true;
    mTranslationLastDelta = delta;
  }

  // compute matrix & delta
  Mat4 deltaMatrixTranslation;
  deltaMatrixTranslation.Translation(delta);
  if (deltaMatrix) {
    memcpy(deltaMatrix, &deltaMatrixTranslation.m00, sizeof(float) * 16);
  }

  const Mat4 res = current.mModelSource * deltaMatrixTranslation;
  *(Mat4*)matrix = res;

  return modified;
}

void
TranslationDragHandle::Draw(const ModelContext& current,
                            const Style& style,
                            std::shared_ptr<DrawList>& drawList)
{
  uint32_t translationLineColor = style.GetColorU32(TRANSLATION_LINE);

  Vec2 sourcePosOnScreen = current.mCameraMouse.WorldToPos(mMatrixOrigin);
  Vec2 destinationPosOnScreen =
    current.mCameraMouse.WorldToPos(current.mModel.position());
  Vec4 dif = { destinationPosOnScreen.x - sourcePosOnScreen.x,
               destinationPosOnScreen.y - sourcePosOnScreen.y,
               0.f,
               0.f };
  dif.Normalize();
  dif *= 5.f;
  drawList->AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
  drawList->AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
  drawList->AddLine(
    Vec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y + dif.y),
    Vec2(destinationPosOnScreen.x - dif.x, destinationPosOnScreen.y - dif.y),
    translationLineColor,
    2.f);

  char tmps[512];
  Vec4 deltaInfo = current.mModel.position() - mMatrixOrigin;
  int componentInfoIndex = (m_type - MT_MOVE_X) * 3;
  static const int translationInfoIndex[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 2,
                                              0, 0, 2, 0, 0, 1, 0, 0, 1, 2 };
  static const char* translationInfoMask[] = {
    "X : %5.3f",
    "Y : %5.3f",
    "Z : %5.3f",
    "Y : %5.3f Z : %5.3f",
    "X : %5.3f Z : %5.3f",
    "X : %5.3f Y : %5.3f",
    "X : %5.3f Y : %5.3f Z : %5.3f"
  };

  snprintf(tmps,
           sizeof(tmps),
           translationInfoMask[m_type - MT_MOVE_X],
           deltaInfo[translationInfoIndex[componentInfoIndex]],
           deltaInfo[translationInfoIndex[componentInfoIndex + 1]],
           deltaInfo[translationInfoIndex[componentInfoIndex + 2]]);
  drawList->AddText(
    Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
    style.GetColorU32(TEXT_SHADOW),
    tmps);
  drawList->AddText(
    Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
    style.GetColorU32(TEXT),
    tmps);
}

} // namespace
