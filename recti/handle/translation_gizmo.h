#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../operation.h"
#include "../vec4.h"
#include "handle.h"
#include <memory>

namespace recti {

struct TranslationGizmo : IGizmo
{
  bool m_allowAxisFlip = true;

  MOVETYPE Hover(const ModelContext& current) override;

  void Draw(const ModelContext& current,
            MOVETYPE active,
            MOVETYPE hover,
            const Style& style,
            DrawList& drawList) override;
};

} // namespace
