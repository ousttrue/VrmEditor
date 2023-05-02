#pragma once
#include "rendering_env.h"
#include "rendertarget.h"
#include <grapho/orbitview.h>
#include <memory>
#include <optional>
#include <string>
#include <vrm/timeline.h>

namespace runtimescene {
struct RuntimeScene;
}

namespace glr {
class Cuber;
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

  void Popup(const std::string& name);
};

struct ScenePreview
{
  std::shared_ptr<RenderTarget> m_rt;
  std::shared_ptr<Cuber> m_cuber;

  // imgui
  std::string m_popupName = "ScenePreviewPopup";
  std::function<void()> m_popup;

  ScenePreview(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
               const std::shared_ptr<RenderingEnv>& env,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<ViewSettings>& settings);

  ScenePreview(const std::shared_ptr<runtimescene::RuntimeScene>& scene)
    : ScenePreview(scene,
                   std::make_shared<RenderingEnv>(),
                   std::make_shared<grapho::OrbitView>(),
                   std::make_shared<ViewSettings>())
  {
  }

  void ShowScreenRect(const char* title,
                      const float color[4],
                      float x,
                      float y,
                      float w,
                      float h);
  void ShowFullWindow(const char* title, const float color[4]);
};

}
