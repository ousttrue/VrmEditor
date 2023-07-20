#pragma once
#include "../model_context.h"
#include "../operation.h"
#include "handle.h"

namespace recti {

struct RotationDragHandle : public IDragHandle
{
  MOVETYPE m_type;

  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  Vec4 mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;

  RotationDragHandle(const ModelContext& current, MOVETYPE type);
  MOVETYPE Type() const override { return m_type; }
  bool Drag(const ModelContext& current,
            const float* snap,
            float* matrix,
            float* deltaMatrix) override;
  void Draw(const ModelContext& current,
            const Style& style,
            DrawList& drawList) override;
};

} // namespace
