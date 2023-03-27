#pragma once
#include "camera.h"
#include "orbitview.h"
#include <functional>
#include <grapho/gl3/fbo.h>
#include <imgui.h>
#include <memory>

#include <ImGuizmo.h>

struct RenderTarget {
  Camera camera;
  OrbitView view;
  std::shared_ptr<grapho::gl3::Fbo> fbo;
  float color[4];
  std::function<void(const Camera &camera)> render;

  uint32_t clear(int width, int height) {
    if (width == 0 || height == 0) {
      return 0;
    }

    if (fbo) {
      if (fbo->texture->width_ != width || fbo->texture->height_ != height) {
        fbo = nullptr;
      }
    }
    if (!fbo) {
      fbo = grapho::gl3::Fbo::Create(width, height);
    }

    fbo->Clear(color, 1.0f);

    return fbo->texture->texture_;
  }

  void show_fbo(float x, float y, float w, float h) {
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, w, h);

    assert(w);
    assert(h);
    auto texture = clear(int(w), int(h));
    if (texture) {
      // image button. capture mouse event
      ImGui::ImageButton((ImTextureID)(intptr_t)texture, {w, h}, {0, 1}, {1, 0},
                         0, {1, 1, 1, 1}, {1, 1, 1, 1});
      ImGui::ButtonBehavior(ImGui::GetCurrentContext()->LastItemData.Rect,
                            ImGui::GetCurrentContext()->LastItemData.ID,
                            nullptr, nullptr,
                            ImGuiButtonFlags_MouseButtonMiddle |
                                ImGuiButtonFlags_MouseButtonRight);

      // update camera
      auto &io = ImGui::GetIO();
      camera.resize((int)w, (int)h);
      view.SetSize((int)w, (int)h);
      if (ImGui::IsItemActive()) {
        if (io.MouseDown[ImGuiMouseButton_Right]) {
          view.YawPitch((int)io.MouseDelta.x, (int)io.MouseDelta.y);
        }
        if (io.MouseDown[ImGuiMouseButton_Middle]) {
          view.Shift((int)io.MouseDelta.x, (int)io.MouseDelta.y);
        }
      }
      if (ImGui::IsItemHovered()) {
        view.Dolly((int)io.MouseWheel);
      }
      view.Update(camera.projection, camera.view);
      if (render) {
        render(camera);
      }
    }
    fbo->Unbind();
  }
};
