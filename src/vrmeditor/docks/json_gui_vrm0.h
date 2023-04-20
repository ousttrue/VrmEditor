#pragma once
#include "json_gui.h"
#include "json_gui_table.h"

inline ShowGui
JsonGuiVrm0SpringList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                      std::string_view jsonpath)
{
  std::string path{ jsonpath.begin(), jsonpath.end() };
  auto springs = scene->m_gltf.Json.at(nlohmann::json::json_pointer(path));
  return [springs]() {
    std::array<const char*, 9> cols = {
      "index",      "comment", "dragForce", "gravity",   "radius",
      "stiffiness", "bones",   "center",    "colliders",
    };
    std::string no_name;
    if (JsonGuiTable("##springs", cols)) {
      for (int i = 0; i < springs.size(); ++i) {
        auto& spring = springs[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", spring.value("comment", no_name).c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", spring.value("dragForce", 0.0f));
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%f", spring.value("hitRadius", 0.0f));
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%f", spring.value("stiffiness", 0.0f));
      }
      ImGui::EndTable();
    }
  };
}
