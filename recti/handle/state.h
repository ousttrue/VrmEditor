#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../style.h"
#include "operation.h"
#include <memory>
#include <stdint.h>

namespace recti {

struct IDragHandle
{
  virtual ~IDragHandle(){};
  virtual bool Drag(const ModelContext& current,
                    const struct State& state,
                    const float* snap,
                    float* matrix,
                    float* deltaMatrix) = 0;

  virtual void Draw(const ModelContext& current,
                    const Style& style,
                    std::shared_ptr<DrawList>& drawList) = 0;
};

struct State
{
  // save axis factor when using gizmo
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];

  std::shared_ptr<IDragHandle> DragHandle;
};

}
