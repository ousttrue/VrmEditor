#pragma once
#include "handle.h"

namespace recti {

struct ScaleDragHandle : public IDragHandle
{
  MOVETYPE m_type;
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  Vec4 mScale;
  Vec4 mScaleValueOrigin;
  Vec4 mScaleLast;
  float mSaveMousePosx;

  ScaleDragHandle(const ModelContext& mCurrent, MOVETYPE type);

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
