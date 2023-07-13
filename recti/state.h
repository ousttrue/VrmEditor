#pragma once
#include "operation.h"
#include <stdint.h>

namespace recti {

struct State
{
  int64_t mEditingID = -1;
  bool mbUsing = false;
  // save axis factor when using gizmo
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];
  recti::MOVETYPE mCurrentOperation;

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
