#pragma once
#include "im_fbo.h"
#include "scene_selection.h"
#include <glr/scene_renderer.h>

class ScenePreview
{
  std::shared_ptr<glr::SceneRenderer> m_renderer;
  std::shared_ptr<ImFbo> m_fbo;
  std::shared_ptr<SceneNodeSelection> m_selection;

public:
  ScenePreview(const std::shared_ptr<glr::RenderingEnv>& env,
               const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<glr::ViewSettings>& settings,
               const std::shared_ptr<SceneNodeSelection>& selection,
               const glr::RenderFunc& callback);

  // runtime scene
  static std::shared_ptr<ScenePreview> Create(
    const std::shared_ptr<libvrm::RuntimeScene>& scene,
    const std::shared_ptr<glr::RenderingEnv>& env = nullptr,
    const std::shared_ptr<grapho::OrbitView>& view = nullptr,
    const std::shared_ptr<glr::ViewSettings>& settings = nullptr,
    const std::shared_ptr<SceneNodeSelection>& selection = nullptr);

  // T-Pose scene
  static std::shared_ptr<ScenePreview> Create(
    const std::shared_ptr<libvrm::GltfRoot>& scene,
    const std::shared_ptr<glr::RenderingEnv>& env = nullptr,
    const std::shared_ptr<grapho::OrbitView>& view = nullptr,
    const std::shared_ptr<glr::ViewSettings>& settings = nullptr,
    const std::shared_ptr<SceneNodeSelection>& selection = nullptr);

  void ShowScreenRect(const char* title,
                      const float color[4],
                      float x,
                      float y,
                      float w,
                      float h);
  void ShowFullWindow(const char* title, const float color[4]);
};
