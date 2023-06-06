#pragma once
#include "docks/scene_selection.h"
#include "rendering_env.h"
#include "renderpass.h"
#include "rendertarget.h"
#include <grapho/orbitview.h>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vrm/timeline.h>

namespace libvrm {
struct RuntimeScene;
struct DrawItem;
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
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<RenderingEnv> m_env;
  std::shared_ptr<ViewSettings> m_settings;
  std::shared_ptr<SceneNodeSelection> m_selection;

  std::shared_ptr<RenderTarget> m_rt;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<LineGizmo> m_gizmo;

  std::vector<RenderPass> m_renderpass;

  ScenePreview(const std::shared_ptr<libvrm::RuntimeScene>& scene,
               const std::shared_ptr<RenderingEnv>& env,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<ViewSettings>& settings,
               const std::shared_ptr<SceneNodeSelection>& selection,
               bool useTPose);

  ScenePreview(const std::shared_ptr<libvrm::RuntimeScene>& scene)
    : ScenePreview(scene,
                   std::make_shared<RenderingEnv>(),
                   std::make_shared<grapho::OrbitView>(),
                   std::make_shared<ViewSettings>(),
                   std::make_shared<SceneNodeSelection>(),
                   false)
  {
  }

  void RenderStatic(const grapho::OrbitView& view);
  void RenderRuntime(const grapho::OrbitView& view);

private:
  void Render(std::span<libvrm::DrawItem> drawables);

public:
  void ShowScreenRect(const char* title,
                      const float color[4],
                      float x,
                      float y,
                      float w,
                      float h);
  void ShowFullWindow(const char* title, const float color[4]);
};

} // namespace
