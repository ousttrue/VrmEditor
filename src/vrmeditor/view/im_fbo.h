#pragma once
#include <functional>
#include <memory>

namespace grapho {
namespace gl3 {
struct RenderTarget;
}

namespace camera {
struct Viewport;
struct MouseState;
}
}

using RenderFunc = std::function<void(const grapho::camera::Viewport&,
                                      const grapho::camera::MouseState&)>;

class ImFbo
{
  std::shared_ptr<grapho::gl3::RenderTarget> m_rt;

public:
  static std::shared_ptr<ImFbo> Create();
  void ShowFbo(const grapho::camera::Viewport& viewport,
               const float color[4],
               const RenderFunc& render);
};
