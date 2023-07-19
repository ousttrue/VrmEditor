#include <GL/glew.h>

#include "im_fbo.h"
#include <grapho/camera/camera.h>
#include <grapho/gl3/fbo.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>

std::shared_ptr<ImFbo>
ImFbo::Create(const RenderFunc& callback)
{
  auto ptr = std::make_shared<ImFbo>();
  ptr->m_rt = std::make_shared<grapho::gl3::RenderTarget>();
  ptr->m_render = callback;
  return ptr;
}

void
ImFbo::ShowFbo(const grapho::camera::Viewport& viewport, const float color[4])
{
  if (viewport.Width <= 0 || viewport.Height <= 0) {
    return;
  }

  if (auto texture =
        m_rt->Begin((int)viewport.Width, (int)viewport.Height, color)) {
    auto [isActive, isHovered] = grapho::imgui::DraggableImage(
      (ImTextureID)(uint64_t)texture, { viewport.Width, viewport.Height });

    auto& io = ImGui::GetIO();

    grapho::camera::MouseState mouse;
    if (isActive) {
      mouse.DeltaX = io.MouseDelta.x;
      mouse.DeltaY = io.MouseDelta.y;
      mouse.RightDown = io.MouseDown[ImGuiMouseButton_Right];
      mouse.MiddleDown = io.MouseDown[ImGuiMouseButton_Middle];
    }
    if (isHovered) {
      mouse.Wheel = io.MouseWheel;
    }

    m_render(viewport, mouse);

    m_rt->End();
  }
}
