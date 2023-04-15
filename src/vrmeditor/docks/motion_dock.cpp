#include <GL/glew.h>

#include "cuber.h"
#include "gl3renderer.h"
#include "motion_dock.h"
#include "rendertarget.h"
#include <imgui.h>

void
MotionDock::Create(const AddDockFunc& addDock,
                   std::string_view title,
                   const std::shared_ptr<Cuber>& cuber,
                   const std::shared_ptr<TreeContext>& context,
                   const std::function<void()>& startUdp,
                   const std::function<void()>& stopUdp)
{
  auto rt =
    std::make_shared<RenderTarget>(std::make_shared<grapho::OrbitView>());
  rt->color[0] = 0.4f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  rt->render = [cuber, selection = context](const ViewProjection& camera) {
    Gl3Renderer::ClearRendertarget(camera);

    cuber->Render(camera);

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

  auto gl3r = std::make_shared<Gl3Renderer>();

  addDock(Dock(title, [rt](const char* title, bool* p_open) {
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

  addDock(Dock("udp-recv", [startUdp, stopUdp, enable = false]() mutable {
    if (enable) {
      if (ImGui::Button("stop")) {
        stopUdp();
        enable = false;
      }
    } else {
      if (ImGui::Button("start:54345")) {
        startUdp();
        enable = true;
      }
    }
  }));
}
