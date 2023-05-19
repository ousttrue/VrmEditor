#pragma once
#include <functional>
#include <memory>

namespace grapho {
struct OrbitView;
namespace gl3 {
struct Fbo;
class Texture;
}
}

namespace glr {
using RenderFunc = std::function<void(const grapho::OrbitView& view)>;

struct RenderTarget
{
  std::shared_ptr<grapho::OrbitView> View;
  std::shared_ptr<grapho::gl3::Fbo> Fbo;
  std::shared_ptr<grapho::gl3::Texture> FboTexture;
  RenderFunc render;

  RenderTarget(const std::shared_ptr<grapho::OrbitView>& view);
  void ShowFbo(float x, float y, float w, float h, const float color[4]);

private:
  uint32_t Clear(int width, int height, const float color[4]);
};

}
