#pragma once

namespace recti {

void
BeginFrame();

void
SetRect(float x, float y, float width, float height);

struct Operation
{
  bool EnableT = false;
  bool EnableR = false;
  bool EnableS = false;
  bool IsLocalSpace = false;
};

bool
Manipulate(void* id,
           const float* view,
           const float* projection,
           // OPERATION operation,
           // MODE mode,
           const Operation& operation,
           float* matrix,
           float* deltaMatrix = nullptr,
           const float* snap = nullptr,
           const float* localBounds = nullptr,
           const float* boundsSnap = nullptr);

} // namespace
