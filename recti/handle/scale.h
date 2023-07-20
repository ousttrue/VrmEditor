#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../style.h"
#include "../vec4.h"
#include <memory>

namespace recti {

struct Scale
{
  static MOVETYPE GetType(const recti::ModelContext& mCurrent,
                          bool mAllowAxisFlip);

  static void DrawGizmo(const ModelContext& mCurrent,
                        bool allowAxisFlip,
                        MOVETYPE type,
                        const Style& mStyle,
                        const std::shared_ptr<DrawList>& drawList);

  static void DrawUniveralGizmo(const ModelContext& mCurrent,
                                bool mAllowAxisFlip,
                                MOVETYPE type,
                                const Style& mStyle,
                                const std::shared_ptr<DrawList>& drawList);

  static void ComputeColors(uint32_t colors[7],
                            MOVETYPE type,
                            const Style& style);
};

} // namespace
