#include "vrm_dock.h"
#include <imgui.h>
#include <vrm/vrm0.h>
#include <vrm/vrm1.h>

void
VrmDock::Create(const AddDockFunc& addDock,
                std::string_view title,
                const std::shared_ptr<gltf::Scene>& scene)
{
  addDock(Dock(title, [scene]() {
    if (auto vrm = scene->m_vrm0) {
      ImGui::Text("%s", "vrm-0.x");
      for (auto expression : vrm->m_expressions) {
        ImGui::SliderFloat(
          expression->label.c_str(), &expression->weight, 0, 1);
      }
    }
    if (auto vrm = scene->m_vrm1) {
      ImGui::Text("%s", "vrm-1.0");
      // for (auto expression : vrm->m_expressions) {
      //   ImGui::SliderFloat(
      //     expression->label.c_str(), &expression->weight, 0, 1);
      // }
    }
  }));
}
