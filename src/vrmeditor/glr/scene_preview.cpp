#include "scene_preview.h"
#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "overlay.h"
#include "rendertarget.h"
#include <DirectXMath.h>
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <runtimescene/scene_updater.h>
#include <vrm/gizmo.h>
#include <vrm/humanbones.h>

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
  const std::shared_ptr<libvrm::gltf::Scene>& scene,
  const std::shared_ptr<libvrm::gltf::SceneContext>& selection,
  const std::shared_ptr<RenderingEnv>& env,
  const std::shared_ptr<grapho::OrbitView>& view,
  const std::shared_ptr<ViewSettings>& settings)
  : m_rt(std::make_shared<RenderTarget>(view))
  , m_cuber(std::make_shared<Cuber>())
{
  DirectX::XMFLOAT4 plane = { 0, 1, 0, 0 };
  DirectX::XMStoreFloat4x4(
    &env->ShadowMatrix,
    DirectX::XMMatrixShadow(DirectX::XMLoadFloat4(&plane),
                            DirectX::XMLoadFloat4(&env->LightPosition)));

  m_popup = std::bind(&ViewSettings::Popup, settings.get(), m_popupName);

  auto runtime = std::make_shared<runtimescene::SceneUpdater>();

  m_rt->render =
    [scene,
     selection,
     settings,
     env,
     runtime,
     cuber = m_cuber,
     gizmo = std::make_shared<LineGizmo>()](const grapho::OrbitView& view) {
      view.Update(env->ProjectionMatrix, env->ViewMatrix);
      env->Resize(view.Width, view.Height);

      glr::ClearRendertarget(*env);

      runtimescene::RenderFunc render =
        [env, settings](const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
                        const libvrm::gltf::MeshInstance& meshInstance,
                        const float m[16]) {
          if (settings->ShowMesh) {
            glr::Render(RenderPass::Color, *env, mesh, meshInstance, m);
          }
          if (settings->ShowShadow) {
            glr::Render(RenderPass::ShadowMatrix, *env, mesh, meshInstance, m);
          }
        };

      runtime->NextSpringDelta = settings->NextSpringDelta;
      settings->NextSpringDelta = {};
      runtime->Render(scene, render, gizmo.get());
      if (settings->ShowLine) {
        glr::RenderLine(*env, gizmo->m_lines);
      }
      gizmo->Clear();

      if (settings->ShowCuber) {
        cuber->Instances.clear();
        // static_assert(sizeof(cuber::Instance) ==
        // sizeof(libvrm::gltf::Instance),
        //               "Instance size");
        for (auto& root : scene->m_roots) {
          root->UpdateShapeInstanceRecursive(
            DirectX::XMMatrixIdentity(),
            [cuber](const libvrm::gltf::Instance& instance) {
              // cuber->Instances.push_back(*((const
              // cuber::Instance*)&instance));
              cuber->Instances.push_back({
                .Matrix = instance.Matrix,
                .PositiveFaceFlag = { 0, 1, 2, 0 },
                .NegativeFaceFlag = { 3, 4, 5, 0 },
              });
            });
        }
        cuber->Render(*env);
      }

      // manipulator
      if (auto node = selection->selected.lock()) {
        // TODO: conflict mouse event(left) with ImageButton
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
        ImGuizmo::GetContext().mAllowActiveHoverItem = true;
        ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
        if (auto humanoid = node->Humanoid) {
          if (humanoid->HumanBone == libvrm::vrm::HumanBones::hips) {
            operation = operation | ImGuizmo::TRANSLATE;
          }
        } else {
          operation = operation | ImGuizmo::TRANSLATE;
        }
        if (ImGuizmo::Manipulate(env->ViewMatrix,
                                 env->ProjectionMatrix,
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
    };
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
  Overlay({ sc.x + w - 10, sc.y + 10 }, title, m_popupName.c_str(), m_popup);
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
