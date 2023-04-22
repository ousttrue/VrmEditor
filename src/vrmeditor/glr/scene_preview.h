#pragma once
// #include "gl3renderer.h"
// #include "line_gizmo.h"
// #include "rendertarget.h"
// #include <vrm/scene.h>
// #include <memory>
#include <grapho/orbitview.h>
#include <optional>
#include <vrm/scene.h>

namespace glr {
class Cuber;
struct RenderTarget;
// struct LineGizmo;
struct ScenePreview
{
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<RenderTarget> m_rt;
  ScenePreview(const std::shared_ptr<libvrm::gltf::Scene>& scene,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<libvrm::gltf::SceneContext>& context);
  ScenePreview(const std::shared_ptr<libvrm::gltf::Scene>& scene)
    : ScenePreview(scene,
                   std::make_shared<grapho::OrbitView>(),
                   std::make_shared<libvrm::gltf::SceneContext>())
  {
  }
  void Show(const char* title, std::optional<DirectX::XMFLOAT4> rect = {});
};
}
