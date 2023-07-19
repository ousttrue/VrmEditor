#include <GL/glew.h>

#include "im_fbo.h"
#include <grapho/camera/camera.h>
#include <grapho/gl3/fbo.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>

std::shared_ptr<ImFbo>
ImFbo::Create()
{
  auto ptr = std::make_shared<ImFbo>();
  ptr->m_rt = std::make_shared<grapho::gl3::RenderTarget>();
  return ptr;
}

void
ImFbo::ShowFbo(const grapho::camera::Viewport& viewport,
               const float color[4],
               const RenderFunc& render)
{
  if (viewport.Width <= 0 || viewport.Height <= 0) {
    return;
  }

  if (auto texture = m_rt->Begin(viewport.Width, viewport.Height, color)) {
    auto [isActive, isHovered] = grapho::imgui::DraggableImage(
      (ImTextureID)(uint64_t)texture, { viewport.Width, viewport.Height });

    auto& io = ImGui::GetIO();

    grapho::camera::MouseState mouse{
      .X = io.MousePos.x - viewport.Left,
      .Y = io.MousePos.y - viewport.Top,
    };
    mouse.LeftDown = io.MouseDown[ImGuiMouseButton_Left];
    mouse.RightDown = io.MouseDown[ImGuiMouseButton_Right];
    mouse.MiddleDown = io.MouseDown[ImGuiMouseButton_Middle];
    if (isActive) {
      mouse.DeltaX = io.MouseDelta.x;
      mouse.DeltaY = io.MouseDelta.y;
    }
    if (isHovered) {
      mouse.Wheel = io.MouseWheel;
    }

    render(viewport, mouse);

    m_rt->End();
  }
}
