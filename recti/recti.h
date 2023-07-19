#pragma once
#include "camera_mouse.h"
#include "drawcommand.h"
#include "operation.h"
#include <memory>

namespace recti {

class Screen
{
  struct ScreenImpl* m_impl;

public:
  Screen();
  ~Screen();

  void Begin(const Camera& camera, const Mouse& mouse);
  const DrawList& End() const;

  bool Manipulate(int64_t actualID,
                  OPERATION operation,
                  MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap);

  bool Manipulate(void* id,
                  const Operation& operation,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr)
  {
    return Manipulate((int64_t)id,
                      ToOperation(operation),
                      ToMode(operation),
                      matrix,
                      deltaMatrix,
                      snap);
  }

  void DrawCubes(const float* cubes, uint32_t count);
};

} // namespace
