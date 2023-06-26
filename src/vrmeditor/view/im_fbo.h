#pragma once
#include <glr/rendertarget.h>
#include <memory>

class ImFbo
{
  std::shared_ptr<glr::RenderTarget> m_rt;

public:
  static std::shared_ptr<ImFbo> Create(
    const std::shared_ptr<grapho::camera::OrbitView>& view,
    const glr::RenderFunc& callback);
  void ShowFbo(float x, float y, float w, float h, const float color[4]);
};
