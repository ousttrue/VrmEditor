#pragma once
#include <grapho/orbitview.h>
#include <optional>
#include <vrm/scene.h>

namespace glr {
class Cuber;
struct RenderTarget;
struct ScenePreview
{
  std::shared_ptr<RenderTarget> m_rt;
  std::shared_ptr<Cuber> m_cuber;

  // mesh
  bool m_showMesh = true;
  bool m_showShadow = true;
  // gizmo
  bool m_showGrid = true;
  bool m_showSpring = true;
  bool m_showCuber = true;

  ScenePreview(const std::shared_ptr<libvrm::gltf::Scene>& scene,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<libvrm::gltf::SceneContext>& context);
  ScenePreview(const std::shared_ptr<libvrm::gltf::Scene>& scene)
    : ScenePreview(scene,
                   std::make_shared<grapho::OrbitView>(),
                   std::make_shared<libvrm::gltf::SceneContext>())
  {
  }

  void ShowScreenRect(const char* title, float x, float y, float w, float h);
  void ShowFullWindow(const char* title);
};
}
