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
    auto preview = std::make_shared<glr::ScenePreview>(scene, view, context);
    addDock(Dock(title, [preview, scene](const char* title, bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin(title,
                       p_open,
                       ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse)) {
        preview->ShowFullWindow(scene->m_title.c_str());
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));
  }
};
