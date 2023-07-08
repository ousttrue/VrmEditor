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
  void Begin(const float* view,
             const float* projection,
             float x,
             float y,
             float width,
             float height,
             const recti::Vec2& mousePos,
             bool mouseLeftDown);
  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr,
                  const float* localBounds = nullptr,
                  const float* boundsSnap = nullptr);
  const DrawList& End();
};

} // namespace
