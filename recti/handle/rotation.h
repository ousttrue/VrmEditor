#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../state.h"
#include "../style.h"
#include "../vec4.h"
#include "result.h"
#include <memory>

namespace recti {

struct Rotation
{
  Vec4 mTranslationPlan;
  Vec4 mTranslationPlanOrigin;
  Vec4 mMatrixOrigin;
  Vec4 mTranslationLastDelta;
  Vec4 mRelativeOrigin;

  Vec4 mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;

  static MOVETYPE GetRotateType(const recti::ModelContext& mCurrent,
                                float mRadiusSquareCenter,
                                const recti::State& mState);

  Result HandleRotation(const ModelContext& mCurrent,
                        float mRadiusSquareCenter,
                        State& mState,
                        const float* snap,
                        float* matrix,
                        float* deltaMatrix);

  void DrawRotationGizmo(const ModelContext& mCurrent,
                         float mRadiusSquareCenter,
                         bool mIsOrthographic,
                         MOVETYPE type,
                         const State& mState,
                         const Style& mStyle,
                         const std::shared_ptr<DrawList>& drawList);
};

} // namespace
