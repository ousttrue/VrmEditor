#pragma once
#include "gl3renderer.h"
#include "rendertarget.h"
#include "treecontext.h"
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/gizmo.h>
#include <vrm/scene.h>

struct ScenePreview
{
  RenderTarget m_rt;
  Gl3Renderer m_gl3r;
  ScenePreview(const std::shared_ptr<grapho::OrbitView>& view,
               const std::shared_ptr<libvrm::Timeline>& timeline,
               const std::shared_ptr<libvrm::gltf::Scene>& scene,
               const std::shared_ptr<TreeContext>& context)
    : m_rt(view)
  {
    m_rt.color[0] = 0.2f;
    m_rt.color[1] = 0.2f;
    m_rt.color[2] = 0.2f;
    m_rt.color[3] = 1.0f;

    m_rt.render = [timeline, scene, this, selection = context](
                    const ViewProjection& camera) {
      Gl3Renderer::ClearRendertarget(camera);

      auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

      libvrm::gltf::RenderFunc render =
        [this, liner, &camera](const libvrm::gltf::Mesh& mesh,
                               const libvrm::gltf::MeshInstance& instance,
                               const float m[16]) {
          m_gl3r.Render(camera, mesh, instance, m);
        };
      scene->Render(timeline->CurrentTime, render);
      liner->Render(camera.projection, camera.view, libvrm::gizmo::lines());

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

  void Show()
  {
    auto pos = ImGui::GetWindowPos();
    pos.y += ImGui::GetFrameHeight();
    auto size = ImGui::GetContentRegionAvail();
    Show(pos.x, pos.y, size.x, size.y);
  }

  void Show(float x, float y, float z, float w) { m_rt.show_fbo(x, y, z, w); }
};
