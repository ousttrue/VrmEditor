#pragma once
#include "glr/scene_preview.h"
#include "gui.h"

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::gltf::Scene>& scene,
                     const std::shared_ptr<libvrm::gltf::SceneContext>& context,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<libvrm::Timeline>& timeline)
  {
    auto preview =
      std::make_shared<glr::ScenePreview>(view, timeline, scene, context);
    addDock(Dock(title, [preview, scene](const char* title, bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin(title,
                       p_open,
                       ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse)) {
        preview->Show();
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));
  }
};
