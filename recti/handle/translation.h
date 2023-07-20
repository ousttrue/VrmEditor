#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../vec4.h"
#include "result.h"
#include "state.h"
#include <memory>

namespace recti {

struct Translation
{
  static MOVETYPE GetType(const ModelContext& current,
                          bool allowAxisFlip,
                          State* state);

  static void DrawGizmo(const ModelContext& current,
                        bool allowAxisFlip,
                        MOVETYPE type,
                        const State& state,
                        const Style& style,
                        const std::shared_ptr<DrawList>& drawList);

  static void ComputeColors(uint32_t colors[7],
                            MOVETYPE type,
                            const Style& style);
};

} // namespace
