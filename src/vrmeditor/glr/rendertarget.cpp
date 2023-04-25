#include <GL/glew.h>

#include "rendering_env.h"
#include "rendertarget.h"
#include <functional>
#include <grapho/gl3/fbo.h>
#include <grapho/orbitview.h>
#include <imgui.h>
#include <memory>

#include <ImGuizmo.h>

namespace glr {
RenderTarget::RenderTarget(const std::shared_ptr<grapho::OrbitView>& view)
  : View(view)
{
}

uint32_t
RenderTarget::Clear(int width, int height, const float color[4])
{
  if (width == 0 || height == 0) {
    return 0;
  }

  if (Fbo) {
    if (Fbo->texture->width_ != width || Fbo->texture->height_ != height) {
      Fbo = nullptr;
    }
  }
  if (!Fbo) {
    Fbo = grapho::gl3::Fbo::Create(width, height);
  }

  Fbo->Clear(color, 1.0f);

  return Fbo->texture->texture_;
}

void
RenderTarget::ShowFbo(float x, float y, float w, float h, const float color[4])
{
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect(x, y, w, h);

  assert(w);
  assert(h);
  auto texture = Clear(int(w), int(h), color);
  if (texture) {
    // image button. capture mouse event
    ImGui::ImageButton((ImTextureID)(intptr_t)texture,
                       { w, h },
                       { 0, 1 },
                       { 1, 0 },
                       0,
                       { 1, 1, 1, 1 },
                       { 1, 1, 1, 1 });
    ImGui::ButtonBehavior(ImGui::GetCurrentContext()->LastItemData.Rect,
                          ImGui::GetCurrentContext()->LastItemData.ID,
                          nullptr,
                          nullptr,
                          ImGuiButtonFlags_MouseButtonMiddle |
                            ImGuiButtonFlags_MouseButtonRight);

    // update camera
    auto& io = ImGui::GetIO();
    View->SetSize((int)w, (int)h);
    if (ImGui::IsItemActive()) {
      if (io.MouseDown[ImGuiMouseButton_Right]) {
        View->YawPitch((int)io.MouseDelta.x, (int)io.MouseDelta.y);
      }
      if (io.MouseDown[ImGuiMouseButton_Middle]) {
        View->Shift((int)io.MouseDelta.x, (int)io.MouseDelta.y);
      }
    }
    if (ImGui::IsItemHovered()) {
      View->Dolly((int)io.MouseWheel);
    }
    if (render) {
      render(*View);
    }
  }
  Fbo->Unbind();
}
}


