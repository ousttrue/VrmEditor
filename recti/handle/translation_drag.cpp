#include "translation_drag.h"

namespace recti {

inline Vec4
CalcNormal(const CameraMouse& cameraMouse, const Mat4& model, MOVETYPE type)
{
  // find new possible way to move
  Vec4 movePlanNormal[] = {
    model.right(),           // x
    model.up(),              // y
    model.dir(),             // z
    model.right(),           // yz
    model.up(),              // zx
    model.dir(),             // xy
    -cameraMouse.CameraDir() // screen
  };

  auto cameraToModel = Normalized(model.position() - cameraMouse.CameraEye());
  for (unsigned int i = 0; i < 3; i++) {
    Vec4 orthoVector = Cross(movePlanNormal[i], cameraToModel);
    movePlanNormal[i] = Normalized(Cross(movePlanNormal[i], orthoVector));
  }
  return movePlanNormal[type - MT_MOVE_X];
}

TranslationDragHandle::TranslationDragHandle(const CameraMouse& cameraMouse,
                                             const Mat4& model,
                                             MOVETYPE type)
  : m_type(type)
{
  // pickup plan
  auto normal = CalcNormal(cameraMouse, model, type);
  Plain = BuildPlan(model.position(), normal);
  PlainOrigin = cameraMouse.Ray.IntersectPlane(Plain);
  ModelPosition = model.position();
}

bool
TranslationDragHandle::Drag(const ModelContext& current,
                            const float* snap,
                            float* matrix,
                            float* deltaMatrix)
{
  const Vec4 newRayPos = current.CameraMouse.Ray.IntersectPlane(Plain);

  // compute delta
  const Vec4 newModelPos = ModelPosition + (newRayPos - PlainOrigin);
  Vec4 delta = newModelPos - current.Model.position();

  // 1 axis constraint
  if (m_type >= MT_MOVE_X && m_type <= MT_MOVE_Z) {
    const int axisIndex = m_type - MT_MOVE_X;
    const Vec4& axisValue = current.Model.component(axisIndex);
    const float lengthOnAxis = Dot(axisValue, delta);
    delta = axisValue * lengthOnAxis;
  }

  // snap
  if (snap) {
    Vec4 cumulativeDelta = current.Model.position() + delta - ModelPosition;
    const bool applyRotationLocaly =
      current.Mode == LOCAL || m_type == MT_MOVE_SCREEN;
    if (applyRotationLocaly) {
      Mat4 modelSourceNormalized = current.ModelSource;
      modelSourceNormalized.OrthoNormalize();
      Mat4 modelSourceNormalizedInverse;
      modelSourceNormalizedInverse.Inverse(modelSourceNormalized);
      cumulativeDelta.TransformVector(modelSourceNormalizedInverse);
      ComputeSnap(cumulativeDelta, snap);
      cumulativeDelta.TransformVector(modelSourceNormalized);
    } else {
      ComputeSnap(cumulativeDelta, snap);
    }
    delta = ModelPosition + cumulativeDelta - current.Model.position();
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

  const Mat4 res = current.ModelSource * deltaMatrixTranslation;
  *(Mat4*)matrix = res;

  return modified;
}

void
TranslationDragHandle::Draw(const ModelContext& current,
                            const Style& style,
                            DrawList& drawList)
{
  uint32_t translationLineColor = style.GetColorU32(TRANSLATION_LINE);

  Vec2 sourcePosOnScreen = current.CameraMouse.WorldToPos(ModelPosition);
  Vec2 destinationPosOnScreen =
    current.CameraMouse.WorldToPos(current.Model.position());
  Vec4 dif = { destinationPosOnScreen.x - sourcePosOnScreen.x,
               destinationPosOnScreen.y - sourcePosOnScreen.y,
               0.f,
               0.f };
  dif.Normalize();
  dif *= 5.f;
  drawList.AddCircle(sourcePosOnScreen, 6.f, translationLineColor);
  drawList.AddCircle(destinationPosOnScreen, 6.f, translationLineColor);
  drawList.AddLine(
    Vec2(sourcePosOnScreen.x + dif.x, sourcePosOnScreen.y + dif.y),
    Vec2(destinationPosOnScreen.x - dif.x, destinationPosOnScreen.y - dif.y),
    translationLineColor,
    2.f);

  char tmps[512];
  Vec4 deltaInfo = current.Model.position() - ModelPosition;
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
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
    style.GetColorU32(TEXT_SHADOW),
    tmps);
  drawList.AddText(
    Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
    style.GetColorU32(TEXT),
    tmps);
}

} // namespace
