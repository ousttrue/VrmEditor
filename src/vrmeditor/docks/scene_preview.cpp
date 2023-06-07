#include <GL/glew.h>

#include "overlay.h"
#include "scene_preview.h"
#include <DirectXMath.h>
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <glr/cuber.h>
#include <glr/gl3renderer.h>
#include <glr/line_gizmo.h>
#include <vrm/gizmo.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

ScenePreview::ScenePreview(const std::shared_ptr<glr::RenderingEnv>& env,
                           const std::shared_ptr<grapho::OrbitView>& view,
                           const std::shared_ptr<glr::ViewSettings>& settings,
                           const std::shared_ptr<SceneNodeSelection>& selection,
                           const glr::RenderFunc& callback)
  : m_selection(selection)
{
  m_renderer = std::make_shared<glr::SceneRenderer>(env, settings);
  m_fbo = ImFbo::Create(view, callback);
}

std::shared_ptr<ScenePreview>
ScenePreview::Create(const std::shared_ptr<libvrm::RuntimeScene>& scene,
                     const std::shared_ptr<glr::RenderingEnv>& env,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<glr::ViewSettings>& settings,
                     const std::shared_ptr<SceneNodeSelection>& selection)
{
  return std::make_shared<ScenePreview>(
    env,
    view,
    settings,
    selection,
    [scene, renderer = glr::SceneRenderer(env, settings)](auto &view) {
      renderer.RenderRuntime(scene, view);
    });
}

std::shared_ptr<ScenePreview>
ScenePreview::Create(const std::shared_ptr<libvrm::GltfRoot>& scene,
                     const std::shared_ptr<glr::RenderingEnv>& env,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<glr::ViewSettings>& settings,
                     const std::shared_ptr<SceneNodeSelection>& selection)
{
  auto ptr = std::make_shared<ScenePreview>(
    env,
    view,
    settings,
    selection,
    [scene, renderer = glr::SceneRenderer(env, settings)](auto &view) {
      renderer.RenderStatic(scene, view);
    });
  return ptr;
}

void
ScenePreview::ShowScreenRect(const char* title,
                             const float color[4],
                             float x,
                             float y,
                             float w,
                             float h)
{
  if (w <= 0 || h <= 0) {
    return;
  }
  auto sc = ImGui::GetCursorScreenPos();
  m_fbo->ShowFbo(x, y, w, h, color);
  // top, right pivot
  Overlay({ sc.x + w - 10, sc.y + 10 }, title);
}

void
ScenePreview::ShowFullWindow(const char* title, const float color[4])
{
  auto pos = ImGui::GetWindowPos();
  pos.y += ImGui::GetFrameHeight();
  auto size = ImGui::GetContentRegionAvail();
  ShowScreenRect(title, color, pos.x, pos.y, size.x, size.y);
}
