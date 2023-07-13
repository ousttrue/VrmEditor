#pragma once
#include "drawcommand.h"
#include "model_context.h"
#include "operation.h"
#include "state.h"
#include "vec4.h"
#include <memory>

namespace recti {

struct Translation
{
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  struct Result
  {
    MOVETYPE DrawType;
    bool Modified;
  };

  Result HandleTranslation(const ModelContext& current,
                           bool allowAxisFlip,
                           const float* snap,
                           State& state,
                           float* matrix,
                           float* deltaMatrix);

  void Begin(const ModelContext& current, MOVETYPE type);

  bool Drag(const ModelContext& current,
            const State& state,
            const float* snap,
            float* matrix,
            float* deltaMatrix);

  void DrawTranslationGizmo(const ModelContext& current,
                            bool allowAxisFlip,
                            MOVETYPE type,
                            const Style& style,
                            const State& state,
                            const std::shared_ptr<DrawList>& drawList);
};

} // namespace
