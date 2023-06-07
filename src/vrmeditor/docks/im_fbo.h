#pragma once
#include <glr/rendertarget.h>
#include <memory>

struct ImFbo
{
  std::shared_ptr<glr::RenderTarget> m_rt;

  static std::shared_ptr<ImFbo> Create(
    const std::shared_ptr<grapho::OrbitView>& view,
    const glr::RenderFunc& callback);
  void ShowFbo(float x, float y, float w, float h, const float color[4]);
};
