#pragma once
#include <grapho/camera/camera.h>
#include <span>
#include <vector>
#include <vrm/timeline.h>

namespace libvrm {
struct GltfRoot;
struct Node;
struct RuntimeScene;
struct RuntimeNode;
}

namespace recti {
struct Screen;
}

namespace glr {
class Gizmo;
struct LineGizmo;
struct RenderingEnv;

struct ViewSettings
{
  bool EnableSpring = true;
  libvrm::Time NextSpringDelta = libvrm::Time(0.0);

  // mesh
  bool ShowMesh = true;
  bool ShowShadow = true;
  // gizmo
  bool ShowLine = true;
  bool ShowCuber = false;
  bool ShowSpring = true;

  float Color[4] = { 0.2f, 0.2f, 0.2f, 1 };

  bool Skybox = true;
};

struct SceneRenderer
{
  std::shared_ptr<RenderingEnv> m_env;
  std::shared_ptr<ViewSettings> m_settings;
  std::shared_ptr<Gizmo> m_gizmo;
  std::shared_ptr<recti::Screen> m_screen;
  std::shared_ptr<grapho::camera::Camera> m_camera;

  SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                const std::shared_ptr<ViewSettings>& settings);

  void RenderStatic(const std::shared_ptr<libvrm::GltfRoot>& scene,
                    const grapho::camera::Viewport& viewport,
                    const grapho::camera::MouseState& mouse) const;
  void RenderRuntime(const std::shared_ptr<libvrm::RuntimeScene>& scene,
                     const grapho::camera::Viewport& viewport,
                     const grapho::camera::MouseState& mouse) const;
};

} // namespace
