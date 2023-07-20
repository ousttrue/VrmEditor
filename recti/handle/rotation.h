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

struct Rotation
{
  static MOVETYPE GetType(const recti::ModelContext& mCurrent,
                          float mRadiusSquareCenter,
                          const recti::State& mState);

  // Result HandleRotation(const ModelContext& mCurrent,
  //                       float mRadiusSquareCenter,
  //                       State& mState,
  //                       const float* snap,
  //                       float* matrix,
  //                       float* deltaMatrix);

  static void DrawGizmo(const ModelContext& mCurrent,
                        float mRadiusSquareCenter,
                        bool mIsOrthographic,
                        MOVETYPE type,
                        const State& mState,
                        const Style& mStyle,
                        const std::shared_ptr<DrawList>& drawList);
};

} // namespace
