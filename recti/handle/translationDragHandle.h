#pragma once
#include "../operation.h"
#include "handle.h"

namespace recti {

struct TranslationDragHandle : public IDragHandle
{
  MOVETYPE m_type;
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  TranslationDragHandle(const ModelContext& current, MOVETYPE type);

  MOVETYPE Type() const override { return m_type; }

  bool Drag(const ModelContext& current,
            const float* snap,
            float* matrix,
            float* deltaMatrix) override;
  void Draw(const ModelContext& current,
            const Style& style,
            std::shared_ptr<DrawList>& drawList) override;
};

} // namespace
