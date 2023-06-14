#include "scene_preview.h"
#include "im_fbo.h"
#include "overlay.h"
#include <DirectXMath.h>
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <glr/cuber.h>
#include <glr/gl3renderer.h>
#include <glr/line_gizmo.h>
#include <glr/scene_renderer.h>
#include <vrm/gizmo.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

struct ScenePreviewImpl
{
  std::string m_title;
  std::array<float, 4> m_clear{ 0, 0, 0, 0 };
  std::shared_ptr<ImFbo> m_fbo;
  std::shared_ptr<glr::RenderingEnv> m_env;
  std::shared_ptr<grapho::OrbitView> m_view;
  std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::SceneRenderer> m_renderer;

  std::function<void(const grapho::OrbitView& view)> m_show;

  ScenePreviewImpl(const std::shared_ptr<glr::RenderingEnv>& env)
    : m_view(new grapho::OrbitView)
    , m_settings(new glr::ViewSettings)
  {
    m_renderer = std::make_shared<glr::SceneRenderer>(m_env, m_settings);
    m_fbo = ImFbo::Create(m_view,
                          [=](const grapho::OrbitView& view) { m_show(view); });
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
  {
    m_show = [root, renderer = m_renderer](const grapho::OrbitView& view) {
      renderer->RenderStatic(root, view);
    };
  }

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_show = [runtime, renderer = m_renderer](const grapho::OrbitView& view) {
      renderer->RenderRuntime(runtime, view);
    };
  }

  void ShowScreenRect(const char* title,
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

  void ShowFullWindow(const char* title, const float color[4])
  {
    auto pos = ImGui::GetWindowPos();
    pos.y += ImGui::GetFrameHeight();
    auto size = ImGui::GetContentRegionAvail();
    ShowScreenRect(title, color, pos.x, pos.y, size.x, size.y);
  }

  void ShowGui() { ShowFullWindow(m_title.c_str(), m_clear.data()); }
};

//
// ScenePreview
//
ScenePreview::ScenePreview(const std::shared_ptr<glr::RenderingEnv>& env)
  : m_impl(new ScenePreviewImpl(env))
{
}

ScenePreview::~ScenePreview()
{
  delete m_impl;
}

void
ScenePreview::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_impl->SetGltf(root);
}

void
ScenePreview::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_impl->SetRuntime(runtime);
}

void
ScenePreview::ShowScreenRect(const char* title,
                             const float color[4],
                             float x,
                             float y,
                             float w,
                             float h)
{
  m_impl->ShowScreenRect(title, color, x, y, w, h);
}

void
ScenePreview::ShowFullWindow(const char* title, const float color[4])
{
  m_impl->ShowFullWindow(title, color);
}

void
ScenePreview::ShowGui()
{
  m_impl->ShowGui();
}
