#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../style.h"
#include <memory>

namespace recti {

struct IDragHandle
{
  virtual ~IDragHandle(){};

  virtual bool Drag(const ModelContext& current,
                    const float* snap,
                    float* matrix,
                    float* deltaMatrix) = 0;

  virtual void Draw(const ModelContext& current,
                    const Style& style,
                    std::shared_ptr<DrawList>& drawList) = 0;
};

}
