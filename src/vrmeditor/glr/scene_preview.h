#pragma once
#include "gl3renderer.h"
#include "rendertarget.h"
#include <vrm/scene.h>

namespace glr {
struct ScenePreview
{
  RenderTarget m_rt;
  ScenePreview(const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<libvrm::Timeline>& timeline,
               const std::shared_ptr<libvrm::gltf::Scene>& scene,
               const std::shared_ptr<libvrm::gltf::SceneContext>& context);
  void Show();
  void Show(float x, float y, float z, float w) { m_rt.show_fbo(x, y, z, w); }
};
}
