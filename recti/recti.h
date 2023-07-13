#pragma once
#include "camera_mouse.h"
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
  void Begin(const Camera& camera, const Mouse& mouse);
  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr);
  const DrawList& End();

  void DrawCubes(const float* cubes, uint32_t count);
};

} // namespace
