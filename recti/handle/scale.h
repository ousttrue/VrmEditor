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
  static MOVETYPE GetType(const recti::ModelContext& mCurrent,
                          bool mAllowAxisFlip,
                          State* state);

  static void DrawGizmo(const ModelContext& mCurrent,
                        MOVETYPE type,
                        const State& mState,
                        const Style& mStyle,
                        const std::shared_ptr<DrawList>& drawList);

  static void DrawUniveralGizmo(const ModelContext& mCurrent,
                                bool mAllowAxisFlip,
                                MOVETYPE type,
                                const State& mState,
                                const Style& mStyle,
                                const std::shared_ptr<DrawList>& drawList);
};

} // namespace
