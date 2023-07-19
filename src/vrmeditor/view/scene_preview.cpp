#include "scene_preview.h"
#include "im_fbo.h"
#include "overlay.h"
#include <DirectXMath.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <glr/cuber.h>
#include <glr/gl3renderer.h>
#include <glr/line_gizmo.h>
#include <glr/rendering_env.h>
#include <glr/rendertarget.h>
#include <glr/scene_renderer.h>
#include <grapho/camera/camera.h>
#include <imgui.h>
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
  std::shared_ptr<glr::ViewSettings> m_settings;
  std::shared_ptr<glr::SceneRenderer> m_renderer;

  glr::RenderFunc m_show;

  ScenePreviewImpl(const std::shared_ptr<glr::RenderingEnv>& env)
    : m_env(env)
    , m_settings(new glr::ViewSettings)
  {
    m_renderer = std::make_shared<glr::SceneRenderer>(m_env, m_settings);
    m_fbo = ImFbo::Create();
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root,
               const GetSelectedNode& getSelected)
  {
    m_title = root->m_title;
    m_show = [root, getSelected, renderer = m_renderer](
               const grapho::camera::Viewport& viewport,
               const grapho::camera::MouseState& mouse) {
      std::shared_ptr<libvrm::Node> selected;
      if (getSelected) {
        selected = getSelected();
      }
      renderer->RenderStatic(root, viewport, mouse, selected);
    };
    auto [min, max] = root->GetBoundingBox();
    m_renderer->m_camera->Fit(min, max);
    if (m_env) {
      m_env->SetShadowHeight(min.y);
    }
  }

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime,
                  const GetSelectedRuntimeNode& getSelected)
  {
    m_title = runtime->m_base->m_title;
    m_show = [runtime, getSelected, renderer = m_renderer](
               const grapho::camera::Viewport& viewport,
               const grapho::camera::MouseState& mouse) {
      std::shared_ptr<libvrm::RuntimeNode> selected;
      if (getSelected) {
        selected = getSelected();
      }
      renderer->RenderRuntime(runtime, viewport, mouse, selected);
    };
    auto [min, max] = runtime->m_base->GetBoundingBox();
    m_renderer->m_camera->Fit(min, max);
    if (m_env) {
      m_env->SetShadowHeight(min.y);
    }
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
    grapho::camera::Viewport vp{ x, y, w, h };
    m_fbo->ShowFbo(vp, color, [=](const auto& vp, const auto& mouse) {
      if (m_show) {
        m_show(vp, mouse);
      }
    });
    // top, right pivot
    Overlay({ sc.x + w - 10, sc.y + 10 }, title);
  }

  void ShowFullWindow(const char* title, const float color[4])
  {
    // auto pos = ImGui::GetWindowPos();
    // pos.y += ImGui::GetFrameHeight();
    auto pos = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetContentRegionAvail();
    ShowScreenRect(title, color, pos.x, pos.y, size.x, size.y);
  }

  void ShowGui()
  {
    ImGui::Checkbox("grid", &m_renderer->m_settings->ShowLine);
    ImGui::SameLine();
    ImGui::Checkbox("mesh", &m_renderer->m_settings->ShowMesh);
    ImGui::SameLine();
    ImGui::Checkbox("bone", &m_renderer->m_settings->ShowCuber);
    ImGui::SameLine();
    ImGui::Checkbox("shadow", &m_renderer->m_settings->ShowShadow);
    ShowFullWindow(m_title.c_str(), m_clear.data());
  }
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
ScenePreview::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root,
                      const GetSelectedNode& getSelected)
{
  m_impl->SetGltf(root, getSelected);
}

void
ScenePreview::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime,
                         const GetSelectedRuntimeNode& getSelected)
{
  m_impl->SetRuntime(runtime, getSelected);
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

std::shared_ptr<glr::ViewSettings>
ScenePreview::Settings()
{
  return m_impl->m_settings;
}
