#pragma once
#include <grapho/camera/orbitview.h>
#include <span>
#include <vector>
#include <vrm/timeline.h>

namespace libvrm {
struct GltfRoot;
struct RuntimeScene;
}

namespace glr {
class Cuber;
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

  float Color[4] = { 0.2f, 0.2f, 0.2f, 1 };

  bool Skybox = true;
};

struct SceneRenderer
{
  std::shared_ptr<RenderingEnv> m_env;
  std::shared_ptr<ViewSettings> m_settings;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<LineGizmo> m_gizmo;

  SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                const std::shared_ptr<ViewSettings>& settings);

  void RenderStatic(const std::shared_ptr<libvrm::GltfRoot>& scene,
                    const grapho::camera::OrbitView& view) const;
  void RenderRuntime(const std::shared_ptr<libvrm::RuntimeScene>& scene,
                     const grapho::camera::OrbitView& view) const;
};

} // namespace
