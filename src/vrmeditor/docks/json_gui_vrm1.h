#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
JsonGuiVrm1SpringJoints(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                        std::string_view jsonpath)
{
  auto& items = scene->m_gltf.Json.at(nlohmann::json::json_pointer(
    std::string{ jsonpath.begin(), jsonpath.end() }));
  return [items]() {
    ImGui::Text("joints[%zu]", items.size());
    std::array<const char*, 5> cols = {
      "index", "node", "dragForce", "hitRadius", "stiffness",
    };
    if (JsonGuiTable("##spring.joints", cols)) {
      for (int i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", (int)item.at("node"));

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%f", item.value("dragForce", 0.0f));

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%f", item.value("hitRadius", 0.0f));

        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%f", item.value("stiffness", 0.0f));
      }
      ImGui::EndTable();
    }
  };
}
