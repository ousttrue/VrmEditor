#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../style.h"
#include "../vec4.h"
#include "result.h"
#include "state.h"
#include <memory>

namespace recti {

struct Scale
{
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  Vec4 mScale;
  Vec4 mScaleValueOrigin;
  Vec4 mScaleLast;
  float mSaveMousePosx;

  static MOVETYPE GetType(const recti::ModelContext& mCurrent,
                          bool mAllowAxisFlip,
                          State* state);

  Result HandleScale(const ModelContext& mCurrent,
                     bool mAllowAxisFlip,
                     State& mState,
                     const float* snap,
                     float* matrix,
                     float* deltaMatrix);

  void DrawScaleGizmo(const ModelContext& mCurrent,
                      MOVETYPE type,
                      const State& mState,
                      const Style& mStyle,
                      const std::shared_ptr<DrawList>& drawList);

  void DrawScaleUniveralGizmo(const ModelContext& mCurrent,
                              bool mAllowAxisFlip,
                              MOVETYPE type,
                              const State& mState,
                              const Style& mStyle,
                              const std::shared_ptr<DrawList>& drawList);
};

} // namespace
