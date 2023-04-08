#pragma once
#include "gui.h"
#include "imhumanoid.h"

class HumanoidDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<gltf::Scene>& scene)
  {
    auto imHumanoid = std::make_shared<ImHumanoid>();

    addDock(Dock("human-body", [scene, imHumanoid](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("human-body", p_open)) {
        imHumanoid->ShowBody(*scene);
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));
    addDock(Dock("human-fingers", [scene, imHumanoid](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("human-fingers", p_open)) {
        imHumanoid->ShowFingers(*scene);
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));

  }
};
