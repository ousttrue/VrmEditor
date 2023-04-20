#pragma once
#include "rendering_env.h"
#include <functional>
#include <memory>

namespace grapho {
class OrbitView;
namespace gl3 {
struct Fbo;
}
}

namespace glr {
struct RenderTarget
{
  RenderingEnv Env = {};
  std::shared_ptr<grapho::OrbitView> view;
  std::shared_ptr<grapho::gl3::Fbo> fbo;
  float color[4];
  std::function<void(const RenderingEnv& camera)> render;

  RenderTarget(const std::shared_ptr<grapho::OrbitView>& view);
  uint32_t clear(int width, int height);
  void show_fbo(float x, float y, float w, float h);
};
}
