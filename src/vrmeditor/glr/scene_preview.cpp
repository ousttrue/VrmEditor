#include "scene_preview.h"
#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "overlay.h"
#include "rendertarget.h"
#include <DirectXMath.h>
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/gizmo.h>
#include <vrm/humanbones.h>
#include <vrm/runtimescene/node.h>
#include <vrm/runtimescene/scene.h>

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

ScenePreview::ScenePreview(
  const std::shared_ptr<runtimescene::RuntimeScene>& scene,
  const std::shared_ptr<RenderingEnv>& env,
  const std::shared_ptr<grapho::OrbitView>& view,
  const std::shared_ptr<ViewSettings>& settings,
  const std::shared_ptr<SceneNodeSelection>& selection,
  bool useTPose)
  : m_scene(scene)
  , m_env(env)
  , m_settings(settings)
  , m_selection(selection)
  //
  , m_rt(std::make_shared<RenderTarget>(view))
  , m_cuber(std::make_shared<Cuber>())
  , m_gizmo(std::make_shared<LineGizmo>())
{
  if (useTPose) {
    m_rt->render =
      std::bind(&ScenePreview::RenderTPose, this, std::placeholders::_1);
  } else {
    m_rt->render =
      std::bind(&ScenePreview::RenderAnimation, this, std::placeholders::_1);
  }
}

void
ScenePreview::RenderTPose(const grapho::OrbitView& view)
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  for (auto [mesh, m] : m_scene->m_table->Drawables()) {
    auto meshInstance = m_scene->GetRuntimeMesh(mesh);
    if (m_settings->ShowMesh) {
      glr::Render(RenderPass::Color,
                  *m_env,
                  m_scene->m_table->m_gltf,
                  mesh,
                  *meshInstance,
                  m);
    }
  }
  if (m_settings->Skybox) {
    m_env->RenderSkybox();
  }
  for (auto [mesh, m] : m_scene->m_table->Drawables()) {
    auto meshInstance = m_scene->GetRuntimeMesh(mesh);
    if (m_settings->ShowShadow) {
      glr::Render(RenderPass::ShadowMatrix,
                  *m_env,
                  m_scene->m_table->m_gltf,
                  mesh,
                  *meshInstance,
                  m);
    }
  }

  if (m_settings->ShowLine) {
    m_scene->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_env, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : m_scene->m_table->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_env);
  }

  // manipulator
  if (auto node = m_selection->selected.lock()) {
    // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldInitialMatrix());
    ImGuizmo::GetContext().mAllowActiveHoverItem = true;
    ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
    if (auto humanoid = node->Humanoid) {
      if (*humanoid == libvrm::vrm::HumanBones::hips) {
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
ScenePreview::RenderAnimation(const grapho::OrbitView& view)
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  m_scene->NextSpringDelta = m_settings->NextSpringDelta;
  m_settings->NextSpringDelta = {};

  for (auto [mesh, m] : m_scene->Drawables()) {
    auto meshInstance = m_scene->GetRuntimeMesh(mesh);
    if (m_settings->ShowMesh) {
      glr::Render(RenderPass::Color,
                  *m_env,
                  m_scene->m_table->m_gltf,
                  mesh,
                  *meshInstance,
                  m);
    }
  }
  if (m_settings->Skybox) {
    m_env->RenderSkybox();
  }

  for (auto [mesh, m] : m_scene->Drawables()) {
    auto meshInstance = m_scene->GetRuntimeMesh(mesh);
    if (m_settings->ShowShadow) {
      glr::Render(RenderPass::ShadowMatrix,
                  *m_env,
                  m_scene->m_table->m_gltf,
                  mesh,
                  *meshInstance,
                  m);
    }
  }

  if (m_settings->ShowLine) {
    m_scene->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_env, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : m_scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_env);
  }

  // manipulator
  if (auto init = m_selection->selected.lock()) {
    // TODO: conflict mouse event(left) with ImageButton
    auto node = m_scene->GetRuntimeNode(init);
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
    ImGuizmo::GetContext().mAllowActiveHoverItem = true;
    ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
    if (auto humanoid = init->Humanoid) {
      if (*humanoid == libvrm::vrm::HumanBones::hips) {
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
ScenePreview::ShowScreenRect(const char* title,
                             const float color[4],
                             float x,
                             float y,
                             float w,
                             float h)
{
  auto sc = ImGui::GetCursorScreenPos();
  m_rt->ShowFbo(x, y, w, h, color);
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
}
