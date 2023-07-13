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

  bool HandleTranslation(const ModelContext& mCurrent,
                         bool mAllowAxisFlip,
                         float* matrix,
                         float* deltaMatrix,
                         MOVETYPE& type,
                         const float* snap,
                         State& mState);

  void DrawTranslationGizmo(const ModelContext& mCurrent,
                            bool mAllowAxisFlip,
                            MOVETYPE type,
                            Style& mStyle,
                            State& mState,
                            const std::shared_ptr<DrawList>& drawList);
};

} // namespace
