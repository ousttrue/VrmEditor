#pragma once
#include "camera_mouse.h"
#include "drawcommand.h"
#include "operation.h"

namespace recti {

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
