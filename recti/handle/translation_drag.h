#pragma once
#include "../operation.h"
#include "handle.h"

namespace recti {

struct TranslationDragHandle : public IDragHandle
{
  MOVETYPE m_type;
  Vec4 Plain;
  Vec4 PlainOrigin;
  Vec4 ModelPosition;
  Vec4 mTranslationLastDelta;

  TranslationDragHandle(const CameraMouse& cameraMouse,
                        const Mat4& model,
                        MOVETYPE type);

  MOVETYPE Type() const override { return m_type; }

  bool Drag(const ModelContext& current,
            const float* snap,
            float* matrix,
            float* deltaMatrix) override;
  void Draw(const ModelContext& current,
            const Style& style,
            DrawList& drawList) override;
};

} // namespace
