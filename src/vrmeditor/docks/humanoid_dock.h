#pragma once
#include "gui.h"
#include "imhumanoid.h"

class HumanoidDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view body_title,
                     std::string_view finger_title,
                     const std::shared_ptr<libvrm::GltfRoot>& scene)
  {
    auto imHumanoid = std::make_shared<ImHumanoid>();

    addDock(grapho::imgui::Dock(
      body_title, [scene, imHumanoid](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title, p_open)) {
          imHumanoid->ShowBody(*scene);
        }
        ImGui::End();
        ImGui::PopStyleVar();
      }));
    addDock(grapho::imgui::Dock(
      finger_title, [scene, imHumanoid](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title, p_open)) {
          imHumanoid->ShowFingers(*scene);
        }
        ImGui::End();
        ImGui::PopStyleVar();
      }));
  }
};
