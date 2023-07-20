#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../style.h"
#include "../vec4.h"
#include "handle.h"
#include <memory>

namespace recti {

struct Rotation
{
  static MOVETYPE GetType(const recti::ModelContext& mCurrent,
                          float mRadiusSquareCenter);

  static void DrawGizmo(const ModelContext& mCurrent,
                        float mRadiusSquareCenter,
                        bool mIsOrthographic,
                        MOVETYPE type,
                        const Style& mStyle,
                        const std::shared_ptr<DrawList>& drawList);

  static void ComputeColors(uint32_t colors[7],
                            MOVETYPE type,
                            const Style& style);
};

} // namespace
