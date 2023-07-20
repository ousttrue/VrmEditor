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
  int64_t mEditingID = -1;
  bool mbUsing = false;
  // save axis factor when using gizmo
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];
  recti::MOVETYPE mCurrentOperation;

  std::shared_ptr<IDragHandle> DragHandle;

  bool Using(uint64_t actualID) const
  {
    if (mbUsing) {
      if (actualID == -1) {
        return true;
      }
      if (actualID == mEditingID) {
        return true;
      }
    }
    return false;
  }
};

}
