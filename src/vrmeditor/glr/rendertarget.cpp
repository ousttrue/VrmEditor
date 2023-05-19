#include <GL/glew.h>

#include "rendering_env.h"
#include "rendertarget.h"
#include <functional>
#include <grapho/gl3/fbo.h>
#include <grapho/imgui/widgets.h>
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
    if (FboTexture->Width() != width || FboTexture->Height() != height) {
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
    Fbo->AttachTexture2D(FboTexture->Handle());
    Fbo->AttachDepth(width, height);
  }

  Fbo->Bind();
  grapho::gl3::ClearViewport({
    .Width = width,
    .Height = height,
    .Color = { color[0], color[1], color[2], color[3] },
    .Depth = 1.0f,
  });

  return FboTexture->Handle();
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

    auto [isActive, isHovered] =
      grapho::imgui::DraggableImage((ImTextureID)(uint64_t)texture, { w, h });

    // update camera
    auto& io = ImGui::GetIO();
    View->SetSize((int)w, (int)h);
    if (isActive) {
      if (io.MouseDown[ImGuiMouseButton_Right]) {
        View->YawPitch((int)io.MouseDelta.x, (int)io.MouseDelta.y);
      }
      if (io.MouseDown[ImGuiMouseButton_Middle]) {
        View->Shift((int)io.MouseDelta.x, (int)io.MouseDelta.y);
      }
    }
    if (isHovered) {
      View->Dolly((int)io.MouseWheel);
    }
    if (render) {
      render(*View);
    }
  }
  Fbo->Unbind();
}
}
