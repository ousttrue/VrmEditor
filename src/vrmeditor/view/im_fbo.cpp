#include "im_fbo.h"
#include <ImGuizmo.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>

std::shared_ptr<ImFbo>
ImFbo::Create(const std::shared_ptr<grapho::camera::Camera>& camera,
              const glr::RenderFunc& callback)
{
  auto ptr = std::make_shared<ImFbo>();
  ptr->m_rt = std::make_shared<glr::RenderTarget>(camera);
  ptr->m_rt->render = callback;
  return ptr;
}

void
ImFbo::ShowFbo(float x, float y, float w, float h, const float color[4])
{
  if (w <= 0 || h <= 0) {
    return;
  }
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(x, y, w, h);

  if (auto texture = m_rt->Begin((int)w, (int)h, color)) {
    auto [isActive, isHovered] =
      grapho::imgui::DraggableImage((ImTextureID)(uint64_t)texture, { w, h });

    auto& io = ImGui::GetIO();
    m_rt->End(isActive,
              isHovered,
              io.MouseDown[ImGuiMouseButton_Right],
              io.MouseDown[ImGuiMouseButton_Middle],
              (int)io.MouseDelta.x,
              (int)io.MouseDelta.y,
              (int)io.MouseWheel);
  }
}
