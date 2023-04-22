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

namespace glr {
ScenePreview::ScenePreview(
  const std::shared_ptr<libvrm::gltf::Scene>& scene,
  const std::shared_ptr<grapho::OrbitView>& view,
  const std::shared_ptr<libvrm::gltf::SceneContext>& context)
  : m_rt(std::make_shared<RenderTarget>(view))
  , m_cuber(std::make_shared<Cuber>())
{
  m_rt->color[0] = 0.2f;
  m_rt->color[1] = 0.2f;
  m_rt->color[2] = 0.2f;
  m_rt->color[3] = 1.0f;

  scene->m_sceneUpdated.push_back([this](const auto& s) {
    m_cuber->Instances.clear();
    if (s.m_roots.size()) {
      s.m_roots[0]->UpdateShapeInstanceRecursive(DirectX::XMMatrixIdentity(),
                                                 m_cuber->Instances);
    }
  });

  // void OnSceneUpdated(const libvrm::gltf::Scene& scene)
  // {
  // }

  m_rt->render =
    [scene, selection = context, gizmo = std::make_shared<LineGizmo>()](
      const RenderingEnv& env) {
      glr::ClearRendertarget(env);

      auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

      libvrm::gltf::RenderFunc render =
        [liner, &env](const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
                      const libvrm::gltf::MeshInstance& instance,
                      const float m[16]) {
          glr::Render(RenderPass::Color, env, mesh, instance, m);
          glr::Render(RenderPass::ShadowMatrix, env, mesh, instance, m);
        };

      scene->Render(render, gizmo.get());
      liner->Render(env.ProjectionMatrix, env.ViewMatrix, gizmo->m_lines);
      gizmo->Clear();

      // gizmo
      if (auto node = selection->selected.lock()) {
        // TODO: conflict mouse event(left) with ImageButton
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
        ImGuizmo::GetContext().mAllowActiveHoverItem = true;
        if (ImGuizmo::Manipulate(env.ViewMatrix,
                                 env.ProjectionMatrix,
                                 ImGuizmo::TRANSLATE | ImGuizmo::ROTATE,
                                 ImGuizmo::LOCAL,
                                 (float*)&m,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr)) {
          // decompose feedback
          node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
        }
        ImGuizmo::GetContext().mAllowActiveHoverItem = false;
      }
    };
}

void
ScenePreview::ShowScreenRect(const char* title,
                             float x,
                             float y,
                             float w,
                             float h)
{
  auto sc = ImGui::GetCursorScreenPos();
  m_rt->show_fbo(x, y, w, h);
  // top, right pivot
  Overlay({ sc.x + w - 10, sc.y + 10 }, title);
}

void
ScenePreview::ShowFullWindow(const char* title)
{
  auto pos = ImGui::GetWindowPos();
  pos.y += ImGui::GetFrameHeight();
  auto size = ImGui::GetContentRegionAvail();
  ShowScreenRect(title, pos.x, pos.y, size.x, size.y);
}

}
