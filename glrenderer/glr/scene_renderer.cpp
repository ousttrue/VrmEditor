#include <GL/glew.h>

#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include "scene_renderer.h"
#include <boneskin/skinning_manager.h>
#include <recti_imgui.h>
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
  , m_camera(new grapho::camera::Camera)
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
                            const grapho::camera::Viewport& viewport,
                            const grapho::camera::MouseState& mouse,
                            const std::shared_ptr<libvrm::Node>& selected) const
{
  // update camera
  m_camera->Projection.SetViewport(viewport);
  m_camera->MouseInputTurntable(mouse);
  m_camera->Update();

  glr::ClearRendertarget(*m_camera, *m_env);

  auto nodestates = scene->NodeStates();
  auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
    *scene->m_gltf, scene->m_bin, nodestates);
  RenderScene(
    *m_camera, *m_env, *scene->m_gltf, scene->m_bin, nodeMeshes, m_settings);

  if (m_settings->ShowLine) {
    glr::RenderLine(*m_camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();

    for (auto m : scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }

    m_cuber->Render(*m_camera);
  }

  // manipulator
  if (selected) {
    // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, selected->WorldInitialMatrix());

    bool enableTranslation = false;
    if (auto humanoid = selected->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        enableTranslation = true;
      }
    } else {
      enableTranslation = true;
    }

    recti::Camera gizmo_camera{
      *((const recti::Mat4*)&m_camera->ViewMatrix),
      *((const recti::Mat4*)&m_camera->ProjectionMatrix),
      *((const recti::Vec4*)&m_camera->Projection.Viewport),
    };

    auto& io = ImGui::GetIO();
    recti::Mouse mouse{ io.MousePos, io.MouseDown[0] };

    m_screen->Begin(gizmo_camera, mouse);
    if (m_screen->Manipulate(selected.get(),
                             { enableTranslation, true, false, true },
                             (float*)&m)) {
      // decompose feedback
      selected->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
      selected->CalcWorldInitialMatrix(true);

      scene->RaiseSceneUpdated();
    }
    auto& drawlist = m_screen->End();
    recti::Render(drawlist, ImGui::GetWindowDrawList());
  }
}

void
SceneRenderer::RenderRuntime(
  const std::shared_ptr<libvrm::RuntimeScene>& runtime,
  const grapho::camera::Viewport& viewport,
  const grapho::camera::MouseState& mouse,
  const std::shared_ptr<libvrm::RuntimeNode>& selected) const
{
  // update camera
  m_camera->Projection.SetViewport(viewport);
  m_camera->MouseInputTurntable(mouse);
  m_camera->Update();

  glr::ClearRendertarget(*m_camera, *m_env);

  auto nodestates = runtime->m_base->NodeStates();
  if (nodestates.size()) {
    runtime->UpdateNodeStates(nodestates);
    m_settings->NextSpringDelta = {};
    auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
      *runtime->m_base->m_gltf, runtime->m_base->m_bin, nodestates);
    RenderScene(*m_camera,
                *m_env,
                *runtime->m_base->m_gltf,
                runtime->m_base->m_bin,
                nodeMeshes,
                m_settings);
  }

  if (m_settings->ShowLine) {
    runtime->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();
    for (auto m : runtime->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }
    m_cuber->Render(*m_camera);
  }

  // manipulator
  if (selected) {
    //   // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, selected->WorldMatrix());

    bool enableTranslation = false;
    if (auto humanoid = selected->Base->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        enableTranslation = true;
      }
    } else {
      enableTranslation = true;
    }

    recti::Camera gizmo_camera{
      *((const recti::Mat4*)&m_camera->ViewMatrix),
      *((const recti::Mat4*)&m_camera->ProjectionMatrix),
      *((const recti::Vec4*)&m_camera->Projection.Viewport),
    };

    auto& io = ImGui::GetIO();
    recti::Mouse mouse{ io.MousePos, io.MouseDown[0] };

    m_screen->Begin(gizmo_camera, mouse);
    if (m_screen->Manipulate(selected.get(),
                             { enableTranslation, true, false, true },
                             (float*)&m)) {
      // decompose feedback
      selected->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
      selected->CalcWorldMatrix(true);
    }

    // const float cubes[]{
    //   1, 0, 0, 0, //
    //   0, 1, 0, 0, //
    //   0, 0, 1, 0, //
    //   0, 0, 0, 1, //
    // };
    // auto cubes = runtime->ShapeMatrices();
    // m_screen->DrawCubes((const float*)cubes.data(), cubes.size());

    auto& drawlist = m_screen->End();
    recti::Render(drawlist, ImGui::GetWindowDrawList());
  }
}

} // namespace
