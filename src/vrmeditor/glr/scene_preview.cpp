#include <GL/glew.h>

#include "../docks/overlay.h"
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

namespace glr {

void
ViewSettings::Popup(const std::string& name)
{
  if (ImGui::BeginPopupContextItem(name.c_str())) {
    ImGui::Checkbox("mesh", &ShowMesh);
    ImGui::Checkbox("shadow", &ShowShadow);
    ImGui::Checkbox("line", &ShowLine);
    ImGui::Checkbox("bone", &ShowCuber);
    ImGui::EndPopup();
  }
}

ScenePreview::ScenePreview(const std::shared_ptr<libvrm::RuntimeScene>& scene,
                           const std::shared_ptr<RenderingEnv>& env,
                           const std::shared_ptr<grapho::OrbitView>& view,
                           const std::shared_ptr<ViewSettings>& settings,
                           const std::shared_ptr<SceneNodeSelection>& selection,
                           bool useTPose)
  : m_runtime(scene)
  , m_env(env)
  , m_settings(settings)
  , m_selection(selection)
  , m_cuber(std::make_shared<Cuber>())
  , m_gizmo(std::make_shared<LineGizmo>())
{
  if (useTPose) {
    m_fbo = ImFbo::Create(
      view,
      std::bind(&ScenePreview::RenderStatic, this, std::placeholders::_1));
  } else {
    m_fbo = ImFbo::Create(
      view,
      std::bind(&ScenePreview::RenderRuntime, this, std::placeholders::_1));
  }
}

void
ScenePreview::RenderStatic(const grapho::OrbitView& view)
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  auto drawables = m_runtime->m_table->Drawables();
  Render(drawables);

  // manipulator
  if (auto node = m_selection->selected.lock()) {
    // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldInitialMatrix());
    ImGuizmo::GetContext().mAllowActiveHoverItem = true;
    ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
    if (auto humanoid = node->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        operation = operation | ImGuizmo::TRANSLATE;
      }
    } else {
      operation = operation | ImGuizmo::TRANSLATE;
    }
    if (ImGuizmo::Manipulate(&m_env->ViewMatrix._11,
                             &m_env->ProjectionMatrix._11,
                             operation,
                             ImGuizmo::LOCAL,
                             (float*)&m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
      // decompose feedback
      node->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
      node->CalcWorldInitialMatrix(true);
    }
    ImGuizmo::GetContext().mAllowActiveHoverItem = false;
  }
}

void
ScenePreview::RenderRuntime(const grapho::OrbitView& view)
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  m_runtime->NextSpringDelta = m_settings->NextSpringDelta;
  m_settings->NextSpringDelta = {};

  auto drawables = m_runtime->m_table->Drawables();
  if (drawables.size()) {
    m_runtime->UpdateDrawables(drawables);
  }
  Render(drawables);

  // manipulator
  if (auto init = m_selection->selected.lock()) {
    // TODO: conflict mouse event(left) with ImageButton
    auto node = m_runtime->GetRuntimeNode(init);
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
    ImGuizmo::GetContext().mAllowActiveHoverItem = true;
    ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
    if (auto humanoid = init->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        operation = operation | ImGuizmo::TRANSLATE;
      }
    } else {
      operation = operation | ImGuizmo::TRANSLATE;
    }
    if (ImGuizmo::Manipulate(&m_env->ViewMatrix._11,
                             &m_env->ProjectionMatrix._11,
                             operation,
                             ImGuizmo::LOCAL,
                             (float*)&m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
      // decompose feedback
      node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
      node->CalcWorldMatrix(true);
    }
    ImGuizmo::GetContext().mAllowActiveHoverItem = false;
  }
}

void
ScenePreview::Render(std::span<libvrm::DrawItem> drawables)
{
  if (drawables.size()) {
    glDisable(GL_DEPTH_TEST);
    if (m_settings->Skybox) {
      glr::RenderSkybox(m_env->ProjectionMatrix, m_env->ViewMatrix);
    }

    m_renderpass.clear();
    glEnable(GL_DEPTH_TEST);
    if (m_settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Opaque);
    }
    if (m_settings->ShowShadow) {
      m_renderpass.push_back(RenderPass::ShadowMatrix);
    }
    if (m_settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Transparent);
    }
    glr::RenderPasses(m_renderpass,
                      *m_env,
                      *m_runtime->m_table->m_gltf,
                      m_runtime->m_table->m_bin,
                      drawables);
  }

  if (m_settings->ShowLine) {
    m_runtime->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_env, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : m_runtime->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_env);
  }
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

} // namespace
