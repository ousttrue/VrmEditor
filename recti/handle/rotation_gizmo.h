#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../style.h"
#include "../vec4.h"
#include "handle.h"
#include <memory>

namespace recti {

struct RotationGizmo : public IGizmo
{
  MOVETYPE Hover(const ModelContext& current) override;

  void Draw(const ModelContext& current,
            MOVETYPE active,
            MOVETYPE hover,
            const Style& style,
            DrawList& drawList) override;
};

} // namespace
