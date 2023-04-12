#pragma once
#include "gl3renderer.h"
#include "gui.h"
#include "rendertarget.h"
#include "treecontext.h"
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/scene.h>

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<gltf::Scene>& scene,
                     const std::shared_ptr<TreeContext>& context,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<Timeline>& timeline,
                     const std::shared_ptr<Gl3Renderer>& gl3r)
  {
    auto rt = std::make_shared<RenderTarget>(view);
    rt->color[0] = 0.2f;
    rt->color[1] = 0.2f;
    rt->color[2] = 0.2f;
    rt->color[3] = 1.0f;

    rt->render = [timeline, scene, gl3r, selection = context](
                   const ViewProjection& camera) {
      Gl3Renderer::ClearRendertarget(camera);

      auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

      gltf::RenderFunc render =
        [gl3r, liner, &camera](const gltf::Mesh& mesh,
                               const gltf::MeshInstance& instance,
                               const float m[16]) {
          gl3r->Render(camera, mesh, instance, m);
        };
      scene->Render(timeline->CurrentTime, render);
      liner->Render(camera.projection, camera.view, gizmo::lines());

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

    addDock(Dock(title, [rt, scene](const char* title, bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin(title,
                       p_open,
                       ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse)) {
        auto pos = ImGui::GetWindowPos();
        pos.y += ImGui::GetFrameHeight();
        auto size = ImGui::GetContentRegionAvail();
        rt->show_fbo(pos.x, pos.y, size.x, size.y);
      }

      ImGui::End();
      ImGui::PopStyleVar();
    }));
  }
};
