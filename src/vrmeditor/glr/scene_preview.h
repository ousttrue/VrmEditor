#pragma once
#include "docks/scene_selection.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include <grapho/orbitview.h>
#include <memory>
#include <optional>
#include <string>
#include <vrm/animation/timeline.h>

namespace runtimescene {
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
  bool ShowCuber = true;

  float Color[4] = { 0.2f, 0.2f, 0.2f, 1 };

  bool Skybox = true;

  void Popup(const std::string& name);
};

struct ScenePreview
{
  std::shared_ptr<runtimescene::RuntimeScene> m_animation;
  std::shared_ptr<RenderingEnv> m_env;
  std::shared_ptr<ViewSettings> m_settings;
  std::shared_ptr<SceneNodeSelection> m_selection;

  std::shared_ptr<RenderTarget> m_rt;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<LineGizmo> m_gizmo;

  ScenePreview(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
               const std::shared_ptr<RenderingEnv>& env,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<ViewSettings>& settings,
               const std::shared_ptr<SceneNodeSelection>& selection,
               bool useTPose);

  ScenePreview(const std::shared_ptr<runtimescene::RuntimeScene>& scene)
    : ScenePreview(scene,
                   std::make_shared<RenderingEnv>(),
                   std::make_shared<grapho::OrbitView>(),
                   std::make_shared<ViewSettings>(),
                   std::make_shared<SceneNodeSelection>(),
                   false)
  {
  }

  void ShowScreenRect(const char* title,
                      const float color[4],
                      float x,
                      float y,
                      float w,
                      float h);
  void ShowFullWindow(const char* title, const float color[4]);

  void RenderTPose(const grapho::OrbitView& view);
  void RenderAnimation(const grapho::OrbitView& view);
};

}
