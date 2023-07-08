#pragma once
#include "drawcommand.h"

struct Vec2;

namespace recti {

struct Operation
{
  bool EnableT = false;
  bool EnableR = false;
  bool EnableS = false;
  bool IsLocalSpace = false;
};

class Screen
{
  struct ScreenImpl* m_impl;

public:
  Screen();
  ~Screen();
  void SetRect(float x, float y, float width, float height);
  bool Manipulate(void* id,
                  const float* view,
                  const float* projection,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr,
                  const float* localBounds = nullptr,
                  const float* boundsSnap = nullptr);
  const DrawList& GetDrawList();
};

} // namespace
