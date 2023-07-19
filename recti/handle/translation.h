#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../state.h"
#include "../vec4.h"
#include "result.h"
#include <memory>

namespace recti {

struct Translation
{
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  static MOVETYPE GetMoveType(const ModelContext& current,
                              bool allowAxisFlip,
                              State* state);

  Result HandleTranslation(const ModelContext& current,
                           bool allowAxisFlip,
                           State& state,
                           const float* snap,
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
                            const State& state,
                            const Style& style,
                            const std::shared_ptr<DrawList>& drawList);
};

} // namespace
