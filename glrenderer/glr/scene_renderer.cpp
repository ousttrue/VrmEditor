#include <GL/glew.h>

#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include "scene_renderer.h"
#include <vrm/gltfroot.h>
#include <vrm/runtime_scene.h>

namespace glr {

SceneRenderer::SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                             const std::shared_ptr<ViewSettings>& settings)
  : m_env(env ? env : std::make_shared<RenderingEnv>())
  , m_settings(settings ? settings : std::make_shared<ViewSettings>())
  , m_cuber(new Cuber)
  , m_gizmo(new LineGizmo)
{
}

static void
RenderScene(const RenderingEnv& env,
            const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            std::span<libvrm::DrawItem> drawables,
            const std::shared_ptr<ViewSettings>& settings)
{
  if (drawables.size()) {
    glDisable(GL_DEPTH_TEST);
    if (settings->Skybox) {
      glr::RenderSkybox(env.ProjectionMatrix, env.ViewMatrix);
    }

    static std::vector<RenderPass> m_renderpass;
    m_renderpass.clear();
    glEnable(GL_DEPTH_TEST);
    if (settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Opaque);
    }
    if (settings->ShowShadow) {
      m_renderpass.push_back(RenderPass::ShadowMatrix);
    }
    if (settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Transparent);
    }
    glr::RenderPasses(m_renderpass, env, root, bin, drawables);
  }
}

void
SceneRenderer::RenderStatic(const std::shared_ptr<libvrm::GltfRoot>& scene,
                            const grapho::OrbitView& view) const
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  auto drawables = scene->Drawables();
  RenderScene(*m_env, *scene->m_gltf, scene->m_bin, drawables, m_settings);

  if (m_settings->ShowLine) {
    glr::RenderLine(*m_env, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_env);
  }

  // manipulator
  // if (auto node = m_selection->selected.lock()) {
  //   // TODO: conflict mouse event(left) with ImageButton
  //   DirectX::XMFLOAT4X4 m;
  //   DirectX::XMStoreFloat4x4(&m, node->WorldInitialMatrix());
  //   ImGuizmo::GetContext().mAllowActiveHoverItem = true;
  //   ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
  //   if (auto humanoid = node->Humanoid) {
  //     if (*humanoid == libvrm::HumanBones::hips) {
  //       operation = operation | ImGuizmo::TRANSLATE;
  //     }
  //   } else {
  //     operation = operation | ImGuizmo::TRANSLATE;
  //   }
  //   if (ImGuizmo::Manipulate(&m_env->ViewMatrix._11,
  //                            &m_env->ProjectionMatrix._11,
  //                            operation,
  //                            ImGuizmo::LOCAL,
  //                            (float*)&m,
  //                            nullptr,
  //                            nullptr,
  //                            nullptr,
  //                            nullptr)) {
  //     // decompose feedback
  //     node->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
  //     node->CalcWorldInitialMatrix(true);
  //   }
  //   ImGuizmo::GetContext().mAllowActiveHoverItem = false;
  // }
}

void
SceneRenderer::RenderRuntime(const std::shared_ptr<libvrm::RuntimeScene>& scene,
                             const grapho::OrbitView& view) const
{
  view.Update(&m_env->ProjectionMatrix._11, &m_env->ViewMatrix._11);
  m_env->CameraPosition = view.Position;
  m_env->Resize(view.Viewport.Width, view.Viewport.Height);
  glr::ClearRendertarget(*m_env);

  scene->NextSpringDelta = m_settings->NextSpringDelta;
  m_settings->NextSpringDelta = {};

  auto drawables = scene->m_table->Drawables();
  if (drawables.size()) {
    scene->UpdateDrawables(drawables);
  }
  RenderScene(*m_env,
              *scene->m_table->m_gltf,
              scene->m_table->m_bin,
              drawables,
              m_settings);

  if (m_settings->ShowLine) {
    scene->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_env, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_env);
  }

  // manipulator
  // if (auto init = m_selection->selected.lock()) {
  //   // TODO: conflict mouse event(left) with ImageButton
  //   auto node = scene->GetRuntimeNode(init);
  //   DirectX::XMFLOAT4X4 m;
  //   DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
  //   ImGuizmo::GetContext().mAllowActiveHoverItem = true;
  //   ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
  //   if (auto humanoid = init->Humanoid) {
  //     if (*humanoid == libvrm::HumanBones::hips) {
  //       operation = operation | ImGuizmo::TRANSLATE;
  //     }
  //   } else {
  //     operation = operation | ImGuizmo::TRANSLATE;
  //   }
  //   if (ImGuizmo::Manipulate(&m_env->ViewMatrix._11,
  //                            &m_env->ProjectionMatrix._11,
  //                            operation,
  //                            ImGuizmo::LOCAL,
  //                            (float*)&m,
  //                            nullptr,
  //                            nullptr,
  //                            nullptr,
  //                            nullptr)) {
  //     // decompose feedback
  //     node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
  //     node->CalcWorldMatrix(true);
  //   }
  //   ImGuizmo::GetContext().mAllowActiveHoverItem = false;
  // }
}

} // namespace
