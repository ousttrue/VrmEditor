#include "scene_preview.h"
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/gizmo.h>

namespace glr {
ScenePreview::ScenePreview(
  const std::shared_ptr<grapho::OrbitView>& view,
  const std::shared_ptr<libvrm::Timeline>& timeline,
  const std::shared_ptr<libvrm::gltf::Scene>& scene,
  const std::shared_ptr<libvrm::gltf::SceneContext>& context)
  : m_rt(view)
{
  cuber::PushGrid(m_gizmo.m_lines);
  m_gizmo.Fix();

  m_rt.color[0] = 0.2f;
  m_rt.color[1] = 0.2f;
  m_rt.color[2] = 0.2f;
  m_rt.color[3] = 1.0f;

  m_rt.render = [timeline, scene, selection = context, gizmo = &m_gizmo](
                  const ViewProjection& camera) {
    glr::ClearRendertarget(camera);

    auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

    libvrm::gltf::RenderFunc render =
      [liner, &camera](const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
                       const libvrm::gltf::MeshInstance& instance,
                       const float m[16]) {
        glr::Render(camera, mesh, instance, m);
      };
    scene->Render(timeline->CurrentTime, render, gizmo);
    liner->Render(camera.projection, camera.view, gizmo->m_lines);
    gizmo->Clear();

    // gizmo
    if (auto node = selection->selected.lock()) {
      // TODO: conflict mouse event(left) with ImageButton
      DirectX::XMFLOAT4X4 m;
      DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
      ImGuizmo::GetContext().mAllowActiveHoverItem = true;
      if (ImGuizmo::Manipulate(camera.view,
                               camera.projection,
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
ScenePreview::Show()
{
  auto pos = ImGui::GetWindowPos();
  pos.y += ImGui::GetFrameHeight();
  auto size = ImGui::GetContentRegionAvail();
  Show(pos.x, pos.y, size.x, size.y);
}

}