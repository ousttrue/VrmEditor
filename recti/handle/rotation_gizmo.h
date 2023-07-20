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
  float m_radius = 110;
  bool m_isOrthographic = false;

  bool Enabled(OPERATION operation) const override
  {
    return Intersects(operation, ROTATE);
  }

  MOVETYPE Hover(const ModelContext& current) override;

  void Draw(const ModelContext& current,
            MOVETYPE active,
            MOVETYPE hover,
            const Style& style,
            std::shared_ptr<DrawList>& drawList) override;
};

} // namespace
