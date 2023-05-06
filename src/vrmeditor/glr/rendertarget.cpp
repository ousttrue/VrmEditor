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
  , Fbo(new grapho::gl3::Fbo)
{
}

uint32_t
RenderTarget::Clear(int width, int height, const float color[4])
{
  if (width == 0 || height == 0) {
    return 0;
  }

  if (FboTexture) {
    if (FboTexture->width_ != width || FboTexture->height_ != height) {
      FboTexture = nullptr;
    }
  }
  if (!FboTexture) {
    FboTexture = grapho::gl3::Texture::Create({
      width,
      height,
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::Linear,
    });
    Fbo->AttachTexture2D(FboTexture->texture_);
    Fbo->AttachDepth(width, height);
  }

  Fbo->Bind();
  grapho::gl3::ClearViewport({
    .Width = width,
    .Height = height,
    .Color = { color[0], color[1], color[2], color[3] },
    .Depth = 1.0f,
  });

  return FboTexture->texture_;
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
