#pragma once
#include "glr/scene_preview.h"
#include "gui.h"
#include <imgui.h>

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::gltf::Scene>& scene,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<libvrm::gltf::SceneContext>& context)
  {

    auto settings = std::make_shared<glr::ViewSettings>();
    settings->ShowCuber = false;
    auto preview =
      std::make_shared<glr::ScenePreview>(scene, context, view, settings);

    addDock(Dock(title, [preview, scene, settings](const char* title, bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin(title,
                       p_open,
                       ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse)) {
        preview->ShowFullWindow(scene->m_title.c_str(), settings->Color);
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));
  }
};
