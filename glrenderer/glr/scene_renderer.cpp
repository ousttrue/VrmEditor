#include <GL/glew.h>

#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include "scene_renderer.h"
#include <boneskin/skinning_manager.h>
#include <recti.h>
#include <vrm/gltfroot.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

namespace glr {

SceneRenderer::SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                             const std::shared_ptr<ViewSettings>& settings)
  : m_env(env ? env : std::make_shared<RenderingEnv>())
  , m_settings(settings ? settings : std::make_shared<ViewSettings>())
  , m_cuber(new Cuber)
  , m_gizmo(new LineGizmo)
  , m_screen(new recti::Screen)
{
}

static void
RenderScene(const grapho::camera::Camera& camera,
            const RenderingEnv& env,
            const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            std::span<const boneskin::NodeMesh> nodeMeshes,
            const std::shared_ptr<ViewSettings>& settings)
{
  if (nodeMeshes.size()) {
    glDisable(GL_DEPTH_TEST);
    if (settings->Skybox) {
      glr::RenderSkybox(camera.ProjectionMatrix, camera.ViewMatrix);
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

    glr::RenderPasses(m_renderpass, camera, env, root, bin, nodeMeshes);
  }
}

void
SceneRenderer::RenderStatic(const std::shared_ptr<libvrm::GltfRoot>& scene,
                            const grapho::camera::Camera& camera) const
{
  glr::ClearRendertarget(camera, *m_env);

  auto nodestates = scene->NodeStates();
  auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
    *scene->m_gltf, scene->m_bin, nodestates);
  RenderScene(
    camera, *m_env, *scene->m_gltf, scene->m_bin, nodeMeshes, m_settings);

  if (m_settings->ShowLine) {
    glr::RenderLine(camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(camera);
  }

  // manipulator
  if (auto node = scene->m_selected) {
    // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldInitialMatrix());

    bool enableTranslation = false;
    if (auto humanoid = node->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        enableTranslation = true;
      }
    } else {
      enableTranslation = true;
    }

    auto& vp = camera.Projection.Viewport;
    m_screen->SetRect(vp.Left, vp.Top, vp.Width, vp.Height);
    if (m_screen->Manipulate(node.get(),
                             &camera.ViewMatrix._11,
                             &camera.ProjectionMatrix._11,
                             { enableTranslation, true, false, true },
                             (float*)&m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
      // decompose feedback
      node->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
      node->CalcWorldInitialMatrix(true);

      scene->RaiseSceneUpdated();
    }
    m_screen->Render();
  }
}

void
SceneRenderer::RenderRuntime(
  const std::shared_ptr<libvrm::RuntimeScene>& runtime,
  const grapho::camera::Camera& camera) const
{
  glr::ClearRendertarget(camera, *m_env);

  auto nodestates = runtime->m_base->NodeStates();
  if (nodestates.size()) {
    runtime->UpdateNodeStates(nodestates);
    m_settings->NextSpringDelta = {};
    auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
      *runtime->m_base->m_gltf, runtime->m_base->m_bin, nodestates);
    RenderScene(camera,
                *m_env,
                *runtime->m_base->m_gltf,
                runtime->m_base->m_bin,
                nodeMeshes,
                m_settings);
  }

  if (m_settings->ShowLine) {
    runtime->DrawGizmo(m_gizmo.get());
    glr::RenderLine(camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : runtime->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(camera);
  }

  // manipulator
  if (auto node = runtime->m_selected) {
    //   // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());

    bool enableTranslation = false;
    if (auto humanoid = node->Base->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        enableTranslation = true;
      }
    } else {
      enableTranslation = true;
    }

    auto& vp = camera.Projection.Viewport;
    m_screen->SetRect(vp.Left, vp.Top, vp.Width, vp.Height);
    if (m_screen->Manipulate(node.get(),
                             &camera.ViewMatrix._11,
                             &camera.ProjectionMatrix._11,
                             { enableTranslation, true, false, true },
                             (float*)&m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
      // decompose feedback
      node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
      node->CalcWorldMatrix(true);
    }
    m_screen->Render();
  }
}

} // namespace
